#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commands.h"
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

/* Global variables */
pid_t backgroundProcesses[MAXBGPROCESSES] = { 0 };


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
	printf("Good one\n");

	freeUserInput(cmd);
	free(cmdLine);
	return;
}