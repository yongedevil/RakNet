#include <stdio.h>
#include <string.h>
#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"
#include "RakNetTypes.h"  // MessageID

#define MAX_CLIENTS 10
#define SERVER_PORT 60000

enum GameMessages
{
	ID_GAME_MESSAGE_1=ID_USER_PACKET_ENUM+1,
	ID_GAME_MESSAGE_2=ID_GAME_MESSAGE_1+1
};

void InitClient(RakNet::RakPeerInterface *peer);
void InitServer(RakNet::RakPeerInterface *peer, int maxConnections);

void ReadPacket(RakNet::RakPeerInterface *peer, RakNet::Packet *packet, bool isServer=false);
void GetCusstomMessage(char * customMessage);
void ConnectionAccepted(RakNet::RakPeerInterface *peer, RakNet::Packet *packet, bool isServer=false);
void GameMessage1(RakNet::RakPeerInterface *peer, RakNet::Packet *packet, bool isServer=false);
void GameMessage2(RakNet::RakPeerInterface *peer, RakNet::Packet *packet, bool isServer=false);

int main(void)
{
	char str[512];
	
	RakNet::Packet *packet = NULL;
	RakNet::RakPeerInterface *peer = RakNet::RakPeerInterface::GetInstance();
	bool isServer;

	printf("(C) or (S)erver?\n");
	gets_s(str);

	if ((str[0]=='c')||(str[0]=='C'))
	{
		isServer = false;
		InitClient(peer);

	}
	else
	{
		isServer = true;
		InitServer(peer, MAX_CLIENTS);
	}

	while (1)
	{
		ReadPacket(peer, packet, isServer);
	}


	RakNet::RakPeerInterface::DestroyInstance(peer);

	return 0;
}


void InitClient(RakNet::RakPeerInterface *peer)
{
	char str[512];

	RakNet::StartupResult result;
	RakNet::SocketDescriptor sd;

	result = peer->Startup(1,&sd, 1);
		
	//TODO: make sure it started
	if(!RakNet::StartupResult::RAKNET_STARTED == result)
	{
		printf("peer->Startup FAILED!");
	}

	printf("Enter server IP or hit enter for 127.0.0.1\n");
	gets(str);
	if (str[0]==0)
	{
		strcpy(str, "127.0.0.1");
	}
	printf("Starting the client.\n");
	peer->Connect(str, SERVER_PORT, 0,0);
}

void InitServer(RakNet::RakPeerInterface *peer, int maxConnections=10)
{
	RakNet::SocketDescriptor sd(SERVER_PORT,0);
	peer->Startup(maxConnections, &sd, 1);

	printf("Starting the server.\n");
	
	// We need to let the server accept incoming connections from the clients
	peer->SetMaximumIncomingConnections(maxConnections);
}


void ReadPacket(RakNet::RakPeerInterface *peer, RakNet::Packet *packet, bool isServer)
{
	for (packet=peer->Receive(); packet; peer->DeallocatePacket(packet), packet=peer->Receive())
	{
		switch (packet->data[0])
		{
		case ID_REMOTE_DISCONNECTION_NOTIFICATION:
			printf("Another client has disconnected.\n");
			break;
		case ID_REMOTE_CONNECTION_LOST:
			printf("Another client has lost the connection.\n");
			break;
		case ID_REMOTE_NEW_INCOMING_CONNECTION:
			printf("Another client has connected.\n");
			break;
		case ID_CONNECTION_REQUEST_ACCEPTED:
			{
				ConnectionAccepted(peer, packet, isServer);
			}
			break;
		case ID_NEW_INCOMING_CONNECTION:
			printf("A connection is incoming.\n");
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			printf("The server is full.\n");
			break;
		case ID_DISCONNECTION_NOTIFICATION:
			if (isServer)
			{
				printf("A client has disconnected.\n");
			} else
			{
				printf("We have been disconnected.\n");
			}
			break;
		case ID_CONNECTION_LOST:
			if (isServer){
				printf("A client lost the connection.\n");
			} else {
				printf("Connection lost.\n");
			}
			break;
			
		case ID_GAME_MESSAGE_1:
			{
				GameMessage1(peer, packet, isServer);
			}
			break;
		case ID_GAME_MESSAGE_2:
			{
				GameMessage2(peer, packet, isServer);
			}
			break;
		default:
			printf("Message with identifier %i has arrived.\n", packet->data[0]);
			break;
		}
	}
}


void GetCusstomMessage(char * customMessage)
{
	printf("What is your custom message, friend.\n");
	gets_s(customMessage, 512);
}

void ConnectionAccepted(RakNet::RakPeerInterface *peer, RakNet::Packet *packet, bool isServer)
{
	printf("Our connection request has been accepted.\n");
	// Use a BitStream to write a custom user message
	// Bitstreams are easier to use than sending casted structures, and handle endian swapping automatically
	RakNet::BitStream bsOut;
	bsOut.Write((RakNet::MessageID)ID_GAME_MESSAGE_1);
	
	char customMessage[512];
	GetCusstomMessage(customMessage);
	bsOut.Write(customMessage);
	peer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,packet->systemAddress,false);
}

void GameMessage1(RakNet::RakPeerInterface *peer, RakNet::Packet *packet, bool isServer)
{
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data,packet->length,false);
				//read in custom message
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				printf("%s\n", rs.C_String());


				//now the server is sending a message back all client
				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_GAME_MESSAGE_2);

				bsOut.Write(rs.C_String());
				peer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,packet->systemAddress,true);
}

void GameMessage2(RakNet::RakPeerInterface *peer, RakNet::Packet *packet, bool isServer)
{
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data,packet->length,false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				printf("%s\n", rs.C_String());
}