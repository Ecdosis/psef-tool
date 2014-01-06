#!/bin/bash
if [ $USER = "root" ]; then
  gcc -Iinclude src/*.c -lm -o psef-tool
  gzip -c psef-tool.1 > psef-tool.1.gz
  cp psef-tool.1.gz /usr/share/man/man1/
  cp psef-tool /usr/local/bin/
else
  echo "Need to be root. Did you use sudo?"
fi
