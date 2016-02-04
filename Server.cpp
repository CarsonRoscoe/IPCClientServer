/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Server.cpp - Server application that utilizes message queues to communicate with multiple client
-- processes simultaneously.
--
-- PROGRAM: Server.cpp
--
-- FUNCTIONS:
-- int main(int argc, char* argv[])
-- void exitParent(int exitCode)
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
-- NOTES:
-- This program acts as the server for multiple client processes. The client-server model used utilizes messaging queues
-- to communicate with eachother. Clients will send a message of type 1 containing their PID, priority request and the
-- file they want the information from. The server will then fork a child to handle this request. That child will send
-- multiple packetized messages to the messaging queue of the type of the clients PID.
--
-- Priority is achieved by making lower priority processes have smaller packet sizes, forcing them to send more packets.
-- This approach works because the operating system stops a processes timeslice early if the process runs into a blocking
-- state, AKA any I/O work. By sending more packets, they are giving up time slices, therefor allowing the higher priority
-- processes to be given more time by the operating system.
----------------------------------------------------------------------------------------------------------------------*/
#include "SendReceive.h"
#include "Server.h"
#include <signal.h>

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: main
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
-- INTERFACE: int main(void)
--
-- RETURN: int regarding the exit code
--
-- NOTES:
-- Starting point for our Server application. Sets up the main signal handling, creates the mesasge queue that is used
-- to communicate with the client processes, and then looks for client messages. Upon reading a clients message request,
-- forks to make a child process that will handle the clients request, and then goes back to the forever loop of searching
-- for more clients.
----------------------------------------------------------------------------------------------------------------------*/
int main(void) {
	//Message key for our message queue
	key_t messageKey;
	//Setup catching SIGINTs from ctrl-C commands to invoke our cleanup function
	signal(SIGINT, exitParent);
	//Generate our message key for getting the message queue. We used preset FTOKVAL1 & FTOKVAL2
	//as our values since they come from the same header file the clients include, meaning the same key will
	//be generated on both sides.
	messageKey = ftok(FTOKVAL1, FTOKVAL2);

	//Attempt to open the message queue. On fail, exit.
	if ((messageQID = openQueue(messageKey)) == -1) {
		std::cerr << "Failed to open queue" << std::endl;
		exit(1);
	}

	//Forever loop where we read messages from the client. If we fail to read it, we exit as there is a problem.
	//Once we successfully read one, we fork. The child invokes the childProcess function & handles the client interaction
	//before exiting naturally, while the parent continues the loop looking for the next client request.
	while(true) {
		Msg msg;
		if (readMessage(messageQID, &msg, CLIENTTOSERVERTYPE) == -1) {
			std::cerr << "Failed to send message" << std::endl;
			exit(1);
		}
		switch(fork()) {
			case -1:
				//If we fail to fork, invoke cleanup code.
				std::cerr << "Failed to fork" << std::endl;
				removeQueue(messageQID);
				exit(1);
			case 0: //On successful fork, child handles the clients request then exits.
				childProcess(messageQID, msg);
				exit(0);
		}
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: exitParent
--
-- DATE: January 29th, 2016
--
-- REVISIONS: January 29th, 2016: Programmed the entire application (minus thread functionality)
--	      February 2nd, 2016: Commented
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- INTERFACE: void exitParent(int exitCode: Code that specifies the reason the program is exiting. Unused.)
--
-- RETURN: void
--
-- NOTES:
-- In main we catch SIGINT signals and set it up to invoke this function when that signal is caught.
-- We used this function to remove the message queue from memory before killing off the process. Essentially, this is
-- cleanup code for sudden/unexpected exiting.
----------------------------------------------------------------------------------------------------------------------*/
void exitParent(int exitcode) {
	//Remove message queue to cleanup our IPC channel
	removeQueue(messageQID);
	//Kill ourselves, which, as the parent application, kills all children processes running as well.
	kill(getpid(), SIGKILL);
}
