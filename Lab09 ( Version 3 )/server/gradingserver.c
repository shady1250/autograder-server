

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<fcntl.h>
#include<sys/wait.h>
#include <netinet/in.h>
#include<limits.h>
#include <pthread.h>


void error(char *msg) {
  perror(msg);
  exit(1);
}

int front=-1;
int rear=-1;
int max_pool_size;
int *queue;

int isFull() {
    return (front == (rear + 1) % max_pool_size);
}

// Function to check if the circular queue is empty
int isEmpty() {
    return (front == -1);
}

// Function to add an element to the circular queue (push)
void push(int sockfd) {
    if (isFull()) {
        printf("Queue is full. Cannot push.\n");
        return;
    }
    if (front == -1) {
        front = 0;
    }
    rear = (rear + 1) % max_pool_size;
    queue[rear] = sockfd;
}

// Function to remove an element from the circular queue (pop)
int pop() {
    if (isEmpty()) {
        printf("Queue is empty. Cannot pop.\n");
        return -1; // or return an error code
    }
    int item = queue[front];
    if (front == rear) {
        front = rear = -1;
    } else {
        front = (front + 1) % max_pool_size;
    }
    return item;
}

pthread_cond_t space_available=PTHREAD_COND_INITIALIZER, req_available=PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;


char* fun(char fname[], char msg[]){
	int fd=open(fname,O_RDONLY,S_IRWXU);
	long size=lseek(fd,0,SEEK_END);
	char ch[size+1];
	bzero(ch,size+1);
	int msg_len=strlen(msg);
	char str[size+msg_len+2];
	bzero(str,size+msg_len+2);
	lseek(fd,0,SEEK_SET);
	read(fd,ch,size);
	close(fd);
	strcpy(str,msg);
	strcat(str,ch);
	char* ans=str;
	return ans;
}

char* check(char buffer[],int val){
	//char* filename=buffer;
	
	char compile_command[100];
	snprintf(compile_command, sizeof(compile_command), "gcc -o test%d %s 2> Server_Stats/Compile_Out%d.txt", val,buffer,val);

	int compilerror=system(compile_command);
	
	if(compilerror){
		char fname[100];
		snprintf(fname,100,"Server_Stats/Compile_Out%d.txt",val);
		char *ans=fun(fname,"Compile Error\n");
		return ans;
	}
	
	char run_command[100];
	snprintf(run_command,100,"./test%d > Server_Stats/Run_Out%d.txt 2> Server_Stats/Run_Err%d.txt",val,val,val);
	int runerror=system(run_command);
	
	if(runerror){
		char fname[100];
		snprintf(fname,100,"Server_Stats/Run_Err%d.txt",val);
		char* ans=fun(fname,"Runtime Error\n");
		return ans;
	}
	
	
	char diff_command[100];
	snprintf(diff_command,100,"diff output.txt Server_Stats/Run_Out%d.txt > Server_Stats/diff%d.txt",val,val);
	
	int differror=system(diff_command);
	
	if(differror){
		char fname[100];
		snprintf(fname,100,"Server_Stats/diff%d.txt",val);
		char* ans=fun(fname,"Output Error\n");
		return ans;
	}
	char* ans="Pass";
	return ans;
}


void *start_function() {

  while(1){
    int newsockfd;
    pthread_mutex_lock(&mutex);
    while(isEmpty()){
    	pthread_cond_wait(&req_available,&mutex);
    }
    newsockfd=pop();
    printf("Got fd:%d\n",newsockfd);
    pthread_cond_signal(&space_available);
    
    pthread_mutex_unlock(&mutex);
    
    char buffer[40960];
    bzero(buffer, 40960); 

    int n = read(newsockfd, buffer, 40960);
    if (n < 0){
      perror("ERROR reading from socket");
      close(newsockfd);
      pthread_exit(NULL);
    }
      
    char fname[100];
    snprintf(fname,100,"Server_Stats/test-file%d.c",newsockfd);
  
    int fd=open(fname,O_CREAT|O_RDONLY|O_WRONLY|O_TRUNC,S_IRWXU);
    if(fd<0){
    	perror("Error creating the file");
    	close(newsockfd);
    	close(fd);
    	pthread_exit(NULL);
    }
    write(fd,buffer,n);
    close(fd);


 
//    printf("Here is the message: %s", buffer);
    char* result=check(fname,newsockfd);    
//    printf("%s\n",result);
   
    n = write(newsockfd, result, strlen(result));         
    if (n < 0){
         perror("ERROR writing to socket");
    }
    close(newsockfd);
 //   pthread_cancel(pthread_self());
  //  pthread_exit(NULL);
  }
}



int main(int argc, char *argv[]) {
  int sockfd,newsockfd, portno;
  
  socklen_t clilen; 
  char buffer[40960]; 
  struct sockaddr_in serv_addr, cli_addr; 
  int n;

  if (argc < 3) {
    printf("usage %s <port no> <max_pool_size>\n",argv[0]);
    exit(1);
  }
  
  system("mkdir -p Server_Stats"); 
  
  max_pool_size=atoi(argv[2]);
  
  queue=(int *)malloc(sizeof(int)*max_pool_size);
  pthread_t consumer_thread[max_pool_size];
  
  for(int i=0;i<max_pool_size;i++){
  	pthread_create(consumer_thread+i,NULL,start_function,NULL);
  }


  sockfd = socket(AF_INET, SOCK_STREAM, 0); 

  if (sockfd < 0)
    error("ERROR opening socket");

 
  bzero((char *)&serv_addr, sizeof(serv_addr));
  
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;  

  portno = atoi(argv[1]);
  serv_addr.sin_port = htons(portno);


  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");

 
  listen(sockfd, 900); 


  clilen = sizeof(cli_addr); 


while (1){ 
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
     if (newsockfd < 0){
      perror("ERROR on accept"); 
      continue;
    }
    
    pthread_mutex_lock(&mutex);
    while(isFull()){
    	pthread_cond_wait(&space_available,&mutex);
    }
    push(newsockfd);
    pthread_cond_signal(&req_available);
    pthread_mutex_unlock(&mutex);
    
  //   sleep(1);
    }
    close(sockfd);
    return 0;
  }

