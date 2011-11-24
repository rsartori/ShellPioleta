//Inclusão de bibliotecas do sistema
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//Inclusão de bibliotecas próprias
#include "tela.h"
#include "erros.h" 
#include "alocacao.h"
#include "signal_capture.h"
#include "canonical.h"
#include "terminal.h"
#include "comando.h"
#include "jobs.h"

/* 
* Função: Terminal_insereLinhaNoHistorico (char*, char**, int)
* Descrição: Insere uma Linha de Comando no Histórico de Linhas
*/
int Terminal_insereLinhaNoHistorico (char *LinhaComando, char **LinhasComando, int numLinhasComando) {
	//Insere LinhaComando na Lista de Comandos	
	strcpy(LinhasComando[numLinhasComando],LinhaComando);	
	//Retorno
	return 0;
}


char* compararTab(char *buffer) {
	

	return NULL;
}

/*
* Função: Terminal_processaTeclaHistorico (char**, char*, char*, int*, int*, int)
* Descrição: Processa as teclas de comando enviadas pelo usuário
*/
void Terminal_processaTeclaHistorico (char **LinhasComando, char *LinhaComando, char *tecla, int *contadorTeclas, int *linesOver, int numLinhasComando) {
	//Tecla CIMA
	if(tecla[0] == 27 && tecla[1] == 91 && tecla[2] == 65) {
		//Zera buffer das teclas
		Tela_flushKeys(tecla);

		//Linha atual
		if(numLinhasComando-(*linesOver) > 0) {
			//Apaga o buffer escrito na tela
			Tela_apagaCaracteres(TERMINAL_TAMANHOLINHA);
			Tela_imprimeShell();

			//Atualiza o número de linhas além da atual
			(*linesOver)++;

			//Salva a linha anterior em LinhaComando
			strcpy(LinhaComando, LinhasComando[numLinhasComando-(*linesOver)]);

			//Recupera contador de teclas
			*contadorTeclas = strlen(LinhaComando);

			//Imprime Linha de Comando
			printf("%s", LinhaComando);
		}
	}
	//Tecla BAIXO
	else if(tecla[0] == 27 && tecla[1] == 91 && tecla[2] == 66) {
		//Zera buffer das teclas
		Tela_flushKeys(tecla);

		//Apaga o buffer escrito na tela
		Tela_apagaCaracteres(TERMINAL_TAMANHOLINHA);
		Tela_imprimeShell();


		//Última linha
		if(*linesOver == 0) strcpy(LinhaComando,"");

		//Penúltima linha ou anterior
		else if(*linesOver > 0) {
			(*linesOver)--;
			strcpy(LinhaComando, LinhasComando[numLinhasComando - (*linesOver)]);
		}

		//Recupera contador de teclas
		*contadorTeclas = strlen(LinhaComando);

		//Imprime Linha de Comando
		printf("%s", LinhaComando);
	}
}

/*
* Função: Terminal_processaTeclaComando (char*, char*, int)
* Descrição: Processa as teclas de comando enviadas pelo usuário
*/
void Terminal_processaTeclaComando (char *LinhaComando, char *tecla, int contadorTeclas, int modoBackground) {
	Job *jobAux;

	//Salva em jobAux a job em FOREGROUND
	jobAux = Jobs_retornaJobEmForeground(Jobs);

	//Comando CTRL+L (Limpa a tela)
	if((tecla[0] == 12 || tecla[1] == 12 || tecla[2] == 12) && modoBackground == OFF) {
		//Zera buffer das teclas
		Tela_flushKeys(tecla);

		//Limpa a tela
		printf("\n\n\n\n\n\n\n");
		Tela_apagaCaracteres(TERMINAL_TAMANHOLINHA);
        	Tela_imprimeShell();

		//Imprime Linha de Comando
		printf("%s",LinhaComando);
	}
	//Comando CTRL+C (Envia SIGTSTP para todos processos em Foreground)
	else if((tecla[0] == 3 || tecla[1] == 3 || tecla[2] == 3) && contadorTeclas == 0) {
		//Zera buffer das teclas
		Tela_flushKeys(tecla);
		
		//Envia sinal de interrupção
		if(Jobs_retornaJobEmForeground(Jobs) != NULL) kill(Jobs_retornaJobEmForeground(Jobs)->pid,SIGTSTP/*SIGINT*/);
		else {
			printf("\n");
			Tela_apagaCaracteres(TERMINAL_TAMANHOLINHA);
			printf("Nao ha jobs em foreground!\n");
			Tela_apagaCaracteres(TERMINAL_TAMANHOLINHA);
		}

		//Limpa a linha de comando
		strcpy(LinhaComando,"");
		Tela_imprimeShell();
	}
	//Comando CTRL+Z
	else if(tecla[0] == 26 || tecla[1] == 26 || tecla[2] == 26) {
		//Zera buffer das teclas
		Tela_flushKeys(tecla);
		
		//Envia sinal de interrupção
		if(jobAux != NULL) {
			//Opção #1
			//kill(jobAux->pid, SIGTSTP);

			//Opção #2
			//Coloca Job em Background e PAUSADA
			Jobs_colocaJobEmBackground(&Jobs,jobAux->pid);
			jobAux->statusExecucao = PAUSADO;

			//Envia sinal de interrupção para a Job
			kill(jobAux->pid, SIGSTOP);
		}
		else {
			printf("\n");
			Tela_apagaCaracteres(TERMINAL_TAMANHOLINHA);
			printf("Nao ha jobs em foreground!\n");
			Tela_apagaCaracteres(TERMINAL_TAMANHOLINHA);
		}

		//Limpa a linha de comando
		strcpy(LinhaComando,"");

		//Imprime a shell novamente
		//printf("\n");
		//Tela_apagaCaracteres(TERMINAL_TAMANHOLINHA);
		Tela_imprimeShell();
	}
}

/*
* Função: Terminal_processaLinha (char*, char**, int)
* Descrição: Lê e controla a Linha de Comando, de modo não-canônico
*/
char* Terminal_processaLinha (char **LinhasComando, int numLinhasComando) {
	//Variáveis
	int keyIndex, contadorTeclas, linesOver, okTab, i, temAbrev, stopTab;
	char caractereDigitado, tecla[3], *LinhaComando, *bufferAux, *bufA, *bufB;
	FILE *tabelaTab;
	//COMENTAR!!!
	stopTab = 0;
	
	tabelaTab = fopen("tabela.txt", "r+");
	if(tabelaTab != NULL) okTab = 1;
	
	//Aloca a Linha de Comando
	LinhaComando = Alocacao_alocaVetor(TERMINAL_TAMANHOLINHA);

	//Reseta a Linha de Comando
	strcpy(LinhaComando,"");

	//Reseta a Linha de Comando Auxiliar
	//strcpy(LinhaComandoAux, "");

	//Condições iniciais
	keyIndex = 0;
	linesOver = 0;
	contadorTeclas = 0;

	//Zera buffer das teclas
	Tela_flushKeys(tecla);

	while ((caractereDigitado = getchar())) {
		//Corrige o problema de formatação quando a última linha de comando continha um '&'
		if(contadorTeclas == 0) {
			if(numLinhasComando>0)
				if(LinhasComando[numLinhasComando-1][strlen(LinhasComando[numLinhasComando-1])-1] == '&') {
					Tela_apagaCaracteres(TERMINAL_TAMANHOLINHA);
					//Tela_imprimeShell();
				}
		}

		//Armazena a tecla
		tecla[keyIndex] = caractereDigitado;

		//Incrementa o keyIndex
		keyIndex++;
		
		//Reseta o keyIndex a cada 3 caracteres
		if(keyIndex > 2) keyIndex = 0;

		//Teclas de Navegação de Histórico (CIMA e BAIXO)
		if((tecla[0] == 27 && tecla[1] == 91 && tecla[2] == 65) || (tecla[0] == 27 && tecla[1] == 91 && tecla[2] == 66)) {
			Terminal_processaTeclaHistorico (LinhasComando, LinhaComando, tecla, &contadorTeclas, &linesOver, numLinhasComando);
		}


		//Teclas de Comando (Ctrl+L, Ctrl+C, Ctrl+Z)
		else if((	tecla[0] == 12	|| tecla[1] == 12	|| tecla[2] == 12	||
				tecla[0] == 3	|| tecla[1] == 3	|| tecla[2] == 3	||
				tecla[0] == 26	|| tecla[1] == 26	|| tecla[2] == 26)) {
			Terminal_processaTeclaComando(LinhaComando, tecla, contadorTeclas, OFF);
		}
	
		//TAB
		else if(tecla[0] == 9){
			if(strlen(LinhaComando) > 2 && okTab == 1){
				while(stopTab != 1) {
					bufferAux = (char *) malloc(sizeof(char) * 50);

					if(fgets(bufferAux, 50, tabelaTab) != NULL) {
						bufferAux[strlen(bufferAux) - 1] = '\0';

						bufA = (char *) malloc(sizeof(char) * 50);
						bufB = (char *) malloc(sizeof(char) * 50);

						temAbrev = 0;
						for(i=0; bufferAux[i] != '\0'; i++) {
		
							if(bufferAux[i] == ' '){
								temAbrev = 1;
							}
						}
					
						if(temAbrev == 1){
							for(i=0; bufferAux[i] != ' '; i++) {
								bufB[i] = bufferAux[i];
							}	
							bufB[i] = '\0';
							i++;

							for(; bufferAux[i] != '\0'; i++) {
								bufA[i] = bufferAux[i];
							}
							bufA[i] = '\0';
	
							if(strcmp(bufA, LinhaComando) == 0){
								printf("%s", bufB);

								Tela_apagaCaracteres(strlen(bufB));
								contadorTeclas = contadorTeclas - strlen(LinhaComando);
								contadorTeclas = contadorTeclas + strlen(bufB);
							}
						}
						
						free(bufA);
						free(bufB);						
					}
					else {
						stopTab = 1; // acabou a lista
					}

					

					free(bufferAux);
				}
			} 
			
		}

		//Nenhuma tecla acima
		else if(tecla[0] != 27 && tecla[0] != 12 && tecla[0] != 4 && tecla[0] != 26 && tecla[0] != 3) {
			//Zera buffer das teclas
			Tela_flushKeys(tecla);

			//Caracteres imprimíveis
			if(caractereDigitado >= 32 && caractereDigitado <= 126) {				
				//Imprime caractere digitado
				printf("%c", caractereDigitado);

				//Armazena caractere na linha de Comando
				LinhaComando[contadorTeclas] = caractereDigitado;

				//Atualiza contador de teclas digitadas				
				contadorTeclas++;

				//Reseta keyIndex
				keyIndex = 0;

				//Encerra processamento da Linha de Comando
				LinhaComando[contadorTeclas] = '\0';
			}
			
			//Enter 
			else if(caractereDigitado == 13) {
				//Algo foi digitado
				if(strcmp(LinhaComando, "") != 0) printf("%c\n",caractereDigitado);
				else {
					printf("%c\n",caractereDigitado);
					Tela_apagaCaracteres(TERMINAL_TAMANHOLINHA);
					return NULL;
				}
				break;
			}

			//Backspace
			else if(caractereDigitado == 127 && contadorTeclas > 0) {
				//Apaga um caractere da tela
				Tela_apagaCaracteres(1);
				
				//Atualiza contador de teclas digitadas	
				contadorTeclas--;

				//Encerra processamento da Linha de Comando
				LinhaComando[contadorTeclas] = '\0';
			}
		}
	}
	//Retorno
	return LinhaComando;
}

/*
* Função: Terminal_InterpretaLinhaComando (char*, char**)
* Descrição: Interpreta a Linha de Comando digitada
*/
void Terminal_InterpretaLinhaComando (char *LinhaComando, char **LinhasComando) {
	//Variáveis	
	int iContador, numParametros, status;
	char **Parametro, *Comando, *tokenPalavra, *espacoAux;
	
	//Condições iniciais
	iContador = 0;
	numParametros = 0;

	//BACKGROUND
	if(LinhaComando[strlen(LinhaComando)-1] == '&') {
		status = BACKGROUND;
		//Remove ' &' da linha de comando
		LinhaComando[strlen(LinhaComando)-1] = '\0';
		LinhaComando[strlen(LinhaComando)] = '\0';
		Tela_apagaCaracteres(TERMINAL_TAMANHOLINHA);
		//Jobs_imprimeJobs(Jobs);
	}
	//FOREGROUND
	else status = FOREGROUND;

	//Encontra número de parâmetros
	espacoAux = strpbrk(LinhaComando," ");
	while(espacoAux != NULL) {
		numParametros++;
		espacoAux = strpbrk(espacoAux+1," ");
	}
	numParametros++;

	//Aloca conjunto de parâmetros
	Parametro = malloc(numParametros*sizeof(char*));

	//Limpa parâmetros que ficaram alocados
	while(Parametro[iContador] != NULL) {
		free(Parametro[iContador]);
		iContador++;
	}
	iContador = 0;

	//Quebra Linha de Comando em Palavras
	tokenPalavra = (char*) strtok(LinhaComando, " ");
	
	if(tokenPalavra == NULL) {
		Parametro[0] = Alocacao_alocaVetor(strlen(LinhaComando));
		strcpy(Parametro[0],LinhaComando);
	}
	else {
		while(tokenPalavra != NULL) {
			//Aloca parâmetro
			Parametro[iContador] = Alocacao_alocaVetor(strlen(tokenPalavra));
		
			//Salva palavra
			strcpy(Parametro[iContador], tokenPalavra);
			
			//Incrementa o número de palavras
			iContador++;

			//Percorre o token
			tokenPalavra = (char*) strtok(NULL, " ");

			
		}
	}

	//Libera memória alocada
	free(tokenPalavra);

	//Aloca Comando
	Comando = Alocacao_alocaVetor(strlen(Parametro[0]));

	//Armazena Comando
	if(iContador >=0) strcpy(Comando, Parametro[0]);

	//Comandos Built-In
	if(Comando_isBuiltIn(Parametro)) Comando_rodaBuiltIn(Parametro);
	
	//Demais comandos
	else Comando_rodaLinhaComando(Parametro, numParametros, status);

	//Libera memória alocada
	free(tokenPalavra);
	//for(iContador=0;iContador<numParametros;iContador++) free(Parametro[iContador]);
	//free(Parametro);
}

/*
* Função: Terminal_modoBackground(pid_t)
* Descrição: Simula modo Background da Shell (só interpreta teclas de comando)
*/
/*
void Terminal_modoBackground(pid_t pid) {
	//Variáveis
	int countTeclas = 0;
	char caractereDigitado, tecla[3], LinhaComandoVazia[1], *comando;
	Job *jobAux;

	//Busca job
	jobAux = Jobs_retornaJobComPID(&Jobs,pid);

	comando = Alocacao_alocaVetor(TERMINAL_TAMANHOPALAVRA);

	//Avisa usuário
	Tela_apagaCaracteres(TERMINAL_TAMANHOLINHA);
	printf("[MODO BACKGROUND]\n");
	Tela_apagaCaracteres(TERMINAL_TAMANHOLINHA);

	//Inicializa Linha de Comando Vazia
	strcpy(LinhaComandoVazia,"");
	
	//Zera buffer das teclas
	Tela_flushKeys(tecla);
	
	strcpy(comando,"");

	scanf("%s",comando);
	comando[strlen(comando)] = '\0';

	//strcpy(comando,"ctrlz");
	//Ctrl + Z
	if(!strcmp(comando,"ctrlz")) {
		//Coloca Job em Background e PAUSADA
		Jobs_colocaJobEmBackground(&Jobs,jobAux->pid);
		jobAux->statusExecucao = PAUSADO;

		//Envia sinal de interrupção para a Job
		kill(jobAux->pid, SIGSTOP);
		return;
	}

	////Ctrl + C
	else if(!strcmp(comando,"ctrlc")) {
		//IMPLEMENTAR
	}

	strcpy(comando,"");

	//Comando inválido
	if(strlen(comando)>0) Terminal_modoBackground(pid);

	//Coloca no modo canônico
	Canonical_setCanonicalMode();

	//Imprime a shell
	printf("\n");
	Tela_apagaCaracteres(TERMINAL_TAMANHOLINHA);
}
*/
