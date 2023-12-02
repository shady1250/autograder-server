#!/bin/bash

numClients=$1
loopNum=$2
sleepTime=$3

pids=()

gcc -w -o client gradingclient.c -lpthread
command="./client 10.130.154.72 5001 cfile.c $loopNum $sleepTime 5"


mkdir -p load_stats

touch load_stats/cpu_stat.txt
> load_stats/cpu_stat.txt

./client1 10.130.154.72 8090 &
wait



for (( i=1 ; i<=$numClients ; i++ ));
do
   $command > load_stats/out$i.txt & 
done

wait
echo "Program ran"
./client2 10.130.154.72 8091 &
wait

for ((i=2; i<=$numClients ; i++ ));
do
    cat load_stats/out$i.txt >> load_stats/out1.txt &
    wait
done
 
echo $numClients >> load_stats/out1.txt


avg_threads=$(awk '{ print $2 }' load_stats/cpu_stat.txt)

avg_cpu_util=$(awk '{ print $1 }' load_stats/cpu_stat.txt)


echo $numClients $avg_threads >> stats/active_threads.txt
echo $numClients $avg_cpu_util >> stats/cpu_utilization.txt
awk 'BEGIN{FS=":"; } { if($1 ~ /throughput/ ) sum+=$2; clients=$1; } END{ print clients,sum; }' load_stats/out1.txt >> stats/throughput.txt

awk 'BEGIN{FS=":"} { if($1 ~ /Total time/ ) sum1+=$2; if($1 ~ /Successfull responses/ ) sum2+=$2; clients=$1; } END{ ans=sum1/sum2; print clients,ans; } ' load_stats/out1.txt >> stats/response.txt

awk 'BEGIN{FS=":"; } { if($1 ~ /Goodput/ ) sum+=$2; clients=$1; } END{ print clients,sum; }' load_stats/out1.txt >> stats/goodput.txt

awk 'BEGIN{FS=":"; } { if($1 ~ /Error/ ) sum+=$2; if($1 ~ /Requests/ ) sum2+=$2; clients=$1; } END{ ans=sum/sum2; print clients,ans; }' load_stats/out1.txt >> stats/error_rate.txt

awk 'BEGIN{FS=":"; } { if($1 ~ /Timeout/ ) sum+=$2; if($1 ~ /Requests/ ) sum2+=$2; clients=$1; } END{ ans=sum/sum2; print clients,ans; }' load_stats/out1.txt >> stats/timeout.txt

awk 'BEGIN{FS=":"; } { if($1 ~ /request rate/ ) sum+=$2;  clients=$1; } END{ print clients,sum; }' load_stats/out1.txt >> stats/requestRate.txt

rm load_stats/out[0-9]*.txt

