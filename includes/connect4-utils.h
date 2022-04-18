#ifndef CONNECT4_UTILS_H
#define CONNECT4_UTILS_H

#define BUFFER_SIZE 2048
#define GRID_LENGTH 10
#define NOT_PLAYED ' '
#define GENERIC_TOKEN 'O'
#define PLAYER1_TOKEN '1'
#define PLAYER2_TOKEN '2'

void printRedToken();
void printYellowToken();
void printGrid(char grid[][GRID_LENGTH]);
int isThereAWinner(char grid[][GRID_LENGTH]);
int isColumnPlayable(char grid[][GRID_LENGTH], int column);
int play(char grid[][GRID_LENGTH], char token, int column);

#endif