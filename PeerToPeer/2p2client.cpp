#include "p2pclient.h"

#include <assert.h>
#include <cstdio>

#include <DS_List.h>

#include "gameState.h"

using namespace RakNetLabs;

const int P2PClient::DEFAULT_PORT = 60000;
const char * P2PClient::DEFAULT_PORT_STR = "60000";
const int P2PClient::DEFAULT_MAX_CONNECTIONS = 8;

P2PClient::P2PClient() :
	m_peer(NULL),
	m_serverPort(DEFAULT_PORT),
	m_name(""),
	m_curState(NULL),
	m_quit(false)
{

}
P2PClient::~P2PClient()
{
}

void P2PClient::startup()
{
	startup(m_serverPort);
}
void P2PClient::startup(int & serverPort)
{
	char name_str[512];
	printf("Please enter your name: ");
	gets_s(name_str);
	m_name = std::string(name_str);

	m_peer = RakNet::RakPeerInterface::GetInstance();

	m_peer->AttachPlugin(&m_readyEventPlugin);
	m_peer->AttachPlugin(&m_fcm2);
	m_peer->AttachPlugin(&m_cg2);
	m_peer->SetMaximumIncomingConnections(DEFAULT_MAX_CONNECTIONS);

	m_fcm2.SetAutoparticipateConnections(true);
	m_fcm2.SetConnectOnNewRemoteConnection(true, "");
	m_cg2.SetAutoProcessNewConnections(true);

	RakNet::SocketDescriptor sd(serverPort, 0);
	while(RakNet::IRNS2_Berkley::IsPortInUse(sd.port, sd.hostAddress, sd.socketFamily, SOCK_DGRAM))
		sd.port++;

	m_serverPort = sd.port;

	RakNet::StartupResult result = m_peer->Startup(DEFAULT_MAX_CONNECTIONS, &sd, 1);
	assert(RakNet::RAKNET_STARTED == result);

	printf("Started on port %i\n", sd.port);

	RakSleep(200);

	char address_str[512];
	char port[64];
	int connectPort;

	printf("Enter IP or leave blank for 127.0.0.1");
	gets_s(address_str);

	if(0 == address_str[0])
	{
		strcpy_s(address_str, "127.0.0.1");
	}

	printf("Enter port or leave blank for %i", DEFAULT_PORT);
	gets_s(port);
	if(0 ==port[0])
	{
		connectPort = DEFAULT_PORT;
	}
	else
	{
		connectPort = atoi(port);
	}

	RakNet::ConnectionAttemptResult connectResult = m_peer->Connect(address_str, connectPort, 0, 0, 0);
	
	assert(RakNet::CONNECTION_ATTEMPT_STARTED == connectResult);

	printf("connecting...\n");

	//setup states
	GameState::state_connecting->init(this);
	GameState::state_lobby->init(this);
	GameState::state_turn->init(this);
	GameState::state_end->init(this);

	m_curState = GameState::state_connecting;
}

void P2PClient::shutdown()
{
	unsigned short numConnections;

	m_peer->Shutdown(100, 0);
	RakNet::SystemAddress * systemAddressList = NULL;
	m_peer->GetConnectionList(systemAddressList, &numConnections);

	for(int i = 0; i < numConnections; ++i)
	{
		m_peer->CloseConnection(systemAddressList[i], true, 0, HIGH_PRIORITY);
	}


	m_peer->CloseConnection(m_hostAddress, true, 0, HIGH_PRIORITY);

	
	RakNet::RakPeerInterface::DestroyInstance(m_peer);
}

void P2PClient::play()
{
	char choice = 0;

	while(!m_quit)
	{
		if(m_curState)
		{
			m_curState->display();
			m_curState->input('c');
		}
	}
}


void P2PClient::setState(GameState * state)
{
	m_curState = state;
	
	if(m_curState)
		m_curState->enter();
}


void P2PClient::readPackets()
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

void P2PClient::readPacket(RakNet::Packet *packet)
{
	printf("\n");

	switch(packet->data[0])
	{

	case ID_NEW_INCOMING_CONNECTION:
		printf("ID_NEW_INCOMING_CONNECTION\n");
		incomingConnection(packet);
		break;
	case ID_CONNECTION_REQUEST_ACCEPTED:
		printf("ID_CONNECTION_REQUEST_ACCEPTED\n");
		connectionAccepted(packet);
		break;
	case ID_READY_EVENT_ALL_SET:
		printf("Got ID_READY_EVENT_ALL_SET from %s\n", packet->guid.ToString());
		readyEventAllSet(packet);
		break;

	case ID_READY_EVENT_SET:
		printf("Got ID_READY_EVENT_SET from %s\n", packet->guid.ToString());
		readyEventSet(packet);
		break;

	case ID_READY_EVENT_UNSET:
		printf("Got ID_READY_EVENT_UNSET from %s\n", packet->guid.ToString());
		readyEventUnset(packet);
		break;

		
	case ID_DISCONNECTION_NOTIFICATION:
		disconnectMessage(packet);
		break;
	case ID_ALREADY_CONNECTED:
		// Connection lost normally
		printf("ID_ALREADY_CONNECTED with guid %" PRINTF_64_BIT_MODIFIER "u\n", packet->guid);
		break;
		
	case ID_INCOMPATIBLE_PROTOCOL_VERSION:
		printf("ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
		break;
	case ID_REMOTE_DISCONNECTION_NOTIFICATION: // Server telling the clients of another client disconnecting gracefully.  You can manually broadcast this in a peer to peer enviroment if you want.
		printf("ID_REMOTE_DISCONNECTION_NOTIFICATION\n"); 
		break;
	case ID_REMOTE_CONNECTION_LOST: // Server telling the clients of another client disconnecting forcefully.  You can manually broadcast this in a peer to peer enviroment if you want.
		printf("ID_REMOTE_CONNECTION_LOST\n");
		break;
	case ID_REMOTE_NEW_INCOMING_CONNECTION: // Server telling the clients of another client connecting.  You can manually broadcast this in a peer to peer enviroment if you want.
		printf("ID_REMOTE_NEW_INCOMING_CONNECTION\n");
		break;
	case ID_CONNECTION_BANNED: // Banned from this server
		printf("We are banned from this server.\n");
		break;			
	case ID_CONNECTION_ATTEMPT_FAILED:
		printf("Connection attempt failed\n");
		break;
	case ID_NO_FREE_INCOMING_CONNECTIONS:
		// Sorry, the server is full.  I don't do anything here but
		// A real app should tell the user
		printf("ID_NO_FREE_INCOMING_CONNECTIONS\n");
		break;
	case ID_INVALID_PASSWORD:
		printf("ID_INVALID_PASSWORD\n");
		break;
	case ID_CONNECTION_LOST:
		// Couldn't deliver a reliable packet - i.e. the other system was abnormally
		// terminated
		printf("ID_CONNECTION_LOST\n");
		break;
	case ID_CONNECTED_PING:
	case ID_UNCONNECTED_PING:
		printf("Ping from %s\n", packet->systemAddress.ToString(true));
		break;
	
	//custom packets
	case ClientGameMessages::ID_CHAT_MESSAGE:
		chatMessage(packet);
		break;
	case ClientGameMessages::ID_START_GAME:
		chatMessage(packet);
		break;
	case ClientGameMessages::ID_END_TURN:
		chatMessage(packet);
		break;
	case ClientGameMessages::ID_END_GAME:
		chatMessage(packet);
		break;
	default:
		printf("Message with identifier %i has arrived.\n", packet->data[0]);
		break;
	}
}

void P2PClient::incomingConnection(RakNet::Packet *packet)
{
	if(GameState::state_connecting == m_curState)
	{
		m_readyEventPlugin.AddToWaitList(static_cast<int>(eReadyEvents::EVENT_ENDTURN), packet->guid);
	}
}

void P2PClient::connectionAccepted(RakNet::Packet *packet)
{
	if(GameState::state_connecting == m_curState)
	{
		m_readyEventPlugin.AddToWaitList(static_cast<int>(eReadyEvents::EVENT_ENDTURN), packet->guid);
		m_connected = true;

		setState(GameState::state_lobby);
	}
}

void P2PClient::disconnectMessage(RakNet::Packet *packet)
{
	RakNet::RakString rs;
	RakNet::BitStream bsIn(packet->data,packet->length,false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
	bsIn.Read(rs);
	printf("%s\n", rs.C_String());
}

void P2PClient::chatMessage(RakNet::Packet *packet)
{
	RakNet::RakString rs;
	RakNet::BitStream bsIn(packet->data,packet->length,false);
	//read in custom message
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));

	bsIn.Read(rs);
	printf("%s: ", rs.C_String());
	bsIn.Read(rs);
	printf("%s\n", rs.C_String());
}


void P2PClient::printConnections()
{
	int i,j;
	char ch=0;
	RakNet::SystemAddress systemAddress;
	
	printf("--------------------------------\n");

	RakNet::SystemAddress remoteSystems[8];
	unsigned short numberOfSystems;
	m_peer->GetConnectionList(remoteSystems, &numberOfSystems);

	for (i=0; i < numberOfSystems; i++)
	{
		printf("%i (Conn): ", 60000+i);
		for (j=0; j < numberOfSystems; j++)
		{
			systemAddress=m_peer->GetSystemAddressFromIndex(j);
			if (systemAddress!=RakNet::UNASSIGNED_SYSTEM_ADDRESS)
				printf("%i ", systemAddress.GetPort());
		}

		printf("\n");
	}
	printf("\n");

	printf("--------------------------------\n");
}

void P2PClient::readyEventSet(RakNet::Packet *packet)
{
	eReadyEvents eventID;
	RakNet::BitStream bs(packet->data, packet->length, false);

	printf("Got ID_READY_EVENT_SET from %s: ", packet->guid.ToString());
	bs.IgnoreBytes(sizeof(RakNet::MessageID));
	bs.Read(eventID);

	switch(eventID)
	{
	case eReadyEvents::EVENT_READYSTART:
		printf("ready to start\n");
		break;

	case eReadyEvents::EVENT_ENDTURN:
		printf("end turn\n");
		break;

	case eReadyEvents::EVENT_ENDGAME:
		printf("end game\n");
		break;

	default:
		printf("unknown\n");
	}
}

void P2PClient::readyEventUnset(RakNet::Packet *packet)
{
	eReadyEvents eventID;
	RakNet::BitStream bs(packet->data, packet->length, false);
	
	printf("Got ID_READY_EVENT_UNSET from %s: ", packet->guid.ToString());
	bs.IgnoreBytes(sizeof(RakNet::MessageID));
	bs.Read(eventID);
	
	switch(eventID)
	{
	case eReadyEvents::EVENT_READYSTART:
		printf("ready to start\n");
		break;

	case eReadyEvents::EVENT_ENDTURN:
		printf("end turn\n");
		break;

	case eReadyEvents::EVENT_ENDGAME:
		printf("end game\n");
		break;

	default:
		printf("unknown\n");
	}
}

void P2PClient::readyEventAllSet(RakNet::Packet *packet)
{
	eReadyEvents eventID;
	RakNet::BitStream bs(packet->data, packet->length, false);

	printf("Got ID_READY_EVENT_ALL_SET from %s: ", packet->guid.ToString());
	bs.IgnoreBytes(sizeof(RakNet::MessageID));
	bs.Read(eventID);

	switch(eventID)
	{
	case eReadyEvents::EVENT_READYSTART:
		setState(GameState::state_turn);
		printf("ready to start\n");
		break;

	case eReadyEvents::EVENT_ENDTURN:
		setState(GameState::state_turn);
		printf("end turn\n");
		break;

	case eReadyEvents::EVENT_ENDGAME:
		setState(GameState::state_end);
		printf("end game\n");
		break;

	default:
		printf("unknown\n");
	}
}

void P2PClient::sendText(char * msg)
{
	if(0 != msg[0] || true)
	{
		RakNet::BitStream bsOut;
		bsOut.Write((RakNet::MessageID)ClientGameMessages::ID_CHAT_MESSAGE);
		bsOut.Write(m_name.c_str());
		bsOut.Write(msg);
		m_peer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,RakNet::UNASSIGNED_SYSTEM_ADDRESS,true);
	}
}

void P2PClient::sendCommand(char * msg)
{
	if(0 == strcmp("/ready", msg))
	{
		if(m_readyEventPlugin.SetEvent(static_cast<int>(eReadyEvents::EVENT_ENDTURN), true))	
			printf("This system is signaled\n");
		else
			printf("This system is signaled FAILED\n");
	}

	else if(0 == strcmp("/unready", msg))
	{
		if(m_readyEventPlugin.SetEvent(static_cast<int>(eReadyEvents::EVENT_ENDTURN), false))
			printf("This system is unsignaled\n");
		else
			printf("This system is unsignaled FAILED\n");
	}
}

int P2PClient::getNumPlayersWaiting(eReadyEvents eventID)
{
	int numPlayer = 0;

	DataStructures::List<RakNet::SystemAddress> addresses;
	DataStructures::List<RakNet::RakNetGUID> guids;
	m_peer->GetSystemList(addresses, guids);
	
	for(int i = 0; i < guids.Size(); ++i)
	{
		RakNet::ReadyEventSystemStatus status = m_readyEventPlugin.GetReadyStatus(static_cast<int>(eventID), guids[i]);

		if(RakNet::RES_NOT_WAITING != status && RakNet::RES_UNKNOWN_EVENT != status)
		{
			numPlayer++;
		}
	}

	return numPlayer;
}

int P2PClient::getNumPlayersReady(eReadyEvents eventID)
{
	int numReady = 0;

	DataStructures::List<RakNet::SystemAddress> addresses;
	DataStructures::List<RakNet::RakNetGUID> guids;
	m_peer->GetSystemList(addresses, guids);

	for(int i = 0; i < guids.Size(); ++i)
	{
		RakNet::ReadyEventSystemStatus status = m_readyEventPlugin.GetReadyStatus(static_cast<int>(eventID), guids[i]);

		if(RakNet::RES_READY == status || RakNet::RES_ALL_READY == status )
		{
			numReady++;
		}
	}

	return numReady;
}