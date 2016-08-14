#include <iostream>
#include <string.h>
#include <cstdio>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <pthread.h>
#include <chrono>
#include <unistd.h>
#include <random>
using namespace std;

int main(int argc, char*argv[]){
	if(argc != 5){
		cout<<"Usage : "<<argv[0]<<" <file-name> <server-address> <port> <mode (display file or not)>\n";
		return 0;
	}

	char filename[100];
	filename[0]='g';filename[1]='e';filename[2]='t';filename[3]=' ';
	for(int i=4;i<strlen(argv[1])+4;i++){
		filename[i] = argv[1][i-4];
	}
	filename[4+strlen(argv[1])] = '\0';
	// printf("%s %d\n",argv[1], strlen(argv[1]) );
	// for(int i=0;filename[i] != '\0' && i<100;i++){
	// 	cout<<filename[i];
	// }
	int port = atoi(argv[3]);
	bool display = strcmp("display", argv[4]) == 0;
	struct sockaddr_in serv_addr;
	char buffer[512];
	int recvd_bytes;

	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd < 0){
		cout<<"Error while opening the socket ...\n";
		return 0;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = inet_addr(argv[2]);

	int conn = connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if(conn < 0){
		cout<<"Error while connecting to the server ...\n";
		return 0;
	}

	int sendfilename = write(sock_fd, filename, strlen(filename));
	int num_bytes_received = 0;

	while(1){
		recvd_bytes = read(sock_fd, buffer, 512);
		if(recvd_bytes < 0){
			perror("Error while reading from the socket ...");
			cout<<"Number of bytes received = "<<num_bytes_received<<endl;
			return 0;
		}
		if(recvd_bytes == 0){
			cout<<"File received successfully.\n";
			return 0;
		}
		if(display){
			buffer[recvd_bytes] = '\0';
			printf("%s", buffer);
		}
		num_bytes_received += recvd_bytes;
	}
}
