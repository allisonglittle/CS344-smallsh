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

	// Character pointer for creating tokens
	char* saveptr;

	// First token: command
	char* token = strtok_r(line, " ", &saveptr);
	cmd->command = calloc(strlen(token) + 1, sizeof(char)); // allocate memory
	strcpy(cmd->command, token); // copy token into the user input command

	// Second set of tokens: arguments
	token = strtok_r(NULL, "<", &saveptr);
	// new pointer for tracking inside the token
	char* saveArgPtr;
	// Move through the arguments array
	for (int i = 0; i < MAXARGS; i++) {
		token = strtok_r(token, " ", &saveArgPtr);
		// If the token is null, break out of loop
		if (token == NULL) {
			break;
		}
		// Assign the token string to the arguments array
		cmd->userArgs[i] = calloc(strlen(token) + 1, sizeof(char));
		strcpy(cmd->userArgs[i], token);
	}

	// Return the user command with parsed data
	return cmd;
}