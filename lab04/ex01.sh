#! /bin/sh


read -p "Scrieti valoarea lui p: " p

sum=0

if test $# -eq 0
then
	read -p  "Dati lungimea secventei de numere: " n
	for i in $(seq 1 $n)
	do
		read -p "Dati al $i-lea numar: " nr
		if [ $nr -le $p ]
		then		
			let cub=nr*nr*nr
			let sum=sum+cub
		fi
	done			
else
	for x in $*
	do
		if [ $x -le $p ]
		then
			let cub=x*x*x
			let sum=sum+cub
		fi
	done
fi	

echo "Suma cuburilor este $sum"

