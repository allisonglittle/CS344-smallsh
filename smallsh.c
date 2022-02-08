/*
Author: Allison Little
Course: OSU CS344 Winter 2022
Assignment 3 - smallsh
*/
#include "smallsh.h"

int main() {
	// Set up signal handlers
	struct sigaction ignore_action = { 0 }, SIGTSTP_action = { 0 };
	// Parent process should ignore SIGINT
	ignore_action.sa_handler = SIG_IGN;
	sigaction(SIGINT, &ignore_action, NULL);

	while (1) {
		getUserInput();
	}
	return 0;
}