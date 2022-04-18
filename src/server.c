#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../includes/connect4-utils.h"
#include "../includes/string-utils.h"



#define MAX_CLIENTS 10



typedef struct client_t
{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[32];
	volatile int isPlaying;
}client_t;



void getIp(struct sockaddr_in addr, char ip[]);
void queueAdd(client_t *client);
void queueRemove(client_t* client);
void detachClient(client_t* client);
void sendMessage(char text[], client_t* client);
void sendGrid(char grid[][GRID_LENGTH], client_t* client);
int getPlayerMove(client_t* arg, char grid[][GRID_LENGTH]);
void *waitForOpponent(void *arg);
void *launchGame(void *arg);
int launchServer(char ip[], int port);



static int cli_count = 0;
static int uid = 10;
static client_t *clients[MAX_CLIENTS];
static pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;



int main(int argc, char **argv)
{
	if(argc != 3)
	{
		printf("Usage: %s <IP> <PORT>\n", argv[0]);
		return EXIT_FAILURE;
	}
	char* ip = argv[1];
	int port = atoi(argv[2]);
	return launchServer(ip, port);
}



void getIp(struct sockaddr_in addr, char ip[])
{
	sprintf(ip, "%d.%d.%d.%d",
		(addr.sin_addr.s_addr & 0xff),
		(addr.sin_addr.s_addr & 0xff00) >> 8,
		(addr.sin_addr.s_addr & 0xff0000) >> 16,
		(addr.sin_addr.s_addr & 0xff000000) >> 24);
}



void queueAdd(client_t *client)
{
	pthread_mutex_lock(&clients_mutex);
	for(int i=0; i < MAX_CLIENTS; ++i)
	{
		if(!clients[i])
		{
			clients[i] = client;
			break;
		}
	}
	pthread_mutex_unlock(&clients_mutex);
}



void queueRemove(client_t* client)
{
	int uid = client->uid;
	pthread_mutex_lock(&clients_mutex);
	for(int i=0; i < MAX_CLIENTS; ++i)
	{
		if(clients[i])
		{
			if(clients[i]->uid == uid)
			{
				clients[i] = NULL;
				break;
			}
		}
	}
	pthread_mutex_unlock(&clients_mutex);
}



void detachClient(client_t* client)
{
	close(client->sockfd);
	queueRemove(client);
	free(client);
	cli_count--;
}



void sendMessage(char text[], client_t* client)
{
	char message[BUFFER_SIZE];
	strcpy(message, text);
	if(send(client->sockfd, message, sizeof(message), 0) < 0)
	{
		puts("ERROR: write to descriptor failed");
	}
}



void sendGrid(char grid[][GRID_LENGTH], client_t* client)
{
	char message[BUFFER_SIZE] = {};
	char flatten_grid[100] = {};
	memcpy((char*)flatten_grid, grid, GRID_LENGTH*GRID_LENGTH);
	sprintf(message, "GRID:%s\n", flatten_grid);
	sendMessage(message, client);
}



int getPlayerMove(client_t* arg, char grid[][GRID_LENGTH])
{
	client_t* player = (client_t*)arg;
	char message[BUFFER_SIZE] = {};
	int receive = 0;
	int column = -1;
	do
	{
		sendMessage("\r[?] Which column do you want to play ? \n", player);
		memset(message, 0, sizeof(message));
		receive = recv(player->sockfd, message, sizeof(message), 0);
		if (receive > 0)
		{
			stringTrimLF(message, sizeof(message));
			column = (strlen(message) == 1 && isdigit(*message) > 0) ? atoi(message) : -1;
		}
		else
		{
			return -1;
		}
	}
	while(isColumnPlayable(grid, column) != 1);
	return column;
}



void *launchGame(void *arg)
{
	client_t** players = (client_t**)arg;

	char message[150];
	memset(message, 0, sizeof(message));
	sprintf(message, "[!] Opponent found : %s !\n", players[1]->name);
	sendMessage(message, players[0]);
	sendMessage("[!] The match is about to start...\n", players[0]);
	
	memset(message, 0, sizeof(message));
	sprintf(message, "[!] Opponent found : %s !\n", players[0]->name);
	sendMessage(message, players[1]);
	sendMessage("[!] The match is about to start...\n", players[1]);

	sleep(3);

	char grid[GRID_LENGTH][GRID_LENGTH];
	memset(grid, NOT_PLAYED, GRID_LENGTH*GRID_LENGTH);
	int there_is_a_winner = 0;
	int turn = 0;
	int activePlayer = 0;
	char player_token;
	int column = -1;
	int played = -1;

	sendGrid(grid, players[activePlayer]);
	sendGrid(grid, players[!activePlayer]);

	sleep(0.5);

	while(!there_is_a_winner && turn != (GRID_LENGTH*GRID_LENGTH))
	{
		player_token = (activePlayer==0) ? PLAYER1_TOKEN : PLAYER2_TOKEN;

		int column = getPlayerMove(players[activePlayer], grid);
		if(column >= 0 && column < GRID_LENGTH)
		{
			if((played = play(grid, player_token, column)) == 1)
				printf("[!] %s played column %d\n", players[activePlayer]->name, column);
			else
				printf("[!] %s tried to play column %d\n", players[activePlayer]->name, column);
		}
		else
		{
			printf("/!\\ %s quit...\n", players[activePlayer]->name);
			printf("/!\\ End of the match : %s VS %s => %s wins by surrender !!!\n", players[0]->name, players[1]->name, players[!activePlayer]->name);
			sendMessage("/!\\  Opponent has quit... You win !\n\n", players[!activePlayer]);
			detachClient(players[0]);
			detachClient(players[1]);
			return NULL;
		}

		sendGrid(grid, players[activePlayer]);
		sendGrid(grid, players[!activePlayer]);
		sleep(0.5);

		if(isThereAWinner(grid))
		{
			there_is_a_winner = 1;
			printf("/!\\ End of the match : %s VS %s => %s wins !!!\n", players[0]->name, players[1]->name, players[activePlayer]->name);
			sendMessage("/!\\  Congratulations, you win !\n\n", players[activePlayer]);
			sendMessage("/!\\ Sorry, you lose...\n\n", players[!activePlayer]);
			detachClient(players[0]);
			detachClient(players[1]);
			return NULL;
		}

		activePlayer++;
		activePlayer%=2;
		turn++;
	}
	
	printf("/!\\ End of the match : %s VS %s => DRAW !!!\n", players[0]->name, players[1]->name);
	sendMessage("/!\\ DRAW !\n", players[activePlayer]);
	sendMessage("/!\\ DRAW !\n", players[!activePlayer]);
	detachClient(players[0]);
	detachClient(players[1]);
	return NULL;
}


void *waitForOpponent(void *arg)
{
	char buffer[BUFFER_SIZE] = {};
	char name[BUFFER_SIZE] = {};
	client_t *cli = (client_t*)arg;
	cli_count++;

	if(recv(cli->sockfd, name, BUFFER_SIZE, 0) <= 0 || strlen(name) <  2 || strlen(name) > 32)
	{
		printf("Client did not enter the name correctly\n");
		detachClient(cli);
		return NULL;
	}
	else
	{
		strcpy(cli->name, name);
		printf("[+] %s has joined\n", cli->name);
	}

	memset(buffer, 0, BUFFER_SIZE);

	for(int i=0; i<20; i++)
	{

		if(cli->isPlaying == 1)
			return NULL;
		else
		{
			if(i%5==0)
				sendMessage("[!] Waiting for an opponent...\n", cli);
		}
		sleep(1);
	}
	sendMessage("[!] No opponent found !\n", cli);
	detachClient(cli);
	return NULL;
}


int launchServer(char ip[], int port)
{
	signal(SIGPIPE, SIG_IGN);

	char client_ip[15] = {};
	int option = 1;
	int listenfd = 0;
	int connfd = 0;
	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);
	pthread_t wait_opponent_thread;
	pthread_t game_thread;
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(ip);
	serv_addr.sin_port = htons(port);
	client_t* match[2] = {NULL};

	if(port < 0 || port > 65536)
	{
		puts("ERROR: Port must be in range 0-65536");
		return EXIT_FAILURE;
	}
	if(setsockopt(listenfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char*)&option, sizeof(option)) < 0)
	{
		puts("ERROR: setsockopt failed");
		return EXIT_FAILURE;
	}
	if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
	{
		puts("ERROR: Socket binding failed");
		return EXIT_FAILURE;
	}
	if (listen(listenfd, 10) < 0)
	{
		puts("ERROR: Socket listening failed");
		return EXIT_FAILURE;
	}
	printf("Server listening on port %d...\n", port);


	while(1)
	{
		connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);
		if((cli_count + 1) > MAX_CLIENTS)
		{
			getIp(cli_addr, client_ip);
			printf("[!] Maximum clients reached. Rejected => %s:%d\n", client_ip, cli_addr.sin_port);
			close(connfd);
			continue;
		}

		client_t *cli = (client_t *)malloc(sizeof(client_t));
		cli->address = cli_addr;
		cli->sockfd = connfd;
		strcpy(cli->name, "_anon_");
		cli->uid = uid++;
		cli->isPlaying=0;

		queueAdd(cli);
		pthread_create(&wait_opponent_thread, NULL, &waitForOpponent, (void*)cli);

		match[0] = NULL;
		match[1] = NULL;

		pthread_mutex_lock(&clients_mutex);
		for(int i=0; i<MAX_CLIENTS; ++i)
		{
			if(clients[i])
			{
				if(clients[i]->isPlaying == 0)
				{
					if(match[0] == NULL)
					{
						match[0] = clients[i];
					}
					else
					{
						match[1] = clients[i];
						match[0]->isPlaying = 1;
						match[1]->isPlaying = 1;						

						sleep(1);
						printf("[+] New match : %s VS %s\n", match[0]->name, match[1]->name);
						pthread_create(&game_thread, NULL, &launchGame, (void*)&match);

						break;
					}
				}
			}
		}
		pthread_mutex_unlock(&clients_mutex);
		sleep(1);
	}
	return EXIT_SUCCESS;
}