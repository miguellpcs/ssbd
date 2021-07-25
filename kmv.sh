#!/bin/bash
rm results.txt
touch results.txt
echo "Estimate,Total,Error" >> results.txt
mem_pred
for i in {1..100000}
do
    ./bin/kmv --target 7 --eps 0.05 --delta 0.01 network_flows.csv >> results.txt
done