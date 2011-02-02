#include <iostream>
#include <string>
#include "/home/jamie/Programming/C++/GameBot/irc.h"
#include "/home/jamie/Programming/C++/GameBot/game.h"
#include "/home/jamie/Programming/C++/GameBot/modules.h"

using namespace std;

class ModLogin : public Module
{
public:
	ModLogin(int ID);
	void OnMessage();
};

/**********************************************************************
*This module is responsible for logging in, logging out, and ghosting.*
**********************************************************************/

// Functions
void LogIn();	// Lets the user login
void LogOut();	// Lets the user logout
void Ghost();	// Lets the user log someone else out who is using their player

// Init
ModLogin::ModLogin(int ID)
{
	// Don't forget to register triggers, otherwise nothing will happen!
	Modules.Register("login", ID);
	Modules.Register("logout", ID);
	Modules.Register("ghost", ID);
}

// The entry point of the program, this gets called whenever one of the triggers is activated
void ModLogin::OnMessage()
{
	string PlayerNick = Game.GetCurrSpeaker();
	string Command = ConvertToLower(Game.GetCurrMessage(0)); // Determine which trigger was hit
	if (Command == "login") {
		if (Game.GetCurrMessageSize() < 3) Game.Help("login", PlayerNick, true);   
		else LogIn();
	}
	else if (Command == "logout") LogOut();
	else if (Command == "ghost") {
		if (Game.GetCurrMessageSize() < 3) Game.Help("ghost", PlayerNick, true);   
		else Ghost();
		
	}
}

// Logs the user in
void LogIn()
{
	string PlayerNick = Game.GetCurrSpeaker();
	string Host = Game.GetCurrHost();
	string PlayerName = Game.GetCurrMessage(1);
	string Password = Game.GetCurrMessage(2);

	// Makes sure we aren't already logged in as someone else
	for (unsigned int i = 0; i < Game.GetPlayerSize(); ++i)
	{
		PlayerStruct Player = Game.GetPlayer(i);
		if (ConvertToLower(Player.Nick) == ConvertToLower(PlayerNick)) {
		    // Breaks if the names match
		    if (ConvertToLower(PlayerName) == ConvertToLower(Player.Name)) break;
			stringstream Msg;
			Msg << "Error: You are already logged in as " << Player.Name << ". If you want to login as " << PlayerName << " instead, please /msg GameServ logout first.";
			IRCServer.SendNotice(Msg.str(), PlayerNick);
			return;
		}
	}

	// Loop through the players
	for (unsigned int i = 0; i < Game.GetPlayerSize(); ++i)
	{
		PlayerStruct Player = Game.GetPlayer(i);
		// Successful login?
		if (ConvertToLower(Player.Name) == ConvertToLower(PlayerName) && Player.Password == Password) {
			Game.SetPlayerNick(i, PlayerNick);
			Game.SetPlayerHost(i, Host);
			stringstream Msg;
			Msg << "You are now logged in as " << Player.Name << ".";
			IRCServer.SendNotice(Msg.str(), PlayerNick);
			//ShowStats(Player[i].Name, PlayerNick);
			return;
		}
	}
	IRCServer.SendNotice("Your login credentials are invalid.", PlayerNick);
}

// Logs the player out
void LogOut()
{
	string PlayerNick = Game.GetCurrSpeaker();
	string Host = Game.GetCurrHost();

	// Sees if Player actually exists
	int PlayerNum = -1;
	for (unsigned int i = 0; i < Game.GetPlayerSize(); ++i)
	{
		if (ConvertToLower(Game.GetPlayer(i).Nick) == ConvertToLower(PlayerNick)) PlayerNum = i;
	}

	// No player with this name?
	if (PlayerNum == -1) {
		IRCServer.SendNotice(ERROR_BADLOGIN, PlayerNick);
		return;
	}

	PlayerStruct Player = Game.GetPlayer(PlayerNum);

	// We know we have a proper PlayerNum, but we need to verify the user is who they say they are still!
	if (Player.Host != Host)
	{
		IRCServer.SendNotice(ERROR_BADHOST, PlayerNick);
		return;
	}

	// We can logout!
	Game.SetPlayerNick(PlayerNum, "");
	Game.SetPlayerHost(PlayerNum, "");
	IRCServer.SendNotice("You have successfully logged out.", PlayerNick);
}

// Logs out someone else (if we know the password!)
// This is useful if for some reason GameServ thinks we are logged in but our hosts don't match
void Ghost()
{
	string PlayerNick = Game.GetCurrSpeaker();
	string PlayerName = Game.GetCurrMessage(1);
	string Password = Game.GetCurrMessage(2);
	// Sees if Player actually exists
	int PlayerNum = -1;
	for (unsigned int i = 0; i < Game.GetPlayerSize(); ++i)
	{
		PlayerStruct Player = Game.GetPlayer(i);
		if (ConvertToLower(Player.Name) == ConvertToLower(PlayerName) && Player.Password == Password) PlayerNum = i;
	}

	// No player with this name?
	if (PlayerNum == -1) {
		IRCServer.SendNotice("Your ghosting credentials are invalid.", PlayerNick);
		return;
	}

	// We can logout!
	Game.SetPlayerNick(PlayerNum, "");
	Game.SetPlayerHost(PlayerNum, "");
	IRCServer.SendNotice("You have successfully logged out.", PlayerNick);
}

INIT_MOD(ModLogin)

