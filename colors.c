//Inclusão de bibliotecas do sistema
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Inclusão de bibliotecas próprias
#include "colors.h"

void Color_red (char string[]) {
	printf("%s%s $%s ", C_RED, string, RESTORE);
}

void Color_green (char string[]) {
	printf("%s%s $%s ", C_GREEN, string, RESTORE);
}

void Color_orange (char string[]) {
	printf("%s%s $%s ", C_ORANGE , string, RESTORE);
}

void Color_blue(char string[]) {
	printf("%s%s $%s ", C_BLUE, string, RESTORE);
}

void Color_pink(char string[]) { 
	printf("%s%s $%s ", C_PINK, string, RESTORE);
}

void Color_cyan(char string[]) {
	printf("%s%s $%s ", C_CYAN, string, RESTORE);
}

void Color_white (char string[]) { 
	printf("%s%s $%s ", C_WHITE, string, RESTORE);
}

void Color_black (char string[]) {
	printf("%s%s $%s  ", C_BLACK, string, RESTORE);
}
