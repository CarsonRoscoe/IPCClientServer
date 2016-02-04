#include "SendReceive.h"

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: removeQueue
--
-- DATE: January 29th, 2016
--
-- REVISIONS: January 29th, 2016: Created
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- INTERFACE: int removeQueue(int messageQID: Message Queue ID to remove the queue of)
--
-- RETURN: whether it was deleted or not
--
-- NOTES:
-- Removes the message queue and returns whether it was a success or not
----------------------------------------------------------------------------------------------------------------------*/
int removeQueue(int messageQID) {
	return msgctl(messageQID, IPC_RMID, 0) == -1 ? -1 : 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: openQueue
--
-- DATE: January 29th, 2016
--
-- REVISIONS: January 29th, 2016: Created
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- INTERFACE: int openQueue(key_t keyvalue: The key of the message queue to create/open)
--
-- RETURN: the message queue ID of the queue
--
-- NOTES:
-- Opens and/or creates the message queue
----------------------------------------------------------------------------------------------------------------------*/
int openQueue(key_t keyvalue) {
	return msgget(keyvalue, IPC_CREAT | 0666);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendMessage
--
-- DATE: January 29th, 2016
--
-- REVISIONS: January 29th, 2016: Created
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- INTERFACE: int sendMessage(int messageQID: The message queue ID to send the message to
--			      Msg *qbuffer: Pointer to the Msg structure that holds the data to send)
--
-- RETURN: -1 on fail, anything else on success
--
-- NOTES:
-- Sends a Msg structure of data to a specific message queue pertaining to the messageQID
----------------------------------------------------------------------------------------------------------------------*/
int sendMessage(int messageQID, Msg *qbuffer) {
	int length, result;
	length = qbuffer->mlen + MSGHDRSIZE;
	if ((result = msgsnd(messageQID, qbuffer, length, 0)) == -1) {
		std::cout << "Errno #: " << errno << " in sendMessage" << std::endl;	
		return -1;
	}
	return result;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: readMessage
--
-- DATE: January 29th, 2016
--
-- REVISIONS: January 29th, 2016: Created
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- INTERFACE: int readMessage(int messageQID: Message queue ID to read the message from
--			      Msg *qbuffer: Pointer to the Msg structure we want to store the data in
--			      long type: Type of message to read for)
--
-- RETURN: -1 on fail, anything else on success
--
-- NOTES:
-- Reads the next FIFO message from the given message queue of type type and stores it in Msg qbuffer.
-- If the type is 0, it does pure FIFO and ignores the type parameter.
----------------------------------------------------------------------------------------------------------------------*/
int readMessage(int messageQID, Msg *qbuffer, long type) {
	int length, result;
	length = MAXMESSAGEDATA;
	if ((result = msgrcv(messageQID, qbuffer, length, type, 0)) == -1) {
		std::cout << "Errno #: "  << errno << " in readMessage" << std::endl;
		return -1;
	}
	return result;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: readMessageNonBlocking
--
-- DATE: January 29th, 2016
--
-- REVISIONS: January 29th, 2016: Created
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- INTERFACE: int readMessageNonBlocking(int messageQID: Message queue ID to read the message from
--			      Msg *qbuffer: Pointer to the Msg structure we want to store the data in
--			      long type: Type of message to read for)
--
-- RETURN: -1 on fair OR if there is nothing to read. Anything else if something was read.
--
-- NOTES:
-- Reads the next FIFO message from the given message queue of type type and stores it in Msg qbuffer.
-- If the type is 0, it does pure FIFO and ignores the type parameter. This does not block, so if there is
-- nothing to be read it returns -1 instantly.
----------------------------------------------------------------------------------------------------------------------*/
int readMessageNonBlocking(int messageQID, Msg *qbuffer, long type) {
	int length, result;
	length = MAXMESSAGEDATA;
	if ((result = msgrcv(messageQID, qbuffer, length, type, IPC_NOWAIT)) == -1) {
		return -1;
	}
	return result;
}
