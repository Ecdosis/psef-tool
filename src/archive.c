/*
 * This file is part of mmpupload.

    mmpupload is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    mmpupload is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with mmpupload.  If not, see <http://www.gnu.org/licenses/>.*/
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <math.h>
#include "archive.h"
#include "node.h"
#include "response.h"
#include "url.h"
#include "config.h"
#include "utils.h"
#include "pathset.h"
#include "mmp.h"
#include "single_file.h"
// test memory
#ifdef MEMWATCH
#include "memwatch.h"
#endif
// test timing of read and write routines
#ifdef PROFILE
static long read_time = 0;
static long write_time = 0;
#endif

#define SOCK_TIMEOUT 0.2f
#define MAXLINE 4096
#define MVD_PATH "/import/mvd/"
#define TEXT_PATH "/import/text/"
#define XML_PATH "/import/xml/"
#define LITERAL_PATH "/upload/"
#define VERSION_KEY "VERSION_"
/*
 * Scan a website repository for uploading
 */
static char line[MAXLINE];
static int archive_scan_dir( char *docid, char *relpath, config *cf );
static int archive_upload_all( url *host, unsigned char *body, int len );

/**
 * Upload a composed mmp message to a url
 * @param my_mmp the mmyp ready to go
 * @param base_url the the url to send it to
 * @param res_path the resource path within that service
 * @return 1 if successful else 0
 */
static int upload_to_url( mmp *my_mmp, char *base_url, char *res_path )
{
    int res = 1;
    int len = 0;
    unsigned char *text = NULL;
    if ( ends_with(base_url,"/")&&starts_with(res_path,"/") )
        res_path++;
    int ulen = 1+strlen(base_url)+strlen(res_path);
    char *my_url = malloc( ulen );
    if ( my_url != NULL )
    {
        snprintf( my_url, ulen, "%s%s", base_url, res_path );
        url *u = url_create( my_url );
        if ( u != NULL )
        {
            text = mmp_get( my_mmp, &len, "POST", url_get_host(u), 
                url_get_path(u) );
            if ( text != NULL )
            {
                FILE *dst = fopen("mmp.txt","w");
                if ( dst != NULL )
                {
                    fwrite( text, 1, len, dst );
                    fclose( dst );
                }  
                res = archive_upload_all( u, text, len );
                free( text );
            }
            url_dispose( u );
        }
        free( my_url );
    }
    return res;
}
/**
 * Upload an individal file in a mime multipart
 * @param sf the single file object
 * @param cf the config file applying to this upload
 * @return 1 if it worked else 0
 */
static int upload_single_file( single_file *sf, config *cf )
{
    int res = 0;
    mmp *my_mmp = mmp_create();
    if ( my_mmp != NULL )
    {
        char *path = single_file_path(sf);
        res = mmp_add_file( my_mmp, file_name(path), path );
        if ( res )
        {
            char *new_docid = allocate_docid( single_file_docid(sf), 
                single_file_fname(sf) );
            if ( new_docid != NULL )
            {
                res = mmp_add_field( my_mmp, "DOC_ID", new_docid );
                if ( res )
                {
                    char *base_url = config_get( cf, "base_url" );
                    if ( base_url != NULL )
                    {
                        res = upload_to_url( my_mmp, base_url, 
                            LITERAL_PATH );
                    }
                    else
                        fprintf(stderr,
                            "archive: no url specified!\n");
                }
                free( new_docid );
            }
            else
            {
                fprintf(stderr,"archive: failed to allocate docid\n");
                res = 0;
            }
        }
        mmp_dispose( my_mmp );
    }
    else
        res = 0;
    return res;
}
/**
 * Scan a folder for FILES only
 * @param docid the docid accumulated so far
 * @param relpath the relative path to the parent directory
 * @param cf VAR param: a config object or to NULL
 * @return 1 if it worked OK, else 0
 */
int archive_scan_for_files( char *docid, char *relpath, config **cf )
{
    int res = 1;
    config *old = *cf;
    config *initial = *cf;
    single_file *sf = NULL;
    DIR *dirp = opendir(relpath);
    if ( dirp != NULL )
    {
        struct dirent *dp = readdir(dirp);
        while ( dp != NULL )  
        {
            if ( !is_directory(relpath,dp->d_name) )
            {
                char *path = allocate_path( relpath, dp->d_name );
                if ( path != NULL )
                {
                    if ( ends_with(dp->d_name,".conf") )
                    {
                        *cf = config_update( path, *cf );
                        if ( old != NULL && old != initial )
                            config_dispose( old );
                        old = *cf;
                    }
                    else if ( docid != NULL && is_uploadable(dp->d_name) )
                    {
                        single_file *temp = single_file_create(path, docid, 
                            dp->d_name);
                        sf = single_file_append( sf, temp );
                    }
                    free( path );
                }
                else
                {
                    fprintf(stderr,"archive: allocate_path failed\n");
                    res = 0;
                }
            }
            // else ignore all directories
            if ( !res )
                break;
            dp = readdir(dirp);
        } 
        closedir( dirp );
        // upload the saved single files
        single_file *temp = sf;
        while ( temp != NULL && res )
        {
            res = upload_single_file( temp, *cf );
            temp = single_file_next( temp );
        }
    }
    else
    {
        res = 0;
        fprintf(stderr,"archive: path %s was not a directory\n",relpath);
    }
    return res;
}
/**
 * Scan the folder, building a list of paths
 * @param docid the docid pointing to the parent directory
 * @param path the relative path from WD to the raw directory
 * @param cf the current active config
 * @return 1 if it worked, else 0
 */
static int import_dir( char *docid, char *path, config *cf )
{
    config *old_cf = cf;
    int res = archive_scan_for_files( docid, path, &cf );
    pathset *ps = pathset_create( path, docid, mvd_name(file_name(path)), 
        PATHSET_UNSET );
    if ( ps != NULL && res )
    {
        mmp *my_mmp = mmp_create();
        if ( my_mmp != NULL )
        {
            int i;
            for ( i=0;i<pathset_size(ps);i++ )
            {
                char *sub_path = pathset_get_path( ps, i );
                res = mmp_add_file( my_mmp, file_name(sub_path), sub_path );
                if ( !res )
                    break;
            }
            // add docid
            if ( res )
                res = mmp_add_field( my_mmp, "DOC_ID", pathset_get_docid(ps) );
            // add filter name
            if ( res && config_get(cf,"filter") != NULL )
            {
                char *filter = config_get(cf,"filter");
                res = mmp_add_field( my_mmp, "FILTER", filter );
            }
            // add stripper config name
            if ( res && config_get(cf,"stripper") != NULL )
            {
                char *stripper = config_get(cf,"stripper");
                res = mmp_add_field( my_mmp, "stripper", stripper );
            }
            // add splitter config name
            if ( res && config_get(cf,"splitter") != NULL )
            {
                char *splitter = config_get(cf,"splitter");
                res = mmp_add_field( my_mmp, "splitter", splitter );
            }
            // add specified or default corform
            if ( res && config_get(cf,"corform")!= NULL )
            {
                char *corform = config_get(cf,"corform");
                res = mmp_add_field( my_mmp, "STYLE", corform );
            }
            // add versions if present
            if ( res )
            {
                int i = 0;
                char **versions = (char**)config_get(cf, "versions" );
                if ( versions != NULL && versions[i] != NULL )
                {
                    while ( versions[i] != NULL )
                    {
                        int keylen = strlen(versions[i])+1+strlen(VERSION_KEY);
                        char *key = malloc( keylen );
                        if ( key != NULL )
                        {
                            snprintf(key,keylen,"%s%s",VERSION_KEY,versions[i]);
                            mmp_add_field(my_mmp,key,versions[i+1]);
                            free( key );
                        }
                        i += 2;
                    }
                }
            }
            // determine res path
            if ( res )
            {
                char *base_url = config_get( cf, "base_url" );
                if ( base_url != NULL )
                {
                    // decide which url to send to
                    int kind = pathset_kind( ps );
                    char *res_path = NULL;
                    if ( kind == PATHSET_MVD )
                        res_path = MVD_PATH;
                    else if ( kind==PATHSET_TEXT )
                        res_path = TEXT_PATH;
                    else if ( kind==PATHSET_XML )
                        res_path = XML_PATH;
                    else
                    {
                        res = 0;
                        fprintf(stderr,"archive: unknown pathset type %d\n",
                            kind);
                    }
                    if ( res )
                    {
                        res = upload_to_url(my_mmp, base_url, res_path);
                    }
                }
            }
            mmp_dispose( my_mmp );
        }
        pathset_dispose( ps );
    }
    if ( old_cf != cf && cf != NULL )
        config_dispose( cf );
    return res;
}
/**
 * Process a directory we found during recursion and recurse some more
 * @param parent the relative path to the parent dir
 * @param docid the docid accumulated so far
 * @param name the name of the directory
 * @param cf the config file we have defined so far
 * @return 1 if it worked else 0
 */
static int archive_process_dir( char *docid, char *parent, char *name, 
    config *cf )
{
    int res = 1;
    char *path = allocate_path(parent,name);
    if ( path != NULL )
    {
        if ( is_literal_name(name) )
        {
            // assuming the docid prefix is 1 char in length...
            char *db = allocate_path("",&name[1] );
            if ( db != NULL )
            {
                if ( !archive_scan_dir(db,path,cf) )
                {
                    free( db );
                    free( path );
                    res = 0;
                }
                else
                    free( db );
            }
            else
            {
                fprintf(stderr,
                    "archive: failed to allocate db name\n");
                res = 0;
            }
        }
        else if ( is_docid_name(name) )
        {
            // assuming the docid prefix is 1 char in length...
            char *new_docid = allocate_docid("",&name[1] );
            if ( new_docid != NULL )
            {
                if ( !archive_scan_dir(new_docid,path,cf) )
                {
                    free( new_docid );
                    free( path );
                    res = 0;
                }
                else
                    free( new_docid );
            }
            else
            {
                fprintf(stderr,"archive: failed to allocate docid\n");
                res = 0;
            }
        }
        else if ( is_mvd_name(name) )
        {
            char *new_docid = allocate_docid( docid, &name[1] );
            if ( new_docid != NULL )
            {
                res = import_dir( new_docid, path, cf );
                free( new_docid );
            }
            else
            {
                fprintf(stderr,"archive: failed to allocate docid\n");
                res = 0;
            }
        }
        else // ordinary directory
        {
            if ( docid != NULL )
            {
                char *new_docid;
                if ( count_chars(docid,'/')==1 )
                    new_docid = allocate_path( docid, name );
                else
                    new_docid = allocate_docid( docid, name );
                if ( new_docid != NULL )
                {
                    res = archive_scan_dir( new_docid, path, cf );
                    free( new_docid );
                }
                else
                {
                    fprintf(stderr,
                        "archive: failed to allocate docid\n");
                    res = 0;
                }
            }
            else // just continue looking
                res = archive_scan_dir( docid, path, cf );
        }
        free( path );
    }
    else
    {
        fprintf(stderr,"archive: failed to allocate path\n");
        res = 0;
    }
    return res;
}
/**
 * Scan a folder for DIRS only
 * @param docid the docid accumulated so far
 * @param relpath the relative path to the parent directory
 * @param cf a config object or to NULL
 * @return 1 if it worked OK, else 0
 */
int archive_scan_for_dirs( char *docid, char *relpath, config *cf )
{
    // failure to find any is not an error
    int res = 1;
    DIR *dirp = opendir(relpath);
    if ( dirp != NULL )
    {
        struct dirent *dp = readdir(dirp);
        while ( dp != NULL ) 
        {
            if ( is_directory(relpath,dp->d_name) )
                res = archive_process_dir( docid, relpath, dp->d_name, cf );
            if ( !res )
                break;
            dp = readdir(dirp);
        } 
        closedir( dirp );
    }
    return res;
}
/**
 * Scan all files/folders within a particular folder
 * 1. for bare folders add their name to the path and docid if any and recurse
 * 2. for folders starting with % turn them into path sets
 * 3. for folders starting with @ upload their contents as literal files
 * 4. for additional .conf files which override or extend the archive's 
 * own conf file
 * @param docid the docid accumulated so far
 * @param relpath the current path relative to the root directory
 * @param cf the config file or NULL if not yet created
 * @return 1 if it worked, else 0
 */
static int archive_scan_dir( char *docid, char *relpath, config *cf )
{
    int res = 0;
    config *old = cf;
    if ( res=archive_scan_for_files(docid,relpath,&cf) )
        res = archive_scan_for_dirs( docid, relpath, cf );
    if ( old != cf && cf != NULL )
        config_dispose( cf );
    return res;
}
/**
 * Scan a directory for archives. The only public function.
 * @param path the directory to look for archives or the archive folder itself
 * @return 1 if it worked, else 0
 */
int archive_scan( char *path )
{
    char *name,*rel_path;
    rel_path = path;
    int res = split_path( &rel_path, &name );
    if ( res )
    {
        // check for top-level literal, mvd folders etc
        if ( is_mvd_name(name)||is_docid_name(name)||is_literal_name(name) )
            res = archive_process_dir( "", rel_path, name, NULL );
        else // just scan contents of path
            res = archive_scan_dir( NULL, path, NULL );
        free( name );
        free( rel_path );
    }
#ifdef PROFILE
    fprintf(stderr,"read_time=%ld microseconds; write_time=%ld microseconds\n",
        read_time,write_time);
#endif
    return res;
}
/**
 * Write n bytes 
 * @param fd the descriptor to write to
 * @param vptr the data to write
 * @param n its length
 * @return -1 on error, else number of bytes written
 */
static ssize_t writen( int fd, const void *vptr, size_t n )
{
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;
#ifdef PROFILE
    long start = epoch_time();
#endif
    ptr = vptr;
    nleft = n;
    while ( nleft > 0 )
    {
        //printf("about to call write with nleft=%d\n",(int)nleft);
        if ((nwritten = write(fd,ptr,nleft)) <= 0 )
        {
            //printf("nwritten=%d errno=%d\n",(int)nwritten,errno);
            if ( errno == EINTR )
                nwritten = 0;
            else
            {
                n = -1;
                break;
            }
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
#ifdef PROFILE
    long end = epoch_time();
    write_time += end-start;
#endif
    return n;
}
/**
 * Read from a socket and verify that the HTML response was OK
 * @param sock the socket to read from
 * @return 1 if it succeeds or 0 if not
 */
static int readn( int sock )
{
	int n,res=0;
#ifdef PROFILE
    long start = epoch_time();
#endif
    response *r = response_create();
	if ( r != NULL )
    {
        for ( ; ; )
        {
            n=read( sock, line, MAXLINE );
            if ( n < 0 )
            {
                if ( errno == EINTR )
                    fprintf(stderr,"archive: interrupted system call, wrote "
                        "%d bytes already\n",response_get_len(r));
                else if ( errno != EAGAIN )
                {
                    printf( "archive: failed to read. err=%s socket=%d\n",
                    strerror(errno),sock);
                    break;
                }
                // try again
            }
            else if ( n == 0 )
            {
                // just finished reading
                res = response_ok( r );
                response_dump( r );
                break;
            }
            else
            {
                response_append( r, line, n );
            }
        }
        response_dispose( r );
    }
#ifdef PROFILE
    long end = epoch_time();
    read_time += end-start;
#endif
    return res;
}
/**
 * Set the timeout on a socket
 * @param sockfd the socket file descriptor
 * @param secs the number of seconds to timeout after
 * @return 1 if it worked, else 0
 */
static int socket_set_timeout( int sockfd, float secs )
{
    int res;
    struct timeval tv;
    float scaled = 1000000.0f*secs;
    tv.tv_sec = scaled/1000000;
    tv.tv_usec = scaled-(float)(tv.tv_sec*1000000);
    res = setsockopt( sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv,sizeof(tv) );
    if ( res != -1 )
        res = setsockopt( sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv,sizeof(tv) );
    if ( res == -1 )
        fprintf(stderr,"archive: failed to set timeout on socket\n");
    return res==0;
}
/**
 * Upload a composed mmp block to the server
 * @param host the url of the upload service
 * @param body the body of the mmp
 * @param len its length
 * @return 1 if it worked, else 0
 */
static int archive_upload_all( url *host, unsigned char *body, int len )
{
    int res = 0;
    //puts(body);
    char *address = url_get_host( host );
    int port = url_get_port( host );
    int sock = socket( AF_INET, SOCK_STREAM, 0 );
    if ( sock != -1 )
    {
        struct sockaddr_in addr;
        if ( socket_set_timeout(sock, SOCK_TIMEOUT) )
        {
            memset( &addr, 0, sizeof(addr) );
            struct hostent *he = gethostbyname( address );
            if ( he != NULL )
            {
                memcpy( &addr.sin_addr, he->h_addr_list[0], he->h_length );
                addr.sin_family = AF_INET;
                addr.sin_port = htons(port);
                res = connect( sock, (const struct sockaddr *)&addr, sizeof(addr) );
                if ( res == 0 )
                {
                    //fwrite( body, 1, len, stdout );
                    ssize_t n = writen( sock, body, len );
                    printf("wrote %d, tried to write %d bytes\n",(int)n,len);
                    if ( n != len )
                    {
                        fprintf(stderr,
                            "archive: can't send to %s on port %d. error=%s\n",
                            address, port, strerror(errno) );
                        return 0;
                    }
                    else
                    {
                        return readn( sock );
                    }
                }
                else
                    fprintf(stderr,"archive: failed to connect to %s\n",address );
            }
            else
                fprintf(stderr,"archive: couldn't resolve host %s\n",address);
        }
        close( sock );
    }
    else
    {
        fprintf( stderr,
            "archive: failed to open socket. error=%s\n", strerror(errno) );
    }
    return res;
}
