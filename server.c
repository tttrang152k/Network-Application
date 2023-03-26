// Thi Thuy Trang Tran, 74889299
// Vikasni Kalahasthi 78601545

// Server side
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>
#define PORT 58130

#define MAX_BYTES 256
#define MAX_NUMBER_STOCKS 503     // used for creating data structures to data from csv files
#define MAX_CHAR_DATE 12        // storing date strings
#define MAX_FILENAME 80         // max string for file name
#define MAX_NUMBER_FILE 2
#define MAX_LINE 80

// Global socket file descriptors. Only closes when Ctrl + C is hit (will be handled in interruptHandler)
int serverfd = -1;     
int connectfd = -1;

void distributeInput(char* input, int* argc, char** argv) { // distributes input into argc & argv
    char* token = strtok(input, " \t\n");
    *argc = 0;

    while (token != NULL) {
        argv[(*argc)++] = token;
        token = strtok(NULL, " \t\n");
    }
    argv[(*argc)] = token;
}

// stock price information. Including date and closing price associated with date
struct info {
    char date[MAX_CHAR_DATE];
    double closingPrice;

};

// data structure for two kinds of stocks. One for MRNA and another one for PFE
struct stock {
    char stockName[MAX_FILENAME];
    int size;   // size of stock == number of dates in csv
    struct info stockInfo[MAX_NUMBER_STOCKS];
}stockList[MAX_NUMBER_FILE];   // global stock list. Only contains two stocks: MRNA and PFE

void readFromFiles(int index){ // read from File and put data into data structure for each stock
    //0 is always PFE and 1 is always MRNA
    FILE* fd;
    if(index == 0){
        fd = fopen("./PFE.csv","r");
    }
    else{
        fd = fopen("./MRNA.csv","r");
    }
    
    char buffer[1024];
    int row = 0;
    int column = 0;
    int counter = 0;
    char *eptr;
    while(fgets(buffer,1024,fd)){
        column = 0;
        row++;
        if(row == 1){ // skip first row
            continue;
        }
        char* value = strtok(buffer,",");
        while(value){
            if(column == 0){
               
               strcpy(stockList[index].stockInfo[counter].date,value);
            }
            if(column == 4){
              //printf(" Value Inputted: %s\n",value);
              stockList[index].stockInfo[counter].closingPrice = strtod(value,&eptr);
              //printf(" Value Inputted: %f\n",stockList[index].stockInfo[counter].closingPrice);
            }
            value = strtok(NULL,",");
            column++;


        }
        counter++;
        //print line if needed
    }
    fclose(fd);
}

void printPFEStockList(){ // helper function
    printf("Reading %s stocks\n",stockList[0].stockName);
    for(int i = 0; i < stockList[0].size; i++){
        printf("%s | %f\n",stockList[0].stockInfo[i].date, stockList[0].stockInfo[i].closingPrice);
    }
}
void printMRNAStockList(){ // helper function
    printf("Reading %s stocks\n",stockList[1].stockName);
    for(int i = 0; i < MAX_NUMBER_STOCKS;i++){
        printf("%s | %f\n",stockList[1].stockInfo[i].date,stockList[1].stockInfo[i].closingPrice);
    }
}

void maxPossibleProfit_Loss(char* type, char* stockName,char* startTime, char* endTime, char* message){
    // figure out dimensions by finding out how many days are in between start and end time
    // make the profit array
    double profit[MAX_NUMBER_STOCKS];
    int counter = 0;
    int flag = 0;
    int index;
    if(strcmp(stockName,"PFE")== 0){
        index = 0; //PFE
    }
    else{
        index = 1; //MRNA
    }

    for(int i = 0; i < MAX_NUMBER_STOCKS; i++){
        if(strcmp(stockList[index].stockInfo[i].date,startTime) == 0){
            //printf("Start date found.\n");
            flag = 1;
            profit[counter] = stockList[index].stockInfo[i].closingPrice;
            counter++;
        }
        if(flag == 1 && strcmp(stockList[index].stockInfo[i].date,startTime) != 0){
            profit[counter] = stockList[index].stockInfo[i].closingPrice;
            counter++;
        }
        if(strcmp(stockList[index].stockInfo[i].date,endTime) == 0){
            flag = 0;
        }
    }

    double answer = 0;
    int buyDay = 0;
    int sellDay = 0;

    if(strcmp(type,"profit") == 0){ // if the type is profit calc profit
        for( int i = 0; i < counter;i++){
                for(int j = i; j < counter; j++){
                    if(answer <= (profit[j]-profit[i])){
                        answer = profit[j]-profit[i];
                        buyDay = i;
                        sellDay = j;
                    }
                }
        }
    }
    else{ // calc loss
        for( int i = 0; i < counter;i++){
                for(int j = i; j < counter; j++){
                    if(answer > (profit[j]-profit[i])){
                        answer = profit[j]-profit[i];
                        buyDay = i;
                        sellDay = j;
                    }
                }
        }
    }
    answer = fabs(answer);
    sprintf(message, "%.2f", answer);
}

int pricesOnDate(char* date, char* message) {   // get prices from given date. If date is not found -> return 0 else 1
    // assumes that both lists have the same dates and in the same order
    int datefound = 0;
    char result[MAX_LINE] = "PFE: ";
    char PFEnum[MAX_LINE];
    char MRNAnum[MAX_LINE];
    for(int i = 0; i < MAX_NUMBER_STOCKS;i++){
        if(strcmp(stockList[0].stockInfo[i].date,date) == 0 && strcmp(stockList[1].stockInfo[i].date,date) == 0){
            sprintf(PFEnum, "%.2f",stockList[0].stockInfo[i].closingPrice);
            sprintf(MRNAnum,"%.2f",stockList[1].stockInfo[i].closingPrice);
            strcat(result, PFEnum);
            strcat(result," | MRNA: ");
            strcat(result, MRNAnum); 
            datefound = 1;
        }
    }
    strcpy(message, result);
    return datefound;
}

void interruptHandler(int signalNum) {  // quit server when Ctrl-C is hit
    printf("Ends the server\n");
    close(serverfd);
    close(connectfd);
    exit(0);    // terminate the process
}     

// Create a listening descriptor that can be used to accept connection requests from clients
// refering to the lecture codes
int open_listenfd(char *port) {
    struct addrinfo hints, *listp, *p;
    int listenfd, optval = 1;

    // Get a list of potential server addresses
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;                // Accept connect..
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;    // ..on any IP address
    hints.ai_flags |= AI_NUMERICSERV;               // ..using port no. 
    getaddrinfo(NULL, port, &hints, &listp);

    // Walk the list for one that we can bind to
    for (p = listp; p; p = p->ai_next) {
        // Create a socket descriptor
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
            continue;   // socket failed, try the next one
        }

        // Eliminates "Address already in use" error from bind
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));

        // Bind the descriptor to the address
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) {
            break;  // Bind successfully
        }
        close(listenfd);    // Bind failed, try the next
    }

    // Clean up
    freeaddrinfo(listp);
    if (!p) {   // No address worked
        return -1;
    }

    // Make it a listening socket ready to accept connection requests
    if (listen(listenfd, 8) < 0) {    
        close(listenfd);
        return -1;
    }
    return listenfd;
}

void reformatDate(char* date) { // reformat the date to same format in data sheet. MM/DD/YYYY instead of YYYY-MM-DD
    int yearMonthDay[3];    // array to store year, month and day respectively
    // extract and convert into each category
    char* token;
    int i = 0;
    const char* delims = "-";
    token = strtok(date, delims);       // first token is the year
    while (token != NULL) {             // second is month and third is day
        yearMonthDay[i++] = atoi(token);
        token = strtok(NULL, delims);
    }
    sprintf(date, "%d/%d/%d", yearMonthDay[1], yearMonthDay[2], yearMonthDay[0]);
}

int main(int argc, char* argv[]) {
    // Files must be handled before listening to client requests
    // Read in filenames and assign each stock - first is always PFE and second is always MRNA
    strcpy(stockList[0].stockName, argv[1]);
    strcpy(stockList[1].stockName, argv[2]);
    readFromFiles(0);
    readFromFiles(1);
    
    // Ctrl + C handler function initialization
    signal(SIGINT, interruptHandler);

    char clientMessage[MAX_BYTES];
    char serverMessage[MAX_BYTES];

    // Socket address and address length initialization
    struct sockaddr_in clientAddress;
    socklen_t  clientLen = sizeof(clientAddress);
    // Open listening descriptor to accept connection request from clients
    int listenfd, connectfd;
    serverfd = open_listenfd(argv[3]);      // passing in the PORT

    printf("Server started\n");
    // Accept connection from client socket
    connectfd = accept(serverfd, (struct sockaddr*)&clientAddress,  &clientLen);
    
    while(1) {
        // Server command
        char input[MAX_LINE];
        char* u_argv[MAX_LINE];
        int u_argc = 0;

        // set back and clean up
        fflush(stdin);
        fflush(stdout);
        memset(clientMessage, 0, sizeof(clientMessage));
        memset(serverMessage, 0, sizeof(serverMessage));

        // Check if server accepts the connection
        if (connectfd < 0) {
            printf("Failed to accept connection\n");
            return -1;
        }
        // Read client request message 
        if (read(connectfd, clientMessage, sizeof(clientMessage)) < 0) {
            printf("Fail to read client message\n");
            return -1;
        }

        // Continue server even though client ends connection. only terminates with Ctrl + C
        // If client ended the program. Do not quit the server
        if (strlen(clientMessage) == 0) {
            continue;
        }
        
        printf("%s\n", clientMessage);
        // Distribute client request to correct format to arguments
        distributeInput(clientMessage, &u_argc, u_argv);

        if (strcmp(u_argv[0], "PricesOnDate") == 0) {
            char priceMessage[MAX_BYTES];
            reformatDate(u_argv[1]);
            if (pricesOnDate(u_argv[1], priceMessage) == 1)
                strcpy(serverMessage, priceMessage);
            else
                strcpy(serverMessage, "Unknown");
        }

        if (strcmp(u_argv[0], "MaxPossible") == 0) {
            char priceMessage[MAX_BYTES];
            reformatDate(u_argv[3]);
            reformatDate(u_argv[4]);
            if (strcmp(u_argv[1], "profit") == 0) {
                // calculate and return profit from given date range
                // params: profit, data name, starting date, ending date
                maxPossibleProfit_Loss(u_argv[1], u_argv[2], u_argv[3], u_argv[4], priceMessage);  
            }
            else {  // calculate and return loss from given date range
                maxPossibleProfit_Loss(u_argv[1], u_argv[2], u_argv[3], u_argv[4], priceMessage);
            }
            strcpy(serverMessage, priceMessage);
        }

        // Send back the response to client side
        if (send(connectfd, serverMessage, strlen(serverMessage), 0) < 0) {
            printf("Fail to send message to client\n");
            return -1;
        }
    }

    return 0;
}





