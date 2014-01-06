/* 
 * File:   moddate.h
 * Author: desmond
 *
 * Created on October 10, 2013, 11:10 AM
 */

#ifndef MODDATE_H
#define	MODDATE_H

#ifdef	__cplusplus
extern "C" {
#endif
#define MODDATE_FILE ".moddate"

typedef struct moddate_struct moddate;
moddate *moddate_create( char *path );
void moddate_dispose( moddate *md );
void moddate_save( moddate *md );
int moddate_is_later( moddate *md, char *path );


#ifdef	__cplusplus
}
#endif

#endif	/* MODDATE_H */

