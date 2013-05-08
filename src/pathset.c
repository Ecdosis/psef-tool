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
    along with mmpupload.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include "pathset.h"
#include "utils.h"
#ifdef DEBUG_PATHSET
#include "textbuf.h"
#endif
#ifdef MEMWATCH
#include "memwatch.h"
#endif

#define BLOCK_SIZE 12
#define XML_SUFFIX ".xml"
#define JPG_SUFFIX ".jpg"
#define GIF_SUFFIX ".gif"
#define PNG_SUFFIX ".png"
#define CONF_SUFFIX ".conf"
#define CRLEN strlen("\n")
#define PDEF_CORTEX "cortex"
#define PDEF_CORCODE "corcode"
#define PDEF_TEXT "TEXT"
#define PDEF_MVD "MVD"
#define PDEF_XML "XML"
#define PDEF_MIXED "MIXED"
#define PDEF_CORTEX_FILE "cortex.mvd"
/*
 * Represent a set of paths that make up a single MVD
 */
struct pathset_struct
{
    char *name;
    char *docid;
    char **paths;
    int kind;
    int allocated;
    int used;
    pathset *next;
};
/**
 * Add a new path to a pathset
 * @param ps the pathset in question
 * @param path the relative path to the new directory
 * @param dir the actual directory or file being added
 * @return 1 if it worked, else 0
 */
static int add_path( pathset *ps, char *path, char *dir )
{
    int plen = strlen(path)+strlen(dir)+2;
    char *full_path = malloc( plen );
    if ( full_path != NULL )
    {
        snprintf( full_path, plen, "%s/%s", path, dir );
        if ( ps->used == ps->allocated-1 )
        {
            int i,new_allocated = ps->allocated + BLOCK_SIZE;
            char **new_paths = calloc( new_allocated, sizeof(char*) );
            if ( new_paths != NULL )
            {
                for ( i=0;i<ps->used;i++ )
                    new_paths[i] = ps->paths[i];
                free( ps->paths );
                ps->paths = new_paths;
                ps->allocated = new_allocated;
            }
            else
            {
                fprintf(stderr,"pathset: failed to reallocate paths\n");
                return 0;
            }
        }
        ps->paths[ps->used++] = full_path;
        return 1;
    }
    else
    {
        fprintf(stderr,"pathset: failed to create full path\n");
        return 0;
    }
}
/**
 * Replace the existing docid
 * @param ps the pathset object
 * @param rep the replacement docid (maybe NULL)
 */
void pathset_replace_docid( pathset *ps, char *rep )
{
    if ( ps->docid != NULL )
        free( ps->docid );
    ps->docid = rep;
}
/**
 * Create an empty new pathset. Don't copy existing paths
 * @param old the old pathset to clone
 * @return the new pathset or NULL
 */
pathset *pathset_clone( pathset *old )
{
    pathset *ps = calloc( 1, sizeof(pathset) );
    if ( ps != NULL )
    {
        ps->paths = calloc( BLOCK_SIZE, sizeof(char*) );
        ps->allocated = BLOCK_SIZE;
        ps->used = 0;
        ps->kind = old->kind;
        ps->name = strdup(old->name);
        if ( ps->docid != NULL )
            ps->docid = strdup( old->docid );
        if ( ps->paths == NULL || ps->docid==NULL )
        {
            pathset_dispose( ps );
            ps = NULL;
        }
    }
    return ps;
}
/**
 * Add a new pathset on the end of the old
 * @param head the head of the list
 * @param ps the new pathset to append
 */
static void pathset_append( pathset *head, pathset *ps )
{
    pathset *next = head;
    while ( next->next != NULL )
        next = next->next;
    next->next = ps;
}
/**
 * Create a new pathset and add it to the old one
 * @param ps the pathset to update
 * @param kind the new kind of pathset
 * @return the new pathset or NULL if it failed
 */
static pathset *pathset_update( pathset *ps, int kind )
{
    if ( ps->used == 0 || ps->kind == kind )
        ps->kind = kind;
    else 
    {
        pathset *old = ps;
        ps = pathset_clone( ps );
        if ( ps != NULL )
            pathset_append( old, ps );
        ps->kind = kind;
    }
    return ps;
}
/**
 * Read a directory containing only MVD corcodes
 * @param ps the pathset to record in or its parent
 * @param path the path up to this directory
 * @param dir the name of this directory
 * @return 1 if it worked else 0
 */
static int pathset_mvd_corcode( pathset *ps, char *path, char *dir )
{
    int res = 1;
    DIR *dp;
    int first = 1;
    char *new_path = allocate_path( path, PDEF_CORCODE );
    if ( new_path != NULL )
    {
        dp = opendir( new_path );
        if ( dp != NULL )
        {
            struct dirent *ep = readdir(dp);
            while ( ep != NULL && res )
            {
                if ( !is_directory(new_path,ep->d_name) )
                {
                    if ( first )
                    {
                        ps = pathset_update( ps, PATHSET_CORCODE );
                        if ( ps == NULL )
                            res = 0;
                        first = 0;
                    }
                    add_path( ps, new_path, ep->d_name );
                }
            }
        }
        else
            fprintf(stderr,"pathset: failed to open %s\n",new_path);
        free( new_path );
    }
    return res;
}
/**
 * Process a directory containing only MVDs
 * @param ps the pathset to add to
 * @param path the path that led us here, plus "MVD"
 * @return 1 if it worked else 0
 */
static int pathset_mvd( pathset *ps, char *path )
{
    int res = 1;
    // augment path
    char *new_path = allocate_path( path, PDEF_MVD );
    if ( new_path != NULL )
    {
        DIR *dp;
        dp = opendir( new_path );
        if ( dp != NULL )
        {
            struct dirent *ep = readdir(dp);
            while ( ep != NULL && res )
            {
                if ( is_directory(new_path,ep->d_name) )
                {
                    if ( strcmp(ep->d_name,PDEF_CORCODE)==0 )
                        res = pathset_mvd_corcode( ps, new_path, 
                            ep->d_name );
                    else if ( strcmp(ep->d_name,"..")!=0
                        && strcmp(ep->d_name,".")!=0 )
                        fprintf(stderr,"pathset: unrecognised directory %s\n",
                            ep->d_name );
                }
                else if ( strcmp(ep->d_name,PDEF_CORTEX_FILE)==0 )
                {
                    ps = pathset_update( ps, PATHSET_CORTEX );
                    if ( ps != NULL )
                        add_path( ps, new_path, "cortex.mvd" );
                    else
                        res = 0;
                }
                else
                    fprintf(stderr,
                        "pathset: unknown file %s ignored\n",ep->d_name);
            }
        }
        else
        {
            fprintf(stderr,"pathset: couldn't open %s\n",new_path);
            res = 0;
        }
        free( new_path );
    }
    return res;
}
/**
 * Process a cortex directory within a TEXT directory
 * @param ps the pathset to add it to
 * @param path the path of the TEXT directory
 * @return 1 if it worked else 0
 */
int pathset_text_cortex( pathset *ps, char *path )
{
    int res = 1;
    char *new_path = allocate_path( path, PDEF_TEXT );
    if ( new_path != NULL )
    {
        DIR *dp;
        dp = opendir( new_path );
        if ( dp != NULL )
        {
            struct dirent *ep = readdir(dp);
            while ( ep != NULL && res )
            {
                if 
            }
        }
    }
    return res;
}
                    else if ( strcmp(ep->d_name,PDEF_CORCODE)==0 )
                        res = pathset_text_corcode(ps,new_path);
/**
 * Process the TEXT directory inside 
 */
static int pathset_text( pathset *ps, char *path )
{
    int res = 1;
    char *new_path = allocate_path( path, PDEF_TEXT );
    if ( new_path != NULL )
    {
        DIR *dp;
        dp = opendir( new_path );
        if ( dp != NULL )
        {
            struct dirent *ep = readdir(dp);
            while ( ep != NULL && res )
            {
                if ( is_directory(new_path,ep->d_name) )
                {
                    if ( strcmp(ep->d_name,PDEF_CORTEX)==0)
                        res = pathset_text_cortex(ps,new_path);
                    else if ( strcmp(ep->d_name,PDEF_CORCODE)==0 )
                        res = pathset_text_corcode(ps,new_path);
                    else
                    {
                        res = 0;
                        fprintf(stderr,
                            "pathset: ignoring unrecognised subdir %s\n",
                            ep->d_name);
                    }
                }
            }
        }
    }
    return res;
}
static int pathset_xml( pathset *ps, char *path )
{
    return 0;
}
static int pathset_mixed( pathset *ps, char *path )
{
    return 0;
}
/**
 * Build the paths from the given directory
 * @param ps the pathset in question
 * @param path the relative path from CWD to the file or dir
 * @return 1 if it worked else 0
 */
static int build_paths( pathset *ps, char *path )
{
    DIR *dp;
    int res = 1;
    int nimgs=0;
    dp = opendir( path );
    if (dp != NULL)
    {
        struct dirent *ep = readdir(dp);
        while ( ep != NULL )
        {
            if ( strcmp(ep->d_name,"..")!=0&&strcmp(ep->d_name,".")!=0 )
            {
                if ( is_directory(path,ep->d_name) )
                {
                    if ( strcmp(ep->d_name,PDEF_MVD)==0 )
                        res = pathset_mvd(ps,path);
                    else if( strcmp(ep->d_name,PDEF_TEXT)==0 )
                        res = pathset_text(ps,path);
                    else if ( strcmp(ep->d_name,PDEF_XML)==0 )
                        res = pathset_xml(ps,path);
                    else if ( strcmp(ep->d_name,PDEF_MIXED)==0 )
                        res = pathset_mixed(ps,path);
                    else
                    {
                        res = 0;
                        fprintf(stderr,
                            "pathset: ignoring unrecognised subdir %s\n",
                            ep->d_name);
                    }
                }
                else if ( !ends_with(ep->d_name,CONF_SUFFIX) )
                {
                    if ( ends_with(ep->d_name,PNG_SUFFIX)
                        ||ends_with(ep->d_name,JPG_SUFFIX)
                        ||ends_with(ep->d_name,GIF_SUFFIX))
                    {
                        nimgs++;
                        res = add_path( ps, path, ep->d_name );
                    }
                    else
                        fprintf(stderr,
                            "pathset: misplaced file %s ignored\n",
                            ep->d_name);
                }
            }
            ep = readdir( dp );
            if ( !res )
                break;
        }
        closedir (dp);
    }
    if ( ps->kind == PATHSET_UNSET && nimgs > 0 )
        ps->kind = PATHSET_IMG;
    return res;
}
/**
 * Get the kind of this set
 * @param ps the pathset
 * @return its kind as an int
 */
int pathset_kind( pathset *ps )
{
    return ps->kind;
}
/**
 * Get the next pathset to upload
 * @param ps the current pathset
 * @return the next pathset
 */
pathset *pathset_next( pathset *ps )
{
    return ps->next;
}
/**
 * Create a pathset and discover all its paths
 * @param folder the relative path to the folder
 * @param docid the docid for this pathset, excluding its name
 * @param name a suggested name or NULL
 * @param kind the kind of this pathset
 * @return a finished pathset
 */
pathset *pathset_create( char *folder, char *docid, char *name, int kind )
{
    pathset *ps = calloc( 1, sizeof(pathset) );
    if ( ps != NULL )
    {
        ps->paths = calloc( BLOCK_SIZE, sizeof(char*) );
        ps->allocated = BLOCK_SIZE;
        ps->used = 0;
        ps->kind = kind;
        ps->name = strdup(name);
        if ( docid != NULL )
            ps->docid = strdup( docid );
        if ( (docid==NULL||ps->docid!=NULL) && ps->name != NULL )
        {
            int res = build_paths( ps, folder );
            if ( !res )
            {
                pathset_dispose( ps );
                ps = NULL;
            }
        }
        else
        {
            fprintf(stderr,"pathset: failed to copy name or docid\n");
            pathset_dispose( ps );
            ps = NULL;
        }
    }
    else
        fprintf(stderr,"pathset:failed to create object\n");
    return ps;
}
/**
 * Dispose of a pathset
 * @param ps the patheset to destroy
 */
void pathset_dispose( pathset *ps )
{
    int i;
    for ( i=0;i<ps->used;i++ )
        free( ps->paths[i] );
    if ( ps->paths != NULL )
        free( ps->paths );
    if ( ps->docid != NULL )
        free( ps->docid );
    if ( ps->name != NULL )
        free( ps->name );
    pathset *child = ps->next;
    while ( child != NULL )
    {
        // child->next will become invalid after disposal
        pathset *next = child->next;
        pathset_dispose( child );
        child = next;
    }
    free( ps );
}
/**
 * Get the pathset's name
 * @param ps the pathset in question
 * @return the pathset name/kind
 */
char *pathset_name( pathset *ps )
{
    return ps->name;
}
/**
 * Get the number of paths in the pathset
 * @param ps a pathset
 * @return the number of paths
 */
int pathset_size( pathset *ps )
{
    return ps->used;
}
/**
 * Get a particular path from the set
 * @param ps the pathset
 * @param index index into the pathset
 * @return a path or NULL
 */
char *pathset_get_path( pathset *ps, int index )
{
    if ( index < ps->used )
        return ps->paths[index];
    else
        fprintf(stderr,"pathset: index too big: %d (max %d)\n",
            index,ps->used);
    return NULL;
}
/**
 * Get a docid (not the same as the path)
 * @param ps the pathset in question
 * @return the docid 
 */
char *pathset_get_docid( pathset *ps )
{
    return ps->docid;
}
/**
 * Work out the string length for pathset_tostring
 * @param ps a pathset
 * @return the number of bytes required by a null-termianted string for ps
 */
static int pathset_measure( pathset *ps )
{
    int i,size;
    if ( ps->docid != NULL )
        // CR, NULL, ": "
        size = strlen(ps->name) + 3 + CRLEN + strlen(ps->docid);
    else
        // NULL
        size = 1;
    for ( i=0;i<ps->used;i++ )
        size += CRLEN + strlen(ps->paths[i]);
    return size;
}
/**
 * Debug: convert a pathset to a string
 * @param ps the pathset to convert
 * @return an allocated string with the docid and all the paths
 */
static char *pathset_tostring( pathset *ps )
{
    int i,plen = pathset_measure( ps );
    int slen = 0;
    char *str = calloc( 1, plen );
    if ( str != NULL )
    {
        if ( ps->docid != NULL )
        {
            snprintf( str, plen, "%s: %s\n",ps->name,ps->docid );
            slen = strlen(ps->name)+strlen(ps->docid)+3+CRLEN;
        }
        for ( i=0;i<ps->used;i++ )
        {
            strncat( str, ps->paths[i], plen-slen );
            slen += strlen( ps->paths[i] );
            strncat( str, "\n", plen-slen );
            slen += CRLEN;
        }
    }
    return str;
}
#ifdef DEBUG_PATHSET
/**
 * Search in all sub-directories for dirs not starting with "%2F" and 
 * make pathsets out of them. Then convert the pathsets into one long string.
 * @param path the path to the root dir to search from
 * @param tb the textbuf to append the text to
 * @return 1 if it returned without =error, else 0
 */
int recurse( char *path, textbuf *tb )
{
    int res = 1;
    DIR *dp = opendir( path );
    if (dp != NULL)
    {
        struct dirent *ep = readdir(dp);
        while ( ep != NULL )
        {
            if ( strcmp(ep->d_name,"..")!=0&&strcmp(ep->d_name,".")!=0 )
            {
                if ( is_directory(path,ep->d_name) )
                {
                    char *full_path = allocate_path( path, ep->d_name );
                    if ( full_path != NULL )
                    {
                        if ( starts_with(ep->d_name,"%2F") )
                            recurse( full_path, tb );
                        else
                        {
                            pathset *ps = pathset_create( full_path, NULL );
                            if ( ps != NULL )
                            {
                                char *text = pathset_tostring( ps );
                                if ( text == NULL || !textbuf_add(tb,text) )
                                {
                                    res = 0;
                                    break;
                                }
                                else if ( text != NULL )
                                    free( text );
                                pathset_dispose( ps );
                            }
                        }
                        free( full_path );
                    }
                    else
                    {
                        fprintf(stderr,"pathset: allocate path failed\n");
                        res = 0;
                        break;
                    }
                }
            }
            ep = readdir( dp );
        }
        closedir( dp );
    }
    return res;
}
/*
 * Main entry point
 */
int main(int argc, char** argv) 
{
    if ( argc == 2 )
    {
        textbuf *tb = textbuf_create();
        if ( tb != NULL )
        {
            if ( recurse(argv[1],tb) )
            {
                char *result = textbuf_get( tb );
                int line,pos;
                char *file_name = swap_file(argv[1],"pathset.txt");
                if ( file_name != NULL && compare_to_file(file_name,result,
                    &line,&pos) )
                    printf("pathset test passed\n");
                else
                {
                    printf("pathtest failed. mismatch on line %d char %d\n",
                        line,pos );
                    // for printing comparison text uncomment
                    // printf("%s",result);
                }
                if ( file_name != NULL )
                    free( file_name );
                textbuf_dispose( tb );
            }
        }
    }
    else
        fprintf(stderr,"usage: mmpupload folder\n");
    return (EXIT_SUCCESS);
}
#endif
