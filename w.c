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

#define BUF_SIZE 			27
#define END_CALC			'q'
#define RUN_CALC			'a'
#define LAST_TO_FORK 		'f'
#define SEND_ALL_BACK		's'
#define CODE_LENGTH			3


int main(int argc, char* argv[])
{
	int pipe_dsc_back [2];	/* pipe for sending back to pascal */
	int pipe_dsc_forth [2];	/* pipe for sending forth */
	long int result = 0;	/* current value */
	long int old_value = 0;	/* value from previous iteration */
	char buf [BUF_SIZE];	/* buffer used to read and write messages */
	int buf_len = 0;
	int is_last = 1;		/* am I the last process */ 
	char state = RUN_CALC;	/* last calcualtion (END_CALC), calculation (RUN_CALC),
							   forlast process (LAST_TO_FORK), send all back */



	while ((state == RUN_CALC) || (state == LAST_TO_FORK)) {
		if (is_last == 1) { 
			/* last process in line
			   read from parent(pascal/w(i-1)), start sending back */

			if ((buf_len = read (STDIN_FILENO, buf, sizeof(buf))) == -1)
	    		syserr("Process w nr: %d. Error in read\n", getpid());

	    	/* update state */
			state = atoi(buf);

			/* last process always has values set to 1 */
			result = 1;
			old_value = 1;

			/* clear buffer */
			memset(buf, 0, BUF_SIZE);
			
			/* check for errors */
		  	if (buf_len == 0)                           
		    	fatal("Process w nr: %d. Read: unexpected end-of-file\n", getpid());
		  	else {  		
		  		/* if none - send back to parent */
		  		if (state == END_CALC) {
					buf_len = sprintf(buf, "%d %ld", SEND_ALL_BACK, old_value);
		  			if(write(STDOUT_FILENO, buf, sizeof(buf)) == -1) 
		  				syserr("Process w with pid %d: Error in write", getpid());
		    		fflush(stdout);
		    	}
		    	else {
					buf_len = sprintf(buf, "%d %ld", state, old_value);
			  		printf ("%s", buf);
			    	fflush(stdout);
			    }
		 
		    }

		    /* if it's not the final iteration, create next process */
		    if ((state == RUN_CALC) || (state == LAST_TO_FORK)) {
				/* create pipes to communicate with child process */
				if(pipe(pipe_dsc_back) == -1) {
					syserr("Process w with pid: %d. Error in creating pipe_dsc_back\n", getpid());
				}
				if(pipe(pipe_dsc_forth) == -1) {
					syserr("Process w with pid: %d. Error in creating pipe_dsc_forth\n", getpid());
				}
			 	is_last = 0;
				switch (fork ()) {
				    case -1: 
				      syserr("Process w with pid: %d. Error in fork\n", getpid());
				    case 0:
						/* duplicate corresponding descriptors */
						if(dup2(pipe_dsc_back[1], STDOUT_FILENO) == -1) {
							syserr("Process w with pid: %d. Error in dup pipe_dsc_back[1]\n", getpid());
						}
						if(dup2(pipe_dsc_forth[0], STDIN_FILENO) == -1) {
							syserr("Process w with pid: %d. Error in dup pipe_dsc_forth[0]\n", getpid());
						}
						
						/* close unused ones */
						if(close(pipe_dsc_back[0]) == -1) {
							syserr("Process w with pid: %d. Error in close pipe_dsc_back[0]\n", getpid());
						}
						if(close(pipe_dsc_back[1]) == -1) {
							syserr("Process w with pid: %d. Error in close pipe_dsc_back[1]\n", getpid());
						}
						if(close(pipe_dsc_forth[0]) == -1) {
							syserr("Process w with pid: %d. Error in close pipe_dsc_forth[0]\n", getpid());
						}
						if(close(pipe_dsc_forth[1]) == -1) {
							syserr("Process w with pid: %d. Error in close pipe_dsc_forth[1]\n", getpid());
						}

						/* run next w process */
						execl("./w", "w", (char *) 0);
						syserr("Error in execl(w).\n");
				    default:
				    	if(close(pipe_dsc_back[1]) == -1) {
							syserr("Process w with pid: %d. Error in close pipe_dsc_back[1]\n", getpid());
						}
						if(close(pipe_dsc_forth[0]) == -1) {
							syserr("Process w with pid: %d. Error in close pipe_dsc_forth[0]\n", getpid());
						}
				}
			}
		}
		else {
			/* read from parent, write to child,
			   then read from child and forward to parent */
			
			if ((buf_len = read (STDIN_FILENO, buf, sizeof(buf))) == -1)
	    		syserr("Process w nr: %d. Error in read\n", getpid());
		  	if (buf_len == 0)	/* check for errors */
		    	fatal("Process w nr: %d. Read: unexpected end-of-file\n", getpid());
		  	else {  		
	    		/* update state */
				state = atoi(buf);
				
				/* current value becomes old_value+value read from buffer */
				result = old_value + strtol(buf+CODE_LENGTH, NULL, 10);

				/* write result to buffer */
				memset(buf, 0, BUF_SIZE);
				buf_len = sprintf(buf, "%d %ld", state, old_value);

				/* update old_value */
					old_value = result;

				/* write to child */
				if (write (pipe_dsc_forth[1], buf, sizeof(buf)) == -1)
					syserr("Process w with pid %d: Error in write", getpid());

				if (state != END_CALC) {
					/* read from child */
			  		if ((buf_len = read (pipe_dsc_back[0], buf, sizeof(buf))) == -1)
		    			syserr("Process w nr: %d. Error in read\n", getpid());
				  	if (buf_len == 0)                           
				    	fatal("Process w nr: %d. Read: unexpected end-of-file\n", getpid());
				  	else { 		
		  		  		printf ("%s", buf);
			    		fflush(stdout);
			    	}			    	
			    }
			    else { /* END_CALC */
					/* write result to father */
					memset(buf, 0, BUF_SIZE);
					buf_len = sprintf(buf, "%d %ld", state, result);

				    if (write(STDOUT_FILENO, buf, sizeof(buf)) == -1)
				    	syserr("Process w with pid %d: Error in write", getpid());
		    		fflush(stdout);

					do { /* read from child until not the last flag */
				  		if ((buf_len = read (pipe_dsc_back[0], buf, sizeof(buf))) == -1)
			    			syserr("Process w nr: %d. Error in read\n", getpid());
					  	if (buf_len == 0)	/* check for errors */                  
					    	fatal("Process w nr: %d. Read: unexpected end-of-file\n", getpid());
					  	else { 		
					  		/* only forward */
					  		state = atoi(buf);
					  		if (write(STDOUT_FILENO, buf, sizeof(buf)) == -1)
				    			syserr("Process w with pid %d: Error in write", getpid());
		    				fflush(stdout);
				    	}	
				    } while (state != SEND_ALL_BACK);
		    	}
		    }
		}
	}

	/* if I hava a child, wait */
	if (is_last == 0) {
		if(wait(0) == -1) {
			syserr("Error in wait\n");
		}
	}
	return 0;
}