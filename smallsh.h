#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Maximum length of a user's command line */
#define MAXCMDLENGTH = 2048
/* Maximum number of arguments */
#define MAXARGS = 512

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

	// Return the user command with parsed data
	return cmd;
}