# IPCClientServer

IPC Client-Server applications to send the contents of local files across via message queues in Linux.

The purpose of these applications is to utilize message queues to send & receive file data. The clients request a file name & a priority between low/medium/high, and in turn the server fetches that data from the file and returns it to the client via messaging queues. 

The server needs to be able to handle multiple clients, as well as have both the server and client handle cleanup when something goes wrong, like the other dies unexpectedly.   

# How To Compile

Download/Clone the repository, locate the folder in your terminal and then simply type "make" into the terminal.

Must be run on Linux. Requires the G++ compiler to be installed.

# How To Run

After compiling, type "./Server" in the terminal to run the server, and "./Client" into another terminal to run the client.

Must be run on Linux.

# Design/Testing

All design & testing work can be found in UserManual.pdf, along with step by step visuals on how to run and compile the program.
