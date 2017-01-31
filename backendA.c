#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<arpa/inet.h>


void error(char *msg) {
	perror(msg);
	exit(-1);
}

int main() {
	int sock, length, fromlen, n;
	struct sockaddr_in server;
	struct sockaddr_in from;
	char buf[1024];
	int data[1024];
	int i;
	const char *type = NULL;
	int count, j, answer;
	int computed_value;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock<0) {
		perror("Opening socket");
		exit(-1);
	} 
	printf("The Server A is up and running using udp port 21510\n");	


	length = sizeof(server);
	bzero(&server, length);

	server.sin_family=AF_INET;
	server.sin_addr.s_addr=INADDR_ANY;
	server.sin_port=htons(21510);
	if(bind(sock, (struct sockaddr *)&server, length)<0) {
		perror("binding");
	}

	fromlen = sizeof(struct sockaddr_in);

	while(1) {
		n=recvfrom(sock,data, 4096,0,(struct sockaddr *)&from, &fromlen);
		if(n<0) {
			error("recvfrom");
		}
		
		count = data[1];
		printf("The Server A has received %d numbers\n", count);
		if(data[0]==0) {
			type ="MIN";
			answer=data[2];
			for(i=2; i<=count+1; i++) {
				if(data[i]<answer) {
					answer = data[i];
				}
			}
		} else if(data[0]==1) {
			type ="MAX";
			answer=data[2];
			for(i=2; i<=count+2; i++) {
				if(data[i]>answer) {
					answer = data[i];
				}
			}
		} else if(data[0]==2) {
			type ="SUM";
			answer=0;
			for(i=2; i<=count+3; i=i+2) {
				answer += data[i]+data[i+1];
			}
		} else if(data[0]==3) {
			type ="SOS";
			answer=0;
			for(i=2; i<=count+2; i=i+2) {
				answer += (data[i] * data[i]) + (data[i+1] * data[i+1]);
			}
		} 

		computed_value = htonl(answer);		
		printf("The Server A has successfully finished the %s: %d\n", type, answer);
		
		n=sendto(sock, &computed_value, sizeof(computed_value),0,(struct sockaddr *)&from, fromlen);
		if(n<0){
			error("sendto");
		}
		printf("The Server A has successfully finished sending the reduction value to AWS server\n");
	}

	return 0;
}

