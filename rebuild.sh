#!/bin/bash
if [ $USER = "root" ]; then
  gcc -Iinclude src/*.c -lm -o pdef-tool
  gzip -c pdef-tool.1 > pdef-tool.1.gz
  cp pdef-tool.1.gz /usr/share/man/man1/
  cp pdef-tool /usr/local/bin/
else
  echo "Need to be root. Did you use sudo?"
fi
