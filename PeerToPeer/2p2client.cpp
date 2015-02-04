#include "p2pclient.h"

#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include <DS_List.h>
#include "Kbhit.h"

#include "gameState.h"

using namespace RakNetLabs;

const int P2PClient::DEFAULT_PORT = 60002;
const char * P2PClient::DEFAULT_PORT_STR = "60002";
const int P2PClient::DEFAULT_MAX_CONNECTIONS = 8;

P2PClient::P2PClient() :
	m_peer(NULL),
	m_serverPort(DEFAULT_PORT),
	m_name(""),
	m_curState(NULL),
	m_quit(false),
	m_redisplay(false)
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

	RakNet::RakPeerInterface::DestroyInstance(m_peer);
}

void P2PClient::play()
{
	char choice = 0;

	while(!m_quit)
	{
		if(m_curState)
		{
			if (m_redisplay)
			{
				system("CLS");
				m_curState->display();
				std::cout << "RakNet: " << m_msgRak << std::endl;
				//std::cout << "Chat: " << m_msgChat << std::endl;
				//std::cout << "Msg: " << m_msg << std::endl;
				m_redisplay = false;
			}

			if(_kbhit())
			{
				choice = _getch();
				m_curState->input(choice);
			}

		}
	}
}


void P2PClient::setState(GameState * state)
{
	m_curState = state;
	
	if(m_curState)
		m_curState->enter();

	m_redisplay = true;
}


void P2PClient::readPackets()
{
	RakNet::Packet * packet = NULL;

	while(!m_quit)
	{
		//clearMsgRak();
		for(packet = m_peer->Receive(); packet; m_peer->DeallocatePacket(packet), packet = m_peer->Receive())
		{
			readPacket(packet);
		}
	}
}

void P2PClient::readPacket(RakNet::Packet *packet)
{
	std::stringstream msgStream;
	switch(packet->data[0])
	{

	case ID_NEW_INCOMING_CONNECTION:
		msgStream << "ID_NEW_INCOMING_CONNECTION" << std::endl;
		setMsgRak(msgStream.str());
		incomingConnection(packet);
		break;
	case ID_CONNECTION_REQUEST_ACCEPTED:
		msgStream << "ID_CONNECTION_REQUEST_ACCEPTED" << std::endl;
		setMsgRak(msgStream.str());
		connectionAccepted(packet);
		break;
	case ID_READY_EVENT_ALL_SET:
		readyEventAllSet(packet);
		break;

	case ID_READY_EVENT_SET:
		readyEventSet(packet);
		break;

	case ID_READY_EVENT_UNSET:
		readyEventUnset(packet);
		break;

		
	case ID_DISCONNECTION_NOTIFICATION:
		disconnectMessage(packet);
		break;
	case ID_ALREADY_CONNECTED:
		// Connection lost normally
		msgStream << "ID_ALREADY_CONNECTED with guid " << packet->guid.ToString() << std::endl;
		setMsgRak(msgStream.str());
		break;
		
	case ID_INCOMPATIBLE_PROTOCOL_VERSION:
		msgStream << "ID_INCOMPATIBLE_PROTOCOL_VERSION" << std::endl;
		setMsgRak(msgStream.str());
		break;
	case ID_REMOTE_DISCONNECTION_NOTIFICATION: // Server telling the clients of another client disconnecting gracefully.  You can manually broadcast this in a peer to peer enviroment if you want.
		msgStream << "ID_REMOTE_DISCONNECTION_NOTIFICATION" << std::endl;
		setMsgRak(msgStream.str());
		break;
	case ID_REMOTE_CONNECTION_LOST: // Server telling the clients of another client disconnecting forcefully.  You can manually broadcast this in a peer to peer enviroment if you want.
		msgStream << "ID_REMOTE_CONNECTION_LOST" << std::endl;
		setMsgRak(msgStream.str());
		break;
	case ID_REMOTE_NEW_INCOMING_CONNECTION: // Server telling the clients of another client connecting.  You can manually broadcast this in a peer to peer enviroment if you want.
		msgStream << "ID_REMOTE_NEW_INCOMING_CONNECTION" << std::endl;
		setMsgRak(msgStream.str());
		break;
	case ID_CONNECTION_BANNED: // Banned from this server
		msgStream << "We are banned from this server." << std::endl;
		setMsgRak(msgStream.str());
		break;			
	case ID_CONNECTION_ATTEMPT_FAILED:
		msgStream << "Connection attempt failed" << std::endl;
		setMsgRak(msgStream.str());
		break;
	case ID_NO_FREE_INCOMING_CONNECTIONS:
		// Sorry, the server is full.  I don't do anything here but
		// A real app should tell the user
		msgStream << "ID_NO_FREE_INCOMING_CONNECTIONS" << std::endl;
		setMsgRak(msgStream.str());
		break;
	case ID_INVALID_PASSWORD:
		msgStream << "ID_INVALID_PASSWORD" << std::endl;
		setMsgRak(msgStream.str());
		break;
	case ID_CONNECTION_LOST:
		// Couldn't deliver a reliable packet - i.e. the other system was abnormally
		// terminated
		msgStream << "ID_CONNECTION_LOST" << std::endl;
		setMsgRak(msgStream.str());
		break;
	case ID_CONNECTED_PING:
	case ID_UNCONNECTED_PING:
		msgStream << "Ping from " << packet->systemAddress.ToString(true) << std::endl;
		setMsgRak(msgStream.str());
		break;
	
	//custom packets
	case static_cast<int>(ClientGameMessages::ID_CHAT_MESSAGE):
		chatMessage(packet);
		break;
	case static_cast<int>(ClientGameMessages::ID_START_GAME):
		chatMessage(packet);
		break;
	case static_cast<int>(ClientGameMessages::ID_END_TURN):
		chatMessage(packet);
		break;
	case static_cast<int>(ClientGameMessages::ID_END_GAME):
		chatMessage(packet);
		break;
	default:
		msgStream << "Message with identifier " << static_cast<int>(packet->data[0]) << " has arrived." << std::endl;
		setMsgRak(msgStream.str());
		break;
	}
}

void P2PClient::incomingConnection(RakNet::Packet *packet)
{
	if(packet->guid != m_peer->GetMyGUID())
	{
		m_readyEventPlugin.AddToWaitList(static_cast<int>(eReadyEvents::EVENT_READYSTART), packet->guid);
	}
}

void P2PClient::connectionAccepted(RakNet::Packet *packet)
{
	if(packet->guid != m_peer->GetMyGUID())
	{
		m_readyEventPlugin.AddToWaitList(static_cast<int>(eReadyEvents::EVENT_READYSTART), packet->guid);
	}
	setState(GameState::state_lobby);
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
	std::stringstream msgStream;
	RakNet::RakString rs;
	RakNet::BitStream bsIn(packet->data,packet->length,false);
	//read in custom message
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));

	bsIn.Read(rs);
	msgStream << rs.C_String() << ": ";
	bsIn.Read(rs);
	msgStream << rs.C_String() << std::endl;

	setMsgChat(msgStream.str());
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
	std::stringstream msgStream;
	eReadyEvents eventID;
	RakNet::BitStream bs(packet->data, packet->length, false);

	msgStream << "Got ID_READY_EVENT_SET from " << packet->guid.ToString() << ": ";
	bs.IgnoreBytes(sizeof(RakNet::MessageID));
	bs.Read(eventID);

	switch(eventID)
	{
	case eReadyEvents::EVENT_READYSTART:
		msgStream << "ready to start" << std::endl;
		break;

	case eReadyEvents::EVENT_ENDTURN:
		msgStream << "end turn" << std::endl;
		break;

	case eReadyEvents::EVENT_ENDGAME:
		msgStream << "end game" << std::endl;
		break;

	default:
		msgStream << "unknown" << std::endl;
	}

	setMsgRak(msgStream.str());
}

void P2PClient::readyEventUnset(RakNet::Packet *packet)
{
	std::stringstream msgStream;
	eReadyEvents eventID;
	RakNet::BitStream bs(packet->data, packet->length, false);
	
	msgStream << "Got ID_READY_EVENT_UNSET from " << packet->guid.ToString() << ": ";
	bs.IgnoreBytes(sizeof(RakNet::MessageID));
	bs.Read(eventID);
	
	switch(eventID)
	{
	case eReadyEvents::EVENT_READYSTART:
		msgStream << "ready to start" <<std::endl;
		break;

	case eReadyEvents::EVENT_ENDTURN:
		msgStream << "end turn" <<std::endl;
		break;

	case eReadyEvents::EVENT_ENDGAME:
		msgStream << "end game" <<std::endl;
		break;

	default:
		msgStream << "unknown" <<std::endl;
	}

	setMsgRak(msgStream.str());
}

void P2PClient::readyEventAllSet(RakNet::Packet *packet)
{
	std::stringstream msgStream;
	eReadyEvents eventID;
	RakNet::BitStream bs(packet->data, packet->length, false);

	msgStream << "Got ID_READY_EVENT_ALL_SET from " << packet->guid.ToString() << ": ";
	bs.IgnoreBytes(sizeof(RakNet::MessageID));
	bs.Read(eventID);

	switch(eventID)
	{
	case eReadyEvents::EVENT_READYSTART:
		setState(GameState::state_turn);
		msgStream << "ready to start" << std::endl;
		break;

	case eReadyEvents::EVENT_ENDTURN:
		setState(GameState::state_turn);
		msgStream << "end turn" << std::endl;
		break;

	case eReadyEvents::EVENT_ENDGAME:
		setState(GameState::state_end);
		msgStream << "end game" << std::endl;
		break;

	default:
		msgStream << "unknown" << std::endl;
	}

	setMsg(msgStream.str());
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

	if(m_readyEventPlugin.HasEvent(static_cast<int>(eventID)))
	{
		++numPlayer;
	}

	numPlayer += m_readyEventPlugin.GetRemoteWaitListSize(static_cast<int>(eventID));

	return numPlayer;
}

int P2PClient::getNumPlayersReady(eReadyEvents eventID)
{
	int numReady = 0;
	RakNet::ReadyEventSystemStatus status;

	//check self
	if(m_readyEventPlugin.HasEvent(static_cast<int>(eventID)) && m_readyEventPlugin.IsEventSet(static_cast<int>(eventID)))
	{
		numReady++;
	}

	//check others
	DataStructures::List<RakNet::SystemAddress> addresses;
	DataStructures::List<RakNet::RakNetGUID> guids;
	m_peer->GetSystemList(addresses, guids);

	for(int i = 0; i < guids.Size(); ++i)
	{
		status = m_readyEventPlugin.GetReadyStatus(static_cast<int>(eventID), guids[i]);

		if(RakNet::RES_READY == status || RakNet::RES_ALL_READY == status )
		{
			numReady++;
		}
	}



	return numReady;
}


void P2PClient::setReadyEvent(eReadyEvents eventID, bool value)
{
	m_readyEventPlugin.SetEvent(static_cast<int>(eventID), value);
}


void P2PClient::setMsg(std::string msg)
{
	m_msg += msg;
	m_redisplay = true;
}

void P2PClient::clearMsg()
{
	m_msg = "";
}

void P2PClient::setMsgRak(std::string msg)
{
	m_msgRak += msg;
	m_redisplay = true;
}

void P2PClient::clearMsgRak()
{
	m_msgRak = "";
}

void P2PClient::setMsgChat(std::string msg)
{
	m_msgChat += msg;
	m_redisplay = true;
}

void P2PClient::clearMsgChat()
{
	m_msgChat = "";
}