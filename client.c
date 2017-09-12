/* Client program which send data to be perfomed various reduction type i.e., min, max, sum and sos to the server 
/* and obtain back the result. 
/* This program has taken some code snippet taken from http://www.beej.us/guide/bgnet for setting up socket, 
/* connecting, sending and receiving sockets*/


#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<strings.h>
#include<arpa/inet.h>
#include<string.h>

#define ERROR -1
#define BUFFER 4096

int main(int argc, char **argv) {
	
	struct sockaddr_in remote_server;
	int sock;
	char input[BUFFER];
	char output[BUFFER];
	int len;
	FILE *inputFile;
	int count=0, j=0;
	char c[1024];
	int data[1024]= {};
	const char *type = NULL;
	char *token;
	long answer[10], result;

	/*valid correct number of argument given at command line*/
	if(argc != 2) {
		printf("Invalid number of argument\n");
		exit(-1);		
	}

	type = argv[1];
	/*passing reduction type in data of index 0 along the data of csv file */
	if(strcmp("min", type)==0) {
		data[count]=0;
	} else if(strcmp("max", type)==0) {
		data[count]=1;
	} else if(strcmp("sum", type)==0) {
		data[count]=2;
	} else if(strcmp("sos", type)==0) {
		data[count]=3;
	} else {
		printf("Enter valid Function type, one among min, max, sum or sos.\n");
		exit(-1);
	}
	count++;

	/*creating socket with IPV4, TCP*/
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) == ERROR) {
		perror("socket");
		exit(-1);
	}
	
	remote_server.sin_family = AF_INET;
	remote_server.sin_port = htons(25510);
	remote_server.sin_addr.s_addr = inet_addr("127.0.0.1");   //Localhost = "127.0.0.1"
	 
	/*Connect to AWS server*/
	if((connect(sock, (struct sockaddr *)&remote_server, sizeof(struct sockaddr_in))) == ERROR) {
		perror("connect");
		exit(-1);
	}

	printf("The client is up and running. \n");

	/*Reading the csv file data, storing those numbers in csv file to data array and passing entire array to aws server and receiving the result from the aws server*/
	while(1) {
		
		inputFile = fopen("nums.csv", "r");
		count = 2; 

		
		if(inputFile != NULL) {
			while(fgets(c, 1024, inputFile)) {
				token = strtok(c,";");
				data[count] = atoi(token);
				count = count+1;
			}
			data[1] = count;
		    	send(sock, data, 10000, 0);
			printf("The client has sent the reduction %s to AWS\n", type);
			printf("The client has sent %d numbers to AWS\n", count-2);
			
			recv(sock, answer, 1024, 0);
			result=ntohl(answer[0]);
			printf("The client has received reduction %s: %ld \n", type, result);			

			fclose(inputFile);
			exit(-1);
		} else {
		    	printf("cannot open file");
			exit(-1);
		}
	}

	close(sock);	

}
