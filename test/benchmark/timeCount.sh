#! /bin/sh
# 计算时间差，并格式化为秒，分，时，天输出
function usertimes() {
beginTime=$(date "+%Y-%m-%d %H:%M:%S")
echo "beginTime:" $beginTime
 
sleep 1
 
endTime=$(date "+%Y-%m-%d %H:%M:%S")
echo "endtime:" $endTime
 
duration=$(($(date +%s -d "${endTime}")-$(date +%s -d "${beginTime}")));
echo "时间差:" $duration"s"

echo $(($(date +%s -d "${endTime}") - $(date +%s -d "${beginTime}"))) | awk '{t=split("60 s 60 m 24 h 999 d",a);for(n=1;n<t;n+=2){if($1==0)break;s=$1%a[n]a[n+1]s;$1=int($1/a[n])}print s}'

timeelapse=`echo $(($(date +%s -d "${endTime}") - $(date +%s -d "${beginTime}"))) | awk '{t=split("60 s 60 m 24 h 999 d",a);for(n=1;n<t;n+=2){if($1==0)break;s=$1%a[n]a[n+1]s;$1=int($1/a[n])}print s}'`

echo "elapse time:"${timeelapse}
}


# @args <beg> <end>
# return random integer in [<beg>, <end>)
function random_range() {
    local beg=$1
    local end=$2
    echo $(($(od -An -N2 -i /dev/random) % ($end - $beg) + $beg))
}

glob=1000
# @args null
# return null
function test() {
    glob=1001
}

function test1() {
    test
}
test1
echo "glob="$glob

#max=1000
#random_num=`echo $( random_range -5000 $max )`
#echo $random_num

#beginTimes=$(date +%s)
#echo $beginTimes