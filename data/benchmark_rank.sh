#!/bin/bash
#

echo "id,val,epsilon,univ,query,time_update,time_query,mem_usage,comp_count,comp_mem_avg,value,weight" > baseline_rank_trace_2.csv
#echo "id,val,epsilon,univ,query,time_update,time_query,mem_usage,comp_count,comp_mem_avg,value" > gk_rank_trace.csv
#echo "id,val,epsilon,univ,query,time_update,time_query,mem_usage,comp_count,comp_mem_avg,value" > qdig_update_rank_trace.csv
#echo "id,val,epsilon,univ,query,time_update,time_query,mem_usage,comp_count,comp_mem_avg,value" > qdig_weight_rank_trace.csv
#echo "id,val,epsilon,univ,query,time_update,time_query,mem_usage,comp_count,comp_mem_avg,value" > qdig_capacity_rank_trace.csv

baseline_rank_func(){
    analysis=$( (./bin/baseline --val "$2" --eps "$3" --univ "$4" "data/network_flows.csv" --rank "$5" --verbose))
    echo "$1,$2,$3,$4,$5,$analysis" >> baseline_rank_trace_2.csv
}

gk_rank_func(){
    analysis=$( (./bin/gk --val "$2" --eps "$3" --univ "$4" "data/network_flows.csv" --rank "$5" --verbose))
    echo "$1,$2,$3,$4,$5,$analysis" >> gk_rank_trace.csv
}

qdig_update_rank_func(){
    analysis=$( (./bin/qdig --val "$2" --eps "$3" --univ "$4" "data/network_flows.csv" --rank "$5" --strategy update --verbose))
    echo "$1,$2,$3,$4,$5,$analysis" >> qdig_update_rank_trace.csv
}

qdig_weight_rank_func(){
    analysis=$( (./bin/qdig --val "$2" --eps "$3" --univ "$4" "data/network_flows.csv" --rank "$5" --strategy weight --verbose))
    echo "$1,$2,$3,$4,$5,$analysis" >> qdig_weight_rank_trace.csv
}

qdig_capacity_rank_func(){
    analysis=$( (./bin/qdig --val "$2" --eps "$3" --univ "$4" "data/network_flows.csv" --rank "$5" --strategy capacity --verbose))
    echo "$1,$2,$3,$4,$5,$analysis" >> qdig_capacity_rank_trace.csv
}

while IFS=, read -r id t e u d0;
do
    d=`echo $d0 | sed 's/\\r//g'`
    univ_min=$(( $u - 1))
    univ_div=$(( univ_min / d ))
    baseline_rank_func "$id" "$t" "$e" "$u" $univ_div &
#    gk_rank_func "$id" "$t" "$e" "$u" $univ_div &
#    qdig_update_rank_func "$id" "$t" "$e" "$u" $univ_div &
#    qdig_capacity_rank_func "$id" "$t" "$e" "$u" $univ_div &
#    qdig_weight_rank_func "$id" "$t" "$e" "$u" $univ_div &
done < data/executions.csv