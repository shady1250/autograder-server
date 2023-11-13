/* run using ./server <port> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<fcntl.h>
#include <netinet/in.h>


void error(char *msg) {
  perror(msg);
  exit(1);
}



int main(int argc, char *argv[]) {
  int sockfd, newsockfd, portno; 

  socklen_t clilen; 
  char buffer[256]; 
  struct sockaddr_in serv_addr, cli_addr; 
  int n;

  if (argc < 2) {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
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

  

  listen(sockfd, 2); 
  


  clilen = sizeof(cli_addr); 

while(1){
  newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
  if (newsockfd < 0)
    error("ERROR on accept");

  bzero(buffer, 256);


  system("bash server_script2.sh");
  
 
  
  int fd=open("server_stats/to_send_file.txt", O_RDONLY);
  if(fd<0){
      error("Error opening the stat file");
  }
  long size=lseek(fd,0,SEEK_END);
  lseek(fd,0,SEEK_SET);
  char result[size+1];
  bzero(result,size+1);
  read(fd,result,size+1);
  close(fd);
  
  n= write(newsockfd,result,sizeof(result));
  if (n < 0)
    error("ERROR writing to socket");
  close(newsockfd);

}
  return 0;
}
