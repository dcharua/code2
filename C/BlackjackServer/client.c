// BlackJack game with server client side - Daniel Charua - A01017419

#include "socketsLib.h"

#define SERVICE_PORT 8642
#define BUFFER_SIZE 1024
#define MAXCARDS 21

void usage(char * program);
void connectToServer(char * address, char * port);
void communicationLoop(int connection_fd);
void printCards(char *cards, int size);
int getTotal (char *cards, int size);
char receiveCard(int connection_fd, int player);
void printCard(char card);

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
  char buffer[BUFFER_SIZE], players_cards[MAXCARDS], dealers_cards[MAXCARDS];
  int pot, opt, bet, i = 0 , k = 0, over21 = 0;

  // Handshake
  sprintf(buffer, "Lets play");
  sendMessage(connection_fd, buffer, strlen(buffer));
  receiveMessage(connection_fd, buffer, BUFFER_SIZE);
  //set the initial pot
  if (strncmp(buffer, "POT", 3) == 0){
    printf("Enter your intial pot: ");
    scanf("%d", &pot);
  }
  //send initial pot to server
  sprintf(buffer, "%d", pot);
  sendMessage(connection_fd, buffer, strlen(buffer));
  receiveMessage(connection_fd, buffer, BUFFER_SIZE);
  // Start game
  if (strncmp(buffer, "PLAY?", 5) == 0){
    while (strncmp(buffer, "GAMEOVER", 8) != 0){
      do{
        printf("\n\n======= BlackJack 21 ==========\n");
        printf("===================  Pot %d \n", pot);
        printf("= 1.- Deal Hand      ========== \n");
        printf("= 2.- Leave Game     ==========\n");
        printf("= Enter the number:");
        scanf("%d", &opt);
      } while (opt!=1 && opt !=2 );
      sprintf(buffer, "%d", opt);
      sendMessage(connection_fd, buffer, strlen(buffer));
      //if user whats to leave break from loop
      if (opt == 2)
        break;
      //clean memory every hand
      memset(players_cards, 0, MAXCARDS);
      memset(dealers_cards, 0, MAXCARDS);
      i = 0; k = 0;
      //ask user for the bet for each hand and send to server
      printf("Place a bet:");
      scanf("%d", &bet);
      sprintf(buffer, "%d", bet);
      sendMessage(connection_fd, buffer, strlen(buffer));
      //Get players first card and send acknowledge in function
      printf("\n");
      players_cards[i++] = receiveCard(connection_fd, 1);
      //Get dealers first card and send acknowledge in function
      dealers_cards[k++] = receiveCard(connection_fd, 2);
      //Get players first card and send acknowledge in function
      players_cards[i++] = receiveCard(connection_fd, 1);
      //Get dealers second card and send acknowledge in function
      dealers_cards[k++] = receiveCard(connection_fd, 3);
      //print players cards
      printf("-------------\nYour cards ");
      printCards(players_cards,  i);

      //Looping until player wants to stand
      while (opt == 1){
        do{
          printf("\n= 1.- Hit            ========== \n");
          printf("= 2.- Hold           ==========\n");
          printf("= Enter the number:");
          scanf("%d", &opt);
        } while (opt!=1 && opt !=2 );  
        sprintf(buffer, "%d", opt);
        sendMessage(connection_fd, buffer, strlen(buffer));
        //if player whats a new card
        if (opt == 1){
          players_cards[i++] = receiveCard(connection_fd, 1);
          printf("your cards ");
          printCards(players_cards, i);
          //If player is over 21 break out of loop and print the he lost
          if (getTotal(players_cards, i) > 21){
            opt = 2;
            over21 = 1;
            printf("You went over 21 and lost\n");
          }
        }
      }
      //If player is not over 21
      if (over21 == 0){
        //Show dealears hidden card
        printf("\nDealer opened closed card: %c\n", dealers_cards[1]);
        printf("Dealers cards ");
        printCards(dealers_cards, k);
        //Loop for dealers new cards until STAND is received
        while(1){
          receiveMessage(connection_fd, buffer, BUFFER_SIZE);
          if (strncmp(buffer, "STAND", 5) == 0)
            break;
          sscanf(buffer, "%c", &dealers_cards[k]);
          sprintf(buffer, "RECEIVED");
          sendMessage(connection_fd, buffer, strlen(buffer));
          printf("Dealer got card ");
          printCard(dealers_cards[k++]);
          printf("Dealers cards ");
          printCards(dealers_cards, k);
        }
        //Send stand acknowledge
        sprintf(buffer, "STANDREAD");
        sendMessage(connection_fd, buffer, strlen(buffer));
      }

      //reset over21 for new hand
      over21 = 0;
      //receive win or lost massage
      receiveMessage(connection_fd, buffer, BUFFER_SIZE);
      if (strncmp(buffer, "WON", 3) == 0)
        printf("You WON\n");
      else
        printf("You LOST\n");
      //send game status acknowledge
      sprintf(buffer, "RESULTREAD");
      sendMessage(connection_fd, buffer, strlen(buffer));
      //receive pot ammount after the hand
      receiveMessage(connection_fd, buffer, BUFFER_SIZE);
      sscanf(buffer, "%d", &pot);
      printf("Your pot total is: %d\n", pot);
      //ask for game status ( if the pot is negative)
      sprintf(buffer, "STATUS");
      sendMessage(connection_fd, buffer, strlen(buffer));
      //Game status, if GAMOVER break loop
      receiveMessage(connection_fd, buffer, BUFFER_SIZE);
    }
  }
  //print final message for user if pot is negative user lost, otherwise he quitted
  if (pot <= 0 )
    printf("You are out of money, better luck next time \n");
  else
    printf("Thank you for playing, come back soon \n");
}

//Function to print cards and get the total for user display
void printCards(char *cards, int size){
  for(int i = 0; i<size; i++)
    printCard(cards[i]);
  printf("\n");
  printf("total : %d \n", getTotal(cards, size));
}

//function to receive card print and send acknowledge
char receiveCard(int connection_fd, int player){
  char card;
  char buffer[BUFFER_SIZE];
  receiveMessage(connection_fd, buffer, BUFFER_SIZE);
  sscanf(buffer, "%c", &card);
  sprintf(buffer, "RECEIVED");
  sendMessage(connection_fd, buffer, strlen(buffer));
  // printing card with player
  if(player == 1){
    printf("You got card ");
    printCard(card);
    printf("\n");
  }
  if(player == 2){
    printf("Dealer got card ");
    printCard(card);
    printf("\n");
  }
  return card;
}

//function to get total from a players hand
int getTotal (char *cards, int size){
  int total = 0;
  int as = 0;
  for (int i = 0; i < size; i++){
    //if there is an As add 11 and incremnet As counter
    if (cards[i] == 'A'){
      total += 11;
      as ++;
    }
    //convert the letters to 10 and the the rest from char to int
    else if (cards[i] == 'J' || cards[i] == 'Q' || cards[i] == 'K' || cards[i] == '0')
      total += 10;
    else
      total += cards[i] - '0';
  }
  //for every As check if total is over 21, is so deduct 10
  for (int i = 0; i < as; i++){
    if (total > 21)
      total -= 10;
  }
  return total;
}

//function made in order to handle the 0 char to 10
void printCard(char card){
  if (card == '0')
    printf ("1%c ", card);
  else
    printf ("%c ", card);
}
