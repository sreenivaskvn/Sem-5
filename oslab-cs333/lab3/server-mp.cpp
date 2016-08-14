#include <iostream>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h>
using namespace std;

void * reap(void*i){
    int pid = *((int*)i);
    int status;
    pid_t w = waitpid(-1, &status, 0); // -1 is for any process that is a child
              // Need not lock. waitpid is  thread-safe
}

int main(int argc, char** argv){

    int port; // The port on which we are listening to new connections
    int sock_desc; // The socket descriptor
    int sock_fd; // The socket file descriptor
    int fd; // file descriptor for file to send
    struct sockaddr_in addr; // socket parameters for bind
    struct sockaddr_in addr1; // socket parameters for acept
    int addrlen; // argument to accept
    struct stat stat_buf; // argument to stat
    off_t offset; // file offset
    char filename[25]; // should be changed if filename is more than 20 characters long
    int ret_val; // return value for some system calls
    int filename_len;
    int numbytes_sent;
    size_t numbytes_to_be_sent;
    vector<pthread_t> child_pid;

    if(argc == 2){
        port = atoi(argv[1]);
    }else{
        // ERROR MESSAGE
    }

    sock_desc = socket(AF_INET, SOCK_STREAM, 0); // create internet socket domain
    if(sock_desc == -1){
        // Error Message. Couldn't create socket.
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

    while(1){
        sock_fd = accept(sock_desc, (struct sockaddr *)& addr1, (socklen_t*)&addrlen); /// what is need of addrlen
        if(sock_fd == -1){
            // ERROR MESSAGE
        }

        int ret_fork = fork();
        pthread_t t;
        child_pid.push_back(t);
        if(ret_fork != 0){
            pthread_create(&child_pid.back(), NULL, reap, (void*)&ret_fork);
            if(close(sock_fd)<0){
                perror("socket close error");
                exit(EXIT_FAILURE);
            }
            continue;
        }else{
            filename_len = read(sock_fd, filename, 50);
            if (filename_len == -1) {
              fprintf(stderr, "recv failed: %s\n", strerror(errno));
              exit(1);
            }
            filename[filename_len] = '\0';
            printf("%s\n",filename );
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
                cout<<"The size remaining : "<<numbytes_to_be_sent<<endl;
                numbytes_sent = sendfile(sock_fd, fd, &offset, 512);
                numbytes_to_be_sent -= numbytes_sent;
                sleep(1);
            }
            if(close(sock_fd)<0){
                perror("socket close error");
                exit(EXIT_FAILURE);
            }
            return 0; // This child should exit
        }
    }
}
