#include "gameState.h"

#include <cstdlib>
#include <cstdio>

#include "p2pclient.h"

using namespace RakNetLabs;

GameState *const GameState::state_connecting = new GameState_Connecting();
GameState *const GameState::state_lobby = new GameState_Lobby();
GameState *const GameState::state_turn = new GameState_Turn();
GameState *const GameState::state_end = new GameState_End();


GameState::GameState() : 
	m_p2pclient(0)
{
}
void GameState::init(P2PClient * client)
{
	m_p2pclient = client;
}
	

void GameState_Connecting::enter()
{
}
void GameState_Connecting::display()
{
	printf("CONNECTING...\n\n");
}
void GameState_Connecting::input(char ch)
{
}


void GameState_Lobby::enter()
{
}
void GameState_Lobby::display()
{
	printf("LOBBY\n");
	printf("Awaiting players: %i of %i ready\n", m_p2pclient->getNumPlayersReady(eReadyEvents::EVENT_READYSTART), m_p2pclient->getNumPlayersWaiting(eReadyEvents::EVENT_READYSTART));
	printf("\t('r') to ready up\n");
	printf("\t('u') to unready\n\n");
}
void GameState_Lobby::input(char ch)
{
	switch(ch)
	{
	case 'r':
	case 'R':
		m_p2pclient->setReadyEvent(eReadyEvents::EVENT_READYSTART, true);
		break;

	case 'u':
	case 'U':
		m_p2pclient->setReadyEvent(eReadyEvents::EVENT_READYSTART, false);
		break;

	case 'p':
	case 'P':
		m_p2pclient->printConnections();
		break;
	}
}



void GameState_Turn::init(P2PClient * client)
{
	GameState::init(client);
	m_maxTurns = 2;
	m_turnCounter = 0;

}
void GameState_Turn::enter()
{
	m_turnCounter++;
	if(m_turnCounter > m_maxTurns)
	{
		m_p2pclient->setState(GameState::state_end);
	}
}
void GameState_Turn::display()
{
	printf("TURN: %i\n", m_turnCounter);
	printf("%i of %i players finished their turn\n", m_p2pclient->getNumPlayersReady(eReadyEvents::EVENT_ENDTURN), m_p2pclient->getNumPlayersWaiting(eReadyEvents::EVENT_ENDTURN));
	printf("\t('e') to end turn\n");
	printf("\t('q') to end game\n\n");
}
void GameState_Turn::input(char ch)
{
	switch(ch)
	{
	case 'e':
	case 'E':
		m_p2pclient->setReadyEvent(eReadyEvents::EVENT_ENDTURN, true);
		break;

	case 'q':
	case 'Q':
		m_p2pclient->setReadyEvent(eReadyEvents::EVENT_ENDGAME, true);
		break;
	}
}


void GameState_End::enter()
{
}
void GameState_End::display()
{
	printf("GAMEOVER\n");
}
void GameState_End::input(char ch)
{
}