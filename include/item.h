#ifndef ITEM_H
#define ITEM_H
// define types
#define NO_TYPE 0
#define MVD_CORTEX 1
#define MVD_CORCODE 2
#define TEXT_CORTEX 3
#define TEXT_CORCODE 4
#define XML 5
#define MIXED 6
typedef struct item_struct item;
item *item_create( char *docid, int type, char *db );
void item_dispose( item *it );
char *item_key( item *i );
char *item_type( item *it );
config *item_config( item *it );
char *item_docid( item *it );
void item_versionID( item *it );
int item_num_paths( item *it );
void item_print( item *it );
void item_set_config( item *it, config *cf );
void item_set_versionID( item *it, char *versionID );
int item_path_starts( item *it, char *p );
#endif

