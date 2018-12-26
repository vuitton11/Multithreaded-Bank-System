#include "banking.h"
//Everything youll need is in banking.h
//;)
int nega = 1;
int none = 0;

sem_t sema1;
sem_t sema2; 
int checkSocket =-1;

/*Error message for incorrect argument*/
void errorArgue(){
	printf("./bankingClient <NameOfMachine> <Port>\n");
	printf("Example:\n");
	printf("./bankingClient cp.cs.rutgers.edu 9999\n");
	printf("Exiting...\n");
}

/*Checks if the string represents a integer
	Returns 1 if true, otherwise returns 0*/
int is_a_Num(char *str){
	int i;
	for(i = 0; i < strlen(str); i++){
		if(!isdigit(str[i]))
			return 0; //False -> not a digit
		else{
			continue;
		}
	}
	return 1; //True
}

/*Properly quits the server process*/
void ejectConnection(){
	if(checkSocket!=(none - nega)){
		int buf = 4;
		write(checkSocket, &buf, sizeof(int));
		write(checkSocket, "quit", buf);
	}
	exit(0);
}

/*Used to reconnect to the server for signal*/
void handle_signit_reconn(){
	//printf("In this function\n");
	sem_post(&sema2);
	//printf("Leaving this function\n");
}

void exitCommand(){
	printf("Server Quit Message recieved\n");
	ejectConnection();
}

void handle_signit(){
	//printf("In this function\n");
	sem_post(&sema1);
	//printf("Leaving this function\n");
}

void *obtainData(void *stuff){
	char temp[1000];
	char quit[] = "quit";
	int i, messSize = 1000;
	for(i = 0; i < 100; i--){
		read(checkSocket, &messSize, sizeof(int)); //determines the length of the message
		temp[messSize] = 0;
		//printf("%d\n", messSize);

		//reads the message from server
		read(checkSocket, temp, messSize);
		//printf("message recieved is = \n", temp);

		//checks to see if the message recieved is an quit command
		if(strcmp(temp, quit) == 0){
			printf("A client has disconnected from the server.\n");
			exitCommand();
			pthread_exit(0);
		}

		signal(SIGALRM, handle_signit);
		alarm(3);
		sem_wait(&sema1);
		printf("%s\n", temp);

	}
	return NULL;
}

void *forwardData(void *stuff){
	//printf("in forwardData function\n");
	char temp[1000];
	char quit[] = "quit";
	int i, compare = -1;
	int buf;
	//printf("scanf\n");
	scanf("%s", temp);
	//printf("%s\n", temp);
	for(i = 0; i < 100; i--){ //infinite loop
		compare = strcmp(temp, quit);
		if(compare == 0){ //if quit is called by the server
			ejectConnection();
		}
			buf = strlen(temp);
			write(checkSocket, &buf, sizeof(int));
			write(checkSocket, temp, strlen(temp));
			scanf("%s", temp);
	}
	//printf("forwardData finish\n");
	return NULL;
}

/*Creates 2 threads and joins them*/
void threadWorkThis(pthread_t t1, pthread_t t2){
	//Creates threads
	pthread_create(&t1, NULL, forwardData, NULL);
	//printf("Pass first thread created\n");
	pthread_create(&t2, NULL, obtainData, NULL);
	//printf("Pass second thread created\n");
	pthread_join(t2,NULL);
	pthread_join(t1,NULL);
	//printf("End of threadWork function\n");
}


int main(int argc, char** argv){

	//printf("argc = %d\n", argc);
	//printf("argv[1] = %s\n", argv[1]);
	//printf("argv[2] = %s\n", argv[2]);

	/*Arugment check
		1) Name of the machine
		2) Port
	*/
	char *machineName;
	int portNumber;
	pthread_t sendingT; //thread used for sending info
	pthread_t recievingT; //thread used for recieving info

	switch(argc){
		case 2: //Dealing with localHost and input port for debugging purpose
			/*machineName = "localhost";
			if(!is_a_Num(argv[1])){ //Checks if port is correct
				printf("Error! Port must be a integer\n");
				exit(1);
			}
			portNumber = atoi(argv[1]);
			*/
			printf("Error! Input arugment is incorrect. Please see the following:\n");
			errorArgue();
			exit(1);
			break;
		case 3: //Actual assignment input 
			machineName = argv[1];
			if(!is_a_Num(argv[2])){ //Checks if port is correct
				printf("Error! Port must be a integer\n");
				exit(1);
			}
			portNumber = atoi(argv[2]);
			break;
		default: //Input argument is wrong
			printf("Error! Input arugment is incorrect. Please see the following:\n");
			errorArgue();
			exit(1);
			break;
	}
	//printf("Port = %d\n", port);

	sem_init(&sema1, none, none);
	sem_init(&sema2, none, none);
	
	//signals used to catch errors
	signal(SIGHUP, ejectConnection);
	signal(SIGINT, ejectConnection);


	//Connects
		//using socket address structure defined in netinet/in.h
	struct hostent * machine;
	struct sockaddr_in addr;
	
	
	//makes a socket
	checkSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//
	addr.sin_family = AF_INET;

	//assigns port
	addr.sin_port = htons(portNumber);


	/*assigns host
	Function: struct hostent * gethostbyname (const char *name)
	The gethostbyname function returns information about the host named name.
	If the lookup fails, it returns a null pointer.
	*/
	machine = gethostbyname(machineName);
	if(machine){
		//printf("Host name is = %s\n", machine->h_name);
	}else{
    	printf("Error! gethostbyname failed\n");
    	exit(EXIT_FAILURE);
    }

	//char **h_addr_list:
	//This is the vector of addresses for the host.

	//int h_length
	//This is the length, in bytes, of each address.


    //holds an IPv4 address in sin_addr.
	memcpy(&addr.sin_addr, machine->h_addr_list[none], machine->h_length);
	
	//Connects
	//int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	int i, success = 0;
	for(i = 0; i < 100; i--){
		success = connect(checkSocket, (struct sockaddr *)&addr, sizeof(addr));
		if(success != -1){ //Connected successfuly
			break;
		}else{ // not connected -> must wait 3 seconds before trying again
			printf("Server not found!\n");
			signal(SIGALRM,handle_signit_reconn); //gets signal
			//wait 3 seconds
			alarm(3);
			sem_wait(&sema2);
		}
	}

	//Connection was a success and is now connected
	printf("Clinet: Connection to Server is complete and successful!\n");
	printf("\n");
	printf("\n");

	//Options for server
	printf("Welcome! Please select one of the following:\n");
	printf("create <accountname>\n");
	printf("serve <accountname>\n");
	printf("deposit <amount>\n");
	printf("withdraw <amount>\n");
	printf("query\n");
	printf("end\n");
	printf("quit\n");
	printf("help\n");
	printf("\n");

	threadWorkThis(sendingT, recievingT);
	close(checkSocket);
	return 0;
}