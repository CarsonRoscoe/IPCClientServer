#ifndef CLIENT_H
#define CLIENT_H

#include <algorithm>
#include <signal.h>
#include <pthread.h>
#include <vector>

//Key to exit
#define QUITKEY 'q'
//Key for usage help
#define HELPKEY 'h'
//Usage define to be outputted to screen
#define USAGE "Usage: First paremeter must be a valid file name." << std::endl << "(Optional) Second parameter must be a valid priority within the range of " << LOWPRIORITY << " to " << HIGHPRIORITY << "." << std::endl << "If there is no second parameter, the program will default to low priority mode"

//Invoked by SIGINT on client
void exitClient(int exitCode);
//Posix thread starting position
void* handleInput(void* unused);

//Servers PID
int serverPID;
//Priority
int priority;
//Name of the file to request reading for from server
std::string fileName;

#endif //CLIENT_H
