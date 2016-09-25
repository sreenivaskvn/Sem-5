#include <iostream>
#include <vector>
#include <queue>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <limits.h>
using namespace std;

pthread_cond_t buffer_has_space=PTHREAD_COND_INITIALIZER, buffer_has_data=PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
queue<int> request_queue;


void * send_file(void *arg);

int main(int argc, char* argv[]){

    int port; // The port on which we are listening to new connections
    int sock_desc; // The socket descriptor
    int sock_fd; // The socket file descriptor
    struct sockaddr_in addr; // socket parameters for bind
    struct sockaddr_in addr1; // socket parameters for acept
    int addrlen = sizeof(addr1); // argument to accept
    int ret_val;
    int NumThreads;
    int MaxQueue;
    int request_count=0;
    pthread_t *worker_threads;
    int s;

    NumThreads = atoi(argv[2]);
    worker_threads = new pthread_t[NumThreads];
    MaxQueue = atoi(argv[3]);

    if(MaxQueue == 0){
        MaxQueue = INT_MAX;
    }

    //printf("%d \n", MaxQueue);

    if(argc == 4){
        port = atoi(argv[1]);
    }else{
        // ERROR MESSAGE
        printf("wrong usage\n");
        return 0;
    }

    sock_desc = socket(AF_INET, SOCK_STREAM, 0); // create internet socket domain
    if(sock_desc == -1){
        // Error Message. Couldn't create socket.
        printf("Couldn't create socket.\n");
        return 0;
    }

    memset(&addr, 0, sizeof(addr)); /// need is not clear
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    ret_val = bind(sock_desc, (struct sockaddr *)&addr, sizeof(addr));
    if(ret_val == -1){
        // ERROR MESSAGE
    }

    ret_val = listen(sock_desc, SOMAXCONN);
    if(ret_val == -1){
        // ERROR MESSAGE
    }

    for(int i_thread=0;i_thread<NumThreads;i_thread++){
        s=pthread_create(&worker_threads[i_thread], NULL, send_file, NULL);
        if(s!=0){
            //handle_error_en(s, "pthread_create");   
        }
    }

    while(1){
	    sock_fd = accept(sock_desc, (struct sockaddr *)& addr1, (socklen_t*)&addrlen); /// what is need of addrlen
        if(sock_fd == -1){
            // ERROR MESSAGE
            continue;
        }
        pthread_mutex_lock(&mutex);
        while(request_queue.size() >= MaxQueue){
            pthread_cond_wait(&buffer_has_space, &mutex);
            // wait(buffer_has_space)
        }  
        request_queue.push(sock_fd);
        pthread_cond_signal(&buffer_has_data);
        // signal(buffer_has_data);
        pthread_mutex_unlock(&mutex);
	}
}

void * send_file(void *arg){
    while(true)
    {
        pthread_mutex_lock(&mutex);
        while(request_queue.size() == 0){
            pthread_cond_wait(&buffer_has_data, &mutex);
            //wait(buffer_has_data)
        }
        int sock_fd = request_queue.front();
        request_queue.pop();
        pthread_cond_signal(&buffer_has_space);
        //signal(buffer_has_space)
        pthread_mutex_unlock(&mutex);

        /* Sending the file to 
        the client */
        int fd; // file descriptor for file to send
        struct stat stat_buf; // argument to stat
        off_t offset; // file offset
        char filename[25]; // should be changed if filename is more than 20 characters long
        int filename_len;
        int numbytes_sent;
        size_t numbytes_to_be_sent;

        filename_len = recv(sock_fd, filename, sizeof(filename), 0);
        if (filename_len == -1) {
          fprintf(stderr, "recv failed: %s\n", strerror(errno));
          exit(1);
        }
        filename[filename_len] = '\0';
        string extract(filename);
        for(int i=4;i<filename_len+1;i++){
            filename[i-4] = extract[i];
        }
        fd = open(filename, O_RDONLY);
        if (fd == -1) {
            fprintf(stderr, "unable to open '%s': %s\n", filename, strerror(errno));
            exit(1);
        }

        fstat(fd, &stat_buf); // get the stats of file like filesize in bytes
        numbytes_to_be_sent = stat_buf.st_size;
        string s = to_string(numbytes_to_be_sent);
        offset = 0;
        while(numbytes_to_be_sent > 0){
            //cout<<"The size remaining : "<<numbytes_to_be_sent<<endl;
            numbytes_sent = sendfile(sock_fd, fd, &offset, numbytes_to_be_sent);
            //cout<<numbytes_sent<<endl;
            numbytes_to_be_sent -= numbytes_sent;
            //cout<<"Offset now is : "<<offset<<endl;
        }
        close(fd);
        if(close(sock_fd)<0){
            perror("socket close error");
            exit(EXIT_FAILURE);
        }
        else{
            //cout<<"socket "<<sock_fd<<" closed\n";
        }
        /* File sending completed */


    }
    pthread_exit(NULL);
}
