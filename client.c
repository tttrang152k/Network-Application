// Thi Thuy Trang Tran, 74889299
// Vikasni Kalahasthi 78601545

// Client side
#include <netinet/in.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#define PORT 58130  // one of the available ports from ICS Computer Support domain

#define MAX_BYTES 256
#define MAX_LINE 80


// Establish a connection with a server
// Original code from lecture
int open_clientfd(char* hostname, char* port) {
    int clientfd;
    struct addrinfo hints, *listp, *p;

    // Get a list of potential server addresses
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;    // Open a connection
    hints.ai_flags = AI_NUMERICSERV;    // ..using numeric port arg
    hints.ai_flags |= AI_ADDRCONFIG;    // Recommended for connections
    hints.ai_family = AF_INET;          // Didn't see it in lecture codes but discussion notes
    hints.ai_protocol = IPPROTO_TCP;    // ^. actually not sure about this. might be 0
    getaddrinfo(hostname, port, &hints, &listp);

    // Walk the list for one that we can successfully connect to
    for (p = listp; p; p = p->ai_next) {
        // Create a socket descriptor
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) 
            continue;   // Socket creation failed, try the next
    
        // Connect to the server
        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != 1)
            break;      // Success
        close(clientfd);    // Connect failed, try another
    }

    // Clean up
    freeaddrinfo(listp);
    if (!p)     // All connections failed
        return -1;
    else        // The last connect succeeded
        return clientfd;

}

void distributeInput(char* input, int* argc, char** argv) { //distributes input into argc & argv
    // we would need to keep input for future usage hence we would define a temp here
    // should not modify the orginial input for any cases
    char tempInput[MAX_LINE];
    strcpy(tempInput, input);

    char* token;        
    const char* delims = " \t\n";
    token = strtok(tempInput, delims);  // first token is the command
    while (token != NULL) {             // getting next arguments in to argv
        argv[(*argc)++] = token;
        token = strtok(NULL, delims);
    }
}

int isInvalidFormatDate(char* date) {     // if valid date -> return 0 else 1
    // Check the date format 
    // Correct format would be: YYYY-MM-DD
    if (strlen(date) != 10) 
        return 1;
    for (int i = 0; i < strlen(date); i++) {
        if (i == 4 || i == 7) {
            if (date[i] != '-')
                return 1;
        }
        else {
            if (!isdigit(date[i]))
                return 1;
        }
    }
    return 0;
}

// Assuming dates are correct
int isInvalidDate(char* date) {
    printf("Check if valid date: %s\n", date);
    // Check if valid date
    int daysEachMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};     // Max days in each month
    int yearMonthDay[3];    // array to store year, month and day respectively
    // extract and convert into each category
    int i = 0;
    //const char* delims = "-";
    char* token = strtok(date, "-");       // first token is the year
    while (token != NULL) {             // second is month and third is day
        printf("token: %s", token);
        yearMonthDay[i++] = atoi(token);
        token = strtok(NULL, "-");
    }
    printf("year: %d month: %d day: %d", yearMonthDay[0], yearMonthDay[1], yearMonthDay[2]);
    // Modify max number of days in Feb if leap year
    if (yearMonthDay[0] % 4 == 0) 
        daysEachMonth[1]++;     
    // Check year
    // --- might add something here
    // Check month
    if (yearMonthDay[1] <= 0 || yearMonthDay[1] >= 13)
        return 1;
    // Check day
    if (yearMonthDay[2] <= 0 || yearMonthDay[2] > daysEachMonth[yearMonthDay[1] - 1])
        return 1;
    
    return 0;   
}

int main(int argc, char* argv[]) {
    // User query command
    char input[MAX_LINE];
    char* u_argv[MAX_LINE];
    int u_argc = 0;

    char request[MAX_BYTES];    // User request message sent from client to server
    char response[MAX_BYTES];   // Response message from server for user's request

    // Initialize the socket_fd. -1 if connection fails
    int socket = 0;             
    int isNotConnected = 1;
    
    while (1) {
        // set back and clean up
        fflush(stdin);
        fflush(stdout);
        memset(response, 0, sizeof(response));
        u_argc = 0;


        // Make connection to the server. Keep looping until the connection succeeds
        while(isNotConnected == 1){
            // argv[1]: hostname/domain name ; argv[2]: PORT number
            // using either hocalhost or 127.0.0.1 for hostname
            // sending given hostname and PORT to open clientfd to make connection to server
            socket = open_clientfd(argv[1], argv[2]);
            if (socket < 0){
                printf("Fail to make connection to server!\n");
            }
            else {
                printf("Connected to server\n");    // for testing sake
                isNotConnected = 0;
            }
        }

        // Once the connection has made successfuly, proceed with query command
        printf("> ");
        fgets(input, MAX_LINE, stdin);
        distributeInput(input, &u_argc, u_argv);

        // Evaluate the input function
        // Only a few jobs needed to be done while many parameters needed to pass around. Can just do it here
        // Check for valid command, send client request to server, read response message from server, display the received message
        if (strcmp(u_argv[0], "quit") == 0) {
            printf("Command to quit.\n");
            break;          // break from client program (ends while loop)
        }

        // validate the client command
        // Whatever invalid format of a date (YYYY-MM-DD)
        // Stock names as ticker symbols (PFE or MRNA)
        // Also need to check for case-sensitive
        if (u_argc == 0) {
            continue;       // Move to wait for next command without sending user command to server
        }
        else if (strcmp(u_argv[0], "PricesOnDate") == 0 || strcmp(u_argv[0], "MaxPossible") == 0 ) {
            if (strcmp(u_argv[0], "PricesOnDate") == 0) {
                if (u_argc != 2) {
                    printf("Invalid syntax\n");
                    continue;
                }
                else {
                    if (isInvalidFormatDate(u_argv[1]) == 1) {
                        printf("Invalid syntax\n");
                        continue;
                    }
                }
            }
            else if (strcmp(u_argv[0], "MaxPossible") == 0) {
                if ((strcmp(u_argv[1], "profit") == 0 || strcmp(u_argv[1], "loss") == 0)
                     && (strcmp(u_argv[2], "PFE") == 0 || strcmp(u_argv[2], "MRNA") == 0)) {
                    if (isInvalidFormatDate(u_argv[3]) == 1) {
                        if (isInvalidFormatDate(u_argv[4]) == 1) {
                            printf("Invalid syntax\n");
                            continue;
                        }
                    }     
                }
                else {
                    printf("Invalid syntax\n");
                    continue;
                }
            }
        }
        else {
            printf("Invalid syntax\n");
            continue;
        }
        // -- end checking user command
        
        // Send client request (only those are valid) to server
        if (send(socket, input, strlen(input), 0) < 0) {
            printf("Fail to send message to server\n");
            break;
        }
        // Read response message from server (should receive corresponding ones)
        if (read(socket, response, sizeof(response)) < 0) {
            printf("Fail to read message from server\n");
            break;
        }
        // Display the response message in client side
        printf("%s\n", response);

    }
    
    // Close the connected socket
    close(socket);
    return 0;

}
