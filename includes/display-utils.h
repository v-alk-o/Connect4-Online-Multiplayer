#ifndef DISPLAY_UTILS_H
#define DISPLAY_UTILS_H

#define COLOR_RESET "\e[0m"
#define RED "\e[0;31m"
#define YEL "\e[0;33m"
#define BLU "\e[0;34m"

void clearScreen();
void printBanner();
void printBlueString(char str[]);

#endif