#!/bin/bash
### ex01

if [ $# -ne 1 ]
then
    echo "Utilizare: $0 director număr"
fi

if [ ! -d $1 ]
then
    echo "Primul argument trebuie să fie director."
fi

lista=$(find $1 -type f -name "*.sh")

for fis in $lista
do
       	new_name=$(sha256sum $fis | cut -d" " -f1)
	cp $fis ~/Desktop/Labs/lab05/backup_sh/$new_name 
done
