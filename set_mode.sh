#!/bin/bash

if test "$#" -eq 2
then 
echo "Numarul de parametri este ok"
else
echo "Numarul de parametri nu este ok"
fi

director=$1
caracter=$2

#nerecursiv
for fisier in "$director"/*.txt 
do 
if [ $caracter="x" ] || [ $caracter="r" ] || [ $caracter="w" ]
then 
chmod "+$caracter" "$fisier"
fi
done

#recursiv
for fisier in $director/*
do 
if [ -d "$fisier" ]
then bash $0 $fisier $caracter
fi
done


