#!/bin/sh

i=1
for file in $(find *.fbs); do
   echo "[$i]:find $file"
   flatc --cpp -o ../3rd $file
   echo "\t\t...generate success!"
   i=i+1
done
exit 0