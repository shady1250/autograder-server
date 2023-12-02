#!/bin/bash

if [ $# -ne 5 ]; then
    echo "Usage <filename> <pooling-time> <output-file-no> <ip> <port>"
    exit
fi

mkdir -p received_Response

SERVER_IP=$4
SERVER_PORT=$5
SUBMISSION_FILE=$1
POOL_INTERVAL_SECONDS=$2 


send_submission_request() {
    ./submit $SERVER_IP $SERVER_PORT 0 $SUBMISSION_FILE
}




#start_time=$(date +%s)



send_submission_request  > received_Response/out$3.txt  &
wait
echo "sent submission request $3"

request_id=$(awk 'BEGIN{FS=":"}{ans=$2;}END{print ans;}' received_Response/out$3.txt)

send_check_status_request() {
    ./submit $SERVER_IP $SERVER_PORT 1 $request_id
}

while true; do
   
    sleep $POOL_INTERVAL_SECONDS
    current_time=$(date +%s)
    elapsed_time=$((current_time - start_time))

    check_status_output=$(send_check_status_request| tr -d '\0')
    if [[ $check_status_output == *"DONE"* ]]; then
        echo $check_status_output
        exit 0
    fi

    echo "Current Status: $check_status_output"

  
done

total_time=$((current_time - start_time))
echo "Total time taken: $total_time seconds"

