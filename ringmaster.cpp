#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <vector>

/**
citations:
    1) TCP example by Brian Rogers, updated by Rabih Younes. Duke University.
*/

class playerClass {
 public:
  int playerNo;
  int socketFd;
  char hostName[512];
  char port[512];
};

/**
    To Set up primary Socket to isten from
*/
int setUpSocket(struct addrinfo ** hosts, char * port) {
  struct addrinfo hints;

  memset(&hints, 0, sizeof(hints));

  hints.ai_flags = AF_INET;         // To return address family from both IPV4 and IPV6.
  hints.ai_socktype = SOCK_STREAM;  // Connection based protocol (i.e. TCP).
  hints.ai_flags = AI_PASSIVE;      // Return Socket will be suitable for bind and accept.

  int status = getaddrinfo(NULL, port, &hints, hosts);
  if (status != 0) {
    std::cerr << "Error cannot get the addresses" << std::endl;
    exit(EXIT_FAILURE);
  }

  int socket_fd =
      socket((*hosts)->ai_family, (*hosts)->ai_socktype, (*hosts)->ai_protocol);
  if (socket_fd == -1) {
    std::cerr << "Error cannot create socket" << std::endl;
    exit(EXIT_FAILURE);
  }

  int yes = 1;
  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
  status = bind(socket_fd, (*hosts)->ai_addr, (*hosts)->ai_addrlen);
  if (status == -1) {
    std::cerr << "Error cannot bind socket" << std::endl;
    exit(EXIT_FAILURE);
  }

  status = listen(socket_fd, 100);
  if (status == -1) {
    std::cerr << "Error cannot listen on socket" << std::endl;
    exit(EXIT_FAILURE);
  }
  return socket_fd;
}

int main(int argc, char * argv[]) {
  if (argc < 4) {
    std::cerr << "Required no of arguments is not provided for this function"
              << std::endl;
    exit(EXIT_FAILURE);
  }

  char * port = argv[1];
  int no_players = atoi(argv[2]);
  int no_hops = atoi(argv[3]);
  struct addrinfo * hosts;

  std::cout << "Potato Ringmaster" << std::endl;
  std::cout << "Players = " << no_players << std::endl;
  std::cout << "Hops = " << no_hops << std::endl;
  std::vector<playerClass> players;

  int socket_fd = setUpSocket(&hosts, port);

  //Setting up all palyers info
  for (int i = 0; i < no_players; i++) {
    playerClass player;
    player.playerNo = i;

    //Receving socket
    player.socketFd = accept(socket_fd, NULL, NULL);
    if (player.socketFd == -1) {
      std::cerr << "Error cannot accept connection on socket" << std::endl;
      exit(EXIT_FAILURE);
    }

    //Receving hostname
    recv(player.socketFd, player.hostName, 512, 0);

    //Receving portname
    recv(player.socketFd, player.port, 512, 0);

    players.push_back(player);

    std::cout << player.hostName << std::endl;
    std::cout << player.port << std::endl;

    std::cout << "Player " << i << " is ready to play" << std::endl;
  }

  //Sending player info
  for (int i = 0; i < no_players; i++) {
    //Send Left player info
    int left = (no_players + (i - 1) % no_players) % no_players;

    send(players[i].socketFd, players[left].hostName, 512, 0);
    send(players[i].socketFd, players[left].port, 512, 0);

    //Send Right player info
    int right = (i + 1) % no_players;

    send(players[i].socketFd, players[right].hostName, 512, 0);
    send(players[i].socketFd, players[right].port, 512, 0);

    //Send Current Player Info
    const char * curr_player_info = std::to_string(players[i].playerNo).c_str();
    send(players[i].socketFd, curr_player_info, 512, 0);

    //Send total Player
    const char * total_player_info = std::to_string(no_players).c_str();
    send(players[i].socketFd, total_player_info, 512, 0);

    close(players[i].socketFd);
  }

  freeaddrinfo(hosts);
  close(socket_fd);

  return EXIT_SUCCESS;
}
