#!/bin/bash

numClients=$1
loopNum=$2
sleepTime=$3

mkdir -p load_stats

gcc -w -o client gradingclient.c
gcc -w -o client1 signal_client.c
command="./client 127.0.0.1 5000 cfile.c $loopNum $sleepTime"
for (( i=1 ; i<=$numClients ; i++ ));
do
   $command > load_stats/out$i.txt & 
done
wait

for ((i=2; i<=$numClients ; i++ ));
do
    cat load_stats/out$i.txt >> load_stats/out1.txt &
    wait
done

echo $numClients >> load_stats/out1.txt
awk 'BEGIN{FS=":"; } { if($1 ~ /Throughput/ ) sum+=$2; clients=$1; } END{ print clients,sum; }' load_stats/out1.txt >> stats/throughput.txt

awk 'BEGIN{FS=":"} { if($1 ~ /Total time/ ) sum1+=$2; if($1 ~ /Successfull responses/ ) sum2+=$2; clients=$1; } END{ ans=sum1/sum2; print clients,ans; } ' load_stats/out1.txt >> stats/response.txt

./client1 127.0.0.1 8080 &
wait
sleep 3
rm load_stats/out[0-9]*.txt


