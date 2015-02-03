
#include <vector>

namespace RakNetLabs
{
	class P2PClient;

	class GameState
	{
	public:
		static GameState *const state_connecting;
		static GameState *const state_lobby;
		static GameState *const state_turn;
		static GameState *const state_end;

		GameState();

		virtual void init(P2PClient * client);
		virtual void enter() = 0;
		virtual void display() = 0;
		virtual void input(char ch) = 0;

	protected:
		P2PClient * m_p2pclient;
		std::vector<GameState *> m_validStates;
	};

	class GameState_Connecting : public GameState
	{
	public:
		void enter();
		void display();
		void input(char ch);
	};

	class GameState_Lobby : public GameState
	{
	public:
		void enter();
		void display();
		void input(char ch);
	};

	class GameState_Turn : public GameState
	{
	public:
		void init(P2PClient * client);
		void enter();
		void display();
		void input(char ch);

		int getMaxTurns() const { return m_maxTurns; }
		int getTurns() const { return m_turnCounter; }

	private:
		int m_turnCounter;
		int m_maxTurns;
	};
	class GameState_End : public GameState
	{
	public:
		void enter();
		void display();
		void input(char ch);
	};
}