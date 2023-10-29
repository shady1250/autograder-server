/* run using ./server <port> */

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

pthread_mutex_t lock;


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


void *start_function(void *sockfd) {
    int newsockfd = *(int *)sockfd;
    pthread_mutex_unlock(&lock);
    
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
    pthread_exit(NULL);
}



int main(int argc, char *argv[]) {
  int sockfd,newsockfd, portno;

  socklen_t clilen; 
  char buffer[40960]; 
  struct sockaddr_in serv_addr, cli_addr; 
  int n;

  if (argc < 2) {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }
  
  system("mkdir -p Server_Stats");

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

pthread_mutex_init(&lock, NULL);
while (1){
    pthread_mutex_lock(&lock);   
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
   
    if (newsockfd < 0){
      perror("ERROR on accept"); 
      pthread_mutex_unlock(&lock);
      continue;
    }

     pthread_t thread;
     if (pthread_create(&thread, NULL, start_function, &newsockfd) != 0){
       perror("Failed to create Thread");
       close(newsockfd);
       pthread_mutex_unlock(&lock);
     }
  //   sleep(1);
    }
    close(sockfd);
    return 0;
  }

