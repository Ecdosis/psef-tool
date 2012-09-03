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
#define CONF_SUFFIX ".conf"
#define CRLEN strlen("\n")
#define PATHSET_CORTEX_NAME "cortex"
#define PATHSET_CORCODE_NAME "corcode"
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
    pathset *children;
    pathset *next;
};
/**
 * Add a child to a pathset (a corcode or cortex)
 * @param ps the pathset to have children
 * @param child the child
 */
void pathset_add_child( pathset *ps, pathset *child )
{
    if ( ps->children == NULL )
        ps->children = child;
    else 
    {
        pathset *temp = ps->children;
        while ( temp->next != NULL )
            temp = temp->next;
        temp->next = child;
    }
}
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
 * Build the paths from the given directory
 * @param ps the pathset in question
 * @param path the relative path from CWD to the file or dir
 * @return 1 if it worked else 0
 */
static int build_paths( pathset *ps, char *path )
{
    DIR *dp;
    int res = 1;
    int has_corcode = 0;
    int has_cortex = 0;
    int nxmls=0;
    int ntxts=0;
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
                    if ( strcmp(ep->d_name,PATHSET_CORTEX_NAME)==0 )
                    {
                        char *fullpath = allocate_path(path,ep->d_name);
                        if ( fullpath != NULL )
                        {
                            pathset *child = pathset_create(fullpath,
                                NULL, ps->name, PATHSET_CORTEX);
                            pathset_add_child( ps, child );
                            has_cortex = 1;
                            free( fullpath );
                        }
                        else
                            res = 0;
                    }
                    else if( strcmp(ep->d_name,PATHSET_CORCODE_NAME)==0 )
                    {
                        char *fullpath = allocate_path(path,ep->d_name);
                        if ( fullpath != NULL )
                        {
                            pathset *child = pathset_create(fullpath,
                                NULL, ps->name, PATHSET_CORCODE);
                            pathset_add_child(ps, child);
                            has_corcode = 1;
                            free( fullpath );
                        }
                        else
                            res = 0;
                    }
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
                    if ( ends_with(ep->d_name,XML_SUFFIX))
                        nxmls++;
                    else
                        ntxts++;
                    res = add_path( ps, path, ep->d_name );
                }
            }
            ep = readdir( dp );
            if ( !res )
                break;
        }
        closedir (dp);
    }
    // set name
    if ( ps->kind == PATHSET_UNSET )
    {
        if ( has_corcode && has_cortex )
            ps->kind = PATHSET_MVD;
        else if ( nxmls > ntxts )
            ps->kind = PATHSET_XML;
        else
            ps->kind = PATHSET_TEXT;
    }
    return res;
}
/**
 * Get the knd of this set
 * @param ps the pathset
 * @return its kind as an int
 */
int pathset_kind( pathset *ps )
{
    return ps->kind;
}
/**
 * Create a pathset and discover all its paths
 * @param folder the relative path to the folder
 * @param docid the docid for this pathset, excluding its name
 * @param name a suggested name or NULL
 * @param kind the kind of this pathset
 * @return a finished pathset
 */
pathset *pathset_create( char *folder, char *docid, char *name,int kind )
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
    pathset *child = ps->children;
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
 * A pathset can be a set of XML files OR a set of single versions 
 * split into markup and text OR a set of plain text file versions
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
    if ( ps->children == NULL )
        return ps->used;
    else
    {
        int total = 0;
        pathset *temp = ps->children;
        while ( temp != NULL )
        {
            total += pathset_size( temp );
            temp = temp->next;
        }
        return total;
    }
}
/**
 * Get a particular path from the set
 * @param ps the pathset
 * @param index index into the pathset
 * @return a path or NULL
 */
char *pathset_get_path( pathset *ps, int index )
{
    if ( ps->used > 0 )
    {
        if ( index < ps->used )
            return ps->paths[index];
        else
            fprintf(stderr,"pathset: index too big: %d (max %d)\n",
                index,ps->used);
    }
    else 
    {
        int start = 0;
        pathset *temp = ps->children;
        while ( temp != NULL )
        {
            if ( index < start+temp->used )
                return temp->paths[index-start];
            start += temp->used;
            temp = temp->next;
        }
        fprintf(stderr,"pathset: index too big: %d (max %d)\n", index,start);
    }
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
    int size;
    if ( ps->docid != NULL )
        // CR, NULL, ": "
        size = strlen(ps->name) + 3 + CRLEN + strlen(ps->docid);
    else
        // NULL
        size = 1;
    if ( ps->children != NULL )
    {
        pathset *child = ps->children;
        while ( child != NULL )
        {
            size += pathset_measure( child );
            child = child->next;
        }
    }
    else 
    {
        int i;
        for ( i=0;i<ps->used;i++ )
            size += CRLEN + strlen(ps->paths[i]);
    }
    return size;
}
/**
 * Debug: convert a pathset to a string
 * @param ps the pathset to convert
 * @return an allocated string with the docid and all the paths
 */
static char *pathset_tostring( pathset *ps )
{
    int plen = pathset_measure( ps );
    int slen = 0;
    char *str = calloc( 1, plen );
    if ( str != NULL )
    {
        if ( ps->docid != NULL )
        {
            snprintf( str, plen, "%s: %s\n",ps->name,ps->docid );
            slen = strlen(ps->name)+strlen(ps->docid)+3+CRLEN;
        }
        if ( ps->children != NULL )
        {
            pathset *child = ps->children;
            while ( child != NULL )
            {
                char *child_str = pathset_tostring( child );
                if ( child_str != NULL )
                {
                    strncat( str, child_str, plen-slen );
                    slen += strlen( child_str );
                    free( child_str );
                }
                child = child->next;
            }
        }
        else
        {
            int i;
            for ( i=0;i<ps->used;i++ )
            {
                strncat( str, ps->paths[i], plen-slen );
                slen += strlen( ps->paths[i] );
                strncat( str, "\n", plen-slen );
                slen += CRLEN;
            }
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
