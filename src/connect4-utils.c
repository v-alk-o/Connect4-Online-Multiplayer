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
#include "../includes/display-utils.h"



void printRedToken()
{
	printf(RED "%c  " COLOR_RESET, GENERIC_TOKEN);
}



void printYellowToken()
{
	printf(YEL "%c  " COLOR_RESET, GENERIC_TOKEN);
}



void printGrid(char grid[][GRID_LENGTH])
{
	printf("\n     ");
	for(int cpt=0; cpt<GRID_LENGTH; cpt++)
		printf("%d  ", cpt);
	printf("\n");
	for(int i=0; i<GRID_LENGTH; i++)
	{
		printf("%d |  ", (GRID_LENGTH-1-i));
		for(int j=0; j<GRID_LENGTH; j++)
		{
			if(grid[i][j] == NOT_PLAYED)
				printf("%c  ",  grid[i][j]);
			else if(grid[i][j] == PLAYER1_TOKEN)
				printRedToken();
			else
				printYellowToken();
		}
		printf("| %d \n", (GRID_LENGTH-1-i));
	}
	printf("     ");
	for(int cpt=0; cpt<GRID_LENGTH; cpt++)
		printf("%d  ", cpt);
	printf("\n\n");
}



int isThereAWinner(char grid[][GRID_LENGTH])
{
	for(int j=0; j<GRID_LENGTH; j++)
	{
		for(int i=GRID_LENGTH-1; i>=0; i--)
		{
			if(grid[i][j] != NOT_PLAYED)
			{
				if(i+3<GRID_LENGTH)
				{
					if(grid[i][j] == grid[i+1][j] && grid[i][j] == grid[i+2][j] && grid[i][j] == grid[i+3][j])
						return 1;
				}
				if(j+3<GRID_LENGTH)
				{
					if(grid[i][j] == grid[i][j+1] && grid[i][j] == grid[i][j+2] && grid[i][j] == grid[i][j+3])
						return 1;
				}
				if(i+3<GRID_LENGTH && j+3<GRID_LENGTH)
				{
					if(grid[i][j] == grid[i+1][j+1] && grid[i][j] == grid[i+2][j+2] && grid[i][j] == grid[i+3][j+3])
						return 1;
				}
				if(i+3<GRID_LENGTH && j-3>=0)
				{
					if(grid[i][j] == grid[i+1][j-1] && grid[i][j] == grid[i+2][j-2] && grid[i][j] == grid[i+3][j-3])
						return 1;
				}
			}
			else
				break;
		}
	}
	return 0;
}



int isColumnPlayable(char grid[][GRID_LENGTH], int column)
{
	if(column>=0 && column<=(GRID_LENGTH-1))
	{
		for(int i=GRID_LENGTH; i>=0; i--)
		{
			if(grid[i][column] == NOT_PLAYED)
			{
				return 1;
			}
		}
	}
	return 0;
}



int play(char grid[][GRID_LENGTH], char token, int column)
{
	if(column>=0 && column<=(GRID_LENGTH-1))
	{
		for(int i=GRID_LENGTH; i>=0; i--)
		{
			if(grid[i][column] == NOT_PLAYED)
			{
				grid[i][column] = token;
				return 1;
			}
		}
	}
	return -1;
}