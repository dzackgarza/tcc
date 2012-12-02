#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>



int main(int argc, char **argv)
{
	
    struct sockaddr_in sa;
    int sockfd;

    if (argc < 3) 		// IP address and login name must be passed in as arguments to connect
    {
        fprintf(stderr, "Usage: %s ip name\n", argv[0]);
	fprintf(stderr, "Example: %s 198.58.101.64 zack\n", argv[0]);
	exit(1);
    }

    sa.sin_family = AF_INET;
    sa.sin_port = htons(1234); 				// Assigns the port
    inet_pton(AF_INET, argv[1], &sa.sin_addr);
    
    sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);	// Create socket and catch errors
    if (sockfd == -1) {
        fprintf(stderr, "Can't create socket\n");
	exit(3);
    }

    int res = connect(sockfd, (struct sockaddr *)&sa, sizeof(sa));
    if (res == -1) {
        fprintf(stderr, "Can't connect\n");
	exit(2);
    }

   	char s[100]; 			// Temp string used to hold commands to send to server. sprintf your commands into this.
    	char buf[1000]; 		// Holds the server's response. Use in recv()
   	size_t size;
	//char result[100];

    	sprintf(s, "LOGIN %s\n", argv[2]);
    	send(sockfd, s, strlen(s), 0);		// Send LOGIN command (LOGIN 'name')
	size = recv(sockfd, buf, 1000, 0);	// Get the response from the server, storing it in buf
	puts("\nLogging in..");
	
				
	if (*buf ==  '-') // All errors begin with '-'. Checks for them
		// If this point is reached, we are connected - therefore, the only possible error is "name taken"
	{  
		puts("Login failed - Name taken, please choose another."); 
		exit(3); 
	}

	else if (*buf == '+') puts(buf+4); 			// Since there was no error, print the server's response
	
	// Print a menu of options
	puts("Type in a message to chat.\n");
	puts("Commands:\n'/l': List logged in users \n'/q': View queue size \n'/quit': Logout\n");
	printf(">");

	char command[256];	// Raw text that was typed in
	char msgrec[256];	// Holds messages received from other users
	char msgsend[256];	// Holds your command, minus the trailing newline
	//int i = 0;

	while(fgets(command, 256, stdin) != NULL) { 		// Get a command from stdin and loop as long as it isn't 'exit'.
		
		// Flush the buffers
		memset(buf, 0, sizeof(buf)); 
		memset(msgsend, 0, sizeof(msgsend));
		memset(msgrec, 0, sizeof(msgrec));

		sscanf(command, "%[^\n]", msgsend);	// Stores your command in msgsend, stripping off the newline.

		if( strcmp(msgsend, "/quit") == 0) {
			puts("Quitting..");
			sprintf(s, "QUIT\n");

			send(sockfd, s, strlen(s), 0); 		// Send QUIT command
			size = recv(sockfd, buf, 1000, 0); 	// What is the server's response?

			puts("Logged out.");
			exit(0);
		}

		else if( strcmp(msgsend, "/l") == 0)	{
			puts("Listing..");
			sprintf(s, "LIST\n");

			send(sockfd, s, strlen(s), 0); 		// Send LIST command
			size = recv(sockfd, buf, 1000, 0); 	// Get response

			printf("%s",buf+4); 	// +4 is used to skip over the status message (eg '+ OK' or '- ERR')
		}

		else if( strcmp(msgsend, "/q") == 0) {
			int q = 0;

			sprintf(s, "QSIZE\n");

			send(sockfd, s, strlen(s), 0);		// Send QSIZE command
			size = recv(sockfd, buf, 1000, 0); 	// Receive QSIZE

			sscanf(buf, "%s %d\n", msgrec, &q);	
			// Search the server's response. Grab the status msg and a number representing the size of the queue.

			printf("Queue size: %d\n", q);
		}

//		else if( strcmp(msgsend, "R") == 0) {
//		}
		else if( strcmp(msgsend, "") == 0) {
			int q = 0;				// Holds the size of the queue
			sprintf(s, "QSIZE\n");		
			send(sockfd, s, strlen(s), 0);		// Send QSIZE command
			size = recv(sockfd, buf, 1000, 0); 	// Receive server response

			sscanf(buf+3, "%d\n", &q);
				// Search the server's response. Grab the number representing the size of the queue.
			if (q != 0) puts("*** Messages in queue ***"); else puts("*** No New Messages ***");
			while (q>=1) { 				// While there are msgs in the q, print them
				//memset(msgrec, 0, sizeof(msgrec));	// Clear the previous message.


				sprintf(s, "RECV\n");		// Receive the next message
				send(sockfd, s, strlen(s), 0);

				size = recv(sockfd, buf, 1000, 0);
				sscanf(buf, "+OK %[^\n]", msgrec);	// Grab the next message
				printf("%d: '%s'\n", q, msgrec);	// Print the message
				q--;					// Since we printed one queue item

			}
			
		}
		else {
		// First, grab the queue size so we know if there are messages waiting to be received/
			int q = 0;				// Holds the size of the queue
			sprintf(s, "QSIZE\n");		
			send(sockfd, s, strlen(s), 0);		// Send QSIZE command
			size = recv(sockfd, buf, 1000, 0); 	// Receive server response

			sscanf(buf+3, "%d\n", &q);
				// Search the server's response. Grab the number representing the size of the queue.
			
		// Now handle sending the actual message
			puts("Sending..\n");
			sprintf(s, "SEND %s\n", msgsend); 	// Sends whatever you typed in.
			send(sockfd, s, strlen(s), 0);

			size = recv(sockfd, buf, 1000, 0); 	// Did the message go through?
			sscanf(buf, "%[^\n]", msgrec);		// Grab the entire response
			printf("%s: %s\n", argv[2], msgsend);

			
		// Then, if there is a queue, start processing it
			if (q != 0) puts("*** Messages in queue ***");
			while (q>=1) { 				// While there are msgs in the q, print them
				//memset(msgrec, 0, sizeof(msgrec));	// Clear the previous message.


				sprintf(s, "RECV\n");		// Receive the next message
				send(sockfd, s, strlen(s), 0);

				size = recv(sockfd, buf, 1000, 0);
				sscanf(buf, "+OK %[^\n]", msgrec);	// Grab the next message
				printf("%d: '%s'\n", q, msgrec);	// Print the message
				q--;					// Since we printed one queue item

			}
				
		}
		printf("\n>");						// Prompt for more commands

		

/*		select(int r, , 0);
		r = 0;
		r = r | (1<<1);
		r = r | (1<<variable);	`	
*/				

	} // End while loop (input = "/quit")
    while ((size = recv(sockfd, buf, 1000, 0)) > 0)
    {
        fwrite(buf, size, 1, stdout);
    }
    close(sockfd);
	return 0;
}
