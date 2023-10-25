#!/bin/bash

numClients=$1
loopNum=$2
sleepTime=$3

gcc -w -o client gradingclient.c
command="./client 127.0.0.1 5000 cfile.c $loopNum $sleepTime"
for (( i=1 ; i<=$numClients ; i++ ));
do
   $command > out$i.txt & 
done
wait

for ((i=2; i<=$numClients ; i++ ));
do
    cat out$i.txt >> out1.txt &
    wait
done
 
echo $numClients >> out1.txt
awk 'BEGIN{FS=":"; } { if($1 ~ /Throughput/ ) sum+=$2; clients=$1; } END{ print clients,sum; }' out1.txt >> throughput.txt

awk 'BEGIN{FS=":"} { if($1 ~ /Total time/ ) sum1+=$2; if($1 ~ /Successfull responses/ ) sum2+=$2; clients=$1; } END{ ans=sum1/sum2; print clients,ans; } ' out1.txt >> response.txt

rm out[0-9]*.txt

