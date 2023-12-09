# Sa se implementeze:
# A) un shell script care se apelează sub forma "bash script <c>", unde <c> este un caracter
# alfanumeric.
# Script-ul va verifica faptul că a primit exact un argument. În caz contrar, va afișa un mesaj de
# utilizare și se va încheia.
# Script-ul va citi continuu linii de la intrarea standard pana la intalnirea end-of-file (atunci când
# intrarea standard e tastatura/terminal, condiția se poate genera prin combinatia de butoane
# CTRL+D) și va contoriza numărul de “propoziții corecte” care conțin caracterul primit ca și
# argument.
# O propoziție se considera corecta dacă respectă următoarele condiții:
# - începe cu litera mare
# - conține doar litere mari, mici, cifre, spații, virgula, punct semnul exclamarii sau
# semnul întrebării
# - se termina cu semnul întrebării, semnul exclamarii sau punct
# - nu contine virgula (,) înainte de și
# La final va afișa acest contor pe ecran (doar valoarea numerică).

# !/bin/bash

if [ "$#" -eq 1 ]; 
   then
    caracter=$1
    count=0

    while IFS= read -r line; 
      do
        if echo "$line" | grep -E "^[A-Z][a-zA-Z0-9\ \.\,!?-]+[\.!?]$" | grep -vE "(si[\ ]*,)" | grep -v "n[pb]" ;
         then
            if [[ $line == *"$caracter"* ]]; then
                ((count++))
            fi
        fi
    done
    echo $count
fi



