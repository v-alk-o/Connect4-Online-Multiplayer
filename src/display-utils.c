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

#include "../includes/display-utils.h"


void clearScreen()
{
    system("clear");
}



void printBanner()
{
	printf(BLU);
	puts(
		"___________________________________________________________\n"
		"               __  __    _  _____ ____ _   _               \n"
		"              |  \\/  |  / \\|_   _/ ___| | | |            \n"
		"              | |\\/| | / _ \\ | || |   | |_| |            \n"
		"              | |  | |/ ___ \\| || |___|  _  |             \n"
		"              |_|  |_/_/   \\_|_| \\____|_| |_|           \n"
		"___________________________________________________________\n"
		);
	printf(COLOR_RESET);
}


void printBlueString(char str[])
{
	printf(BLU "%s" COLOR_RESET, str);
}