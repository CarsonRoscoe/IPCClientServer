#ifndef SHARED_H
#define SHARED_H

#include <string.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>
#include <sstream>
#include <vector>
#include <string>

//Regarding message data
#define MAXMESSAGEDATA (4096-16) //Dont want sizeof(msg) > 4096
#define MSGHDRSIZE (sizeof(Msg) - MAXMESSAGEDATA) //length of msg_len and msg_type

//End of transmission character
#define ETX (char)3

//Fail error message
#define FAILEDTOREAD "Error: Failure to open file"

//Priority defines for values & names
#define LOWPRIORITY 1
#define MEDIUMPRIORITY 2
#define HIGHPRIORITY 3
#define LOW "low"
#define MEDIUM "medium"
#define HIGH "high"

//Msgbuf type for client to server messages
#define CLIENTTOSERVERTYPE 1

//Defines for ftok function so client and server generate the same result
#define FTOKVAL1 "../."
#define FTOKVAL2 'c'

//Data structure used in place of msgbuf
typedef struct {
	long mtype; //message type
	char mtext[MAXMESSAGEDATA]; //data
	int mlen; //bytes of date in Msg
} Msg;

//Prototypes for all sending and receiving functions
int removeQueue(int messageQID);
int openQueue(key_t keyvalue);
int sendMessage(int messageQID, Msg *qbuffer);
int readMessage(int messageQID, Msg *qbuffer, long type);
int readMessageNonBlocking(int messageQID, Msg *qbuffer, long type);

#endif
