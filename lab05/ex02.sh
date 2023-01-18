#!/bin/bash

if [ $# -eq 0 ]
then
	read -p "Dati numele directorului de start: " numedir
fi
if [ $# -eq 1 ]
then 
	numedir=$1
	read -p "Dati adancimea " adancime
fi
if [ $# -eq 2 ]
then 
	numedir=$1
	adancime=$2
fi

function parcurgere_director ()
{
    for fis in $(ls -A $1)
    do
	if [ $2 -gt 0 ]
	then
		ale=$1/$fis
        	if [ -d $cale ]
	 	then
	    		let new_adancime=$2-1
            		parcurgere_director $cale $new_adancime
		else
			if grep -q bin/bash  "$cale"
			 then
  				echo "$cale Este script bash"
			else
				echo "$cale Nu este script bash"
 			fi 
        	fi
	fi
    done
}

parcurgere_director $numedir $adancime
