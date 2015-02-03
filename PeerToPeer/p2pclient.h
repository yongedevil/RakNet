#ifndef _P2PCLIENT_H
#define _P2PCLIENT_H

#include <string>

#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "ReadyEvent.h"
#include "SocketLayer.h"
#include "FullyConnectedMesh2.h"
#include "ConnectionGraph2.h"

#include "BitStream.h"
#include "RakSleep.h"

namespace RakNetLabs
{
	enum class eReadyEvents
	{
		EVENT_READYSTART,
		EVENT_ENDTURN,
		EVENT_ENDGAME
	};

	enum class ClientGameMessages
	{
		ID_CHAT_MESSAGE=ID_USER_PACKET_ENUM+1,
		ID_START_GAME,
		ID_END_TURN,
		ID_END_GAME
	};

	class GameState;

	class P2PClient
	{
	private:
		static const int DEFAULT_PORT;
		static const char * DEFAULT_PORT_STR;
		static const int DEFAULT_MAX_CONNECTIONS;

	public:
		P2PClient();
		~P2PClient();

		void startup();
		void startup(int & serverPort);
		void shutdown();

		void play();
		void readPackets();

		int getNumPlayersWaiting(eReadyEvents eventID);
		int getNumPlayersReady(eReadyEvents eventID);
		
		void setState(GameState * state);

		void setReadyEvent(eReadyEvents eventID, bool value);
		void printConnections();

	private:		
		void readPacket(RakNet::Packet *packet);
		void connectionAccepted(RakNet::Packet *packet);
		void incomingConnection(RakNet::Packet *packet);
		void disconnectMessage(RakNet::Packet *packet);
		void readyEventSet(RakNet::Packet *packet);
		void readyEventUnset(RakNet::Packet *packet);
		void readyEventAllSet(RakNet::Packet *packet);
		void chatMessage(RakNet::Packet *packet);
		
		void sendText(char * msg);
		void sendCommand(char * msg);


	private:
		RakNet::RakPeerInterface * m_peer;
		RakNet::ReadyEvent m_readyEventPlugin;
		RakNet::FullyConnectedMesh2 m_fcm2;
		RakNet::ConnectionGraph2 m_cg2;

		GameState * m_curState;
		bool m_quit;
		bool m_redisplay;

		bool m_connected;
		int m_serverPort;
		std::string m_name;
		RakNet::SystemAddress m_hostAddress;
	};

}


#endif