#ifndef _CLIENT_H
#define _CLIENT_H

#include <string>
#include <iostream>

#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"
#include "RakNetTypes.h"

namespace RakNetTutorial
{
	enum class ClientGameMessages
	{
		ID_GAME_MESSAGE_1=ID_USER_PACKET_ENUM+1,
		ID_GAME_MESSAGE_2
	};

	class Client
	{
	private:
		static const int DEFAULT_SERVER_PORT;

		bool m_listen;
		bool m_connected;
		std::string m_name;
		RakNet::RakPeerInterface * m_peer;
		RakNet::SystemAddress m_hostAddress;
		int m_serverPort;

	public:
		Client();
		~Client();
		
		void startup();
		void startup(int serverPort);
		void shutdown();
		
		void getMessages();
		void readPackets();
		void readPacket(RakNet::Packet *packet);

	private:
		void log(std::string msg);

		//void GetCusstomMessage();
		void ConnectionAccepted(RakNet::Packet *packet);
		void DisconnectMessage(RakNet::Packet *packet);
		void GameMessage1(RakNet::Packet *packet);
		void GameMessage2(RakNet::Packet *packet);

		void sendText(char * msg);
		void sendCommand(char * msg);

	};
}


#endif