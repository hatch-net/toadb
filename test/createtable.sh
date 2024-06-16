#!/bin/sh
# insert data

tsql=../src/tools/tsql/toadsql
datadir=../src/toadbtest

tablenum=$1
rownum=$2

a=0
while [ $a -le ${tablenum} ]
do
	echo $a
#	$(tsql) -D /home/senllang/toadbtest -C "insert into account1(id, name, tel) values($a,'zhangwanglizhao104101','address101104');"
	tablename=account${a}
	../src/tools/tsql/toadsql -D ${datadir}  -C "create table ${tablename}(id int, name varchar, tel varchar);"
	./insertdata.sh ${tablename} ${rownum} ${datadir}
	let a++
done
