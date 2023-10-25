#!/bin/bash
echo "Generating graph."

clients='2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24'
touch response.txt
> response.txt
touch throughput.txt
> throughput.txt

for i in ${clients};
do
  ./loadtest.sh $i 3 0 &
  wait
done

cat response.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Avg. Response time vs Clients" -X "Number of clients" -Y "Avg response time(in s)"  -r 0.25> ./response.png
cat throughput.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Throughput vs Clients" -X "Number of clients" -Y "Throughput(Requests/s)"  -r 0.25> ./throughput.png
