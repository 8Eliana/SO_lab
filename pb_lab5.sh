#!/bin/bash

if test "$#" -eq 2
then 
echo "Numarul de parametri este ok"
else
echo "Numarul de parametri nu este ok"
fi

director=$1
outfile=$2
count=0
count2=0

for fisier in $director/*
do 
if [ -f $fisier ]
  then var=$(cat $fisier | grep -E "^[A-Z][a-zA-Z0-9\ \.\,]+[\.]$" | grep -vE "(si[\ ]*,)" | grep -v "n[pb]")
  if [[ -n $var ]]
    then echo $var >> $outfile
  fi
elif [ -L $fisier ]
  then count=`expr $count + 1`
elif [ -d $fisier ]
 then
  count2=`bash $0 $fisier $outfile`
  count=`expr $count + $count2`
fi
done
echo $count