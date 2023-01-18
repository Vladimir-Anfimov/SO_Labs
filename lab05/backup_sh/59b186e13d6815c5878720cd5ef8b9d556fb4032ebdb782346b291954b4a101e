#!/bin/bash

if [ $# -eq 0 ]
then
	read -p "Dati un n: " n
else
	n=$1
fi

function puteri()
{
	if [ $1 -le $2 ]
	then
		echo "$3"
		let pw=$3*3
		let k=$1+1
		puteri $k $2 $pw
	fi
}

puteri 0 $n 1
