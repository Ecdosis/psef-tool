/* 
 * File:   archive.h
 * Author: desmond
 *
 * Created on May 27, 2012, 10:09 AM
 */

#ifndef ARCHIVE_H
#define	ARCHIVE_H
#ifdef	__cplusplus
extern "C" {
#endif
#define UPLOADED_ALL 2
#define UPLOADED_SOME 1
#define UPLOADED_NONE 0
int archive_scan();
int upload( moddate *md, char *dir );

#ifdef	__cplusplus
}
#endif

#endif	/* ARCHIVE_H */

