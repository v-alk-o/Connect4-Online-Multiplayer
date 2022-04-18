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
#include "../includes/display-utils.h"



static volatile int flag = 0;
static int sockfd = 0;



void sigintHandler(int sig);
void sendingMessageHandler();
void receivingMessageHandler();
int launchClient(char ip[], int port);



int main(int argc, char **argv)
{
	if(argc != 3)
	{
		printf("Usage: %s <IP> <PORT>\n", argv[0]);
		return EXIT_FAILURE;
	}
	char* ip = argv[1];
	int port = atoi(argv[2]);
	return launchClient(ip, port);
}



void sigintHandler(int sig)
{
	flag = 1;
}



void sendingMessageHandler()
{
	char message[BUFFER_SIZE] = {};
	while(1)
	{
		memset(message, 0, sizeof(message));
		fgets(message, sizeof(message), stdin);
		stringTrimLF(message, sizeof(message));
		send(sockfd, message, sizeof(message), 0);
	}
}



void receivingMessageHandler()
{
	char message[BUFFER_SIZE] = {};
	char grid[GRID_LENGTH][GRID_LENGTH];
	while(1)
	{
		int receive = recv(sockfd, message, sizeof(message), 0);
		if (receive > 0)
		{
			if(strstr(message, "[!]") != NULL)
			{
				printf("%s", message);
			}
			else if(strstr(message, "[?]") != NULL)
			{
				printf("%s", message);
				printf("> ");
				fflush(stdout);
			}
			else if(strstr(message, "GRID:") != NULL)
			{
				memcpy(grid, message+5, GRID_LENGTH*GRID_LENGTH);
				clearScreen();
				printBanner();
				printGrid(grid);
			}
			else if(strstr(message, "/!\\") != NULL)
			{
				printBlueString(message);
				flag=1;
			}
			else
			{
				printf("ERROR: Could not understand reponse from the server\n");
			}
		}
		else if (receive == 0)
		{
			break;
		}
		memset(message, 0, sizeof(message));
	}
}



int launchClient(char ip[], int port)
{
	signal(SIGINT, sigintHandler);

	struct sockaddr_in server_addr;
	pthread_t sending_message_thread, receiving_message_thread;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);
	server_addr.sin_port = htons(port);

	printf("Enter your name : ");
	fflush(stdout);
	char name[BUFFER_SIZE] = {};
	fgets(name, BUFFER_SIZE, stdin);
	stringTrimLF(name, sizeof(name));

	if(strlen(name) < 2 || strlen(name) > 32)
	{
		puts("ERROR: Your name must be more than 2 and less than 30 characters\n");
		return EXIT_FAILURE;
	}
	if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
	{
		puts("ERROR: Failed to connect to the server\n");
		return EXIT_FAILURE;
	}

	send(sockfd, name, 32, 0);

	if(pthread_create(&sending_message_thread, NULL, (void*)sendingMessageHandler, NULL) != 0)
	{
		puts("ERROR: Failed creating sending_message_thread\n");
		return EXIT_FAILURE;
	}
	if(pthread_create(&receiving_message_thread, NULL, (void*)receivingMessageHandler, NULL) != 0)
	{
		puts("ERROR: Failed creating receiving_message_thread\n");
		return EXIT_FAILURE;
	}

	while(1)
	{
		if(flag)
		{
			break;
		}
	}
	close(sockfd);
	return EXIT_SUCCESS;
}