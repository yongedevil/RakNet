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
	enum class eReadyEvents : int
	{
		EVENT_ENDTURN,
		EVENT_ENDGAME
	};

	enum class ClientGameMessages
	{
		ID_GAME_MESSAGE_1=ID_USER_PACKET_ENUM+1,
		ID_GAME_MESSAGE_2
	};

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

		void getMessages();
		void readPackets();
		void readPacket(RakNet::Packet *packet);

	private:
		void connectionAccepted(RakNet::Packet *packet);
		void disconnectMessage(RakNet::Packet *packet);
		void readyEventSet(RakNet::Packet *packet);
		void readyEventUnset(RakNet::Packet *packet);
		void readyEventAllSet(RakNet::Packet *packet);
		void GameMessage1(RakNet::Packet *packet);
		void GameMessage2(RakNet::Packet *packet);

		void printConnections();
		
		void sendText(char * msg);
		void sendCommand(char * msg);

	private:
		RakNet::RakPeerInterface * m_peer;
		RakNet::ReadyEvent m_readyEventPlugin;
		RakNet::FullyConnectedMesh2 m_fcm2;
		RakNet::ConnectionGraph2 m_cg2;

		bool m_listen;
		bool m_connected;
		bool m_isHost;
		int m_serverPort;
		std::string m_name;
		RakNet::SystemAddress m_hostAddress;
	};

}


#endif