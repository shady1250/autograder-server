Design of asynchronous server: 

Client: 

When we want to send a file to the client, we send 0 as one of the arguments to the server with the filename that has to be evaluated. When we want to check status, we send 1 as one as one of  the arguments along with the request ID that needs to be checked. Based on the response of the server as (0,1,2,3) the client also reacts accordingly which will be be discussed below 

 

The design: 

There are three queues in total. The first queue is called “QUEUE” which is of size pool size. The second queue is called “FILEQUEUE” which will apped all the incoming request from the client and when the “QUEUE” is free, the data is moved from “FILEQUEUE” to “QUEUE”. The third queue is the “STATUSQUEUE” which will have all the processed requests. 

The items in the queues are uniquely identified by the requestid which are a combination of timestamp and random number. 

Server: 

Server accepts port and pool size, and pool size would be assigned as the size of the queue. The number of threads created will also be equal to the pool size. 

In the start_worker function, threads are created of pool size and whenever an entry is made in QUEUE, it gets processed and saved in the the STATUSQUEUE 

Grader function make sures that items from the filequeue are shifted to the queuem so that it can be processed and pushed inside the status queue. Filequeue and queue mutually wait for each other, using conditional wait. Whenever an item is moved from filequeue to the queue, a conditional signal is give to the start_worker threads, similarly, when space is available on the queue, its conditionally signaled to the grader threads.   

Now based on the request from the client, handle_new (0) and handle_status (1) are called respectivly. When a new request comes from the client, its inserted inside the filequeue and conditionally signals, the grader threads to start pushing the item inside the queue. 

When a status check is requested, it first checks the queue and he filequeue, and we can find the position in the queue, if the item is available. If its not in both the queues, then the grading server. 

 The server sends 0, when it wants to send the position in the queue.
 The server sends 1, whem no request are found.
 The server sends 2, when it wants to send the output file back.
 The server sends 3, when the file is still processed at the server.