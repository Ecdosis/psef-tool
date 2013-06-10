pdef‐tool(1)                                                      pdef‐tool(1)



NNAAMMEE
       pdef‐tool ‐ uploads or downloads a digital scholarly edition


SSYYNNOOPPSSIISS
       UUppllooaaddiinngg:: pdef‐tool <source‐folder>

       DDoowwnnllooaaddiinngg::

       ‐‐hh <<hhoosstt>> the url for download (defaults to http://localhost:8080/)

       ‐‐ff  <<ffoorrmmaattss>> a comma‐separated list of TEXT,XML,MVD,MIXED (defaults to
       MVD)

       ‐‐dd <<ddoocciidd>> the  prefix  of  a  docid  as  a  regular  expression,  e.g.
       english/poetry.* (defaults to ".*")

       ‐‐nn <<nnaammee>> the name of the archive to download (defaults to archive)

       ‐‐zz  <<zziipp‐‐ttyyppee>>  specifies the type of zip archive, either tar_gz or zip
       (defaults to tar_gz)

       ‐‐rr download required corforms and all configs on server


DDEESSCCRRIIPPTTIIOONN
       ppddeeff‐‐ttooooll is used to upload  or  download  digital  scholarly  editions
       using  the  PDEF (portable digital edition format). A PDEF archive is a
       specially formatted collection of nested folders and files.

       Some folders begin with metacharacters, which are used to  specify  the
       docids of the data on the server. Paths may be literal or relative.


CCOONNFFIIGG ffiilleess
       Config  files,  ending  in  ".conf" are JSON files containing key‐value
       pairs as described below. A config file’s values apply to the directory
       in which it occurs and also to any subordinate folders.


LLIITTEERRAALL PPAATTHHSS
       A folder name beginning with ’@’ designates a literal path. The folder‐
       name minus the ’@’ designates the database collection to which the con‐
       tained  files  will be uploaded. The remaining folders and files nested
       within a literal path folder form the docids. e.g. a file in  a  folder
       structure:

       capuana/@config/stripper/play/italian/capuana.json

       will upload the file "capuana.json" to the database collection "config"
       with the docid "stripper/play/italian/capuana.json". Literal paths  are
       useful for specifying images, configs and corforms.


RREELLAATTIIVVEE PPAATTHHSS
       These begin with "+" and end with "%", so the directory path:

       archive/+english/shakespeare/kinglear/act1/%scene1/

       will    upload    its    contents    to   the   docid   "english/shake‐
       speare/kinglear/act1/scene1".

       Relative paths contain at least one subordinate  folder  called  "MVD",
       "TEXT", "XML" or "MIXED". Their formats are as follows:


MMVVDD ffoollddeerrss
       An MVD folder contains one cortex.mvd file containing all the text ver‐
       sions at that docid. It also must contain a folder "corcode" containing
       all  the corcodes of that cortex, for example the file "default" in the
       corcode sub‐folder would be the default corcode for that cortex.

       Other information about the corcode or cortex may  be  contained  in  a
       .conf  file with the same name as the cortex/corcode, e.g. for the cor‐
       tex.mvd file the file cortex.conf would be a JSON file containing  keys
       about  the  document. The following keys are recognised for cortexs and
       corcodes:

       aauutthhoorr:: The author’s name

       ttiittllee:: The title of the work

       ssttyyllee:: the docid of the desired corform

       ffoorrmmaatt:: One of "TEXT" (cortexs) or "STIL" (corcodes)

       sseeccttiioonn:: The section of the document this MVD refers to  e.g.  "Act  1,
       scene 1"

       vveerrssiioonn11:: The short ID of the first version to display by default, e.g.
       "/Base/F1". (Version IDs always start with a slash, docids do not)


TTEEXXTT ffoollddeerrss
       These contain files whose names will be used to compose the  short‐ver‐
       sion  names. If there are subordinate folders with a TEXT folder, these
       are used to specify group‐names. A versions.conf file may  be  used  to
       specify  the long names of these short‐names. This consists of a single
       array keyed with the tag "versions". The array consists of objects with
       the  keys  "key"  and "value", where "key" refers to the version short‐
       name and "value" to its long name. The short‐names must be the same  as
       the file‐names.


XXMMLL ffoollddeerrss
       May  be specified as per TEXT folders, but extra config keys are recog‐
       nised to facilitate import. In addition  to  the  versions  key,  other
       recognised keys are:

       ccoorrffoorrmm::  specifies  the docid of a corform file (a CSS file wrapped in
       JSON) as the default format for files in this and child directories.

       ssttrriippppeerr:: specifies the docid of  a  stripper  config  file  to  direct
       stripping of markup from files in this and in child directories.

       sspplliitttteerr::  specifies  the  docid of the splitter config to use for this
       and all child directories.

       ffiilltteerr:: designates the name of a Java filter program  to  be  used  for
       filtering text files.


OOtthheerr ccoonnffiigg kkeeyyss
       At the topmost level a PDEF archive should contain a .conf file with at
       least

       bbaassee__uurrll:: The url to upload to, e.g. http://localhost:8080/


EEXXAAMMPPLLEE
       pdef‐tool archive

       pdef‐tool ‐a shakespeare ‐d "english/shakespeare/*"

       (the quotes are required to get around substitution by bash)



                                   29‐5‐2013                      pdef‐tool(1)
