/* 
 * File:   main.c
 * Author: desmond
 *
 * Created on July 15, 2012, 8:37 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include "archive.h"
#ifdef MEMWATCH
#include "memwatch.h"
#endif
#ifdef COMMANDLINE
/*
 * Main entry point
 */
int main( int argc, char** argv ) 
{
    if ( argc == 2 )
    {
        int res = archive_scan( argv[1] );
        if ( !res )
            fprintf(stderr,"archive: failed to load\n");
    }
    else
        fprintf(stderr,"usage: mmpuload <dir>\n");
}
#endif