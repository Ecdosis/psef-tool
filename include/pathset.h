/* 
 * File:   pathset.h
 * Author: desmond
 *
 * Created on May 29, 2012, 6:52 AM
 */

#ifndef PATHSET_H
#define	PATHSET_H
#define PATHSET_MVD 1
#define PATHSET_CORCODE 2
#define PATHSET_CORTEX 3
#define PATHSET_TEXT 4
#define PATHSET_XML 5
#define PATHSET_IMG 6
#define PATHSET_UNSET 0
#define PATHSET_LITERAL "literal"
#ifdef	__cplusplus
extern "C" {
#endif
typedef struct pathset_struct pathset;
pathset *pathset_create( char *folder, char *docid, char *name, int kind );
int pathset_size( pathset *ps );
char *pathset_get_path( pathset *ps, int index );
int pathset_kind( pathset *ps );
char *pathset_get_docid( pathset *ps );
char *pathset_name( pathset *ps );
void pathset_dispose( pathset *ps );
const char *pathset_compute_name( char *path );
pathset *pathset_next( pathset *ps );

#ifdef	__cplusplus
}
#endif

#endif	/* PATHSET_H */
