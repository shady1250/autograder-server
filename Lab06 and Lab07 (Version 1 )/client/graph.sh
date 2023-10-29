#!/bin/bash
echo "Generating graph."
mkdir -p stats
mkdir -p Images

#clients='2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24'
clients='2 10 20 40 50 100 150 200 250 300 350 400 450 500 550 600 650 700 750'
touch stats/response.txt
> stats/response.txt
touch stats/throughput.txt
> stats/throughput.txt

for i in ${clients};
do
  ./loadtest.sh $i 3 0 &
  wait
done

cat stats/response.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Avg. Response time vs Clients" -X "Number of clients" -Y "Avg response time(in s)"  -r 0.25> Images/response.png
cat stats/throughput.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Throughput vs Clients" -X "Number of clients" -Y "Throughput(Requests/s)"  -r 0.25> Images/throughput.png
