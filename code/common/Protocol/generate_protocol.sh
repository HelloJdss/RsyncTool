#!/bin/sh

i=1
for file in $(find *.fbs); do
   echo "[$i]:find $file"
   flatc --cpp -o ../3rd $file
   let i++
done
exit 0