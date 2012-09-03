/* 
 * File:   single_file.h
 * Author: desmond
 *
 * Created on August 2, 2012, 7:10 AM
 */

#ifndef SINGLE_FILE_H
#define	SINGLE_FILE_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct single_file_struct single_file;
single_file *single_file_append( single_file *parent, single_file *child );
single_file *single_file_create( char *path, char *docid, char *fname );
void single_file_dispose( single_file *sf );
char *single_file_fname( single_file *sf );
char *single_file_path( single_file *sf );
char *single_file_docid( single_file *sf );
single_file *single_file_next( single_file *sf );

#ifdef	__cplusplus
}
#endif

#endif	/* SINGLE_FILE_H */

