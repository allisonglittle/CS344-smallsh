/*
Author: Allison Little
Course: OSU CS344 Winter 2022
Assignment 3 - smallsh
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "commands.h"
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

/* Maximum length of a user's command line */
#define MAXCMDLENGTH 2048
/* Maximum number of arguments */
#define MAXARGS 512
/* Maximum number of background processes */
#define MAXBGPROCESSES 100

/* --------------------------------------------------------------------------------------------------------- */
/* Global variables */
/* --------------------------------------------------------------------------------------------------------- */
/* Array of PIDs of background processes */
pid_t backgroundProcesses[MAXBGPROCESSES] = { 0 };
/* Boolean for if the process can run processes in the background */
bool foregroundOnlyMode = false;
/* Exit status for most recently terminated foreground child process */
int exitStatus = 0;

/* --------------------------------------------------------------------------------------------------------- */
/* Custom signal handler for SIGTSTP */
/* --------------------------------------------------------------------------------------------------------- */
void handle_SIGTSTP(int signo) {
	// Check starting status of foreground-only mode
	if (foregroundOnlyMode) {
		// Turn off foreground only mode
		foregroundOnlyMode = false;
		char* message = "\nExiting foreground-only mode\n: ";
		write(STDOUT_FILENO, message, 33);
	}
	else {
		// Turn on foreground only mode
		foregroundOnlyMode = true;
		char* message = "\nEntering foreground-only mode (& ignored)\n: ";
		write(STDOUT_FILENO, message, 46);
	}
	fflush(stdout);
	return;
}

/* --------------------------------------------------------------------------------------------------------- */
/* Custom signal handler for SIGINT */
/*   Only used for foreground child processes */
/* --------------------------------------------------------------------------------------------------------- */
void handle_SIGINT(int signo) {
	// Process was terminated, send message
	char* message[40];
	sprintf(message, "\nProcess terminated by signal % d\n", signo);
	write(STDOUT_FILENO, message, 41);
	fflush(stdout);
	// Update exit status
	exitStatus = signo;
	exit(2);
}


/* --------------------------------------------------------------------------------------------------------- */
/* Struct to hold user's command */
/* --------------------------------------------------------------------------------------------------------- */
struct userInput {
	char* command;
	char* userArgs[MAXARGS];
	char* inputFile;
	char* outputFile;
	bool runBackground;
};

/* --------------------------------------------------------------------------------------------------------- */
/* Free memory for user input */
/* --------------------------------------------------------------------------------------------------------- */
void freeUserInput(struct userInput* cmd) {
	// Command
	free(cmd->command);
	// Arguments
	for (int i = 0; i < MAXARGS; i++) {
		if (cmd->userArgs[i] == NULL) {
			break;
		}
		else {
			free(cmd->userArgs[i]);
		}
	}
	// Input file
	free(cmd->inputFile);
	// Output file
	free(cmd->outputFile);
	// Entire user input
	free(cmd);
	return;
}

/* --------------------------------------------------------------------------------------------------------- */
/* Replaces $$ in a string with the process ID value */
/* --------------------------------------------------------------------------------------------------------- */
char* variableReplacement(char* searchText) {
	// Check if token contains '$$'
	char* varPtr = strstr(searchText, "$$");
	if (varPtr != NULL) {
		// text contains $$
		char* saveptr;
		char* token = strtok_r(searchText, "$$", &saveptr);
		while (token != NULL) {

		}
	}
	char* newString = "Placeholder";
	return newString;
}


/* --------------------------------------------------------------------------------------------------------- */
/* Takes in user input command line and parses into the user input struct */
/* --------------------------------------------------------------------------------------------------------- */
struct userInput* parseCommand(char* line) {

	// Allocate memory for user command
	struct userInput* cmd = malloc(sizeof(struct userInput));
	
	// Set the command to run in the foreground by default
	cmd->runBackground = false;
	// Set all command details to null
	cmd->command = NULL;
	cmd->inputFile = NULL;
	cmd->outputFile = NULL;
	for (int i = 0; i < MAXARGS; i++) {
		cmd->userArgs[i] = NULL;
	}

	// Character pointer for creating tokens
	char* saveptr;

	// First token: command
	char* token = strtok_r(line, " \n", &saveptr);
	cmd->command = calloc(strlen(token) + 1, sizeof(char)); // allocate memory
	strcpy(cmd->command, token); // copy token into the user input command

	// Second set of tokens: arguments
	token = strtok_r(NULL, " \n", &saveptr);
	// Move through the arguments array
	for (int i = 0; i < MAXARGS; i++) {
		// If the token is null, input/output or background symbols, break out of loop
		if (token == NULL) {
			// There are no more tokens in the command, return the user input 
			return cmd;
		}
		else if (strcmp(token, "<") == 0) {
			// There is an input file
			break;
		}
		else if (strcmp(token, ">") == 0) {
			// There is an output file
			break;
		}
		else if (strcmp(token, "&") == 0) {
			// The command should run in the background
			cmd->runBackground = true;
			// Return the user input
			return cmd;
		}
		// Assign the token string to the arguments array
		cmd->userArgs[i] = calloc(strlen(token) + 1, sizeof(char));
		strcpy(cmd->userArgs[i], token);
		// Next token
		token = strtok_r(NULL, " \n", &saveptr);
	}

	// If the token is an input token, save the input file
	if (strcmp(token, "<") == 0) {
		// Read in the input file
		token = strtok_r(NULL, " \n", &saveptr);
		cmd->inputFile = calloc(strlen(token) + 1, sizeof(char)); // allocate memory
		strcpy(cmd->inputFile, token); // copy token into the user input command
		// Read next token
		token = strtok_r(NULL, " \n", &saveptr);
		if (token == NULL) {
			// No more tokens, return command
			return cmd;
		}
	}
	
	// If the token is an output token, save the output file
	if (strcmp(token, ">") == 0) {
		// Read in the output file
		token = strtok_r(NULL, " \n", &saveptr);
		cmd->outputFile = calloc(strlen(token) + 1, sizeof(char)); // allocate memory
		strcpy(cmd->outputFile, token); // copy token into the user output command
		// Read next token
		token = strtok_r(NULL, " \n", &saveptr);
		if (token == NULL) {
			// No more tokens, return command
			return cmd;
		}
	}

	// If the token is the background symbol, set the command to run in the background
	if (strcmp(token, "&") == 0) {
		// The command should run in the background
		cmd->runBackground = true;
	}

	// Return the user command with parsed data
	return cmd;
}

/* --------------------------------------------------------------------------------------------------------- */
/* Custom command: exit */
/*	Terminates all background children processes and parent process */
/* --------------------------------------------------------------------------------------------------------- */
void exitProcess() {
	// Move through all background processes
	for (int i = 0; i < MAXBGPROCESSES; i++) {
		if (backgroundProcesses[i] > 0) {
			// Terminate background process
			kill(backgroundProcesses[i], SIGTERM);
		}
	}
	// Exit parent process
	exit(0);
}

/* --------------------------------------------------------------------------------------------------------- */
/* Custom command: cd */
/*	Changes directory */
/* --------------------------------------------------------------------------------------------------------- */
void changeDirectory(struct userInput* cmd) {
	// If there is no argument for new directory, navigate to HOME
	if (cmd->userArgs[0] == NULL) {
		// Navigate to HOME
		chdir(getenv("HOME"));
	}
	else {
		// Change directory to specified path
		if (chdir(cmd->userArgs[0]) != 0) {
			// There was an error, print message
			perror("Could not change directory: ");
			
		}
	}
	// Buffer to hold current working directory path name
	char cwd[200];
	getcwd(cwd, sizeof(cwd));
	printf("%s\n", cwd);
	fflush(stdout);
	return;
}

/* --------------------------------------------------------------------------------------------------------- */
/* Custom command: status */
/*	Returns the status of the most recently terminated foreground process */
/* --------------------------------------------------------------------------------------------------------- */
void returnStatus() {
	// Print the most recent status
	printf("Exit value %d\n", exitStatus);
	fflush(stdout);
	return;
}

/* --------------------------------------------------------------------------------------------------------- */
/* Executing a standard command */
/*	Uses exec and fork to create a child process */
/* --------------------------------------------------------------------------------------------------------- */
void standardCommand(struct userInput* cmd) {
	// Count the number of arguements in user's command
	int argCount = 0;
	for (int i = 0; i < MAXARGS; i++) {
		if (cmd->userArgs[i] != NULL) {
			argCount++;
		}
		else {
			break;
		}
	}
	// Create an array to contain the command and user's arguments
	char* cmdArg[argCount + 2];
	// First value: command
	cmdArg[0] = cmd->command;
	// Next values: the user's arguements
	for (int i = 0; i < argCount; i++) {
		cmdArg[i + 1] = cmd->userArgs[i];
	}
	// Last value: NULL
	cmdArg[argCount + 1] = NULL;
	
	// Fork a new process
	pid_t spawnPid = fork();

	switch (spawnPid) {
	case -1:
		// Fork unsuccessful
		perror("fork()\n");
		exit(1);
		break;
	case 0:
		// In the child process
		// Check if background process
		if (cmd->runBackground) {
			// Print background pid
			printf("Background pid: %d\n", getpid());
			fflush(stdout);
		}
		else {
			// This is a foreground process, so update process to allow SIGINT as usual
			struct sigaction SIGINT_action = { {0} };
			SIGINT_action.sa_handler = handle_SIGINT;
			SIGINT_action.sa_flags = 0;
			sigaction(SIGINT, &SIGINT_action, NULL);
		}
		// Execute the command
		execvp(cmdArg[0], cmdArg);
		// If there is an error with the execution
		perror("execvp");
		fflush(stdout);
		exitStatus = 1;
		exit(2);
		break;
	default:
		// In the parent process
		// If this is a foreground process, do not wait for termination
		if (cmd->runBackground) {
			int childStatus;
			spawnPid = waitpid(spawnPid, &childStatus, WNOHANG);
			// Add process id to the background array
			for (int i = 0; i < MAXBGPROCESSES; i++) {
				if (backgroundProcesses[i] == 0) {
					// Found a vacant background process id spot in array
					backgroundProcesses[i] = spawnPid;
					break;
				}
			}
		}
		else {
			// This is a foreground process, wait for the child termiantion
			spawnPid = waitpid(spawnPid, &exitStatus, 0);
		}
		
		break;
	}
	fflush(stdout);
	return;
}



/* --------------------------------------------------------------------------------------------------------- */
/* Prompt user for a command input, parse command and execute */
/* --------------------------------------------------------------------------------------------------------- */
void getUserInput() {
	// Prompt user
	printf(": ");
	fflush(stdout);

	// Create string to hold user's input
	char* cmdLine = NULL;
	size_t len = 0;
	// Read in string, if it is blank, leave function
	if (getline(&cmdLine, &len, stdin) == 1) {
		// Blank command
		return;
	}
	else if (cmdLine[0] == '#') {
		// User entered a comment
		return;
	}
	// User entered a command, parse command into user input
	struct userInput* cmd = parseCommand(cmdLine);
	
	// Check for custom commands
	if (strcmp(cmd->command, "exit") == 0) {
		exitProcess();
	}
	else if (strcmp(cmd->command, "cd") == 0) {
		changeDirectory(cmd);
	}
	else if (strcmp(cmd->command, "status") == 0) {
		returnStatus();
	}
	// Run a standard command
	else {
		standardCommand(cmd);
	}

	freeUserInput(cmd);
	free(cmdLine);
	return;
}