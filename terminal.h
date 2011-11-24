#ifndef TERMINAL_H_INCLUDED
#define TERMINAL_H_INCLUDED
#endif

//Inclusão de bibliotecas do sistema
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
* Conjunto de definições de Terminal
*/
#define TERMINAL_TAMANHODIRETORIO 128
#define TERMINAL_TAMANHOPATH 512
#define TERMINAL_TAMANHOUSUARIO 32
#define TERMINAL_NUMLINHAS 128
#define TERMINAL_TAMANHOLINHA 512
#define TERMINAL_TAMANHOPALAVRA 32
#define ON 1
#define OFF 0

/* Path */
char **Path;

/* 
* Função: Terminal_insereLinhaNoHistorico (char*, char**, int)
* Descrição: Insere uma Linha de Comando no Histórico de Linhas
*/
int Terminal_insereLinhaNoHistorico (char *LinhaComando, char **LinhasComando, int numLinhasComando);

/*
* Função: Terminal_processaTeclaHistorico (char**, char*, char*, int*, int*, int)
* Descrição: Processa as teclas de comando enviadas pelo usuário
*/
void Terminal_processaTeclaHistorico (char **LinhasComando, char *LinhaComando, char *tecla, int *contadorTeclas, int *linesOver, int numLinhasComando);

/*
* Função: Terminal_processaTeclaComando (char*, char*, int)
* Descrição: Processa as teclas de comando enviadas pelo usuário
*/
void Terminal_processaTeclaComando (char *LinhaComando, char *tecla, int contadorTeclas, int modoBackground);


char* compararTab(char *buffer);

/*
* Função: Terminal_processaLinha (char*, char**, int)
* Descrição: Lê e controla a Linha de Comando, de modo não-canônico
*/
char* Terminal_processaLinha (char **LinhasComando, int numLinhasComando);

/*
* Função: Terminal_InterpretaLinhaComando (char*, char**)
* Descrição: Interpreta a Linha de Comando digitada
*/
void Terminal_InterpretaLinhaComando (char *LinhaComando, char **LinhasComando);

/*
* Função: Terminal_modoBackground(pid_t)
* Descrição: Simula modo Background da Shell (só interpreta teclas de comando)
*/
void Terminal_modoBackground(pid_t pid);
