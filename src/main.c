/* 
 * File:   main.c
 * Author: desmond
 *
 * Created on July 15, 2012, 8:37 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "archive.h"
#include "download.h"
#ifdef MEMWATCH
#include "memwatch.h"
#endif
#ifdef COMMANDLINE
/* NULL-terminated array of formats */
char **formats = {"MVD"};
/** docid to query for download */
char *docid = ".*";
/** name for downloaded archive */
char *name = "archive";
/** type of zipping for downloaded archive */
char *zip_type = "TAR_GZ";
/** host to upload to/download from */
char *host = "http://localhost:8080/";
/** the folder to upload */
char *folder = NULL;
/**
 * Read the comma-separated list of formats
 * @param fmts a list of TEXT,MVD,XML,MIXED
 * @return 
 */
char **read_formats( char *fmts )
{
    char **array = calloc( 5, sizeof(char*) );
    if ( array != NULL )
    {
        int i = 0;
        char *text = strdup( fmts );
        if ( text != NULL )
        {
            char *token = strtok( text, "," );
            while ( token != NULL )
            {
                array[i++] = strdup(token);
                token = strtok( NULL, "/" );
            }
            free( text );
        }
    }
    return array;
}
/**
 * Test the first char of a commanline parameter
 * @param str the param
 * @param c the char
 * @return 1 if str starts with c else 0
 */
int static starts_with( char *str, char c )
{
    if ( strlen(str)>0 && str[0] == c )
        return 1;
    else
        return 0;
}
/**
 * Check arguments
 * @param argc the number of arguments
 * @param argv the array of commandline arguments
 * @return 1 if they were sane
 */
static int check_args( int argc, char **argv )
{
    int i,sane = 1;
    for ( i=1;i<argc;i++ )
    {
        if ( strlen(argv[i])>1 && argv[i][0]=='-' )
        {
            switch ( argv[i][1] )
            {
                case 'h':
                    if ( argc >i+1 )
                        host = argv[i+1];
                    break;
                case 'f':
                    if ( argc >i+1 )
                        formats = read_formats( argv[i+1] );
                    break;
                case 'd':
                    if ( argc >i+1 )
                        docid = argv[i+1];
                    break;
                case 'n':
                    if ( argc > i+1 )
                        name = argv[i+1];
                    break;
                case 'z':
                    if ( argc > i+1 )
                    {
                        if ( strcmp(argv[i+1],"tar_gz")==0 )
                            zip_type = "TAR_GZ";
                        else if ( strcmp(argv[i+1],"zip")==0 )
                            zip_type = "ZIP";
                        else
                            sane = 0;
                    }
                    break;
                default:
                    sane = 0;
                    break;
            }
        }
    }
    if ( argc>2 && !starts_with(argv[argc-1],'-') 
        && !starts_with(argv[argc-2],'-') )
        folder = argv[argc-1];
    return sane;
}
/**
 * Tell people how this things should be invoked. This is not a man page.
 */
static void usage()
{
    printf( 
    "pdef-tool [-h host] [-f formats] [-d docid] [-n name] "
    "[-z zip-type] [folder]\n\n"
    "  host: url for download (defaults to http://localhost:8080/)\n"
    "  formats: a comma-separated list of TEXT,XML,MVD,MIXED (defaults to MVD)\n"
    "  docid: wildcard prefix docid, e.g. english/poetry.* (defaults to .*)\n"
    "  name: name of archive to download (defaults to archive)\n"
    "  zip-type: type of zip archive, either tar_gz or zip (defaults to tar_gz)\n"
    "  folder: relative path to folder for uploading\n\n"
    );
}
/*
 * Main entry point
 */
int main( int argc, char** argv ) 
{
    if ( check_args(argc,argv) )
    {
        int res = 1;
        if ( folder != NULL )
            res = archive_scan( name );
        else
            res = download( host, formats, docid, name, zip_type );
        if ( !res )
            fprintf(stderr,"archive: failed to load\n");
    }
    else
        usage();
}
#endif