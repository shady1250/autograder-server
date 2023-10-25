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


char* check(char buffer[]){

	if(fork()){
		wait(NULL);                                           //waiting while the child is compiling the c file
		if(fork()){
			wait(NULL);                         //waiting while child is performing "grep error" on compilation output to check whether it's warning or the error in the compilation output
			int compilerr=open("./Compile_Err.txt",O_RDONLY,S_IRWXU);    //Opening the output of "grep error" generated from the input as Compilation's output
			long size=lseek(compilerr,0,SEEK_END);
			close(compilerr);
			if(size!=0){
				int compilefd=open("./Compile_Out.txt",O_RDONLY,S_IRWXU);  //Opening the output of compilation to copy to the buffer to return it to the main function
				char str[40960]="Compile Error\n";
				char ch[40960];
				bzero(ch, 40960);
				read(compilefd,ch,40960);
				strcat(str,ch);                                           //concatenating msg "Compiler Error" with the compilation output
				char *ans=str;
				close(compilefd);
				return ans;                                              //returning the concatenated string to the main function
			}
		}
		else{
			int compilerr=open("./Compile_Err.txt",O_CREAT|O_RDONLY|O_WRONLY|O_TRUNC, S_IRWXU); //output of "grep error" will be redirected to this file
			dup2(compilerr,STDOUT_FILENO);
			int compilefd=open("./Compile_Out.txt",O_RDONLY,S_IRWXU);               //input of "grep error" will be taken from this file, this file contains compilation output
			dup2(compilefd,STDIN_FILENO);
			char* args[]={"grep","error",NULL}; 
			execvp("grep",args);                                                 //child executing "grep error"
			perror("execvp grep");
			exit(1);
		}
	}
	else{
		int compilefd=open("./Compile_Out.txt",O_CREAT|O_RDONLY|O_WRONLY|O_TRUNC, S_IRWXU);   //compilation error will be redirected to this file
		dup2(compilefd,STDERR_FILENO);                                           
		char* argss[]={"gcc","-o","test", buffer, NULL};
		execvp("gcc",argss);                                                               //child compiling the c file
		perror("execvp gcc"); 
		exit(1);
	}
	
	if(fork()){
		wait(NULL);                                            //parent waiting while child is running the .o file
		int runerr=open("./Run_Err.txt",O_RDONLY,S_IRWXU);     //opening the file which contains the runtime error data
		long size=lseek(runerr,0,SEEK_END);
		close(runerr);
		if(size!=0){                                            //if runtime error file is empty than there is no runtime error
			char str[40960]="Runtime Error\n";
			int run_fd=open("./Run_Err.txt",O_RDONLY,S_IRWXU);      
			char ch2[40960];
			bzero(ch2, 40960);
			read(run_fd,ch2,40960);                       //copying the content of runtime error file to the ch2 buffer
			strcat(str,ch2);
			char* ans=str;
			close(run_fd);
			return ans;
		}
		if(fork()){
			wait(NULL);                                       // parent waiting for the child to execute the diff command
			int diff_fd=open("./diff.txt",O_RDONLY,S_IRWXU);  //opening the diff.txt which contains the output of "diff output.txt run_out.txt"
			long size=lseek(diff_fd,0,SEEK_END);
			close(diff_fd);
			if(size==0){                                //if diff.txt is empty then there is no output error
				char* ans="Pass";
				return ans;
			}
			diff_fd=open("./diff.txt",O_RDONLY,S_IRWXU);         
			char str[40960]="Output Error\n";
			char ch3[40960];
			bzero(ch3, 40960);
			read(diff_fd,ch3,40960);              //copying the content of diff.txt to the ch3 buffer
			strcat(str,ch3); 
			char* ans=str;
			close(diff_fd);
			return ans;
		}
		else{
			int diff_fd=open("./diff.txt",O_CREAT|O_RDONLY|O_WRONLY|O_TRUNC, S_IRWXU);
			dup2(diff_fd,STDOUT_FILENO);
			char* args[]={"diff","output.txt","Run_Out.txt",NULL};
			execvp("diff",args);
			perror("execvp diff");                                   //running the command "diff output.txt Run_out.txt" and redirecting the output to diff.txt
			exit(1);
		}
	}
	else{
		int runout=open("./Run_Out.txt",O_CREAT|O_RDONLY|O_WRONLY|O_TRUNC, S_IRWXU);
		int runerr=open("./Run_Err.txt",O_CREAT|O_RDONLY|O_WRONLY|O_TRUNC, S_IRWXU);
		dup2(runout,STDOUT_FILENO);
		dup2(runerr,STDERR_FILENO);
		char* args[]={"sh","-c","./test",NULL};
		execvp("sh",args);                                      //running the command "sh -c ./test" where test is .o file , and redirecting the error to Run_err.txt and output to Run_out.txt
		perror("execvp ./test");
		exit(1);
	
	}
		

	return "";

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
    int fd=open("./test-file.c",O_CREAT|O_RDONLY|O_WRONLY|O_TRUNC,S_IRWXU);
  write(fd,buffer,n);
  char buf[]="test-file.c";

    if (n < 0)
      error("ERROR reading from socket");

  //some local printing
 //   printf("Here is the message: %s", buffer);
    char* result=check(buf);                          //running the check function , which gets the desired result of the given C file
//    printf("%s\n",result);

    /* send reply to client 
    First argument is the full socket, second is the string, third is   the
  number of characters to write. */
    sleep(5);
    n = write(newsockfd, result, strlen(result));                  //sending the result to the client
    if (n < 0)
      error("ERROR writing to socket");
    }
    return 0;
  }

