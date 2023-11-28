#!/bin/bash
echo "Generating graph."

#clients='2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30'
clients='2 10 20 40 50 100 150 200 250 300 350 400 450 500 550 600 650 700 750'
#clients='2 3 4 5 6 7 8'
mkdir -p stats
mkdir -p images
touch stats/response.txt
> stats/response.txt
touch stats/throughput.txt
> stats/throughput.txt
touch stats/goodput.txt
>stats/goodput.txt
touch stats/error_rate.txt
>stats/error_rate.txt
touch stats/timeout.txt
>stats/timeout.txt
touch stats/requestRate.txt
>stats/requestRate.txt
touch stats/active_threads.txt
>stats/active_threads.txt
touch stats/cpu_utilization.txt
>stats/cpu_utilization.txt

for i in ${clients};
do
  ./loadtest.sh $i 3 1 &
  wait
done


cat stats/response.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Avg. Response time vs Clients" -X "Number of clients" -Y "Avg response time(in s)"  -r 0.25> images/response.png
cat stats/throughput.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Throughput vs Clients" -X "Number of clients" -Y "Throughput(Requests/s)"  -r 0.25> images/throughput.png
cat stats/goodput.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Goodput vs Clients" -X "Number of clients" -Y "Goodput"  -r 0.25> images/goodput.png
cat stats/error_rate.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Error rate vs Clients" -X "Number of clients" -Y "Error rate"  -r 0.25> images/Error_rate.png
cat stats/timeout.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Timeout rate vs Clients" -X "Number of clients" -Y "Timeout rate"  -r 0.25> images/Timeout_rate.png
cat stats/requestRate.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Request rate vs Clients" -X "Number of clients" -Y "Request rate"  -r 0.25> images/Request_rate.png
cat stats/active_threads.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Avg Active threads vs Clients" -X "Number of clients" -Y "Avg. threads"  -r 0.25> images/avg_threads.png
cat stats/cpu_utilization.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Avg CPU utilization vs Clients" -X "Number of clients" -Y "CPU utilization(%)"  -r 0.25> images/cpu_util.png
