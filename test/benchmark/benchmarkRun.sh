#! /bin/sh
# benchmarkBuildData.sh 
# create by senllang 2024/1/8
# mail : study@senllang.onaliyun.com
#
# Copyright (c) 2023-2024 senllang
# 
# toadb is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
# http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.
#

help_output="input parameters:\nscale number (1-10000), \nrunmode(1 tpcb, 2 update, 3 select only), \nrun time(seconds), \nlogprint interval(seconds)[optional] \nexample as (./benchmarkBuildData.sh 1 1 600 10):"

toadb_command="/home/senllang/Dev/toadb/src/tools/tsql/toadsql";
toadb_datadir="/home/senllang/toadbtest/"

toad_executor_command="${toadb_command} -D ${toadb_datadir} -C"

nbranches=1								
ntellers=10
naccounts=100000

#########################################
#  check input scale 
##########################################
# scale factor
scale=$1 
runmode=$2
runtimes=$3
logprint=10

if [ $# -gt 1 ];
then 
    echo "scale=$1 runmode=$2"
elif [ $# -gt 3 ];
then
    logprint=$4
else 
    echo -e "$help_output"
    exit
fi

case "$1" in 
  [1-9]*)  
    scale=$1 
    ;; 
  *)  
    echo "input invalid scale."
    echo -e "$help_output"
    exit
    ;; 
esac 

case "$2" in 
  [1-3])  
    runode=$2 
    ;; 
  *)  
    echo "input invalid runmode."
    echo "$help_output"
    exit
    ;; 
esac 

case "$3" in 
  [1-9]*)  
    runtimes=$3
    ;; 
  *)  
    echo "input invalid runtimes(seconds)."
    echo -e "$help_output"
    exit
    ;; 
esac 


####################################
# functions
######################################

# @args <beg> <end>
# return random integer in [<beg>, <end>)
function random_range() {
    local beg=$1
    local end=$2

    if [ $beg -eq $end ]; then 
        echo $beg 
    else
        echo $(($(od -An -N2 -i /dev/random) % ($end - $beg) + $beg))
    fi
    
}

# @args <bid> <tid> <aid> <delta>
# return null
function tpcb() {
    local bid=$1
    local tid=$2
    local aid=$3
    local delta=$4

# "UPDATE toad_accounts SET abalance = abalance + :delta WHERE aid = :aid;\n"
# "SELECT abalance FROM toad_accounts WHERE aid = :aid;\n"
# "UPDATE toad_tellers SET tbalance = tbalance + :delta WHERE tid = :tid;\n"
# "UPDATE toad_branches SET bbalance = bbalance + :delta WHERE bid = :bid;\n"
# "INSERT INTO toad_history (tid, bid, aid, delta, mtime) VALUES (:tid, :bid, :aid, :delta, CURRENT_TIMESTAMP);\n"

    local datetime=`date`

    # "UPDATE toad_accounts SET abalance = abalance + :delta WHERE aid = :aid;\n"
    local sql="UPDATE toad_accounts SET abalance = abalance + ${delta} WHERE aid = ${aid};"
    local sql_command="${toad_executor_command} \"${sql}\" "
    sh -c "${sql_command}" > /dev/null

    # "SELECT abalance FROM toad_accounts WHERE aid = :aid;\n"
    sql="SELECT abalance FROM toad_accounts WHERE aid = ${aid};"
    sql_command="${toad_executor_command} \"${sql}\" "
    sh -c "${sql_command}" > /dev/null

    # "UPDATE toad_tellers SET tbalance = tbalance + :delta WHERE tid = :tid;\n"
    sql="UPDATE toad_tellers SET tbalance = tbalance + ${delta} WHERE tid = ${tid};"
    sql_command="${toad_executor_command} \"${sql}\" "
    sh -c "${sql_command}"  > /dev/null

    # "UPDATE toad_branches SET bbalance = bbalance + :delta WHERE bid = :bid;\n"
    sql="UPDATE toad_branches SET bbalance = bbalance + ${delta} WHERE bid = ${bid};"
    sql_command="${toad_executor_command} \"${sql}\" "
    sh -c "${sql_command}"  > /dev/null

    # "INSERT INTO toad_history (tid, bid, aid, delta, mtime) VALUES (:tid, :bid, :aid, :delta, CURRENT_TIMESTAMP);\n"
    sql="INSERT INTO toad_history (tid, bid, aid, delta, mtime) VALUES (${tid}, ${bid}, ${aid}, ${delta}, '${datetime}');"
    sql_command="${toad_executor_command} \"${sql}\" "  
    sh -c "${sql_command}"  > /dev/null
}

# @args <bid> <tid> <aid> <delta>
# return null
function updatetest() {

# "UPDATE toad_accounts SET abalance = abalance + :delta WHERE aid = :aid;\n"
# "SELECT abalance FROM toad_history WHERE aid = :aid;\n"
# "INSERT INTO toad_history (tid, bid, aid, delta, mtime) VALUES (:tid, :bid, :aid, :delta, CURRENT_TIMESTAMP);\n" 
    
    local bid=$1
    local tid=$2
    local aid=$3
    local delta=$4

    local datetime=`date`
    
    # "UPDATE toad_accounts SET abalance = abalance + :delta WHERE aid = :aid;\n"
    local sql="UPDATE toad_accounts SET abalance = abalance + ${delta} WHERE aid = ${aid};"
    local sql_command="${toad_executor_command} \"${sql}\" "
    sh -c "${sql_command}" > /dev/null

    # "SELECT abalance FROM toad_history WHERE aid = :aid;\n"
    sql="SELECT abalance FROM toad_history WHERE aid = ${aid};"
    sql_command="${toad_executor_command} \"${sql}\" "
    sh -c "${sql_command}"  > /dev/null

    # "INSERT INTO toad_history (tid, bid, aid, delta, mtime) VALUES (:tid, :bid, :aid, :delta, CURRENT_TIMESTAMP);\n"
    sql="INSERT INTO toad_history (tid, bid, aid, delta, mtime) VALUES (${tid}, ${bid}, ${aid}, ${delta}, '${datetime}');"
    sql_command="${toad_executor_command} \"${sql}\" "
    sh -c "${sql_command}" > /dev/null
}


# @args <aid>
# return null
function onlyselect() {

# "SELECT abalance FROM uxbench_accounts WHERE aid = :aid;\n"
    local aid=$1

    # SELECT abalance FROM uxbench_accounts WHERE aid = :aid;
    sql="SELECT abalance FROM toad_accounts WHERE aid = ${aid};"
    sql_command="${toad_executor_command} \"${sql}\" "
    sh -c "${sql_command}" > /dev/null
}

said=1
stid=1
sbid=1
sdelta=1
snbranches=$(($nbranches * $scale))
sntellers=$(($ntellers * $scale))
snaccounts=$(($naccounts * $scale))

# @args null
# return null
function random_id_delta() {
    said=`echo $( random_range 1 $snaccounts )`
    sbid=`echo $( random_range 1 $snbranches )`
    stid=`echo $( random_range 1 $sntellers )`
    sdelta=`echo $( random_range -5000 5000 )`
}


# @args null
# return null
function runTest() {
    
    random_id_delta;
    case $runmode in 
    1)
        tpcb ${sbid} ${stid} ${said} ${sdelta}
    ;;
    2)
        updatetest ${sbid} ${stid} ${said} ${sdelta}
    ;;
    3)
        onlyselect ${said}
    ;;
    esac
}


beginTimes=$(date +%s)
transactions=1
tps=0
realRunTimes=0
lastlogprintTime=0

# @args null
# return false/true
function isEndtime() {

    if [ $realRunTimes -gt $runtimes ]
    then
        echo "0"
    else 
        echo "1"
    fi
}

# @args null
# return null
function printProcess() {
    newlogprint=$(( $realRunTimes - $lastlogprintTime ))

    if [ $newlogprint -ge $logprint ]
    then
        tps=`echo "scale=2; $transactions/$realRunTimes"|bc`
        echo "Test process elapse(s) "$realRunTimes" tps="$tps
        lastlogprintTime=$realRunTimes
    fi    
}

function main()
{
    while [ $isDone -gt 0 ]; 
    do 
        runTest
        currentTime=$(date +%s)
        realRunTimes=$(( $currentTime - $beginTimes ))

        printProcess;

        let transactions+=1;
        isDone=`echo $( isEndtime )`
        #echo "isdone="$isDone" transaction="$transactions" realRunTimes="$realRunTimes
    done 
}

####################################################
# run
####################################################
echo "command infomation:${toad_executor_command}"
echo "datadir infomation:${toadb_datadir}"
echo "pwd : `pwd`"

echo "nbranches :$snbranches"
echo "ntellers :$sntellers"
echo "naccounts :$snaccounts"

begintimef=$(date "+%Y-%m-%d %H:%M:%S")
echo "beginTimes:" $begintimef

echo "---------------------------------------------------------"
echo "start test............."

currentTime=$(date +%s)
realRunTimes=$(( $currentTime - $beginTimes ))
isDone=`echo $( isEndtime )`

# runTest
main

echo "---------------------------------------------------------"
echo "test end............."
echo "begin time:" $begintimef
echo "  end time:" $(date "+%Y-%m-%d %H:%M:%S")
tps=`echo "scale=2; $transactions/$realRunTimes"|bc`
use_totalTime=`echo $realRunTimes | awk '{t=split("60 s 60 m 24 h 999 d",a);for(n=1;n<t;n+=2){if($1==0)break;s=$1%a[n]a[n+1]s;$1=int($1/a[n])}print s}'`
echo "Test process total runtimes(s)  :"$use_totalTime
echo "Test process total transactions :"$transactions
echo "Test process average tps="$tps


