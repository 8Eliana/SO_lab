#!/bin/bash

while read linie
do
 var=$(echo $linie | grep -E "^[A-Z][a-zA-Z0-9\ \.\,]+[\.]$" | grep -vE "(si[\ ]*,)" | grep -v "n[pb]")
 if [[ -n $var ]]
 then echo "Propozita indeplineste conditille"
 else
 echo "Propozita nu indeplineste conditille"
 fi
done

