#ifndef SERVER_H
#define SERVER_H

//Prototype for the function the client process invokes upon forking. Filled in by ServerChild.cpp/ServerChild.h
extern void childProcess(int messageQID, Msg msg);
//Prototype for the cleanup code function that is invoked when the server catches a SIGINT
void exitParent(int exitcode);

//Global variable holding our message queue ID
int messageQID;

#endif //SERVER_H
