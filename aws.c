/* Client/Server program which various performs various reduction type i.e., min, max, sum and sos operation
/* This program has taken some code snippet taken from http://www.beej.us/guide/bgnet for setting up socket,
/* connecting, sending and receiving sockets */
#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<netdb.h>
#include<errno.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<string.h>
#include<pthread.h>

#define ERROR -1
#define MAX_DATA 4096

/*creating UDP socket connection between AWS and backend Server A, B and C */ 
int createUDPsock(int data[MAX_DATA], int portno, char server) {

	int udpsock, udplength, n, length;
	struct sockaddr_in backend, aws; 
	struct hostent *hp;
	int buffer[4096], aws_portno;
	long answer[10], result;
	const char *type = NULL;
	int i;
	
	/*creating the UDP socket */
	if((udpsock = socket(AF_INET, SOCK_DGRAM, 0)) == ERROR) {
		perror("udp socket: ");
		return -1;
	}

	backend.sin_family = AF_INET;
	aws_portno = aws.sin_port=htons(24510);
	hp = gethostbyname("localhost");

	if(hp==0) {
		perror("unknown host");
		return -1;
	}
	
	bcopy((char *)hp->h_addr, (char *)&backend.sin_addr, hp->h_length);

	/* assigning the port no to backend servers*/
	backend.sin_port = htons(portno);
	length=sizeof(struct sockaddr_in);
	
	

	/* sending the data to the backend server A/B/C*/
	n=sendto(udpsock,data,4096,0,(struct sockaddr *)&backend,length);

	if(n<0) {
		perror("sendto");
		return -1;
	}

	/*obtaining back the result from the UDP packet*/
	n=recvfrom(udpsock,answer,256,0,(struct sockaddr *)&aws, &length);
	if(n<0) {
		perror("recvfrom");	
		return -1;
	}

	/*identify the reduction type to display the necessary message*/
	if(data[0]==0) {
		type ="min";
	} else if(data[0]==1) {
		type ="max";
	} else if(data[0]==2) {
		type ="sum";
	} else if(data[0]==3) {
		type ="sos";
	}
	
	result=ntohl(answer[0]);
	printf("The AWS received reduction result of %s from Backend-Server %c using UDP over port %d and it is %ld\n", type, server, ntohs(aws_portno), result);

	return ntohl(answer[0]);
}

int sendData(int *data, int portno, char server) {
	int backendData[MAX_DATA];

	backendData[0]=data[0];
	backendData[1]=data[1]/3;
	int j=2, i, result;
	for(i=2; i<backendData[1]+2; i++) {
		backendData[j]=data[i];
		j++;
	}
	
	printf("The AWS send %d numbers to Backend-Server %c\n", data[1]/3, server);
	result = createUDPsock(backendData, portno, server);
	return result;
}

void *connectSocket(void  *new) {
	int data[MAX_DATA];
	int data_len = 1;
	long backendA, backendB, backendC, temp, computed_value, answer;
	const char *type = NULL;
	int sock = *(int *)new;

	printf("File Descriptor: %d\n", sock);
	while(data_len) {
		/*receiving the client data*/
		if(data_len = recv(sock, data, MAX_DATA, 0)==-1) {
			perror("receive:");
			exit(-1);
		}
		printf("The AWS has received %d numbers from the client using TCP\n", data[1]-2);
		backendA = sendData(data, 21510, 'A');
		backendB = sendData(data, 22510, 'B');
		backendC = sendData(data, 23510, 'C');		
		/*based on the reduction type, the result obtained from backend server A, B and C is performed approriate reduction tnal result back to the client*/

	}

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
		answer=backendA + backendB + backendC;
	} 

	computed_value = htonl(answer);	
	
	printf("The AWS has successfully finished the reduction %s: %ld\n", type, answer);
	send(sock, &computed_value, MAX_DATA, 0);
	printf("The AWS has successfully finished sending the reduction value to client.\n");
}

int main() {

	struct sockaddr_in server;
	struct sockaddr_in client;
	int sock;
	
	int sockaddr_len = sizeof(struct sockaddr_in);
	int data_len;
	int new;
	int i;
	int backendData[MAX_DATA];
	int j=0;
	long answer;
	pthread_t incoming;

	/*creating the TCP packet with client */

	if((sock = socket(AF_INET, SOCK_STREAM, 0)) == ERROR) 		{
		perror("server socket: ");
		exit(-1);	
	}

	printf("The AWS is up and running\n");

	/*assigning the values to the server*/
	server.sin_family = AF_INET;
	server.sin_port = htons(25510);
	server.sin_addr.s_addr = inet_addr("127.0.0.1"); 	//Localhost = "127.0.0.1"

	/*binding the socket*/
	if((bind(sock, (struct sockaddr *)&server, sockaddr_len)) == ERROR) {
		perror("bind: ");
		exit(-1);
	}
	
	/*looking for any client to connect*/
	if((listen(sock, 5)) == -1) {
		perror("listen");
		exit(-1);
	}

	while(1) {
		/*accept the client connection*/

		if((new = accept(sock, (struct sockaddr *)&client,&sockaddr_len)) == -1 ) { 
			perror("accept:");
			exit(-1);
		} 

		printf("File Descriptor: %d\n", new);
		if(pthread_create(&incoming, NULL, connectSocket, (void *)&new) < 0) {
			perror("Thread creation failed:");
			exit(-1);
		}

		pthread_join(incoming, NULL);
		/*close(new);*/
	}

	close(sock);
	return 0;
}

