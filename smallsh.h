#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Maximum length of a user's command line */
#define MAXCMDLENGTH 2048
/* Maximum number of arguments */
#define MAXARGS 512

/* Struct to hold user's command */
struct userInput {
	char* command;
	char* userArgs[MAXARGS];
	char* inputFile;
	char* outputFile;
	bool runBackground;
};

/* Takes in user input command line and parses into the user input struct */
struct userInput* parseCommand(char* line) {

	// Allocate memory for user command
	struct userInput* cmd = malloc(sizeof(struct userInput));
	
	// Set the command to run in the foreground by default
	cmd->runBackground = false;

	// Character pointer for creating tokens
	char* saveptr;

	// First token: command
	char* token = strtok_r(line, " ", &saveptr);
	cmd->command = calloc(strlen(token) + 1, sizeof(char)); // allocate memory
	strcpy(cmd->command, token); // copy token into the user input command

	// Second set of tokens: arguments
	// Move through the arguments array
	for (int i = 0; i < MAXARGS; i++) {
		token = strtok_r(token, " ", &saveptr);
		// If the token is null or input/output symbols, break out of loop
		if (strcmp(token, "<") != 0) {
			// There is an input file
			break;
		}
		else if (strcmp(token, ">") != 0) {
			// There is an output file
			break;
		}
		else if (token == NULL) {
			break;
		}
		// Assign the token string to the arguments array
		cmd->userArgs[i] = calloc(strlen(token) + 1, sizeof(char));
		strcpy(cmd->userArgs[i], token);
	}

	// If the token is an input token, save the input file
	
	token = strtok_r(NULL, " >", &saveptr);

	// Return the user command with parsed data
	return cmd;
}

/* Prompt user for a command input, parse command and execute */
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
	else {
		// User entered a command, parse command into user input
		struct userInput* cmd = parseCommand(cmdLine);
		printf("Good one\n");
	}
	free(cmdLine);
	return;
}