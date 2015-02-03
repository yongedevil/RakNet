#include <stdio.h>
#include <string.h>
#include <thread>
#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"
#include "RakNetTypes.h"  // MessageID

#include "server.h"

using namespace RakNetTutorial;

Server * server;

void ListenForPackets();

int main(void)
{	
	server = new Server();
	server->startup();
	server->readPackets();

	return 0;
}

void ListenForPackets()
{
	server->readPackets();
}