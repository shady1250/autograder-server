#!/bin/bash

mkdir -p server_stats
touch server_stats/thread_stat.txt
>server_stats/thread_stat.txt

touch server_stats/cpu_stat.txt
> server_stats/cpu_stat.txt

monitor_thread_count() {
  while true; do
    sleep 1
    thread_count=$(ps -eLf | grep "./server 5001" | grep -v "grep" | wc -l)
    echo "$thread_count" >> server_stats/thread_stat.txt
  done
}

monitor_cpu_utilization() {
  sleep 1
  vmstat 1 > server_stats/cpu_stat.txt
}


monitor_thread_count &
thread_pid=$!

monitor_cpu_utilization &
cpu_pid=$!

echo $thread_pid $cpu_pid > server_stats/pids.txt

wait
