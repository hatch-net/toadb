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

toadb_command="/home/senllang/Dev/toadb/src/tools/tsql/toadsql";
toadb_datadir="/home/senllang/toadbtest/"

help_output="input scale number (1-10000), the same as(./benchmarkBuildData.sh 1 ):"

##########################
# data model
##########################
nbranches=1								
ntellers=10
naccounts=100000

# toad_history table
create_history="create table toad_history(tid int, bid int, aid int, delta int,mtime varchar, filler varchar);"
drop_history="drop table toad_history;"
toad_history="toad_history"

# toad_branches
create_branches="create table toad_branches(bid int,bbalance int,filler varchar);"
drop_branches="drop table toad_branches;"
toad_branches="toad_branches"

# toad_tellers
create_tellers="create table toad_tellers(tid int,bid int,tbalance int,filler varchar);"
drop_tellers="drop table toad_tellers;"
toad_tellers="toad_tellers"

# toad_accounts
create_accounts="create table toad_accounts(aid int,bid int,abalance int,filler varchar);"
drop_accounts="drop table toad_accounts;"
toad_accounts="toad_accounts"



#########################################
#  check input scale 
##########################################
if [ $# -gt 0 ];
then 
    echo "scale : $1"
else 
    echo "$help_output"
    exit
fi

case "$1" in 
  [1-9]*)  
    scale=$1 
    ;; 
  *)  
    echo "input not number."
    echo "$help_output"
    exit
    ;; 
esac 

# scale factor
scale=$1 

beginTime=$(date "+%Y-%m-%d %H:%M:%S")
echo "beginTime:" $beginTime

#####################################
# start data generate 
#####################################

toad_executor_command="${toadb_command} -D ${toadb_datadir} -C"
echo "command infomation:${toad_executor_command}"
echo "datadir infomation:${toadb_datadir}"
echo "pwd : `pwd`"
echo "---------------------------------------------------------"

# drop table 
echo "${drop_history}"
sql_command="${toad_executor_command} \"${drop_history}\" "
sh -c "${sql_command}"

echo "${drop_tellers}"
sql_command="${toad_executor_command} \"${drop_tellers}\" "
sh -c "${sql_command}"

echo "${drop_accounts}"
sql_command="${toad_executor_command} \"${drop_accounts}\" "
sh -c "${sql_command}"

echo "${drop_branches}"
sql_command="${toad_executor_command} \"${drop_branches}\" "
sh -c "${sql_command}"


# create table 
echo "${create_history}"
sql_command="${toad_executor_command} \"${create_history}\" "
sh -c "${sql_command}"

echo "${create_tellers}"
sql_command="${toad_executor_command} \"${create_tellers}\" "
sh -c "${sql_command}"

echo "${create_accounts}"
sql_command="${toad_executor_command} \"${create_accounts}\" "
sh -c "${sql_command}"

echo "${create_branches}"
sql_command="${toad_executor_command} \"${create_branches}\" "
sh -c "${sql_command}"

createTime=$(date "+%Y-%m-%d %H:%M:%S")
echo "create endTime:" $createTime

# load data 
echo "---------------------------------------------------------"
echo "load data............."
snbranches=$(($nbranches * $scale))
echo "nbranches :$snbranches"
sntellers=$(($ntellers * $scale))
echo "ntellers :$sntellers"
snaccounts=$(($naccounts * $scale))
echo "naccounts :$snaccounts"

bid=1
tid=1
aid=1

# create table toad_branches(bid int,bbalance int,filler varchar);
tableName=$toad_branches
a=1
while ((a <= $snbranches))
do
  bid=$a

  insertSql="insert into $tableName values($bid,0,'init');"
  sql_command="${toad_executor_command} \"${insertSql}\" "
  sh -c "${sql_command}" 

  echo "insert into $tableName bid:$bid" 
  let a++        
done

toad_branchesTime=$(date "+%Y-%m-%d %H:%M:%S")
echo "load toad_branches endTime:" $toad_branchesTime

# create table toad_tellers(tid int,bid int,tbalance int,filler varchar);
tableName=$toad_tellers
a=1
while ((a <= $sntellers))
do
  tid=$a
  bid=$(($tid / $ntellers + 1))

  insertSql="insert into $tableName values($tid,$bid,0,'init');"
  sql_command="${toad_executor_command} \"${insertSql}\" "
  sh -c "${sql_command}"

  echo "insert into $tableName tid:$tid" 
  let a++        
done

toad_tellersTime=$(date "+%Y-%m-%d %H:%M:%S")
echo "load toad_tellers endTime:" $toad_tellersTime

# create table toad_accounts(aid int,bid int,abalance int,filler varchar);
tableName=$toad_accounts
a=1
while ((a <= $snaccounts))
do
  aid=$a
  bid=$(($aid / $naccounts + 1))

  # insertSql="insert into $tableName values($aid,$bid,0,'init');"
  insertSql="insert into $tableName values($aid,$bid,0,'i')"
  let a++ 

  for number in {1..99}; do   
    aid=$a
    bid=$(($aid / $naccounts + 1))

    values="($aid,$bid,0,'i')"
    insertSql="${insertSql},${values}" 

    let a++  
  done

  # echo ${insertSql}
  
  sql_command="${toad_executor_command} \"${insertSql};\" "
  sh -c "${sql_command}" 
  echo "insert into $tableName aid:$aid"      
done

toad_accountsTime=$(date "+%Y-%m-%d %H:%M:%S")
echo "load toad_accounts endTime:" $toad_accountsTime 

##########################################################
# end loading 
###########################################################

use_totalTime=`echo $(($(date +%s -d "${toad_accountsTime}") - $(date +%s -d "${beginTime}"))) | awk '{t=split("60 s 60 m 24 h 999 d",a);for(n=1;n<t;n+=2){if($1==0)break;s=$1%a[n]a[n+1]s;$1=int($1/a[n])}print s}'`

use_createTime=`echo $(($(date +%s -d "${createTime}") - $(date +%s -d "${beginTime}"))) | awk '{t=split("60 s 60 m 24 h 999 d",a);for(n=1;n<t;n+=2){if($1==0)break;s=$1%a[n]a[n+1]s;$1=int($1/a[n])}print s}'`

use_dataTime=`echo $(($(date +%s -d "${toad_accountsTime}") - $(date +%s -d "${createTime}"))) | awk '{t=split("60 s 60 m 24 h 999 d",a);for(n=1;n<t;n+=2){if($1==0)break;s=$1%a[n]a[n+1]s;$1=int($1/a[n])}print s}'`

use_bdataTime=`echo $(($(date +%s -d "${toad_branchesTime}") - $(date +%s -d "${createTime}"))) | awk '{t=split("60 s 60 m 24 h 999 d",a);for(n=1;n<t;n+=2){if($1==0)break;s=$1%a[n]a[n+1]s;$1=int($1/a[n])}print s}'`

use_tdataTime=`echo $(($(date +%s -d "${toad_tellersTime}") - $(date +%s -d "${toad_branchesTime}"))) | awk '{t=split("60 s 60 m 24 h 999 d",a);for(n=1;n<t;n+=2){if($1==0)break;s=$1%a[n]a[n+1]s;$1=int($1/a[n])}print s}'`

use_adataTime=`echo $(($(date +%s -d "${toad_accountsTime}") - $(date +%s -d "${toad_tellersTime}"))) | awk '{t=split("60 s 60 m 24 h 999 d",a);for(n=1;n<t;n+=2){if($1==0)break;s=$1%a[n]a[n+1]s;$1=int($1/a[n])}print s}'`

echo "begin time:"${beginTime}
echo "end time  :"${toad_accountsTime}

echo "---------------------------------------------------------"
echo "load data finish."
echo "total elapse time: "${use_totalTime}
echo "create table elapse time: "${use_createTime}

echo "total of load table data elapse time: "${use_dataTime}

echo "total of load toad_branches data elapse time: "${use_bdataTime}
echo "total of load toad_tellers data elapse time: "${use_tdataTime}
echo "total of load toad_accounts data elapse time: "${use_adataTime}