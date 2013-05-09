#include "download.h"
/**
 * Download to disk a portion of the server's DSE data
 * @param host the host to query for the PDEF web-server
 * @param formats an array of formats: MIXED,XML,MVD or TEXT
 * @param docid a wildcard-terminated prefix for the documents to download
 * @param name the name of the downloaded archive
 * @param zip_type the type of zip, either zip or tar_gz
 * @return 1 if it worked else 0
 */
int download( char *host, char **formats, char *docid, char *name, 
    char *zip_type )
{
    return 1;
}
