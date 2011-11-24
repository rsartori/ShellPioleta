//Inclusão de bibliotecas do sistema
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>
#include <sys/wait.h>

//Inclusão de bibliotecas próprias
#include "jobs.h"
#include "terminal.h"
#include "canonical.h"
#include "signal_capture.h"
#include "tela.h"



//Sigactions TSTP
static struct sigaction TSTP;
static struct sigaction oldTSTP;

//Sigactions CHLD
static struct sigaction CHLD;
static struct sigaction oldCHLD;

/*
* Função: capturaSigTSTP (int)
* Descrição: Captura o signal SIGTSTP
*/
void Signal_capturaSigTSTP (int signum, siginfo_t *info, void *context) {
	//Variáveis
	Job* jobAux;

	//Busca Job em Foreground
	jobAux = Jobs_retornaJobEmForeground(Jobs);

	//Não encontrado
	if(jobAux == NULL) printf("Nenhum job em foreground\n!");

	//Encontrado
	else {
		Jobs_colocaJobEmBackground(&Jobs,jobAux->pid);
		//Define status para background
		//jobAux->status = BACKGROUND;
		
		//Pausa se a execução não terminou
		if(jobAux->statusExecucao != TERMINOU) {
			//Pausa a Job
			jobAux->statusExecucao = PAUSADO;
			//Mata e envia sinal
			kill(jobAux->pid, SIGSTOP);
		}
	}
}

/*
* Função: capturaSigCHLD (int, siginfo_t)
* Descrição: Captura o signal SIGCHLD
*/
void Signal_capturaSigCHLD (int signum, siginfo_t *info, void *context) {
	Tela_apagaCaracteres(TERMINAL_TAMANHOLINHA);
	//Variáveis	
	int estado;
	Job *jobAux;

	if(waitpid(-1,&estado,WNOHANG)>0) {
		//while(waitpid(-1,NULL,WNOHANG) > 0) {
		//

	
		//Busca PID
		jobAux = Jobs_retornaJobComPID(&Jobs,info->si_pid);
	
		//Coloca o terminal em modo canônico
		Canonical_setCanonicalMode();

		//A Job está PARADA
		if (WIFSTOPPED(estado)) {
			jobAux->status = BACKGROUND;
			jobAux->statusExecucao = PAUSADO;

			//Ativa o modo não-canônico
			Canonical_setCanonicalMode();
		}
		//A Job está RODANDO
		else if (WIFCONTINUED(estado)) {
			jobAux->statusExecucao = RODANDO;
		}
		//A Job TERMINOU
		else {
			jobAux->statusExecucao = TERMINOU;
			jobAux->status = BACKGROUND;
		}
	}
}

/*
* Função: Signal_instalacao (void)
* Descrição: Efetua a instalação dos handlers
*/
void Signal_Instalacao(void) {
	//Configura handler TSTP
	sigemptyset(&TSTP.sa_mask);
	TSTP.sa_flags = SA_SIGINFO;
	TSTP.sa_sigaction = Signal_capturaSigTSTP;

	//Instala handler TSTP
	sigaction(SIGTSTP, &TSTP, &oldTSTP);

	//Configura handler CHLD
	sigemptyset(&CHLD.sa_mask);
	CHLD.sa_flags = SA_SIGINFO;
	CHLD.sa_sigaction = Signal_capturaSigCHLD;

	//Instala handler CHLD
	sigaction(SIGCHLD, &CHLD, &oldCHLD);
}

/**
 * $name children_sig_setup;
 * $proto void children_sig_setup(void);
 *
 * Restaura os handlers anteriores aa execucao de sig_setup(). Para ser
 * usada nos processos filhos do shell.
 */

void Signal_InstalacaoFilhos(void)
{
    sigaction(SIGTSTP, &oldTSTP, NULL);
    sigaction(SIGCHLD, &oldCHLD, NULL);
}
