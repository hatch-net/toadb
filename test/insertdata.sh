#!/bin/sh
# insert data


tsql=../src/tools/tsql/toadsql
tablename=$1
rownum=$2
datadir=$3

a=0
while [ $a -le ${rownum} ]
do
	echo $a
#	$(tsql) -D /home/senllang/toadbtest -C "insert into account1(id, name, tel) values($a,'zhangwanglizhao104101','address101104');"
	../src/tools/tsql/toadsql -D ${datadir} -C "insert into ${tablename}(id, name, tel) values($a,'name${a}zhangtestdata','no${a}zhangtestdatatel123');"
	let a++
done
