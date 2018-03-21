/**
 * Name         : Johnjeno Fox
 * Date         : October 3rd, 2017
 * File Name    : myshell.c
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

//DO NOT CHANGE THIS
#define MAX_BUFF_SIZE 256


int main() {
	
	char buff[MAX_BUFF_SIZE];
	char buff2[MAX_BUFF_SIZE];
	char **myargv[10];
	char *token;
	char delim[2] = " ";
	int myargc= 0;
	pid_t pid, pipePid;
	int amp = 0;
	int piper = 0;
	int outTruncateFlag = 0;
	int outAppendFlag = 0;
	int inputFlag = 0;
	int fd;
	int fdPipe[2];
	const char **prog1[10];
	const char **prog2[10];

	while(1){
		// if else used at beginning for & functionality.
		if(amp == 1){
			amp = 0;
		}
		else
			wait(NULL);
		printf("MyShell@>  ");
		fgets(buff, MAX_BUFF_SIZE, stdin);
		// Used to check if first character entered was new line, if so do nothing.
		if(buff[0] == '\n'){
			memset(buff, '\0', MAX_BUFF_SIZE);
			continue;
		}
		//Delete extra new line char due to fgets()
		for(int i = 0; i <= MAX_BUFF_SIZE; i++){
			if(buff[i] == '\n'){
				buff[i] = '\0';
			}
			if(buff[i] == '>' && buff[i+1] != '>'){
				memset(buff2, '\0', MAX_BUFF_SIZE);
				for(int j = 0; j < MAX_BUFF_SIZE; j++, i++){ // create filename
					buff2[j] = buff[i+2];
					if(buff2[j] == '\n')
						buff2[j] = '\0';
				}
			}
		}
		//exit the shell if command "exit"
		if(buff[0] == 'e' && buff[1] == 'x' && buff[2] == 'i' && buff[3] == 't' && buff[4] == '\0'){
			return -1;
		}
		else{
			// Parsing using space as delimiter for argument tokens
			// Fill myargv array with string tokens until NULL is found, include NULL.
			token = strtok(buff, delim);
			myargv[myargc] = token;
			while(token != NULL){
				myargc++;
				token = strtok(NULL, delim);
				myargv[myargc] = token;
			}
			// Loop to identify if a pipe was found, use "piper" counter in later loops
			// to identify how many tokens are in first and then after pipe was called. 
			for(int i = 0; i < myargc; i++){
				if(strcmp(myargv[i],"|") == 0){
					piper = i;	
				}
				else if(strcmp(myargv[i],">") == 0){
					outTruncateFlag = i;
				}
				else if(strcmp(myargv[i],">>") == 0){
					outAppendFlag = i;
				}
				else if(strcmp(myargv[i],"<") == 0 || strcmp(myargv[i],"<<") == 0){
					inputFlag = i;
				}	
			}
			if(outTruncateFlag > 0){ // Checking for output redirection
				// Loop to put arguments in first array until output flag.
				for(int i = 0; i < outTruncateFlag; i++){
					prog1[i] = myargv[i];
					if((i+1) == outTruncateFlag){
						prog1[i+1] = 0; // include NULL
					}
				}
				// if else to check if file exists. If not create new file.
				if(open(buff2, O_WRONLY) > 0){
					fd = open(buff2, O_WRONLY | O_TRUNC);
				}
				else{
					fd = open(buff2,O_WRONLY | O_CREAT | O_APPEND);
				}
				pid = fork();
				if(pid < 0){
					printf("Fork Failed");
					return -2;
				}
				else if(pid == 0){
					dup2(fd,1); // redirecting from STDOUT to file
					//close(STDOUT_FILENO);
					execvp(prog1[0], prog1);
				}
				else{
					wait(NULL);
				}
				close(fd);
				outTruncateFlag = 0;
			}
			else if(outAppendFlag > 0){ // Checking for output redirection
				// Loop to put arguments in first array until output flag.
				for(int i = 0; i < outAppendFlag; i++){
					prog1[i] = myargv[i];
					if((i+1) == outAppendFlag){
						prog1[i+1] = 0; // include NULL
					}
				}
				// if else to check if file exists. If not create new file.
				if(open(buff2, O_WRONLY) > 0){
					fd = open(buff2, O_WRONLY | O_APPEND);
				}
				else{
					fd = open(buff2,O_WRONLY | O_CREAT | O_APPEND);
				}
				pid = fork();
				if(pid < 0){
					printf("Fork Failed");
					return -2;
				}
				else if(pid == 0){
					//close(STDOUT_FILENO);
					dup2(fd,1); // redirecting from STDOUT to file
					execvp(prog1[0], prog1);
				}
				else{
					wait(NULL);
				}
				close(fd);
				outAppendFlag = 0;
			}
			else if (inputFlag > 0){ // Fork for input redirecting from file
				for(int i = 0; i < myargc; i++){
					if(strcmp(myargv[i],"<") == 0 || strcmp(myargv[i],"<<") == 0){
						myargv[i] = myargv[i+1];
						myargv[i+1] = NULL;
						break;
					}	
				}
				inputFlag = 0; // reset input flag
				pid = fork();
				if(pid < 0){
					printf("Fork Failed");
					return -2;
				}
				else if(pid == 0){
					execvp(myargv[0], myargv);
				}
				else{
					wait(NULL);
				}
			}// Fork if pipe is found.
			else if(piper > 0){
				// Loop to put arguments in first array until pipe token.
				for(int i = 0; i < piper; i++){
					prog1[i] = myargv[i];
					if((i+1) == piper){
						prog1[i+1] = 0; // include NULL
					}
				}
				// Loop to put arguments in secdon array until total arguments found.
				for(int i = 0, k =(piper + 1); k < myargc; i++, k++){
					prog2[i] = myargv[k];
					if((k+1) == myargc){
						prog2[i+1] = 0; // include NULL
					}
				}
				piper = 0; // reset piper flag
				// create pipe and check if successful
				if(pipe(fdPipe) < 0){
					perror("Pipe Failed.");
					return -3;
				}
				pid = fork(); // Fork to first process
				if(pid < 0){
					printf("Fork Failed");
					return -2;
				}
				else if(pid == 0){
					close(STDOUT_FILENO);
					dup(fdPipe[1]);
					close(fdPipe[0]);
					execvp(prog1[0], prog1);
				}
				pipePid = fork(); // Fork to second process
				if(pipePid < 0){
					printf("Fork Failed.");
					return -2;
				}
				else if(pipePid == 0){
					close(STDIN_FILENO);
					dup(fdPipe[0]);
					close(fdPipe[1]);
					execvp(prog2[0], prog2);
				}
				close(fdPipe[0]);
				close(fdPipe[1]);
				wait(NULL);
				wait(NULL);
			}
			// Fork if *** & *** is last token
			else if(strcmp(myargv[myargc-1],"&") == 0){
				myargv[myargc-1] = NULL;
				pid = fork();
				amp = 1;
				if(pid < 0){
					printf("Fork Failed");
					return -2;
				}
				else if(pid == 0){
					execvp(myargv[0], myargv);
				}
			}
			// Fork for normal processes
			else{
				pid = fork();
				if(pid < 0){
					printf("Fork Failed");
					return -2;
				}
				else if(pid == 0){
					execvp(myargv[0], myargv);
					//execlp("cd","cd","/",0);
				}
				else{
					wait(NULL);
				}
			}
			// reinitialize variables and arrays for next iteration of loop.
			myargc = 0;
			memset(buff, '\0', MAX_BUFF_SIZE);
			memset(myargv, '\0', 10);
			memset(prog1, '\0', 10);
			memset(prog2, '\0', 10);
		}
	}
	return 0;
}
