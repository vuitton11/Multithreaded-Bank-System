# Multithreaded-Bank-System
A server program to emulates a bank and a client program to communicate with it 

## ![](http://icons.iconarchive.com/icons/devcom/network/48/server-Vista-icon.png)Server

There is  thread that responds back to the client when the client sends a request to the server.

Our Server holds all the data and bank accounts.


The threads were created asynchronizaly which means that threads don’t interfere with one another.


There is no need to wait for an input on the client, therefore the server can send a message, and otherwise, both sides will have to wait.


When the client sends a command to the server the **tokenizer** file will split the command string into tokens then analyzed based on the possible valid commands for the server.

* Open
* Start 
* debit 
* credit 
* balance 
* Exit

If the string construction is valid we proceed, otherwise, we prompt an error “invalid command”


## ![](https://cdn2.iconfinder.com/data/icons/amazon-aws-stencils/100/Non-Service_Specific_copy_Client-64.png)Client

Client can use open accountName to open a new account and that creates an account on the server with everything initialized
to 0.

If the client chooses to start, they must use type Start and then use their accountName that was created. 

Client will not be able to open an account while another client is opening an account as it’s going to
prompt you to try again.

Client will not be able to open a non-existed account as the server will prompt a message “account doesn’t
exist”

The server max is 20 accounts If the server disconnects, the client gets disconnected automatically.

Our client has a user interface, which the user uses as a tool to connect to the server. It’s as simple as a
terminal that takes commands and sends them over to the server.

We have two threads in our client. The First one sends the command to the server as inputs.
The second thread takes care of the messages coming from the server to the user as a result of the
command that is being initially sent by the first thread.



##![](https://cdn2.iconfinder.com/data/icons/circle-icons-1/64/unlocked-48.png) Mutex Lock

All accounts that get stored in the server will be mutex lock protected. A client that accesses their account gets locked, once desired computation gets proccessed the account gets unlocked. No one will be able to access a single account at the same time.   Other clients will be prompted to wait before entering the accoutn. 
