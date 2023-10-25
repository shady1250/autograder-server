/* run using ./server <port> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<fcntl.h>
#include<sys/wait.h>
#include <netinet/in.h>


void error(char *msg) {
  perror(msg);
  exit(1);
}


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

char* check(char buffer[]){
	//char* filename=buffer;
	
	char compile_command[100];
	snprintf(compile_command, sizeof(compile_command), "gcc -o test %s 2> Compile_Out.txt", buffer);

	int compilerror=system(compile_command);
	
	if(compilerror){
		char *ans=fun("./Compile_Out.txt","Compile Error\n");
		return ans;
	}
	int runerror=system("./test >Run_Out.txt 2>Run_Err.txt");
	if(runerror){
		char* ans=fun("./Run_Err.txt","Runtime Error\n");
		return ans;
	}
	
	int differror=system("diff output.txt Run_Out.txt >diff.txt");
	
	if(differror){
		char* ans=fun("./diff.txt","Output Error\n");
		return ans;
	}
	char* ans="Pass";
	return ans;
}





int main(int argc, char *argv[]) {
  int sockfd, //the listen socket descriptor (half-socket)
   newsockfd, //the full socket after the client connection is made
   portno; //port number at which server listens

  socklen_t clilen; //a type of an integer for holding length of the socket address
  char buffer[40960]; //buffer for reading and writing the messages
  struct sockaddr_in serv_addr, cli_addr; //structure for holding IP addresses
  int n;

  if (argc < 2) {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }

  /* create socket */
  sockfd = socket(AF_INET, SOCK_STREAM, 0); 
  //AF_INET means Address Family of INTERNET. SOCK_STREAM creates TCP socket (as opposed to UDP socket)
 // This is just a holder right now, note no port number either. It needs a 'bind' call


  if (sockfd < 0)
    error("ERROR opening socket");

 
  bzero((char *)&serv_addr, sizeof(serv_addr)); // initialize serv_address bytes to all zeros
  
  serv_addr.sin_family = AF_INET; // Address Family of INTERNET
  serv_addr.sin_addr.s_addr = INADDR_ANY;  //Any IP address. 

//Port number is the first argument of the server command
  portno = atoi(argv[1]);
  serv_addr.sin_port = htons(portno);  // Need to convert number from host order to network order

  /* bind the socket created earlier to this port number on this machine 
 First argument is the socket descriptor, second is the address structure (including port number).
 Third argument is size of the second argument */
  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");

  /* listen for incoming connection requests */

  listen(sockfd, 1); // 1 means 1 connection requests can be in queue. 
  //now server is listening for connections


  clilen = sizeof(cli_addr);  //length of struct sockaddr_in


  /* accept a new request, now the socket is complete.
  Create a newsockfd for this socket.
  First argument is the 'listen' socket, second is the argument 
  in which the client address will be held, third is length of second
  */

while (1){
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
      error("ERROR on accept");

    /* read message from client */

    bzero(buffer, 40960); //set buffer to zero

    //issue read call on the socket, read 255 bytes.
    n = read(newsockfd, buffer, 40960);
    if (n < 0)
      error("ERROR reading from socket");
      
    int fd=open("./test-file.c",O_CREAT|O_RDONLY|O_WRONLY|O_TRUNC,S_IRWXU);
    write(fd,buffer,n);
    char buf[]="test-file.c";
    close(fd);


  //some local printing
 //   printf("Here is the message: %s", buffer);
    char* result=check(buf);                          //running the check function , which gets the desired result of the given C file
    
 //   printf("%s\n",result);

    /* send reply to client 
    First argument is the full socket, second is the string, third is   the
  number of characters to write. */
  //  sleep(5);
    n = write(newsockfd, result, strlen(result));                  //sending the result to the client
    if (n < 0)
      error("ERROR writing to socket");
    close(newsockfd);
    }
    return 0;
  }

