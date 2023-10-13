#!/bin/bash

if test "$#" -eq 2
then 
echo "Numarul de parametri este ok"
else
echo "Numarul de parametri nu este ok"
fi

director=$1
outfile=$2

caracter=0
suma=0

for fisier in "$director"/*.txt 
do
caractere=$(wc -m < "$outfile")
suma=`expr $suma + $caractere`
echo "$fisier $caractere" >> "$outfile"
done

echo "TOTAL $suma" >> "$outfile"