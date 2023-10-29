/* run client using: ./client localhost <server_port> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<fcntl.h>
#include <netdb.h>
#include<assert.h>
#include<sys/time.h>
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
  struct sockaddr_in serv_addr; //Socket address structure
  struct hostent *server; //return type of gethostbyname

  char buffer[40960]; //buffer for message

  if (argc < 6) {
    fprintf(stderr, "usage %s hostname port <sourceCodeFileTobeGraded>  <loopNum> <sleepTimeSeconds>\n", argv[0]);
    exit(0);
  }
  
  portno = atoi(argv[2]); // 2nd argument of the command is port number

  /* create socket, get sockfd handle */
  
  int loopNum=atoi(argv[4]);
  int sleepTime=atoi(argv[5]);
double start_loop=GetTime();
while(loopNum--){  
  sockfd = socket(AF_INET, SOCK_STREAM, 0); //create the half socket. 
  //AF_INET means Address Family of INTERNET. SOCK_STREAM creates TCP socket (as opposed to UDP socket)


  if (sockfd < 0)
    error("ERROR opening socket");

  /* fill in server address in sockaddr_in datastructure */

  server = gethostbyname(argv[1]);
  //finds the IP address of a hostname. 
  //Address is returned in the 'h_addr' field of the hostend struct

  if (server == NULL) {
    fprintf(stderr, "ERROR, no such host\n");
    exit(0);
  }

  bzero((char *)&serv_addr, sizeof(serv_addr)); // set server address bytes to zero

  serv_addr.sin_family = AF_INET; // Address Family is IP

  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
/*Copy server IP address being held in h_addr field of server variable
to sin_addr.s_addr of serv_addr structure */

//convert host order port number to network order
  serv_addr.sin_port = htons(portno);

  /* connect to server 
  First argument is the half-socket, second is the server address structure
  which includes IP address and port number, third is size of 2nd argument
  */

  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR connecting");

  //If here means connection was complete

  /* ask user for input */

 // printf("Please enter the message: ");
  bzero(buffer, 40960); //reset buffer to zero
 // fgets(buffer, 255, stdin); //read message from stdin, into buffer

  /* send user message to server 
  write call: first argument is socket FD, 2nd is the string to write, 3rd is length of 2nd
  */
  char* input=argv[3];                        //input= given c file name
  int fd=open(input,O_RDONLY,S_IRWXU);        
  long size=lseek(fd,0,SEEK_END);             //checking the size of the input c file
  lseek(fd,0,SEEK_SET);
  char input_file[40960];
  read(fd,input_file,size);                   //copying the content of the C file to input_file buffer
  double Tsend=GetTime();
  n = write(sockfd, input_file, strlen(input_file));    //sending the input_file buffer to the server

  if (n < 0){
    perror("ERROR writing to socket");
    continue;
  }
  bzero(buffer, 40960);

  /* read reply from server 
  First argument is socket, 2nd is string to read into, 3rd is number of bytes to read
  */

  n = read(sockfd, buffer, 40960);        //received the result from the server
  double Trecv=GetTime();
  if (n < 0){
    perror("ERROR reading from socket");
    continue;
  }
  printf("Server response: %s\n", buffer);

  double tot_time=Trecv-Tsend;
//  printf("Response Time: %lf\n",tot_time);
  sum+=tot_time;
  success++;
  close(sockfd);
  sleep(sleepTime);
}
  double end_loop=GetTime();
  double loop_time=end_loop-start_loop;
  double avg_res_time=sum/success;
  double throughput=success/sum;
 
  printf("Successfull responses:%d\n",success);
  printf("Total time:%lf\n",sum);
  printf("Loop time:%lf\n",loop_time);
  printf("Avg response time:%lf\n",avg_res_time);
  printf("throughput:%lf\n",throughput);
  return 0;
}
