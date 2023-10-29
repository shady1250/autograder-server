#!/bin/bash
pid1=$(awk '{ print $1 }' server_stats/pids.txt)
killall vmstat
kill "$pid1"


avg_cpu=$(awk '{ if ($15 ~ /^[0-9]+$/) { sum+=1; cpu_usage+=100-$15; } } END{ ans=cpu_usage/sum; print ans } ' server_stats/cpu_stat.txt )
avg_thread=$(awk '{ sum+=$1; i+=1; } END{ ans=sum/i; print ans }' server_stats/thread_stat.txt)
echo $avg_cpu $avg_thread
echo $avg_cpu $avg_thread > server_stats/to_send_file.txt 
#echo $avg_cpu $avg_thread > server_stats/to_send_file.txt 

sleep 10
rm Server_Stats/test*
rm Server_Stats/diff*.txt
rm Server_Stats/Compile_Out*.txt
rm Server_Stats/Run_Err*.txt
rm Server_Stats/Run_Out*.txt
rm test*
echo "removed"
