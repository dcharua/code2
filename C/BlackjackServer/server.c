// BlackJack game with server server side - Daniel Charua - A01017419

#include <time.h>
// My sockets library
#include "socketsLib.h"

#define SERVICE_PORT 8642
#define MAX_QUEUE 5
#define BUFFER_SIZE 1024
#define MAXCARDS 21

void usage(char * program);
void waitForConnections(int server_fd);
void playBlackjack(int connection_fd);
int dealNewGame(int *players_cards, int *dealers_cards, int connection_fd);
int dealNSendCard(int connection_fd);
int getTotal (int *cards, int size);


int main(int argc, char * argv[]){
  srand(time(NULL));
  int server_fd;
  printf("\n=== SERVER PROGRAM ===\n");
  if (argc != 2)
    usage(argv[0]);
  server_fd = startServer(argv[1]);
  printf("Server ready and waiting!\n");
  // Stand by to receive connections from the clients
  waitForConnections(server_fd);
  printf("Closing the server socket\n");
  // Close the socket
  close(server_fd);
  return 0;
}

// Show the user how to run this program
void usage(char * program){
  printf("Usage:\n%s {port_number}\n", program);
  exit(EXIT_FAILURE);
}



// Stand by for connections by the clients
void waitForConnections(int server_fd){
  struct sockaddr_in client_address;
  socklen_t client_address_size;
  char client_presentation[INET_ADDRSTRLEN];
  int connection_fd;
  pid_t pid;
  struct pollfd test_fd[1];
  int timeout = 1000; //time in milliseconds ( 1 second)

  // Loop to wait for client connections
  while (1){
    ///// ACCEPT
    // Receive incomming connections
    // Get the size of the structure where the address of the client will be stored
    client_address_size = sizeof client_address;

    while(1){
      //fill poll struct
      test_fd[0].fd = server_fd;
      test_fd[0].events = POLLIN;
      int poll_result = poll(test_fd, 1, timeout);
      if (poll_result == -1)
        kill("poll");
      //else if ( poll_result == 0)
      //  printf("no connections yet\n");
      else{
        //printf("A connection is about to be accepted\n");
        break;
      }
    }
    connection_fd = accept(server_fd, (struct sockaddr *) &client_address, &client_address_size);
    if (connection_fd == -1)
      kill("accept error");

    // Identify the client
    // Get the ip address from the structure filled by accept
    inet_ntop(client_address.sin_family, &client_address.sin_addr, client_presentation, sizeof client_presentation);
    printf("Received connection from '%s' on port '%d'\n", client_presentation, client_address.sin_port);

    pid = fork();
    // Parent process
    if (pid > 0){
      close(connection_fd);
    }
    // Child process
    else if (pid == 0){
      close(server_fd);
      // Establish the communication
      playBlackjack(connection_fd);
      exit(EXIT_SUCCESS);
    }
    // Fork error
    else{
      kill("fork error");
    }
  }
}

// Do the actual receiving and sending of data
void playBlackjack(int connection_fd){
  char buffer[BUFFER_SIZE], hit;
  //int message_counter = 0;
  int chars_read, pot, players_cards[MAXCARDS], dealers_cards[MAXCARDS],opt,
  bet, player_sum = 0, dealer_sum = 0, i = 0 , k = 0, over21 = 0;
  // Handshake
  receiveMessage(connection_fd, buffer, BUFFER_SIZE);
  sprintf(buffer, "POT");
  sendMessage(connection_fd, buffer, strlen(buffer));

  // Receive the pot from the user and print
  receiveMessage(connection_fd, buffer, BUFFER_SIZE);
  sscanf(buffer, "%d", &pot);
  printf("The player's initial pot is: %d\n", pot);
  //send play signal and recive
  sprintf(buffer, "PLAY?");
  sendMessage(connection_fd, buffer, strlen(buffer));
  //while user has money keep playing otherwise break from game loop
  while (pot > 0){
    receiveMessage(connection_fd, buffer, BUFFER_SIZE);
    sscanf(buffer, "%d", &opt);
    if (opt == 2)
      break;
    //cleaning cards each hand
    memset(players_cards, 0, MAXCARDS);
    memset(dealers_cards, 0, MAXCARDS);
    i = 0; k = 0;

    receiveMessage(connection_fd, buffer, BUFFER_SIZE);
    sscanf(buffer, "%d", &bet);
    //Deal first card
    players_cards[i++] = dealNSendCard(connection_fd);
    receiveMessage(connection_fd, buffer, BUFFER_SIZE);
    //Deal second card check if previous card was recived
    if (strncmp(buffer, "RECEIVED", 8) == 0)
      dealers_cards[k++] = dealNSendCard(connection_fd);
    receiveMessage(connection_fd, buffer, BUFFER_SIZE);
    //Deal third card check if previous card was recived
    if (strncmp(buffer, "RECEIVED", 8) == 0)
      players_cards[i++] = dealNSendCard(connection_fd);
    receiveMessage(connection_fd, buffer, BUFFER_SIZE);
    //Deal forth card check if previous card was recived
    if (strncmp(buffer, "RECEIVED", 8) == 0)
      dealers_cards[k++] = dealNSendCard(connection_fd);
    receiveMessage(connection_fd, buffer, BUFFER_SIZE);

    //While player whats  new card
    while ( opt == 1 && strncmp(buffer, "RECEIVED", 8) == 0 ){
      receiveMessage(connection_fd, buffer, BUFFER_SIZE);
      sscanf(buffer, "%d", &opt);
      //if player wants a new card deal
      if (opt == 1){
        players_cards[i++] = dealNSendCard(connection_fd);
        receiveMessage(connection_fd, buffer, BUFFER_SIZE);
      }
      //if player is over 21 break and set over21 to true
      if (getTotal(players_cards, i) > 21){
        over21 = 1;
        opt = 2;
      }
    }

    //if over21 is false deal dealer cards
    if (over21 == 0){
    // deal until dealer is higher than player
    while (getTotal(dealers_cards, k) < getTotal(players_cards, i) ){
        dealers_cards[k++] = dealNSendCard(connection_fd);
        receiveMessage(connection_fd, buffer, BUFFER_SIZE);
      }
      // when dealer is higher send stand signal
      sprintf(buffer, "STAND");
      sendMessage(connection_fd, buffer, strlen(buffer));
      receiveMessage(connection_fd, buffer, BUFFER_SIZE);
      //if dealer is over 21 player won
      if (getTotal(dealers_cards, k) > 21){
        pot += bet;
        sprintf(buffer, "WON");
        sendMessage(connection_fd, buffer, strlen(buffer));
      // if dealer is 21 or below player lost since dealer is equal or over player
      } else {
        pot -= bet;
        sprintf(buffer, "LOST");
        sendMessage(connection_fd, buffer, strlen(buffer));
      }
    //if player is over 21 player lost
    } else{
      pot -= bet;
      sprintf(buffer, "LOST");
      sendMessage(connection_fd, buffer, strlen(buffer));
    }
    //reset over21
    over21 = 0;
    //recive result acknlowage
    receiveMessage(connection_fd, buffer, BUFFER_SIZE);
    if (strncmp(buffer, "RESULTREAD", 11) == 0){
      //send pot to user and recive ackolowage
      sprintf(buffer, "%d", pot);
      sendMessage(connection_fd, buffer, strlen(buffer));
      receiveMessage(connection_fd, buffer, BUFFER_SIZE);
    }
    // if user is not out money send game on signal
    if (pot > 0 && strncmp(buffer, "STATUS", 8) == 0){
      sprintf(buffer, "GAMEON");
      sendMessage(connection_fd, buffer, strlen(buffer));
    }
  }
  // if user is out of money broke form the loop and send gamover signal
  sprintf(buffer, "GAMEOVER");
  sendMessage(connection_fd, buffer, strlen(buffer));
  printf("player left, waiting for new player\n");
}

//function to deal and send a new card to client
int dealNSendCard(int connection_fd){
  char buffer[BUFFER_SIZE];
  int card;
  card = rand() %  13 + 1;
  if (card > 10)
    card = 10;
  sprintf(buffer, "%d", card);
  sendMessage(connection_fd, buffer, strlen(buffer));
  return card;
}

//function to get total from a players hand
int getTotal (int *cards, int size){
  int total = 0;
  for (int i = 0; i < size; i++)
    total += cards[i];
  return total;
}
