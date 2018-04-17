#!/bin/sh

i=1
for file in $(find *.fbs); do
   echo "[$i]:find $file"
   flatc --cpp -o ../include/ $file
   i=$(($i+1));
done
exit 0