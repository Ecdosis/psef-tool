#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "item.h"
#include "path.h"
#ifdef MEMWATCH
#include "memwatch.h"
#endif
struct item_struct
{
    // unique identifier fields:
    char *docid;
    int type;
    char *db;
    char *versionID;
    // auxilliary data
    path *paths;
    config *cf;
};
item *item_create( char *docid, int type, char *db )
{
    item *i = calloc( 1, sizeof(item) );
    if ( i != NULL )
    {
        i->docid = strdup( docid );
        if ( db != NULL )
            i->db = strdup( db );
        i->type = type;
    }
    else
        fprintf(stderr,"item:failed to create object\n");
    return i;
}
void item_dispose( item *it )
{
    path *p = it->paths;
    while ( p != NULL )
    {
        path *next = path_next( p );
        path_dispose( p );
        p = next;
    }
    if ( it->cf != NULL )
        config_dispose( it->cf );
    if ( it->docid != NULL )
        free( it->docid );
    if ( it->db != NULL )
        free( it->db );
    if ( it->versionID != NULL )
        free( it->versionID );
    free( it );
}
void item_print( item *it )
{
    int npaths = item_num_paths(it);
    char *files = (npaths==1)?"file":"files";
    printf("%s: %s, %d %s", it->docid, item_type(it), 
        npaths, files );
    if ( it->cf != NULL )
    {
        printf(", config: ");
        config_print( it->cf );
    }
    if ( it->versionID != NULL )
        printf(", versionID: %s",it->versionID);
    printf("\n");
}
config *item_config( item *it )
{
    return it->cf;
}
char *item_key( item *i )
{
    char *db = (i->db==NULL)?"":i->db;
    int len = strlen(i->docid)+strlen(db)+2;
    char *key = calloc( 1, len );
    snprintf(key,len,"%s%d%s",i->docid,i->type,db);
    return key;
}
void item_versionID( item *it )
{
    return it->versionID;
}
void item_set_versionID( item *it, char *versionID )
{
    if ( it->versionID != NULL )
        free( it->versionID );
    it->versionID = strdup( versionID );
}
char *item_docid( item *it )
{
    return it->docid;
}
void item_add_path( item *it, char *p )
{
    if ( it->paths == NULL )
        it->paths = path_create(p);
    else
        path_append( it->paths, p );
}
char *item_type( item *it )
{
    switch ( it->type )
    {
        case MVD_CORTEX:
            return "MVD cortex";
            break;
        case MVD_CORCODE:
            return "MVD corcode";
            break;
        case TEXT_CORTEX:
            return "TEXT cortex";
            break;
        case TEXT_CORCODE:
            return "TEXT corcode";
            break;
        case XML:
            return "XML";
            break;
        case MIXED:
            return "MIXED";
            break;
    }
}
void item_set_config( item *it, config *cf )
{
    if ( it->cf != NULL )
        config_dispose( it->cf );
    it->cf = cf;
}
int item_path_starts( item *it, char *p )
{
    path *temp = it->paths;
    int plen = strlen(p);
    while ( temp != NULL )
    {
        char *pt = path_get(temp);
        if ( strlen(pt)>=plen && strncmp(p,pt,plen)==0 )
            return 1;
        else
            temp = path_next(temp);
    }
    return 0;
}
int item_num_paths( item *it )
{
    int num = 0;
    path *p = it->paths;
    while ( p != NULL )
    {
        num++;
        p = path_next(p);
    }
    return num;
}