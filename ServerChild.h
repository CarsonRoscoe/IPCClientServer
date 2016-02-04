#ifndef SERVERCHILD_H
#define SERVERCHILD_H

#include "SendReceive.h"
#include <signal.h>

//Define our packet sizes based on our priority
#define PACKETSIZELOW 8
#define PACKETSIZEMEDIUM 16
#define PACKETSIZEHIGH 4050

//Prototype for our cleanup code, invoked when the server's child receives the signal interrupt
void exitChild(int exitcode);
//Prototype for our child processes main path of execution
void childProcess(int messageQID, Msg msg);

//Reference to our message queue ID variable, which is instantiated by Server.cpp/Server.h
extern int messageQID;

//Stores our clients PID
int clientPID;
//Flag over whether we should stop sending the client packets or not. Defaults to false, turned true when we catch a SIGINT
bool stop;

#endif //SERVERCHILD_H
