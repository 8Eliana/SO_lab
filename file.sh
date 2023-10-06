#!/bin/sh
if test $# -lt 3
then echo "Usage $0 file dir n1 n2 n3..."
exit
fi

count=0
sum=0

file=$1
dir=$2

shift 2
for arg in "$@"
do
sum=`expr $sum + $arg`
if test $arg -gt 10
then count=`expr $count + 1`
fi
done

echo "Argumentele mai mari decat 10 sunt:"$count
echo "Suma argumentelor este:"$sum
echo $sum|wc -L

if test -f "$file"
then  echo "$count">"$file"
      echo "$sum">>"$file"
fi

