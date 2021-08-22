#!/usr/bin/env ipython

import random
import csv

EPS=[0.2, 0.1, 0.05, 0.01]
UNIV=[10000, 100000, 1000000]
TARGET= [0, 3, 4, 5, 6, 7] 
QUANT=[0.0, 0.25, 0.50, 0.75, 1.0]
ID = range(0,10)

executions = [(i,t,e,u,q)
             for i in ID
             for t in TARGET
             for e in EPS
             for u in UNIV
             for q in QUANT]

random.shuffle(executions)

with open('executions_quant.csv','w') as out:
    csv_out=csv.writer(out)
    for row in executions:
        csv_out.writerow(row)

