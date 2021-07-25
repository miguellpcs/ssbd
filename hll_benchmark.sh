#!/bin/bash
echo "id,target,epsilon,delta,tempo_inicio,tempo_fim,prediction" > hll_trace.csv

while IFS=, read -r id t e d0;
do
    d=`echo $d0 | sed 's/\\r//g'`
    echo "$id, $t, $e, $d"
    t1=`(date)`
    mem_pred=`(./bin/hll --target $t --eps $e --delta $d "network_flows.csv")`
    t2=`(date)`
    echo "$id,$t,$e,$d,$t1,$t2,$mem_pred" >> hll_trace.csv
    sleep 1

done < hll_executions.csv
