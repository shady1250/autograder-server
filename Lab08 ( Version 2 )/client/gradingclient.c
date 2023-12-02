#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<fcntl.h>
#include <netdb.h>
#include<assert.h>
#include<sys/time.h>
#include<errno.h>
double GetTime() {
    struct timeval t;
    int rc = gettimeofday(&t, NULL);
    assert(rc == 0);
    return (double) t.tv_sec + (double) t.tv_usec/1e6;
}
void error(char *msg) {
  perror(msg);
  exit(0);
}

int main(int argc, char *argv[]) {
  int sockfd, portno, n;
  int success=0;
  double sum=0;
  struct sockaddr_in serv_addr; 
  struct hostent *server; 

  char buffer[40960]; 

  if (argc < 7) {
    fprintf(stderr, "usage %s hostname port <sourceCodeFileTobeGraded>  <loopNum> <sleepTimeSeconds> <TimeOutSeconds>\n", argv[0]);
    exit(0);
  }
  
  portno = atoi(argv[2]); 
  
  int loopNum=atoi(argv[4]);
  int sleepTime=atoi(argv[5]);
  int time_out=atoi(argv[6]);
  struct timeval timeout;
  timeout.tv_sec = time_out;   
  timeout.tv_usec = 0;
  int Error=0;
  int timeout_err=0;

int req=loopNum;
double start_loop=GetTime();

while(loopNum--){  
  sockfd = socket(AF_INET, SOCK_STREAM, 0); 

  if (sockfd < 0){
    perror("ERROR opening socket");
    continue;
  }
  
  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        perror("ERROR setting socket options");
        close(sockfd);
        continue;
    }

  server = gethostbyname(argv[1]);

  if (server == NULL) {
    fprintf(stderr, "ERROR, no such host\n");
    exit(0);
  }
  

  bzero((char *)&serv_addr, sizeof(serv_addr)); 

  serv_addr.sin_family = AF_INET; 

  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);

  serv_addr.sin_port = htons(portno);

  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
  	if (errno == EWOULDBLOCK || errno == EAGAIN) {
        	perror("Connection timeout occurred");
        	timeout_err++;
        	Error++;
        	close(sockfd);
        	continue;
    	} else {
        	perror("ERROR connecting");
        	Error++;
        	close(sockfd);
        	continue;
    	}
  }

  bzero(buffer, 40960); 

  char* input=argv[3];                        
  int fd=open(input,O_RDONLY,S_IRWXU);        
  long size=lseek(fd,0,SEEK_END);           
  lseek(fd,0,SEEK_SET);
  char input_file[40960];
  read(fd,input_file,size);             
  double Tsend=GetTime();
  n = write(sockfd, input_file, strlen(input_file)); 

  if (n < 0){
    if (errno == EWOULDBLOCK || errno == EAGAIN) {
        	perror("Sending timeout occurred");
        	timeout_err++;
        	Error++;
        	close(sockfd);
        	continue;
    	} else {
        	perror("ERROR Sending");
        	Error++;
        	close(sockfd);
        	continue;
    	}
  }
  bzero(buffer, 40960);


  n = read(sockfd, buffer, 40960);   
  double Trecv=GetTime();
  if (n < 0){
    if (errno == EWOULDBLOCK || errno == EAGAIN) {
        	perror("Receive timeout occurred");
        	timeout_err++;
        	Error++;
        	close(sockfd);
        	continue;
    	} else {
        	perror("ERROR Receiving");
        	Error++;
        	close(sockfd);
        	continue;
    	}
  }
  printf("Server response: %s\n", buffer);

  double tot_time=Trecv-Tsend;
  sum+=tot_time;
  success++;
  close(sockfd);
  sleep(sleepTime);
}
  double end_loop=GetTime();
  double loop_time=end_loop-start_loop;
  double avg_res_time=sum/success;
  double throughput=(success+timeout_err)/loop_time;
  double goodput=success/loop_time;
  printf("Error:%d\n",Error);
  printf("Goodput:%lf\n",goodput);
  printf("Timeout:%d\n",timeout_err);
  printf("request rate:%lf\n",req/loop_time);
  printf("Successfull responses:%d\n",success);
  printf("Requests:%d\n",req);
  printf("Total time:%lf\n",sum);
  printf("Loop time:%lf\n",loop_time);
  printf("Avg response time:%lf\n",avg_res_time);
  printf("throughput:%lf\n",throughput);
  return 0;
}
