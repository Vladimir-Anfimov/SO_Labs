#!/bin/bash

if [ $# -l 2 ]
then
	read -p "Dati numarul a: " a
	read -p "Dati numarul b: " b
else
	let a=$1
	let b=$2
fi

if [ a -eq b ]
then
	echo "$a"	
else
	if [ a -le b]
	then
		let new_b=b-a
		bash a new_b
	else
		let new_a=a-b
		bash new_a b
	fi
	let rezultat=$?
	echo "CMMDC=$rezultat"
fi

