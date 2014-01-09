#!/bin/bash
for file in *
do
  sed -i -e "s/\"uri\", \"id\"/\"uri\", \"id\", \"updated\"/" $file
done
