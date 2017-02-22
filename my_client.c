#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
	
#define BUFSIZE 1024
	
char* skipwhite(char* s)
{
	while (isspace(*s)) ++s;
	return s;
}
	
void send_recv(int i, int sockfd)
{
	char send_buf[BUFSIZE];
	char recv_buf[BUFSIZE];
	int nbyte_recvd;
	
	if (i == 0){  		////////me
		fgets(send_buf, BUFSIZE, stdin);
		char* ptobuf = send_buf;
		ptobuf = skipwhite(ptobuf);
		if (ptobuf[0] == 'e' && ptobuf[1] == 'x' && ptobuf[2] == 'i' && ptobuf[3] == 't') {
			close(sockfd);			
			exit(0);
		}else
			send(sockfd, send_buf, BUFSIZE, 0);
	}else {    		///////other client
		nbyte_recvd = recv(sockfd, recv_buf, BUFSIZE, 0);
		if(nbyte_recvd ==0){
		printf("Server closed.\n");
		exit(0);
		}
		recv_buf[nbyte_recvd] = '\0';
		printf("%s\n" , recv_buf);
		fflush(stdout);
	}
}
		
		
void connect_request(int *sockfd, struct sockaddr_in *server_addr,char* ip_addr , int port)
{	char buffer[50];
	if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket");
		exit(1);
	}
	server_addr->sin_family = AF_INET;
	server_addr->sin_port = htons(port);
	server_addr->sin_addr.s_addr = inet_addr(ip_addr);
	memset(server_addr->sin_zero, '\0', sizeof server_addr->sin_zero);
	
	if(connect(*sockfd, (struct sockaddr *)server_addr, sizeof(struct sockaddr)) == -1) {
		perror("connection error");
		exit(1);
	}
	recv(*sockfd, buffer,50,0);
	printf("%s\n",buffer);
	fflush(stdout);
}
	
int main(int argc, char** argv)
{
	int sockfd, fdmax, i;
	struct sockaddr_in server_addr;
	fd_set master;
	fd_set read_fds;
if(argc!=3)
   printf("usage: ./client [ip] [port](7777)\n");
else{
	connect_request(&sockfd, &server_addr,argv[1],atoi(argv[2]));
	FD_ZERO(&master);
        FD_ZERO(&read_fds);
        FD_SET(0, &master);
        FD_SET(sockfd, &master);
	fdmax = sockfd;
	
	while(1){
		read_fds = master;
		if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){
			perror("select");
			exit(4);
		}
		
		for(i=0; i <= fdmax; i++ )
			if(FD_ISSET(i, &read_fds))
				send_recv(i, sockfd);
	}
	printf("client-quited\n");
	close(sockfd);
	return 0;
}
}