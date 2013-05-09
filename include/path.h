#ifndef PATH_H
#define PATH_H
typedef struct path_struct path;
path *path_create( char *fname );
void path_dispose( path *p );
void path_dispose_all( path *h );
char *path_get( path *p );
path *path_next( path *p );
void path_append( path *fp, char *p );
int path_scan( char *path );
#endif

