#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<netdb.h>
#include<errno.h>
#include<error.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<string.h>

#define ERROR -1
#define MAX_DATA 4096

int createUDPsock(int data[MAX_DATA], int portno) {

	int udpsock, udplength, n, length;
	struct sockaddr_in backend, aws; 
	struct hostent *hp;
	int buffer[4096];
	int answer[4096];
	
	if((udpsock = socket(AF_INET, SOCK_DGRAM, 0)) == ERROR) {
		perror("udp socket: ");
		return -1;
	}

	backend.sin_family = AF_INET;
	backend.sin_port=htons(24510);
	hp = gethostbyname("localhost");

	if(hp==0) {
		perror("unknown host");
		return -1;
	}
	
	bcopy((char *)hp->h_addr, (char *)&backend.sin_addr, hp->h_length);

	backend.sin_port = htons(portno);
	length=sizeof(struct sockaddr_in);

	n=sendto(udpsock,data,4096,0,(struct sockaddr *)&backend,length);

	if(n<0) {
		perror("sendto");
		return -1;
	}

	n=recvfrom(udpsock,answer,256,0,(struct sockaddr *)&aws, &length);
	if(n<0) {
		perror("recvfrom");	
		return -1;
	}

	return ntohl(answer[0]);
}

int main() {

	struct sockaddr_in server;
	struct sockaddr_in client;
	int sock;
	int new;
	int sockaddr_len = sizeof(struct sockaddr_in);
	int data_len;
	int data[MAX_DATA];
	int i;
	int backendA, backendB, backendC;
	int backendData[MAX_DATA];
	int j=0;
	const char *type = NULL;
	int computed_value;
	int temp;
	unsigned long answer;

	if((sock = socket(AF_INET, SOCK_STREAM, 0)) == ERROR) 		{
		perror("server socket: ");
		exit(-1);	
	}

	printf("The AWS is up and running\n");
	server.sin_family = AF_INET;
	server.sin_port = htons(25510);
	server.sin_addr.s_addr = INADDR_ANY;
	bzero(&server.sin_zero, 8);

	if((bind(sock, (struct sockaddr *)&server, sockaddr_len)) == ERROR) {
		perror("bind: ");
		exit(-1);
	}
	
	if((listen(sock, 5)) == -1) {
		perror("listen");
		exit(-1);
	}

	while(1) {
		if((new = accept(sock, (struct sockaddr *)&client,&sockaddr_len)) == -1 ) { 
			perror("accept:");
			exit(-1);
		}

		data_len = 1;
		while(data_len) {
			if(data_len = recv(new, data, MAX_DATA, 0)==-1) {
				perror("recvieve:");
				exit(-1);
			}
			printf("The AWS has received %d numbers from the client using TCP over port %d\n", data[1]-2, ntohs(server.sin_port));

			backendData[0]=data[0];
			backendData[1]=data[1]/3;
			j=2;
			for(i=2; i<backendData[1]+2; i++) {
				backendData[j]=data[i];
				j++;
			}


			backendA = createUDPsock(backendData, 21510);
			printf("The AWS send %d numbers to Backend-Server A\n", data[1]/3);

			j=2;			
			for(i=backendData[1]+2; i<backendData[1]*2+2; i++) {
				backendData[j]=data[i];
				j++;
			}
			backendB = createUDPsock(backendData, 22510);
			printf("The AWS send %d numbers to Backend-Server A\n", data[1]/3);			

			j=2;
			for(i=backendData[1]*2+2; i<backendData[1]*3+2; i++) {
				backendData[j]=data[i];
				j++;
			}
			backendC = createUDPsock(backendData, 23510);
			printf("The AWS send %d numbers to Backend-Server A\n", data[1]/3);


			printf("The AWS received reduction result of %s from Backend-Server A using UDP over port 24510 and it is %d\n", type, backendA);
			printf("The AWS received reduction result of %s from Backend-Server B using UDP over port 24510 and it is %d\n", type, backendB);
			printf("The AWS received reduction result of %s from Backend-Server C using UDP over port 24510 and it is %d\n", type, backendC);

			
			
			if(data[0]==0) {
				type ="min";
				temp = (backendA < backendB) ? backendA : backendB;
				answer = (backendC < temp) ? backendC : temp;
			} else if(data[0]==1) {
				type ="max";
				temp = (backendA > backendB) ? backendA : backendB;
				answer = (backendC > temp) ? backendC : temp;
			} else if(data[0]==2) {
				type ="sum";
				answer= backendA + backendB + backendC;
			} else if(data[0]==3) {
				type ="sos";
				answer=backendA * backendA + backendB * backendB + backendC * backendC;
			} 

			printf("The AWS has successfully finished the reduction %s: %lu\n", type, answer);
			computed_value = htonl(answer);	
			send(new, &computed_value, MAX_DATA, 0);
			printf("The AWS has successfully finished sending the reduction value to client.\n");
		}
	
		printf("Client disconnected\n");
		close(new);
	}
	
	close(sock);
	return 0;
}

