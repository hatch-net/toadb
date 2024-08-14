#! /bin/sh
# toadb stop
pid=`ps -ef|grep toadb-0-01 |grep -v grep| gawk '{ print $2 }'`
echo ${pid}
kill -15 ${pid}
