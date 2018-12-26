#include "banking.h"
//Everything youll need is in banking.h
//;)
sem_t sema1;
sem_t sema2;
int checkSocket;
pthread_mutex_t sLock;
pthread_mutex_t lock12;
pthread_mutex_t lock11;
pthread_t thr1;
pthread_t thr2;
pthread_t thr3;
acc *listOfAcc[50];
int totalAccNum = 0, trueVar = 1;

void errorArgue(){
	printf("./bankingServer <Port>\n");
	printf("Example:\n");
	printf("./bankingServer 9999\n");
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

void handle_signit(){
	//printf("In this function\n");
	sem_post(&sema1);
	//printf("Leaving this function\n");
}


/*Properly exits the server process
	Iterates through the link list of socket and kills
	the connection with every client.
	*/
void ejectConnection(){
	struct node * socket = head;
	int i;
	for(i = 0; i < 100; i--){
		if(socket == NULL){
			//printf("reached last node\n");
			break;
		}else{
			int buf = 4;
			write(socket->thisSocket, &buf, sizeof(int)); //write enough space
			write(socket->thisSocket, "quit", strlen("quit")); //write quit
			close(socket->thisSocket); //close socket
			socket = socket->next;// iterate to the next node in the list
		}
	}
	exit(0);
}

//checks to see what the input of the string is to use in switch statement
/*individual Inputs result in different outputs
	create = 1
	serve = 2
	deposit = 3
	withdraw = 4
	query = 5
	end = 6
	quit = 7
	help = 8
	incorrect input = -1 //Test case
	*/
int inputEquals(char* input){
	if(strcmp(input, "create") == 0){
		return 1;
	}else if(strcmp(input, "serve") == 0){
		return 2;
	}else if(strcmp(input, "deposit") == 0){
		return 3;
	}else if(strcmp(input, "withdraw") == 0){
		return 4;
	}else if(strcmp(input, "query") == 0){
		return 5;
	}else if(strcmp(input, "end") == 0){
		return 6;
	}else if(strcmp(input, "quit") == 0){
		return 7;
	}else if(strcmp(input, "help") == 0){
		return 8;
	}else{
		return -1;
	}
	return -1;
}

/*Does the actual bulk of the assignment
 *Works the online bank with the options provided in the assignment plus help
 *Handles input form users and what needs to be done
 *Used in thread
 */
void* work(void *stuff){ 
	//printf("in this function\n");
	int i, strLen = 1200, inputVal;
	int currActive = -1; //0 if inactive, otherwise 1 is active
	int active = 0; //if a user is active
	char temp[1200]; //it is big just in case of bad inputs
	int checkTHIS = 0; //used for create case -> if there is a user in the system
	//char temp1[1200]; //holds next thing
	conn * link = (conn*) stuff; //converts content of thread back
	int j;
	char helpMessage[1200];
	char wrongInput[1200];
	int helpLen;
	int v = 0;
	for(v = 0; v < 100; v--){
		//Read what is being sent
		read(link->socketLink, &strLen, sizeof(int));


		temp[strLen] = 0;



		read(link->socketLink, temp, strLen);

		//Check what option the user choose
		//temp1 = strlwr(temp);
		for(j = 0; temp[j]; j++){
 			temp[j] = tolower(temp[j]);
 		}

		//funtion to check input
		inputVal = inputEquals((temp));

		//isInSession takes in 2 -> inSession, otherwise 0 -> not insession

		switch(inputVal){
			case -1: //error -> wrong input 
				strcpy(wrongInput, "Error! This is not a proper input! Please select the following options above\n");
				helpLen = strlen(wrongInput);
				write(link->socketLink, &helpLen, sizeof(int));
				write(link->socketLink, wrongInput, strlen(wrongInput));
				break;
			case 1: //create 
				/*Steps into making create
					*Check 1 - if the create is entered by quit -> quit program
					*Check 2 - if the account name exceds limit of 255 chars -> error
					*Check 3 - if there is no account name followed by create -> error
					*Check 4 - if the account name already exist -> if so send message that there is one
					*Check 5 - if the account name isnt in the system -> create it*/
				//printf("this create switch\n");
				//reads message
				read(link->socketLink, &strLen, sizeof(int));
				temp[strLen] = 0;
				read(link->socketLink, temp, strLen);
				if(strcasecmp(temp, "quit") == 0){ //check 1) if quit is presented
					if(currActive == -1){
						break;
					}
					else if(active == 1){
						listOfAcc[currActive]->isInSession = 0;
						break;
					}
					break;
				}
				pthread_mutex_lock(&lock11);
				if(strlen(temp) > 255){ //check 2) string length requirement
					pthread_mutex_unlock(&lock11);
					char buffer11[1200]= "Account name must be 255 characters or less\n";
					strLen=strlen(buffer11);
					write(link->socketLink, &strLen, sizeof(int));
					write(link->socketLink, buffer11, strLen);
					break;
				}
				else if(temp[0] == '\0'){ //check 3) no account name is given
					//printf("Does this work?\n");
					//not sure if it does
					pthread_mutex_unlock(&lock11);
					char buffer11[1200]= "Please include an account name\n";
					strLen=strlen(buffer11);
					write(link->socketLink, &strLen, sizeof(int));
					write(link->socketLink, buffer11, strLen);
					continue;
				}else{
					int k = 0;
					//iterates thru the accounts to check if the name already exisit 
					for(k = 0; k < totalAccNum; k++){ 
						if(listOfAcc[k] == NULL){ //reached the last index
							checkTHIS = 0; //name does not exist
							break;
						}else if(strcasecmp(temp, listOfAcc[k]->user_Name) == 0){// check 4)name is found
							pthread_mutex_unlock(&lock11); 
							checkTHIS = 1; //name exist
							//sends message over about the found user
							char buffer11[1200]= "This account already exist in the system\n";
							strLen=strlen(buffer11);
							write(link->socketLink, &strLen, sizeof(int));
							write(link->socketLink, buffer11, strLen);
							break;	
						}
					}
					//When the user name isnt found so we then create an account with the following username
					if(checkTHIS  == 0){
						acc *createAcc = malloc(sizeof(acc));
						strcpy(createAcc->user_Name, temp);
						createAcc->cur_Balance = 0.0;
						createAcc->isInSession = 0;
						listOfAcc[totalAccNum] = createAcc;
						totalAccNum++;
						pthread_mutex_unlock(&lock11);
						char buffer11[1200]= "New Account has successfully been added\n";
						strLen=strlen(buffer11);
						write(link->socketLink, &strLen, sizeof(int));
						write(link->socketLink, buffer11, strLen);
					}
				}
				//printf("finish with create\n");
				break; 
			//--------------------------------------------------------------------------------
			case 2: //serve
				/*steps for server
					We know that it is not possible to start more than one service session in any single client
					*Check 1 - does the account exist, if so is it already running (CHECK ISINSESSION FLAG)
					if running -> print the account is alreeady in a session, otherwise put account in session
					*Check 2 - if the account doesnt exist -> send message about the account not in the system*/
				//printf("this SERVE switch\n");
				//reads input
				read(link->socketLink, &strLen, sizeof(int));
				temp[strLen] = 0;
				read(link->socketLink, temp, strLen);
				//printf("case 2: = %s\n", temp);
				if(strcasecmp(temp, "quit") == 0){ //if quit is presented
					if(active == 1){
						listOfAcc[currActive]->isInSession = 0;
						break;
					}
					break;
				}

				//Checks to see if someone is already in serve
				if(active == 1){
					//printf("account is in sess already\n");
					char accAlreadyIn[1200]= "There is already another account in serve. Please wait for the other user to finish\n";
					int accAllLen = strlen(accAlreadyIn);
					write(link->socketLink, &accAllLen, sizeof(int));
					write(link->socketLink, accAlreadyIn, accAllLen);
					break;
				}


				int q = 0; 
				//iterates thru the list to find the account
				/*Another test case is when we have no user than we need to say there is no user in the system
				 *Thus, the account isnt in the System*/
				if(totalAccNum == 0){ //acounts
					char noAccFoundSer[1200]= "The account was not found in the system. Please try again or add the account\n";
					int noAccLen = strlen(noAccFoundSer);
					write(link->socketLink, &noAccLen, sizeof(int));
					write(link->socketLink, noAccFoundSer, noAccLen);
					break;
				}
				int checkEnd = 0;
				//printf("total accounts = %d\n", totalAccNum);
				for(q = 0; q < totalAccNum; q++){
					if(listOfAcc[q] == NULL){ //end of the list
						//check 2) no account name found
						//printf("This is no account info\n");
						char noAccFoundSer[1200]= "The account was not found in the system. Please try again or add the account\n";
						int noAccLen = strlen(noAccFoundSer);
						write(link->socketLink, &noAccLen, sizeof(int));
						write(link->socketLink, noAccFoundSer, noAccLen);
						checkEnd = 1;
						break;
					}else if(strcmp(temp, listOfAcc[q]->user_Name) == 0){ //Name is found, now we check status of IsInSession
						//check iSInSession
						pthread_mutex_lock(&lock12);
						if(!listOfAcc[q]->isInSession){ //not in session
							//printf("This account is not in session\n");
							listOfAcc[q]->isInSession = 1;
							currActive = q; //Saves the current user
							active = 1;
							pthread_mutex_unlock(&lock12);
							//print messesage
							char startSess[1200] = "This account is now in session!\n";
							strLen=strlen(startSess);
							write(link->socketLink, &strLen, sizeof(int));
							write(link->socketLink, startSess, strLen);
							checkEnd = 1;
							break;
						}else{ //already in service
							pthread_mutex_unlock(&lock12);
							char alreadySes[1200] = "This account is already in a session\n";
							strLen = strlen(alreadySes);
							write(link->socketLink, &strLen, sizeof(int));
							write(link->socketLink, alreadySes, strLen);
							checkEnd = 1;
							break;
						}
					}
				}
				//printf("Pass function\n");
				//Iff reached to this point account wasnt found -> this fixed the error
				//printf("This is no account info\n");
				if(!checkEnd){
					char noAccFoundSer[1200]= "The account was not found in the system. Please try again or add the account\n";
					int noAccLen = strlen(noAccFoundSer);
					write(link->socketLink, &noAccLen, sizeof(int));
					write(link->socketLink, noAccFoundSer, noAccLen);
				}
				//
				
				break;
			//--------------------------------------------------------------------------------
			case 3: //deposit
				//printf("this deposit switch\n");
				//reads input
				read(link->socketLink, &strLen, sizeof(int));
				temp[strLen] = 0;
				read(link->socketLink, temp, strLen);
				//printf("case 3: = %s\n", temp);
				//needs to find the account and proceed with //actually not really, just check if the user is
				//in session
				if(active == 0){
					//inactive account
					char inactive[1200] = "There is no account in session\n";
					strLen = strlen(inactive);
					write(link->socketLink, &strLen, sizeof(int));
					write(link->socketLink, inactive, strLen);
					break;
				}else if(strcasecmp(temp, "quit") == 0){ //if quit is presented
					if(active == 1){
						listOfAcc[currActive]->isInSession = 0;
						break;
					}
					break;
				}else{ //check if the amount is not negative
					if(atof(temp) < 0){
						char negativeAmount[1200] = "The amount of money to deposit should not be negative. Please try again.\n";
						strLen = strlen(negativeAmount);
						write(link->socketLink, &strLen, sizeof(int));
						write(link->socketLink, negativeAmount, strLen);	
					}else{ //add money to the account
						char moneyAdded[1200];
						listOfAcc[currActive]->cur_Balance += atof(temp);
						//write messages
						snprintf(moneyAdded, sizeof(moneyAdded), "%f was added to your account.\n", atof(temp));
						strLen = strlen(moneyAdded);
						write(link->socketLink, &strLen, sizeof(int));
						write(link->socketLink, moneyAdded, strLen);
					}

				}
				break;
			//--------------------------------------------------------------------------------
			case 4: //withdraw 
							//reads input
				read(link->socketLink, &strLen, sizeof(int));
				temp[strLen] = 0;
				read(link->socketLink, temp, strLen);
				//printf("case 3: = %s\n", temp);
				//needs to find the account and proceed with //actually not really, just check if the user is
				//in session
				if(active == 0){
					//inactive account
					char inactive[1200] = "There is no account in session\n";
					strLen = strlen(inactive);
					write(link->socketLink, &strLen, sizeof(int));
					write(link->socketLink, inactive, strLen);
					break;
				}else if(strcasecmp(temp, "quit") == 0){ //if quit is presented
					if(active == 1){
						listOfAcc[currActive]->isInSession = 0;
						break;
					}
					break;
				}else{ //check if the amount is not negative
					if(atof(temp) < 0){
						char negativeAmount[1200] = "The amount of money to withdraw should not be negative. Please try again.\n";
						strLen = strlen(negativeAmount);
						write(link->socketLink, &strLen, sizeof(int));
						write(link->socketLink, negativeAmount, strLen);	
					}else{ //add money to the account
						char moneyAdded[1200];
						float tempMoney = listOfAcc[currActive]->cur_Balance;
						if((tempMoney - atof(temp)) < 0){ //negative account
							char noEnough[1200] = "This transaction can not be made because there is not enough money in the account\n";
							strLen = strlen(noEnough);
							write(link->socketLink, &strLen, sizeof(int));
							write(link->socketLink, noEnough, strLen);
						}
						listOfAcc[currActive]->cur_Balance -= atof(temp);
						//write messages
						snprintf(moneyAdded, sizeof(moneyAdded), "%f was taken from your account.\n", atof(temp));
						strLen = strlen(moneyAdded);
						write(link->socketLink, &strLen, sizeof(int));
						write(link->socketLink, moneyAdded, strLen);
					}

				}
				break;
			//--------------------------------------------------------------------------------
			case 5: //query 
				if(active == 0){
					//inactive account
					char inactive[1200] = "There is no account in session\n";
					strLen = strlen(inactive);
					write(link->socketLink, &strLen, sizeof(int));
					write(link->socketLink, inactive, strLen);
					break;
				}else{
					char curBal[1200];
					float totalInACC = listOfAcc[currActive]->cur_Balance;
					snprintf(curBal, sizeof(curBal), "The current balance in the account is %f\n", totalInACC);
					strLen = strlen(curBal);
					write(link->socketLink, &strLen, sizeof(int));
					write(link->socketLink, curBal, strLen);
				}
				break;
			//--------------------------------------------------------------------------------
			case 6: //end //finish
				if(active == 0){
					//inactive account
					char inactive[1200] = "There is no account in session\n";
					strLen = strlen(inactive);
					write(link->socketLink, &strLen, sizeof(int));
					write(link->socketLink, inactive, strLen);
					break;
				}
				else{
					active = 0;
					listOfAcc[currActive]->isInSession = 0;
					currActive = -1;
					char freed[1200] = "This account is no longer in session.\n";
					strLen = strlen(freed);
					write(link->socketLink, &strLen, sizeof(int));
					write(link->socketLink, freed, strLen);
				}
				break;
			//--------------------------------------------------------------------------------
			case 7: //quit 
				if(active == 1){ //checks if there is a active session
					if(listOfAcc[currActive]->isInSession != 0){ // if insession, set no longer in session
						listOfAcc[currActive]->isInSession = 0;
					}
				}
					//printf("Client: Disconnenting from server now...\n");
					//char quitMess[1200] = "Client has Disconnenting from Server.\n";
					//strLen = strlen(quitMess);
					//write(link->socketLink, &strLen, sizeof(int));
					//write(link->socketLink, quitMess, strLen);

				break;
			//--------------------------------------------------------------------------------
			case 8: //help
				//printf("Help Option:\n");
				strcpy(helpMessage, "\nHelp info:\nCreate: creates a new account for the bank.\nServe: starts a service session for a specific account.\ndeposit/ withdraw: add/ subtract amounts from an account balance.\nThe query command simply returns the current account balance.\nThe end command ends the current service session.\nQuit: disconnects the client from the server.\nEnd of help!\n");
				helpLen = strlen(helpMessage);
				write(link->socketLink, &helpLen, sizeof(int));
				write(link->socketLink, helpMessage, strlen(helpMessage));
				/*helpMessage[0] = 0;
				strcpy(helpMessage, "Serve: starts a service session for a specific account.");
				helpLen = strlen(helpMessage);
				write(link->socketLink, &helpLen, sizeof(int));
				write(link->socketLink, helpMessage, strlen(helpMessage));
				helpMessage[0] = 0;
				strcpy(helpMessage, "deposit/ withdraw: add/ subtract amounts from an account balance.");
				helpLen = strlen(helpMessage);
				write(link->socketLink, &helpLen, sizeof(int));
				write(link->socketLink, helpMessage, strlen(helpMessage));
				helpMessage[0] = 0;
				strcpy(helpMessage, "The query command simply returns the current account balance.");
				helpLen = strlen(helpMessage);
				write(link->socketLink, &helpLen, sizeof(int));
				write(link->socketLink, helpMessage, strlen(helpMessage));	
				helpMessage[0] = 0;	
				strcpy(helpMessage, "The end command ends the current service session.");
				helpLen = strlen(helpMessage);
				write(link->socketLink, &helpLen, sizeof(int));
				write(link->socketLink, helpMessage, strlen(helpMessage));
				helpMessage[0] = 0;
				strcpy(helpMessage, "Quit: disconnects the client from the server.");
				helpLen = strlen(helpMessage);
				write(link->socketLink, &helpLen, sizeof(int));
				write(link->socketLink, helpMessage, strlen(helpMessage));
				helpMessage[0] = 0;
				*/
				break;
			//--------------------------------------------------------------------------------
			default:
				//printf("Something\n");
				strcpy(wrongInput, "Error! This is not a proper input! Please select the following options above\n");
				helpLen = strlen(wrongInput);
				write(link->socketLink, &helpLen, sizeof(int));
				write(link->socketLink, wrongInput, strlen(wrongInput));

		}

	}
	//Client has been asked to quit
	printf("Existing client now...\n");
	
	//Closing the socket
	close(link->socketLink);

	//Free memory to avoid free errors
	free(link);

	//exit thread
	pthread_exit(0);
}

/*This function prints the list of every account
	for every 15 seconds*/
void *listAllAcc(void *stuff){
	int i;
	int j;
	for(i = 0; i < 100; i--){
		signal(SIGALRM, handle_signit);

		//wait 15 seconds
		alarm(15);
		//waits
		sem_wait(&sema1);
		printf("List of Accounts\n");
		for(i = 0; i < totalAccNum; i++){
			if(listOfAcc[i] == NULL){ //End of list
				break;
			}else{
				/*The information
				printed for each account will include the following:
				account name, a tab, current balance, a tab, and “INSERVICE” 
				if there is an account session open for that particular account.*/
				if(!listOfAcc[i]->isInSession){ //Not in session
					printf("%s\t%f",listOfAcc[i]->user_Name, listOfAcc[i]->cur_Balance);
					printf("\n");
				}else{ //This account is in service
					printf("%s\t%f\t%s",listOfAcc[i]->user_Name, listOfAcc[i]->cur_Balance, "IN SERVICE");
					printf("\n");
				}
			}
		}
	}
	pthread_exit(0);
}

/*Takes in the return value of accept()
 and determines if it was successful or not
 0 if successful, otherwise 1*/
int checkAcceptOutput(int accept){
	if(accept > 0) return 0; //Success
	else return 1; //Failed
}

void beginToHear(int thisSocket){
	int n = 3;
	int i = 0;
	for(i = 0; i < 100; i--){
		conn *link = (conn*)calloc(n, sizeof(conn));
		if(!link){ //If calloc failed, try again
			printf("calloc Failed\n");
			free(link);
			continue;
		}
		//set socket by using accept() to accept a connection request
		//from client
		link->socketLink = accept(thisSocket, &link->addr, &link->addr_len);
		//Checks to see if accept was successful
		//If successful, accept() returns a nonnegative socket descriptor.
		//If unsuccessful, accept() returns -1 and sets errno to one of the following values:
		if(checkAcceptOutput(link->socketLink) == 0){ //Success
			printf("Server: Connection with Client has been Accepted\n");
			//Create a thread
			pthread_create(&thr2, 0, work, (void *)link);

			//printf("Finished with create\n");
			
			pthread_detach(thr2);

			//printf("Finished with detach\n");

			struct node *holder = malloc(sizeof(struct node));

			if(holder == NULL){ //failed to allocate memory
				//printf("Malloc Failed\n"); //deal with this
			}

			//Continue
			holder->thisSocket = link->socketLink;
			
			//printf("socketLink = %d\n", link->socketLink);
			//printf("thisSocket = %d\n", holder->thisSocket);
			
			holder->next = head;
			
			head = holder;

		}else if(checkAcceptOutput(link->socketLink) == 1){ //failed
			printf("Connection with client has failed\n");
			free(link);
		}
	}

}

//creates and handles thread
void threadWork(pthread_t thr1, pthread_t thrTemp, int checkS){
	pthread_create(&thr1, NULL, listAllAcc, NULL);
	//printf("Passed\n");
	//Detach
	pthread_detach(thr1);
	//printf("Passed detach function\n");
	
	//Listens for connections
	beginToHear(checkS);
}

int main(int argc, char **argv){
	int portNumber, none = 0, listenPort = 5;
	struct sockaddr_in addr;

	/*Checking argument
	 *The server takes the port to listen on as the only argument
	 */
	if(argc != 2){
		printf("Error! Arugment is incorrect.\n");
		errorArgue();
		exit(1);
	}

	if(!is_a_Num(argv[1])){ //Checks if port is in a correct format
		printf("Error! Port must be a integer\n");
		exit(1);
	}
	//Saves port number
	portNumber = atoi(argv[1]);

	/*Information on how to use mutex to synchronize threads from geeksforgeeks-
	A mutex is initialized and then a lock is achieved by calling the following two functions: 
	The first function initializes a mutex and through second function any critical region in the code can be locked.

	1) int pthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr):
	Creates a mutex, referenced by mutex, with attributes specified by attr. If attr is NULL, the default mutex attribute (NONRECURSIVE) is used.
	Returned value
	If successful, pthread_mutex_init() returns 0, and the state of the mutex becomes initialized and unlocked.
	If unsuccessful, pthread_mutex_init() returns -1.*/

	//Moving on to the actual work
	if(pthread_mutex_init(&lock11, NULL)){ //mutex failed
		printf("\n mutex init has failed\n"); 
        exit(1);
	}
	//printf("pass mutex init 1\n");
	if(pthread_mutex_init(&sLock, NULL)){ //mutex failed
		printf("\n mutex init has failed\n"); 
        exit(1);
	}
	//printf("pass mutex init 2\n");

	//semaphore is initialised
	sem_init(&sema1, none, none);
	//printf("Pass\n");
	signal(SIGHUP, ejectConnection);
	signal(SIGINT, ejectConnection);

	//Creates socket
	checkSocket = socket(AF_INET, SOCK_STREAM, none);

	//Next we need to prep our info so we can bind the socket and port
	addr.sin_family = AF_INET;
	
	//assigns address
	addr.sin_addr.s_addr = INADDR_ANY;
	
	//assigns port
	addr.sin_port = htons(portNumber);

	//shall assign a local socket address address to a socket identified by descriptor 
	//socket that has no local socket address assigned.
	//int bind(int socket, const struct sockaddr *address,socklen_t address_len);
	if(bind(checkSocket, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1){ //Bind failed
		printf("Error! Bind Failed\n");
		exit(EXIT_FAILURE);
	}	

	//Must listen -> takes the port to listen on 
	//listen() marks the socket referred to by sockfd as a passive socket
	if (listen(checkSocket, listenPort) == -1){
       	printf("Error! Listen Failed\n");
		exit(EXIT_FAILURE);
	}

	//After all this now a server can accept client connections
	//boys we got it!
	printf("Server is up! Accepting a client connection\n");


	//Function for thread works call
	threadWork(thr1, thr3, checkSocket);
	
	return 0;
}


