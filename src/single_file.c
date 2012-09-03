#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "single_file.h"

struct single_file_struct
{
    char *path;
    char *docid;
    char *fname;
    single_file *next;
};
/**
 * Create a single file instance
 * @param path the path including the file name
 * @param docid the docid NOT including its name
 * @param fname the file name
 * @return a single file object
 */
single_file *single_file_create( char *path, char *docid, char *fname )
{
    single_file *sf = calloc( 1, sizeof( single_file ) );
    if ( sf != NULL )
    {
        sf->path = strdup( path );
        sf->fname = strdup( fname );
        sf->docid = strdup( docid );
        if ( sf->path==NULL||sf->fname==NULL||sf->docid==NULL )
        {
            single_file_dispose( sf );
            sf = NULL;
            fprintf(stderr,"single_file: failed to allocate object\n");
        }
    }
    return sf;
}
/**
 * Get the next file in the list
 * @param sf a element in the list
 * @return the next element
 */
single_file *single_file_next( single_file *sf )
{
    return sf->next;
}
/**
 * Get this file's name
 * @param sf the single file object
 * @return its file name
 */
char *single_file_fname( single_file *sf )
{
    return sf->fname;
}
/**
 * Get this file's path (including its name)
 * @param sf the single file object
 * @return its path
 */
char *single_file_path( single_file *sf )
{
    return sf->path;
}
/**
 * Get this file's parent docid (minus its file name
 * @param sf the single file object
 * @return its docid
 */
char *single_file_docid( single_file *sf )
{
    return sf->docid;
}
/**
 * Dispose of a single file object and all its children
 * @param sf the sf head of list object
 */
void single_file_dispose( single_file *sf )
{
    if ( sf->path != NULL )
        free( sf->path );
    if ( sf->docid != NULL )
        free( sf->docid );
    if ( sf->fname != NULL )
        free( sf->fname );
    if ( sf->next != NULL )
        single_file_dispose( sf->next );
    free( sf );
}
/**
 * Append a single_file to itself as a list
 * @param head the head of the list
 * @param child the new child to put at the end
 * @return the new head of the list
 */
single_file *single_file_append( single_file *head, single_file *child )
{
    if ( head == NULL )
        return child;
    else
    {
        single_file *temp = head;
        while ( temp->next != NULL )
            temp = temp->next;
        temp->next = child;
        return head;
    }
}