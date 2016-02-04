/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Client.cpp - Client application that utilizes message queues to communicate with its server application.
--
-- PROGRAM: Client.cpp
--
-- FUNCTIONS:
-- int main(int argc, char* argv[])
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
-- The program sends to the server process the name of a file and its requested priority (defaulting to lowest if
-- none is specified). The name of the file must be located in the folder of the client application, and the priority
-- is on a scale of 1 to 3 where 1 is lowest priority and 3 is highest.
--
-- The program accomplishes the job via message queues. It first sends off to the message queue a message of type 1
-- (where 1 means "client to server message") where the data of the message is three pieces of information. The file name
-- we are requesting the information of, our PID so the server knows who is requesting the data, and our requested priority.
--
-- The client knows the file is finished when it receives one final packet that simply contains the end of transmition (ETX)
-- character. It then exits itself in this case.
----------------------------------------------------------------------------------------------------------------------*/
#include "SendReceive.h"
#include "Client.h"

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
-- Starting point for our Client application. Opens up our message queue, sends a message to the server with our
-- chosen file name, priority & PID. It will get a bunch of packets that relate to the information being sent in.
-- Once it receives a packet with only the end of transmission character, the process dies off.
----------------------------------------------------------------------------------------------------------------------*/
int main(void) {
	//Will hold the message queue ID
	int messageQID;
	//Will hold the message key for the message queue
	key_t messageKey;
	pthread_t inputThread;
	fileName = "";
	//Holds the priority we are requesting, defaulting to low priority (1)
	priority = 0;

	//Create the pthread that will handle all input
	pthread_create(&inputThread, NULL, handleInput, NULL);

	//Wait for the pthread to ge the priority & 
	while(fileName.length() == 0 || priority == 0) {}
	
	//ftok gives us our message key based on two parameters, a directory and a unique char. We get these values
	//From the shared header so both the client and server will generate the same message key.
	messageKey = ftok(FTOKVAL1, FTOKVAL2);

	//Open our message queue with that key.
	if ((messageQID = openQueue(messageKey)) == -1) {
		std::cerr << "Failed to open queue" << std::endl;
		exit(1);
	}
	
	//Prepare our initial message to be sent
	Msg msg;
	sprintf(msg.mtext, "%s%c%d%c%d", fileName.c_str(), ' ', getpid(), ' ', priority);
	msg.mlen = strlen(fileName.c_str()) + 8;
	msg.mtype = 1;

	//Send the message requesting for a files data
	if (sendMessage(messageQID, &msg) == -1) {
		std::cerr << "Failed to send message" << std::endl;
		exit(1);
	}

	//Receive the servers response. This will either have the PID of our server application or an error message.
	Msg serverConnected;
	if (readMessage(messageQID, &serverConnected, getpid()) == -1) {
		std::cerr << "Failed to read message" << std::endl;
		fflush(stdout);
		exit(1);
	}

	//Check if its the PID that was sent
	std::string message(serverConnected.mtext);
	serverPID = -1;
	//If so, store it
	if (std::find_if(message.begin(), message.end(), [](char c) { return !std::isdigit(c); }) == message.end()) {
		serverPID = atoi(message.c_str());
	} else {
		//If it wasnt, print an error and exit.
		std::cout << FAILEDTOREAD << std::endl;
		fflush(stdout);
		exit(1);
	}

	//Forever loop reading each packet until it finds the ETX packet
	while(true) {
		Msg toServer;
		//Read packet
		if (readMessage(messageQID, &toServer, getpid()) == -1) {
			std::cerr << "Failed to read message" << std::endl;
			fflush(stdout);
			exit(1);
		}
			
		//If ETX, exit cause we are done
		if (toServer.mtext[0] == ETX) {
			std::cout << serverPID << std::endl;
			break;
		}

		//Print the packets
		std::cout << toServer.mtext;
		fflush(stdout);
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: handleInput
--
-- DATE: February 2nd, 2016
--
-- REVISIONS: February 2nd, 2016: Added threading & commented
--	      February 3rd, 2016: Added edge-case fixes
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- INTERFACE: void* handleInput(void* unused)
--
-- RETURN: int regarding the exit code
--
-- NOTES:
-- Function where the POSIX thread starts at. Handles input for the process, first when the program first starts
-- off and then when we are reading packets. 'q' will quit, 'h' for help, and the first two things typed are
-- file name & priority
----------------------------------------------------------------------------------------------------------------------*/
void* handleInput(void* unused) {
	//Catch the SIGINTs to call the exitClient function
	signal(SIGINT, exitClient);
	//Flag over if we have finished startup code (getting the priority & file name) or not
	bool finishedSetup = false;
	//Temp priority variable
	int localPriority = 0;
	
	//Get initial input
	while(!finishedSetup) {
		std::cout << "Enter a file name to get the contents of. ('" << QUITKEY << "' to quit or '" << HELPKEY << "' for help)" << std::endl;
		std::cin >> fileName;
		//If they entered one character instead of a name, they were doing a trigger like 'q' or 'h' instead of the file name
		if (fileName.length() == 1) {
			switch(fileName[0]) {
				case QUITKEY:
					kill(getpid(), SIGINT);
					return 0;
				case HELPKEY:
					std::cout << USAGE << std::endl;
					break;
			}
			continue;
		}

		std::cout << "Enter the priority on a scale of " << LOWPRIORITY << " (" << LOW << ")" << " to " << HIGHPRIORITY << " (" << HIGH << ")" << std::endl;
		std::cin >> localPriority;

		//If it is not within the range, we explain the proper useage and exit.
		if (localPriority < 1 || localPriority > 3) {
			std::cerr << USAGE << std::endl;
			break;
		}

		//If it is within the range, we tell them so and continue on
		std::cout << "Requesting for file " << fileName << "." << std::endl << "Priority set to " << localPriority << "("; 
		switch(localPriority) {
			case LOWPRIORITY:
				std::cout << LOW;
				break;
			case MEDIUMPRIORITY:
				std::cout << MEDIUM;
				break;
			case HIGHPRIORITY:
				std::cout << HIGH;
				break;
			default:
				//Should not hit this point as it is out of our low priority to high priority range,
				//however we default to low priority in this case.
				std::cout << LOW;
				localPriority = LOWPRIORITY;
		}
		std::cout << ")" << std::endl;
		finishedSetup = true;
		priority = localPriority;
		std::cout << priority << std::endl;
	}
	
	//Handles input during packets being read
	while(true) {
		std::string input;
		std::cin >> input;
		if (input.length() == 1)
			//On quit key, quic the program. on help key, print usage.
			switch(input[0]) {
				case QUITKEY:
					kill(getpid(), SIGINT);
					return 0;
				case HELPKEY:
					std::cout << USAGE << std::endl;
					break;
				default:
					break;
			}
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: exitClient
--
-- DATE: February 3rd, 2016
--
-- REVISIONS: February 3rd, 2016: Added edge-case fixes
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- INTERFACE: void exitClient(int exitCode)
--
-- RETURN: int regarding the signal or exit code
--
-- NOTES:
-- Invoked when the client control-C's to kill the program. Notifies the server that it is being killed so the server
-- can handle the cleanup.
----------------------------------------------------------------------------------------------------------------------*/
void exitClient(int exitCode) {
	kill(serverPID, SIGINT);
	exit(0);
}
