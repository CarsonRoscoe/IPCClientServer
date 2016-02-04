/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: ServerChild.cpp - Code for the servers children when the server forks to handle a clients request.
--
-- PROGRAM: Server.cpp
--
-- FUNCTIONS:
-- void childProcess(int messageQID, Msg msg)
-- void exitChild(int exitcode)
--
-- DATE: January 29th, 2016
--
-- REVISIONS: January 29th, 2016: Programmed the entire application (minus thread functionality)
--	      February 2nd, 2016: Added threading & commented
--	      February 3rd, 2016: Added edge-case fixes
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- NOTES: (For full program description, see Server.cpp)
-- When Server.cpp forks to create a child that handles a clients requests, it calls this files childProcess function.
-- From here on, the child process is an independent entity to its parent and will terminate itself as needed once it
-- is done its task at hand. It finds the file for the client, reads from it and sends packets to the client that
-- contain all the data from the file. It also handles error checking and some cleanup code.
----------------------------------------------------------------------------------------------------------------------*/
#include "ServerChild.h"

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: childProcess
--
-- DATE: January 29th, 2016
--
-- REVISIONS: January 29th, 2016: Programmed the entire application (minus thread functionality)
--	      February 2nd, 2016: Added threading & commented
--	      February 3rd, 2016: Added edge-case fixes
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- INTERFACE: void childProcess(int messageQID: Message queue ID to read/write messages to/from
--				Msg msg: The msgbuf-like struct that is filled with the clients request data)
--
-- RETURN: void
--
-- NOTES:
-- Starting point for our Server's children's line of execution. First it does setup regarding catching the signal interrupt.
-- It does this to allow the client to notify it upon the client needing to stop mid-transmission. The function we go to when
-- we catch a signal is exitChild. We then try and open the file for the client. If we can't, we notify to the client that
-- there is an error in the file and then exit. If we succeed, we send the client our PID as verification so that the client
-- can send us the necessary signal if needed. From then on it reads from it and sends packets to the client that contain all 
-- the data from the file.
----------------------------------------------------------------------------------------------------------------------*/
void childProcess(int messageQID, Msg msg) {
	//File the client wants data from
	std::ifstream readFile;
	//Used to seperate the messages text as it is dilimited by spaces
	std::istringstream iss(msg.mtext);
	//Temporary string used when splitting the istringstream data
	std::string s;
	//List of strings that will be the result of the messages text after being split
	std::vector<std::string> strings;
	//Will hold the priority that the client is requesting
	int priority; 
	//Flag that determines if we should stop looking and begin a natural exit
	stop = false;

	//Catch interrupt signals and call our exitChild function to handle the cleanup
	signal(SIGINT, exitChild);

	//Split our messages data into the vector of strings named strings
	while (getline(iss, s, ' ')) {
		strings.push_back(s);
	}	

	//Get the clientPID, which is the 2nd value being passed from the message
	clientPID = atoi(strings[1].c_str());
	
	//The third value is priority, so this is essentially getting the requested priority and switch casing it
	switch(atoi(strings[2].c_str())) {
		//If they want low priority, set the priority buffer size to the small packet size
		case 1:
			std::cout << 1 << std::endl;
			priority = PACKETSIZELOW;
			break;
		//If they want medium priority, set the priority buffer size to the medium packet size
		case 2:
			std::cout << 2 << std::endl;
			priority = PACKETSIZEMEDIUM;
			break;
		case 3:
		//If they want high priority, set the priority buffer size to the large packet size
			std::cout << 3 << std::endl;
			priority = PACKETSIZEHIGH;
			break;
		default:
		//This should not be called, however if they somehow are requesting a priority that isnt valid, default to low.
			std::cout << "1 (default)" << std::endl;
			priority = PACKETSIZELOW;
			break;
	}
	fflush(stdout);

	//Try to open the file associated with the file name given in the message, which is the first value of the mssage
	readFile.open(strings[0].c_str(), std::ios::in);
 	
	//If the file is invalid/could not be read, send to the client an error message with length -1, and then exit.
	if (!readFile.is_open()) {
		Msg toClient;
		toClient.mtype = clientPID;
		sprintf(toClient.mtext, "%s", FAILEDTOREAD);
		toClient.mlen = -1;
		sendMessage(messageQID, &toClient);
		std::cout << FAILEDTOREAD << std::endl;
		exit(1);
	//Otherwise, if the file is valid/could be read, send the client the servers PID as verification
	} else {
		Msg toClient;
		toClient.mtype = clientPID;
		sprintf(toClient.mtext, "%d", getpid());
		toClient.mlen = strings[1].length();
		sendMessage(messageQID, &toClient);
	}

	//Go to the end of the file and get the size of it
	readFile.seekg(0, readFile.end);
	int length = readFile.tellg();
	int current = 0;
	//Go back to the start of the file,
	readFile.seekg(current, readFile.beg);

	//While we arent at the end of the file and while we aren't forced to stop
	while (current < length && !stop) {
		//Create a buffer based on priority size
		char buffer [priority] = { '\0' };
		//Read that buffers size's amount of data from the file
		readFile.read(buffer, priority-1);
		//Increment our location based on the read amount
		current += priority;

		//Create our packet to send to the client
		Msg toClient;
		//The text is whatever is in the buffer
		sprintf(toClient.mtext, "%s", buffer);
		//The length is the size read in
		toClient.mlen = priority;
		//The type is our clients PID, so the clients can determine whos packets are for who
		toClient.mtype = clientPID;

		//After constructing the message/packet, send it
		sendMessage(messageQID, &toClient);

		//If we are done and just sent our last packet, send an "end of transmission" packet and then exit from the loop.
		if (current >= length) {
			toClient.mlen = 1;
			toClient.mtype = clientPID;
			sprintf(toClient.mtext, "%c", ETX);
			sendMessage(messageQID, &toClient);
			//Print to console so we know that this client/server interaction is finished
			std::cout << "Client " << clientPID<< " finished. Priority was " << atoi(strings[2].c_str()) << std::endl;
			break;
		}
	}

	//Close the file as we are done with it.
	readFile.close();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: exitChild
--
-- DATE: February 3rd, 2016
--
-- REVISIONS: February 3rd, 2016: Created as a way to handle the edge case of clients closing prematurely
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- INTERFACE: void exitChild(int exitCode: Code that specifies the reason the program is exiting. Unused.)
--
-- RETURN: void
--
-- NOTES:
-- In processChild we catch SIGINT signals and set it up to invoke this function when that signal is caught.
-- This will only be invoked when the client is exiting prematurely, as the client will invoke SIGINT on this process.
-- We read all of the messages that are meant for our client since our client only notifies us when they are dying, AKA
-- when they wont be able to read these messages on their own.
----------------------------------------------------------------------------------------------------------------------*/
void exitChild(int exitcode) {
	Msg msg;
	//Set flag stop to true so we stop creating packets for our client
	stop = true;
	std::cout << "Exiting early, either due to failure or failure on client.\nDisposing of unread messages." << std::endl;
	//Eat up all messages in the message queue under our clients PID, since we were signaled to stop immediately.
	while(readMessageNonBlocking(messageQID, &msg, clientPID)!= -1);
	std::cout << "Messages deleted." << std::endl;
	exit(0);
}
