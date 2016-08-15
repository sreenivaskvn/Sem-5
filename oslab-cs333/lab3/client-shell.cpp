#include <stdio.h>
#include <fstream> // file stream
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
using namespace std;

bool stop_cmd; // indicates whether to stop the 'getsq' and 'getpl' or not. We have to stop 
			   // all the future downloads corresponding to that command.	      
void sig_handler(int signo)
{
  	if (signo == SIGINT){
  		stop_cmd = true;
  		// stop the current running command
	}
}

void *wait_thread(void *i){
	int pid = *((int*)i);
	printf("%d\n", pid);
    int status;
    pid_t w = waitpid(pid, &status, 0);
}

class server_details{
public:
	char ip[20];
	char port[6]; // This is an integer but for simplicity of passing arguments to exec
				  // we store it as a char array
	server_details(){
		strcpy(ip, "\0");
		strcpy(port, "\0");
	}
	void assign(char *ip, char *port){
		strcpy(this->ip,ip);
		strcpy(this->port, port);
	}
};

char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}

// function to check if a file given its full path exists or not
bool exists(char *path){
	ifstream f(path);
	return f.good();
}

int main()
{
    char  line[MAX_INPUT_SIZE];            
    char  **tokens;              
    int i;
    int retfork;
    server_details server;
    bool server_saved = false;
    if (signal(SIGINT, sig_handler) == SIG_ERR)
	printf("\ncan't catch SIGINT\n");
	pthread_t getpl_name[64];
	int getpl_pid[64];

    while (1) {           
       
    printf("\033[1;33mHello>\033[0m");
    	stop_cmd = false;    
        bzero(line, MAX_INPUT_SIZE);
        fgets(line, MAX_INPUT_SIZE, stdin);
        if(strlen(line) == 1){
        	continue;
        }
        line[strlen(line)] = '\n'; //terminate with new line
        tokens = tokenize(line);
   		if(strcmp(tokens[0], "exit") == 0){
   			for(i = 0; tokens[i] != NULL; i++);
   			if(i != 1){
   				printf("%s\n", "No arguments for exit expected.");
   				continue;
   			}else{
   				int status;
   				// reap all child processes till none is present
   				while( waitpid(-1, &status, 0) >= 0 ){}
   				return 0;
   			}
   		}

   		if(strcmp(tokens[0],"server") == 0){
   			for(i = 0; tokens[i] != NULL; i++);
   			if(i != 3){
   				printf("%s\n", "Usage : server <server-ip> <server-port>");
   				continue;
   			}else{
   				server.assign(tokens[1], tokens[2]);
   				printf("Server details saved.\nip : %s\nport : %s\n", tokens[1], tokens[2]);
   				server_saved = true;
   				continue;
   			}
   		}

   		if(strcmp(tokens[0], "getfl") == 0){
   			for(i = 0; tokens[i] != NULL; i++);
   			if(i != 2){
   				printf("%s\n", "Usage : getfl <filename>");
   			}else{
   				if(!server_saved){
   					printf("%s%s\n", "Server-ip and port have not been saved.\n",
   						"To save : server <server-ip> <port>");
   					continue;
   				}
   				retfork = fork();
   				if(retfork == 0){
   					execl("./get-one-file-sig", "./get-one-file-sig", tokens[1],
   							 server.ip, server.port, "display", (char*)NULL);
   				}else{
   					int status;
   					waitpid(retfork, &status, 0);
   				}
   			}
   			continue;
   		}

        if(strcmp(tokens[0], "cd") == 0){
        	for(i = 0; tokens[i] != NULL; i++);
        	if(i != 2){
        		printf("%s\n", "Usage : cd <directory-path>");
        		continue;
        	}
        	if(chdir(tokens[1]) < 0){
        		printf("%s%s\n", "Can not cd into ", tokens[1]);
        		continue;
        	}
        	continue;
        }

        if(strcmp(tokens[0], "getsq") == 0){
        	int num;
        	for(i = 0; tokens[i] != NULL; i++);
   			if(i < 2){
   				printf("%s\n", "Usage : getsq <filename1> <filename2> ...");
   			}else{
   				if(!server_saved){
   					printf("%s%s\n", "Server-ip and port have not been saved.\n",
   						"To save : server <server-ip> <port>");
   					continue;
   				}
   				for(num = 1; num < i && !stop_cmd; num++){
	   				retfork = fork();
	   				if(retfork == 0){
	   					execl("./get-one-file-sig","./get-one-file-sig",tokens[num],
	   							 server.ip, server.port, "nodisplay", (char*)NULL);
	   				}else{
	   					int status;
	   					int x = waitpid(retfork, &status, 0);
	   					printf("%d\n",x );
	   				}
	   			}
   			}	
   			continue;
        }

        if(strcmp(tokens[0], "getpl") == 0){
        	int num;
        	for(i = 0; tokens[i] != NULL; i++);
   			if(i < 2){
   				printf("%s\n", "Usage : getpl <filename1> <filename2> ...");
   			}else{
   				if(!server_saved){
   					printf("%s%s\n", "Server-ip and port have not been saved.\n",
   						"To save : server <server-ip> <port>");
   					continue;
   				}
   				for(num = 1; num < i && !stop_cmd; num++){
	   				getpl_pid[num-1] = fork();
	   				if(getpl_pid[num-1] == 0){
	   					execl("./get-one-file-sig","./get-one-file-sig",tokens[num],
	   							 server.ip, server.port, "nodisplay", (char*)NULL);
	   				}else{
	   					pthread_create(&getpl_name[num-1], NULL, wait_thread, (void*)&getpl_pid[num-1]);
	   				}
	   			}
	   			for(num = 1; num < i && !stop_cmd; num++){
	   				pthread_join(getpl_name[num-1], NULL);
	   			}
   			}	
   			continue;
        }

        // Next if is for shell built-in's
        char *bin = "/bin/";
        char path_builtin[4+strlen(tokens[0])];
        strcpy(path_builtin, bin);
        strcat(path_builtin, tokens[0]);
        // printf("%s\n", path_builtin);
        if(exists(path_builtin)){
        	retfork = fork();
        	if(retfork == 0){
        		execv(path_builtin, &(tokens[0]));
        	}else{
        		int status;
        		waitpid(retfork, &status, 0);
        	}
        }else{
        	printf("%s\n", "Executable doesn't exist in /bin");
        	continue;
        }

        // Freeing the allocated memory	
        for(i=0;tokens[i]!=NULL;i++){
	    	free(tokens[i]);
        }
       	free(tokens);
    } 
     

}

