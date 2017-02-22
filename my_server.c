#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
	
#define PORT 7777
#define BUFSIZE 1024

int count_clients = 0;

struct _clients{
	char name[100];
	struct sockaddr_in addr;
};
struct _clients* clients[1024];

char* skipwhite(char* s)
{
	while (isspace(*s)) ++s;
	return s;
}

void send_to_all(int j, int i, int sockfd, int nbytes_recvd, char *recv_buf, fd_set *master)
{
	if (FD_ISSET(j, master)){
		if (j != sockfd && j != i) {
			if (send(j, recv_buf, nbytes_recvd, 0) == -1) {
				perror("send");
			}
		}
	}
}
void cmd_who(int i, fd_set *master, int sockfd, int fdmax){
	for(int k = 0; k<=fdmax ; k++){
		if(FD_ISSET(k,master) && k!=sockfd){
		char who[BUFSIZE]="[server] ";
		strcat(who,clients[k]->name);
		strcat(who," ");
		strcat(who,inet_ntoa(clients[k]->addr.sin_addr));
		strcat(who,"/");
		char port_str[5];
	sprintf(port_str , "%d" ,ntohs(clients[k]->addr.sin_port));
		strcat(who,port_str);
		if(k==i)
		strcat(who," <-me");
		send(i,who,BUFSIZE,0);
		}
	}
}

void cmd_name(int i, fd_set *master, int sockfd, int fdmax,char* recv_buf){
		char* tmp = strchr(recv_buf ,' ');
if(*(tmp+1) == '\n'){send(i,"[server] ERROR: Error command.",BUFSIZE,0);return;}
		char* name_to_change = strtok(tmp+1,"\n");
		name_to_change = skipwhite(name_to_change);
		if(!strcmp(name_to_change , "anonymous")){
char error_msg[BUFSIZE] = "[server] ERROR: Username cannot be anonymous.";
		send(i , error_msg,BUFSIZE, 0);
		return;
		}
		for(int j=0;j<=fdmax;j++){
		if (FD_ISSET(j, master)){
		if (j!=sockfd && !strcmp(name_to_change,clients[j]->name)) {
			char error_msg1[BUFSIZE] = "[server] ";
			strcat(error_msg1 , name_to_change);
			strcat(error_msg1 , " has been used by others.");
					send(i,error_msg1,BUFSIZE,0);
					return;
				}
			}
		}
	char error_msg2[BUFSIZE] = "[Server] ERROR: Username can only consists of 2~12 English letters.";
	if(strlen(name_to_change)>=2&& strlen(name_to_change) <=12){
	//65-90  97-122
	for(int k = 0;k<strlen(name_to_change); k++){
		if((name_to_change[k]<=90 && name_to_change[k]>=65) || (name_to_change[k]>=97 && name_to_change[k]<=122));
		else{
		send(i , error_msg2, BUFSIZE,0);
		return;
			}
		}
	}
	else {send(i , error_msg2, BUFSIZE,0);
		return;
	}
		char old_name_msg[BUFSIZE] = "[server] ";
		char name_msg[BUFSIZE]="[server] You're now known as ";
		strcat(old_name_msg,clients[i]->name);
		strcat(old_name_msg," is now known as ");
		strcat(old_name_msg,name_to_change);
		strcat(name_msg,name_to_change);
		send(i,name_msg,BUFSIZE,0);	
		for(int k = 0; k <= fdmax; k++){
		send_to_all(k, i, sockfd, BUFSIZE, old_name_msg, master );}
		strcpy(clients[i]->name , name_to_change);
}

void cmd_broadcast(int i, fd_set* master,int sockfd,int fdmax,char* recv_buf)
{
		char* tmp = strchr(recv_buf ,' ');
if(*(tmp+1) == '\n'){send(i,"[server] ERROR: Error command.",BUFSIZE,0);return;}
		char* msg = strtok(tmp+1,"\n");
		char yell_msg[BUFSIZE] = "[server] ";
		strcat(yell_msg,clients[i]->name);
		strcat(yell_msg," yell ");
		strcat(yell_msg,msg);
		send(i,yell_msg,BUFSIZE,0);
	for(int j = 0; j <= fdmax; j++){
		send_to_all(j, i, sockfd, BUFSIZE, yell_msg, master );
		}
}	
void cmd_private_msg(int i, fd_set* master,int sockfd,int fdmax,char* recv_buf)
{
	      char success[BUFSIZE] = "[server] SUCCESS: Your message has been sent.";
	      char error[BUFSIZE] = "[server] ERROR: The receiver doesn't exist."; 
		char* tmp = strchr(recv_buf ,' ');
if(*(tmp+1) == '\n'){send(i,"[server] ERROR: Error command.",BUFSIZE,0);return;}
if(strchr(tmp+1,' ')==NULL){send(i,"[server] ERROR: Error command.",BUFSIZE,0);return;}
		char* name_ = strtok(tmp+1," ");
		char* msg;
if((msg = strtok(NULL,"\n"))==NULL){send(i,"[server] ERROR: Error command.",BUFSIZE,0);return;}
if(!strcmp(name_,"anonymous")){
char error_msgg[BUFSIZE] = "[Server] ERROR: The client to which you sent is anonymous.";
send(i , error_msgg,BUFSIZE , 0);return;}
if(!strcmp(clients[i]->name ,"anonymous")){
char error_msggg[BUFSIZE] = "[Server] ERROR: You are anonymous.";
send(i , error_msggg,BUFSIZE , 0);return;}

		char tell_msg[BUFSIZE] = "[server] ";
		strcat(tell_msg,clients[i]->name);
		strcat(tell_msg," tell you ");
		strcat(tell_msg,msg);
		for(int j=0;j<=fdmax;j++){
			if (FD_ISSET(j, master)){
			if (j!=sockfd && !strcmp(name_,clients[j]->name) && j != i) {
					send(j,tell_msg,BUFSIZE,0);
					send(i,success,BUFSIZE,0);
					return;
				}
			}
		}
		send(i,error,BUFSIZE,0);
}	
void send_recv(int i, fd_set *master, int sockfd, int fdmax)
{
	int nbytes_recvd;
	char recv_buf[BUFSIZE], buf[BUFSIZE];
	
	if ((nbytes_recvd = recv(i, recv_buf, BUFSIZE, 0)) <= 0) {
		if (nbytes_recvd == 0) {
			char offline_msg[BUFSIZE] = "[server] ";
			strcat(offline_msg,clients[i]->name);
			strcat(offline_msg," is offline.");
			for(int j = 0; j <= fdmax; j++){
			       send_to_all(j, i, sockfd, BUFSIZE,offline_msg, master );
			}
			printf("%s\n",offline_msg);
			fflush(stdout);
		}else {
			perror("recv");
		}
		count_clients--;
		close(i);
		FD_CLR(i, master);
	}else {
	//////who 
	char* recv_buff = skipwhite(recv_buf);
	if(recv_buff[0] == 'w' && recv_buff[1] =='h' && recv_buff[2] == 'o'){
			cmd_who(i,master,sockfd,fdmax);
			return ;
		}
	//////name
	else if(recv_buff[0] == 'n' && recv_buff[1] == 'a' && recv_buff[2] == 'm' && recv_buff[3] =='e' && recv_buff[4] == ' '){
			cmd_name(i,master,sockfd,fdmax,recv_buff);
			return;
		}
	//////yell
	else if(recv_buff[0] == 'y' && recv_buff[1] == 'e' && recv_buff[2] == 'l' && recv_buff[3] =='l'&& recv_buff[4] == ' '){
			cmd_broadcast(i,master,sockfd,fdmax,recv_buff);
			return;
		}
	//////tell
	else if(recv_buff[0] =='t' && recv_buff[1] == 'e' && recv_buff[2] == 'l' && recv_buff[3] =='l'&& recv_buff[4] == ' '){
			cmd_private_msg(i,master,sockfd,fdmax,recv_buff);
			return;
		}
	else send(i,"[server] ERROR: Error command.",BUFSIZE,0);
		
	}	
}
		
void connection_accept(fd_set *master, int *fdmax, int sockfd, struct sockaddr_in *client_addr)
{
	socklen_t addrlen;
	int newsockfd;
	
	addrlen = sizeof(struct sockaddr_in);
	if((newsockfd = accept(sockfd, (struct sockaddr *)client_addr, &addrlen)) == -1) {
		perror("accept");
		exit(1);
	}else {
		FD_SET(newsockfd, master);
		if(newsockfd > *fdmax){
			*fdmax = newsockfd;
		}
	struct _clients* new_client = calloc(sizeof(struct _clients), 1);  
	new_client->addr = *client_addr;
	strcpy(new_client->name ,"anonymous");
	clients[newsockfd] = new_client;
	count_clients++;
	
	printf("new connection from %s on port %d  Now has %d clients\n",inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port),count_clients);
	fflush(stdout);

	char hello_msg[50] = "[server] Hello, anonymous! From: " ;
	hello_msg[49] = '\0';
	strcat(hello_msg,inet_ntoa(client_addr->sin_addr));
	strcat(hello_msg,"/");
	char port_str[5];
	sprintf(port_str , "%d" ,ntohs(client_addr->sin_port));
	strcat(hello_msg,port_str);
	send(newsockfd, hello_msg, strlen(hello_msg),0);
	for(int j = 0; j <= *fdmax; j++){
		send_to_all(j, newsockfd, sockfd, 30, "[server] Someone is coming!\0", master );
		}		
	}
}
	
void connect_request(int *sockfd, struct sockaddr_in *my_addr)
{
	int yes = 1;
		
	if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket");
		exit(1);
	}
		
	my_addr->sin_family = AF_INET;
	my_addr->sin_port = htons(PORT);
	my_addr->sin_addr.s_addr = INADDR_ANY;
	memset(my_addr->sin_zero, '\0', sizeof my_addr->sin_zero);
		
	if (setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}
		
	if (bind(*sockfd, (struct sockaddr *)my_addr, sizeof(struct sockaddr)) == -1) {
		perror("Unable to bind");
		exit(1);
	}
	if (listen(*sockfd, 10) == -1) {
		perror("listen");
		exit(1);
	}
	printf("TCP Server Waiting for client on port %d\n",PORT);
	fflush(stdout);
}
int main()
{
	fd_set master;
	fd_set read_fds;
	int fdmax, i;
	int sockfd= 0;
	struct sockaddr_in my_addr, client_addr;
	
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	connect_request(&sockfd, &my_addr);
	FD_SET(sockfd, &master);
	
	fdmax = sockfd;
	while(1){
		read_fds = master;
		if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){
			perror("select");
			exit(4);
		}
		
		for (i = 0; i <= fdmax; i++){
			if (FD_ISSET(i, &read_fds)){
				if (i == sockfd)
					connection_accept(&master, &fdmax, sockfd, &client_addr);
				else
					send_recv(i, &master, sockfd, fdmax);
			}
		}
	}
	return 0;
}
		
