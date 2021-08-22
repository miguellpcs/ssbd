#!/bin/bash

echo "id,val,epsilon,univ,query,time_update,time_query,mem_usage,comp_count,comp_mem_avg,value" > baseline_quant_trace.csv
#echo "id,val,epsilon,univ,query,time_update,time_query,mem_usage,comp_count,comp_mem_avg,value,rank" > gk_quant_trace.csv

baseline_quant_func(){
    analysis=$( (./bin/baseline --val "$2" --eps "$3" --univ "$4" "data/network_flows.csv" --rank "$5" --verbose))
    echo "$1,$2,$3,$4,$5,$analysis" >> baseline_quant_trace.csv
}

gk_quant_func(){
    analysis=$( (./bin/gk --val "$2" --eps "$3" --univ "$4" "data/network_flows.csv" --quant "$5" --verbose))
    echo "$1,$2,$3,$4,$5,$analysis" >> gk_quant_trace.csv
}

#while IFS=, read -r id t e u d0;
#do
#    d=`echo $d0 | sed 's/\\r//g'`
#    gk_quant_func "$id" "$t" "$e" "$u" $d $
#done < data/executions_quant.csv

#wait
while IFS=, read -r id t e u d t1 t2 m cc ca p r0;
do
    r=`echo $r0 | sed 's/\\r//g'`
    baseline_quant_func "$id" "$t" "$e" "$u" $r &
done < gk_quant_trace.csv