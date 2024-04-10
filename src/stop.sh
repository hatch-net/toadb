#! /bin/sh
# toadb stop
pid=`ps |grep toadb |grep -v grep| gawk '{ print $1 }'`

kill -15 ${pid}
