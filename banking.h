#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <signal.h>



typedef struct Account{
	char user_Name[255]; //User's account name
	float cur_Balance; //User's current balance
	int isInSession; //Boolean value of in a session or not
} acc;

/*Used for coonections*/
typedef struct connection{
	int socketLink; //stores socket
	struct sockaddr addr; //address
	socklen_t addr_len; //length of address
} conn;

//
struct node{
	int thisSocket;
	struct node *next;
}*head;
