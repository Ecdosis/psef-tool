#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "moddate.h"

struct moddate_struct
{
    time_t last_mod_date;
    FILE *file;
};

/**
 * Create a moddate oject and initialise it
 * @param path the path to the containing folder
 * @return the object or NULL
 */
moddate *moddate_create( char *path )
{
    moddate *md = calloc( 1, sizeof(moddate) );
    if ( md != NULL )
    {
        int len = strlen(path)+2+strlen(MODDATE_FILE);
        char *temp = malloc(len);
        if ( temp != NULL )
        {
            snprintf(temp,len,"%s/%s",path,MODDATE_FILE);
            md->file = fopen( temp, "r" );
            if ( md->file != NULL )
            {
                int res = fscanf( md->file, "%ld", &md->last_mod_date );
                if ( res == 0 )
                {
                    fprintf(stderr,"moddate: failed to read from file\n");
                    md->last_mod_date = 0;
                }
                fclose( md->file );
            }
            md->file = fopen( temp, "w" );
            if ( md->file == NULL )
            {
                fprintf(stderr,"moddate: failed to open file for writing\n");
                free( md );
                md = NULL;
            }
            free( temp );
        }
    }
    else
        fprintf(stderr,"moddate: failed to allcoate object\n");
    return md;
}
/**
 * Dispose of a possibly open moddate
 * @param md the object in question
 */
void moddate_dispose( moddate *md )
{
    free( md );
}
/**
 * Save the modification date to disk
 * @param md the moddate object
 */
void moddate_save( moddate *md )
{
    if ( md->file != NULL )
    {
        if ( md->last_mod_date == 0 )
            md->last_mod_date = time(NULL);
        fprintf(md->file,"%ld",md->last_mod_date );
        fclose( md->file );
    }
}
/**
 * Save the modification date to disk
 * @param md the moddate object
 * @return 1 if it worked
 */
int moddate_remove( char *folder )
{
    int len = strlen(folder)+2+strlen(MODDATE_FILE);
    char *temp = malloc(len);
    if ( temp != NULL )
    {
        snprintf(temp,len,"%s/%s",folder,MODDATE_FILE);
        int res = remove(temp);
        free( temp);
        return res==0;
    }
    return 0;
}
/**
 * Is a date later than our last moddate?
 * @param md the object to test against
 * @param path the file to compare with
 * @return 1 if it was later else 0
 */
int moddate_is_later( moddate *md, char *path )
{
    struct stat buf;
    int res = stat(path, &buf);
    if ( !res )
    {
        //printf("buf.st_mtime=%ld md->last_mode_date=%ld\n",buf.st_mtime,md->last_mod_date);
        if ( buf.st_mtime > md->last_mod_date )
            return 1;
    }
    else
        fprintf(stderr,"moddate: failed to stat file %s\n",path);
    return 0;
}
/**
 * Update the moddate object to the present
 * @param md the object in question
 */
void moddate_update( moddate *md )
{
    md->last_mod_date = time(NULL);
}