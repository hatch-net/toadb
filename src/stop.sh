#! /bin/sh
# toadb stop
pid=`ps -ef|grep toadb |grep -v grep| gawk '{ print $2 }'`
echo ${pid}
kill -15 ${pid}
