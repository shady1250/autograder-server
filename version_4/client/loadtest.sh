if [ $# -ne 4 ]; then
    echo "Usage <number of clients> <ip> <port> <pool-time>"
    exit
fi

ip=$2
port=$3
pool=$4
g++ -o submit send_recv_utility.cpp submit.cpp &
wait

mkdir -p outputs
#rm -f outputs/*

for (( i=1 ; i<=$1 ; i++ )); 
do
    bash client.sh sampleFilesForTesting/program_runs.cpp $pool ${i} $ip $port &
done
wait

