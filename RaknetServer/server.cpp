#include "server.h"

using namespace RakNetTutorial;

const int Server::DEFAULT_SERVER_PORT = 60000;
const int Server::DEFAULT_MAX_CONNECTIONS = 10;

Server::Server() : m_listen (true), m_serverPort(DEFAULT_SERVER_PORT), m_peer (RakNet::RakPeerInterface::GetInstance())
{
}

Server::~Server()
{

}

void Server::startup()
{
	startup(DEFAULT_SERVER_PORT, DEFAULT_MAX_CONNECTIONS);
}
void Server::startup(int serverPort, int maxConnections)
{
	m_serverPort = serverPort;

	RakNet::StartupResult result;
	RakNet::SocketDescriptor sd(m_serverPort, 0);

	result = m_peer->Startup(maxConnections, &sd, 1);
	assert(RakNet::StartupResult::RAKNET_STARTED == result);

	log("starting the server.");

	m_peer->SetMaximumIncomingConnections(maxConnections);
}

void Server::shutdown()
{
	char * str = "So long and thanks for all the fish.  Server is now shutting down.";

	RakNet::BitStream bsOut;
	bsOut.Write((RakNet::MessageID)ID_DISCONNECTION_NOTIFICATION);
	bsOut.Write(str);

	m_peer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,0,false);

	RakNet::RakPeerInterface::DestroyInstance(m_peer);
}

void Server::readPackets()
{
	RakNet::Packet * packet = NULL;

	while(m_listen)
	{
		for(packet = m_peer->Receive(); packet; m_peer->DeallocatePacket(packet), packet = m_peer->Receive())
		{
			readPacket(packet);
		}
	}
}

void Server::readPacket(RakNet::Packet *packet)
{
	switch(packet->data[0])
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
			ConnectionAccepted(packet);
		}
		break;
	case ID_NEW_INCOMING_CONNECTION:
		printf("A connection is incoming.\n");
		break;
	case ID_NO_FREE_INCOMING_CONNECTIONS:
		printf("The server is full.\n");
		break;
	case ID_DISCONNECTION_NOTIFICATION:
		printf("A client has disconnected.\n");
		break;
	case ID_CONNECTION_LOST:
		printf("A client lost the connection.\n");
		break;
		
	case GameMessages::ID_GAME_MESSAGE_1:
		{
			GameMessage1(packet);
		}
		break;
	case GameMessages::ID_GAME_MESSAGE_2:
		{
			GameMessage2(packet);
		}
		break;
	default:
		printf("Message with identifier %i has arrived.\n", packet->data[0]);
		break;
	}
}

void Server::ConnectionAccepted(RakNet::Packet *packet)
{
	char * str = "message here";
	printf("Our connection request has been accepted.\n");

	// Use a BitStream to write a custom user message
	// Bitstreams are easier to use than sending casted structures, and handle endian swapping automatically
	RakNet::BitStream bsOut;
	bsOut.Write((RakNet::MessageID)GameMessages::ID_GAME_MESSAGE_1);
	bsOut.Write(str);
	
	m_peer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,packet->systemAddress,false);
}

void Server::GameMessage1(RakNet::Packet *packet)
{
	RakNet::RakString rsName;
	RakNet::RakString rsMsg;
	RakNet::BitStream bsIn(packet->data,packet->length,false);
	//read in custom message
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));

	//read name
	bsIn.Read(rsName);
	printf("%s: ", rsName.C_String());
	//read message
	bsIn.Read(rsMsg);
	printf("%s\n", rsMsg.C_String());

	//now the server is sending a message back all client
	RakNet::BitStream bsOut;
	bsOut.Write((RakNet::MessageID)GameMessages::ID_GAME_MESSAGE_2);

	bsOut.Write(rsName.C_String());
	bsOut.Write(rsMsg.C_String());
	m_peer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,packet->systemAddress,true);
}

void Server::GameMessage2(RakNet::Packet *packet)
{
	RakNet::RakString rs;
	RakNet::BitStream bsIn(packet->data,packet->length,false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
	bsIn.Read(rs);
	printf("%s\n", rs.C_String());
}

void Server::log(std::string msg)
{
	printf("%s\n", msg.c_str());
}