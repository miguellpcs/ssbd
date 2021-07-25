#!/bin/bash
TARGET=(0 1 2 3 4 5 6 7 8)
echo "id,target,epsilon,delta,tempo_inicio,tempo_fim,memoria,prediction" > kmv_baseline.csv

for t in "${TARGET[@]}";
do
    echo $t
    t1=`(date)`
    mem_pred=`(./bin/kmv --target $t --eps 0 --delta 0 "network_flows.csv")`
    t2=`(date)`
    echo "$t,$t,0,0,$t1,$t2,$mem_pred" >> kmv_baseline.csv
    sleep 1
done
