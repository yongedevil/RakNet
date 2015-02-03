#include "client.h"


using namespace RakNetTutorial;

const int Client::DEFAULT_SERVER_PORT = 60000;

Client::Client() : m_listen (true), m_connected(false), m_peer (RakNet::RakPeerInterface::GetInstance())
{
}

Client::~Client()
{
}

void Client::startup()
{
	startup(DEFAULT_SERVER_PORT);
}

void Client::startup(int serverPort)
{
	char str[512];
	m_connected = false;

	RakNet::StartupResult result;
	RakNet::SocketDescriptor sd;

	result = m_peer->Startup(1,&sd, 1);
		
	//TODO: make sure it started
	assert(RakNet::StartupResult::RAKNET_STARTED == result);

	std::cout << "Please enter your name: ";
	std::cin >> m_name;
		


	printf("Enter server IP or hit enter for 127.0.0.1\n");
	gets_s(str);
	if (str[0]==0)
	{
		strcpy_s(str, "127.0.0.1");
	}
	printf("Starting the client.\n");

	m_peer->Connect(str, serverPort, 0,0);
}

void Client::shutdown()
{
	char * str = "I'm out!";

	RakNet::BitStream bsOut;
	bsOut.Write((RakNet::MessageID)ID_DISCONNECTION_NOTIFICATION);
	bsOut.Write(str);

	m_peer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,m_hostAddress,false);
	

	m_peer->Shutdown(100);

	unsigned short numConnections;
	RakNet::SystemAddress * systemAddressList = NULL;
	m_peer->GetConnectionList(systemAddressList, &numConnections);

	for(int i = 0; i < numConnections; ++i)
	{
		m_peer->CloseConnection(systemAddressList[i], true, 0, HIGH_PRIORITY);
	}


	m_peer->CloseConnection(m_hostAddress, true, 0, HIGH_PRIORITY);

	
	RakNet::RakPeerInterface::DestroyInstance(m_peer);
}

void Client::readPackets()
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

void Client::readPacket(RakNet::Packet *packet)
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
		ConnectionAccepted(packet);
		break;
	case ID_NEW_INCOMING_CONNECTION:
		printf("A connection is incoming.\n");
		break;
	case ID_NO_FREE_INCOMING_CONNECTIONS:
		printf("The server is full.\n");
		break;
	case ID_DISCONNECTION_NOTIFICATION:
		DisconnectMessage(packet);
		break;
	case ID_CONNECTION_LOST:
		printf("Connection lost.\n");
		break;
		
	case ClientGameMessages::ID_GAME_MESSAGE_1:
		GameMessage1(packet);
		break;
	case ClientGameMessages::ID_GAME_MESSAGE_2:
		GameMessage2(packet);
		break;
	default:
		printf("Message with identifier %i has arrived.\n", packet->data[0]);
		break;
	}
}

void Client::getMessages()
{
	char customMessage[512];

	while(!m_connected)
	{
	}

	while(m_listen)
	{
		printf("You: ");
		gets_s(customMessage, 512);

		if(customMessage[0] == '/')
		{
			sendCommand(customMessage);
		}

		else
		{
			sendText(customMessage);
		}
	}
}

void Client::ConnectionAccepted(RakNet::Packet *packet)
{
	m_hostAddress = packet->systemAddress;
	printf("Our connection request has been accepted.\n");
	m_connected = true;
}

void Client::DisconnectMessage(RakNet::Packet *packet)
{
	RakNet::RakString rs;
	RakNet::BitStream bsIn(packet->data,packet->length,false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
	bsIn.Read(rs);
	printf("%s\n", rs.C_String());
}

void Client::GameMessage1(RakNet::Packet *packet)
{
	RakNet::RakString rs;
	RakNet::BitStream bsIn(packet->data,packet->length,false);
	//read in custom message
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
	bsIn.Read(rs);
	printf("%s\n", rs.C_String());

	//now the server is sending a message back all client
	RakNet::BitStream bsOut;
	bsOut.Write((RakNet::MessageID)ClientGameMessages::ID_GAME_MESSAGE_2);

	bsOut.Write(rs.C_String());
	m_peer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,packet->systemAddress,true);
}

void Client::GameMessage2(RakNet::Packet *packet)
{
	RakNet::RakString rs;
	RakNet::BitStream bsIn(packet->data,packet->length,false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));

	bsIn.Read(rs);
	printf("%s: ", rs.C_String());
	bsIn.Read(rs);
	printf("%s\n", rs.C_String());
}

void Client::log(std::string msg)
{
	printf("%s\n", msg.c_str());
}
		
void Client::sendText(char * msg)
{
	RakNet::BitStream bsOut;
	bsOut.Write((RakNet::MessageID)ClientGameMessages::ID_GAME_MESSAGE_1);
	bsOut.Write(m_name.c_str());
	bsOut.Write(msg);
	m_peer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,m_hostAddress,false);
}

void Client::sendCommand(char * msg)
{
	//parse command and send (using enum?)
}