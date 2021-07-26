#!/usr/bin/env ipython

import random
import csv

EPS=[0.4, 0.3, 0.2, 0.1]
DELTA=[0.1, 0.05, 0.01]
TARGET= [0, 1, 7, 8] # range(0,9)
ID = range(0,20)

executions = [(i,t,e,d)
             for i in ID
             for t in TARGET
             for e in EPS
             for d in DELTA]

random.shuffle(executions)

with open('kmv_executions.csv','w') as out:
    csv_out=csv.writer(out)
    for row in executions:
        csv_out.writerow(row)
