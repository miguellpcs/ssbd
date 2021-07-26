#!/bin/bash
echo "id,target,epsilon,delta,tempo_inicio,tempo_fim,prediction" > kmv_trace.csv

while IFS=, read -r id t e d0;
do
    d=`echo $d0 | sed 's/\\r//g'`
    t1=`(date)`
    mem_pred=`(./bin/kmv --target $t --eps $e --delta $d "network_flows.csv")`
    t2=`(date)`
    echo "$id,$t,$e,$d,$t1,$t2,$mem_pred" >> kmv_trace.csv
    sleep 1

done < kmv_executions.csv
