#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "../includes/string-utils.h"



void stringTrimLF(char str[], int length)
{
	for (int i = 0; i < length; i++)
	{
		if (str[i] == '\n')
		{
			str[i] = '\0';
			break;
		}
	}
}
