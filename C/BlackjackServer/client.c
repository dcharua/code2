// BlackJack game with server client side - Daniel Charua - A01017419

#include "socketsLib.h"

#define SERVICE_PORT 8642
#define BUFFER_SIZE 1024
#define MAXCARDS 21

void usage(char * program);
void connectToServer(char * address, char * port);
void communicationLoop(int connection_fd);
void printCards(int *cards, int size);
int getTotal (int *cards, int size);

int main(int argc, char * argv[]){
  printf("\n=== CLIENT PROGRAM ===\n");
  if (argc != 3)
    usage(argv[0]);
  connectToServer(argv[1], argv[2]);
  return 0;
}

// Show the user how to run this program
void usage(char * program){
  printf("Usage:\n%s {server_address} {port_number}\n", program);
  exit(EXIT_FAILURE);
}

// Establish a connection with the server indicated by the parameters
void connectToServer(char * address, char * port){
  struct addrinfo hints;
  struct addrinfo * server_info;
  int connection_fd;

  // Prepare the information to determine the local address
  bzero(&hints, sizeof hints);
  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  ///// GETADDRINFO
  // Get the actual address of this computer
  if (getaddrinfo(address, port, &hints, &server_info) == -1)
    kill("getaddrinfo error");

  ///// SOCKET
  // Use the information obtained by getaddrinfo
  connection_fd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
  if (connection_fd == -1)
    kill("socket error");

  ///// CONNECT
  // Connect with the desired port
  if (connect(connection_fd, server_info->ai_addr, server_info->ai_addrlen) == -1)
    kill("connect error");

  // Release the memory of the list of addrinfo
  freeaddrinfo(server_info);

  // Establish the communication
  communicationLoop(connection_fd);

  // Close the socket
  close(connection_fd);
}

// Do the actual receiving and sending of data
void communicationLoop(int connection_fd){
  char buffer[BUFFER_SIZE];
  int chars_read, pot, players_cards[MAXCARDS], dealers_cards[MAXCARDS], opt,
  bet, player_sum = 0, dealer_sum = 0, i = 0 , k = 0, hand = 0;

  // Handshake
  sprintf(buffer, "Lets play");
  sendMessage(connection_fd, buffer, strlen(buffer));
  receiveMessage(connection_fd, buffer, BUFFER_SIZE);
  if (strncmp(buffer, "POT", 3) == 0){
    printf("Enter your intial pot: ");
    scanf("%d", &pot);
  }

  sprintf(buffer, "%d", pot);
  sendMessage(connection_fd, buffer, strlen(buffer));
  receiveMessage(connection_fd, buffer, BUFFER_SIZE);
  if (strncmp(buffer, "PLAY?", 5) == 0){
    // Start game
    while (strncmp(buffer, "GAMEOVER", 8) != 0){
      printf("\n\n======= BlackJack 21 ==========\n");
      printf("===================  Pot %d \n", pot);
      printf("= 1.- Deal Hand      ========== \n");
      printf("= 2.- Leave Game     ==========\n");
      printf("= Enter the number:");
      scanf("%d", &opt);
      sprintf(buffer, "%d", opt);
      sendMessage(connection_fd, buffer, strlen(buffer));
      if (opt == 2)
        break;
      memset(players_cards, 0, MAXCARDS);
      memset(dealers_cards, 0, MAXCARDS);
      i = 0; k = 0;
      printf("Place a bet:");
      scanf("%d", &bet);
      sprintf(buffer, "%d", bet);
      sendMessage(connection_fd, buffer, strlen(buffer));
      //Get first card
      receiveMessage(connection_fd, buffer, BUFFER_SIZE);
      sscanf(buffer, "%d", &players_cards[i]);
      printf("\nYou got card %d \n", players_cards[i++]);
      sprintf(buffer, "RECEIVED");
      sendMessage(connection_fd, buffer, strlen(buffer));
      //Get second card
      receiveMessage(connection_fd, buffer, BUFFER_SIZE);
      sscanf(buffer, "%d", &dealers_cards[k]);
      printf("Dealer got card %d \n", dealers_cards[k++]);
      sprintf(buffer, "RECEIVED");
      sendMessage(connection_fd, buffer, strlen(buffer));
      //Get third card
      receiveMessage(connection_fd, buffer, BUFFER_SIZE);
      sscanf(buffer, "%d", &players_cards[i]);
      printf("You got card %d \n", players_cards[i++]);
      sprintf(buffer, "RECEIVED");
      sendMessage(connection_fd, buffer, strlen(buffer));
      //Get forth card
      receiveMessage(connection_fd, buffer, BUFFER_SIZE);
      sscanf(buffer, "%d", &dealers_cards[k++]);
      sprintf(buffer, "RECEIVED");
      sendMessage(connection_fd, buffer, strlen(buffer));
      printf("Your  cards ");
      printCards(players_cards,  i);

      while (opt == 1){
        printf("\n= 1.- Hit            ========== \n");
        printf("= 2.- Hold           ==========\n");
        printf("= Enter the number:");
        scanf("%d", &opt);
        sprintf(buffer, "%d", opt);
        sendMessage(connection_fd, buffer, strlen(buffer));
        if (opt == 1){
          receiveMessage(connection_fd, buffer, BUFFER_SIZE);
          sscanf(buffer, "%d", &players_cards[i]);
          sprintf(buffer, "RECEIVED");
          sendMessage(connection_fd, buffer, strlen(buffer));
          printf("You got card %d \n", players_cards[i++]);
          printf("Your  cards ");
          printCards(players_cards, i);
          if (getTotal(players_cards, i) > 21){
            opt = 2;
            hand = 1;
            printf("You went over 21 and lost\n");
          }
        }
      }
      if (hand == 0){
        printf("\nDealer opened closed card: %d\n", dealers_cards[1]);
        printf("Dealers cards ");
        printCards(dealers_cards, k);
        //Loop for dealers cards
        while(1){
          receiveMessage(connection_fd, buffer, BUFFER_SIZE);
          if (strncmp(buffer, "STAND", 5) == 0)
            break;
          sscanf(buffer, "%d", &dealers_cards[k]);
          sprintf(buffer, "RECEIVED");
          sendMessage(connection_fd, buffer, strlen(buffer));
          printf("Dealer got card %d \n", dealers_cards[k++]);
          printf("Dealers cards ");
          printCards(dealers_cards, k);
        }

        sprintf(buffer, "STANDREAD");
        sendMessage(connection_fd, buffer, strlen(buffer));
      }

      hand = 0;

      receiveMessage(connection_fd, buffer, BUFFER_SIZE);
      if (strncmp(buffer, "WON", 3) == 0)
        printf("You WON\n");
      else
        printf("You LOST\n");

      sprintf(buffer, "RESULTREAD");
      sendMessage(connection_fd, buffer, strlen(buffer));

      receiveMessage(connection_fd, buffer, BUFFER_SIZE);
      sscanf(buffer, "%d", &pot);
      printf("Your pot total is: %d\n", pot);
      //ask for game status
      sprintf(buffer, "STATUS");
      sendMessage(connection_fd, buffer, strlen(buffer));
      //Game status, if GAMOVER break loop
      receiveMessage(connection_fd, buffer, BUFFER_SIZE);
    }
  }
  if (pot <= 0 )
    printf("You are out of money, better luck next time \n");
  else
    printf("Thank you for playing, come back soon \n");
}

void printCards(int *cards, int size){
  for(int i = 0; i<size; i++)
    if (cards[i] > 0 && cards[i] < 11 )
      printf("%d-", cards[i]);
  printf("\n");
  printf("total : %d \n", getTotal(cards, size));
}

int getTotal (int *cards, int size){
  int total = 0;
  for (int i = 0; i < size; i++)
    total += cards[i];
  return total;
}
