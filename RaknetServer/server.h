#ifndef _SERVER_H
#define _SERVER_H

#include <string>

#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"
#include "RakNetTypes.h"


namespace RakNetTutorial
{
	enum class GameMessages
	{
		ID_GAME_MESSAGE_1=ID_USER_PACKET_ENUM+1,
		ID_GAME_MESSAGE_2
	};

	class Server
	{
	private:
		static const int DEFAULT_SERVER_PORT;
		static const int DEFAULT_MAX_CONNECTIONS;
		
		bool m_listen;
		RakNet::RakPeerInterface * m_peer;
		int m_serverPort;

	public:
		Server();
		~Server();

		void startup();
		void startup(int serverPort, int maxConnections);
		void shutdown();

		void readPackets();
		void readPacket(RakNet::Packet *packet);

	private:
		void log(std::string msg);

		void ConnectionAccepted(RakNet::Packet *packet);
		void GameMessage1(RakNet::Packet *packet);
		void GameMessage2(RakNet::Packet *packet);
	};
}


#endif