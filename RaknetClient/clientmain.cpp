#include <stdio.h>
#include <string>
#include <thread>
#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"
#include "RakNetTypes.h"


#include "client.h"

using namespace RakNetTutorial;

Client * client;

void ListenForPackets();
void GetMessages();

int main(void)
{	
	client = new Client();
	client->startup();

	std::thread packetListener(ListenForPackets);
	GetMessages();

	packetListener.join();

	return 0;
}

void ListenForPackets()
{
	client->readPackets();
}

void GetMessages()
{
	client->getMessages();
}