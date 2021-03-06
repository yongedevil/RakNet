#include <thread>

#include "RakPeerInterface.h"

#include "p2pclient.h"

using namespace RakNetLabs;

P2PClient * client;

void ListenForPackets();
void Play();

int main(void)
{
	client = new P2PClient();
	client->startup();

	std::thread packetListener(ListenForPackets);
	Play();

	packetListener.join();

	return 0;
}


void ListenForPackets()
{
	client->readPackets();
}

void Play()
{
	client->play();
}