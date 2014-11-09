/*****************
* Maja Zalewska  *
* nr 336088		 *  
* Zadanie 1		 *
*****************/

#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "err.h"

#define BUF_SIZE 		27
#define END_CALC		'q'
#define RUN_CALC		'a'
#define LAST_TO_FORK	'f'
#define CODE_LENGTH		3

int main(int argc, char* argv[])
{
	int row_number = 1;		/* number of rows to count, starting from 1 */
	int pipe_dsc_back [2];	/* pipe f_or comunication back */
	int pipe_dsc_forth [2];	/* pipe for communication forth */
	int buf_len = 0;		
	char buf [BUF_SIZE];	
	char state;				/* [RUN_CALC, LAST_TO_FORK, END_CALC] */

	/* check number of arguments */
	if (argc < 2) {
		syserr("Too few arguments\n");
	}

	/* read arguments */
	if((row_number = atoi(argv[1])) <= 0) {
		syserr("You have to give number greater than 0\n");
	}

	/* create pipe's and the first process - w(1) */
	if(pipe(pipe_dsc_back) == -1) {
		syserr("Error in creating pipe_forth\n");
	}
	if(pipe(pipe_dsc_forth) == -1)
		syserr("Error in creating pipe_dsc_forth\n");
	switch(fork()) {
		case -1:
			syserr("Error in fork\n");

		case 0:
			/* duplicate corresponding descriptors */
			if(dup2(pipe_dsc_back[1], STDOUT_FILENO) == -1) {
				syserr("P:Error in dup pipe_dsc_back[1]\n");
			}
			if(dup2(pipe_dsc_forth[0], STDIN_FILENO) == -1) {
				syserr("P:Error in dup pipe_dsc_forth[0]\n");
			}

			/* close unused descriptors */
			if(close(pipe_dsc_back[0]) == -1) {
				syserr("P:Error in close pipe_dsc_back[0]\n");
			}
			if(close(pipe_dsc_back[1]) == -1) {
				syserr("P:Error in close pipe_dsc_back[1]\n");
			}
			if(close(pipe_dsc_forth[0]) == -1) {
				syserr("P:Error in close pipe_dsc_forth[0]\n");
			}
			if(close(pipe_dsc_forth[1]) == -1) {
				syserr("P:Error in close pipe_dsc_forth[1]\n");
			}

			execl("./w", "w", (char *) 0);
			syserr("Error in execl(w).\n");

		default:
			if(close(pipe_dsc_back[1]) == -1) {
				syserr("P:Error in close pipe_dsc_back[1]\n");
			}
			if(close(pipe_dsc_forth[0]) == -1) {
				syserr("P:Error in close pipe_dsc_forth[0]\n");
			}
	}
	
	state = (row_number == 1) ? END_CALC : RUN_CALC; 
	
	int i = 1;
	int first_value = 0;
	while (i <= row_number) {
		/* send state and first_value */
		/* when it's forlast iteration -> last new process to create */
		memset(buf, 0, BUF_SIZE);
		if (i == (row_number - 1)) {
			state = LAST_TO_FORK;
		}
		buf_len = sprintf(buf, "%d %d", state, first_value);
		if ((buf_len = write (pipe_dsc_forth[1], buf, sizeof(buf))) == -1)
			syserr("Pascal: iteration nr %d. Error in write", i);
		if (i < row_number)
			if ((buf_len = read (pipe_dsc_back[0], buf, sizeof(buf))) == -1)
				syserr("Pascal. Error in read\n");
		i++;
		/* when last iteration -> no more calculation */
		if (i == row_number)
			state = END_CALC;
	}

	i = 1;
	/* read the result -> wait for n answers */
	while (i <= row_number) {
		if ((buf_len = read (pipe_dsc_back[0], buf, sizeof(buf))) == -1)
			syserr("Pascal. Error in read\n");
		printf("%ld ", strtol(buf+CODE_LENGTH, NULL, 10) );
		i++;
	}
	printf("\n");

	/*czekanie na dzieci*/
	if(wait(0) == -1) {
		syserr("Error in wait\n");
	}

	return 0;

}