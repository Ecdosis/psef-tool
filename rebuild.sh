#!/bin/bash
gcc -Iinclude src/*.c -lm -o pdef-tool
gzip -c pdef-tool.1 > pdef-tool.1.gz 
cp pdef-tool.1.gz /usr/local/share/man/man1/
