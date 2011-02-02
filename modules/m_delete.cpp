#include <iostream>
#include <string>
#include "/home/jamie/Programming/C++/GameBot/irc.h"
#include "/home/jamie/Programming/C++/GameBot/game.h"
#include "/home/jamie/Programming/C++/GameBot/modules.h"

using namespace std;

class ModDelete : public Module
{
public:
	ModDelete(int ID);
	void OnMessage();
};

/********************************************************************
*This module is responsible for letting users delete their accounts.*
********************************************************************/

// Init
ModDelete::ModDelete(int ID)
{
	// Don't forget to register triggers, otherwise nothing will happen!
	Modules.Register("delete", ID);
}

// The entry point of the program, this gets called whenever one of the triggers is activated
void ModDelete::OnMessage()
{
	string PlayerNick = Game.GetCurrSpeaker();
	string PlayerHost = Game.GetCurrHost();
	string Command = ConvertToLower(Game.GetCurrMessage(0)); // Determine which trigger was hit

	if (Game.GetCurrMessageSize() < 3) {
		Game.Help("delete", PlayerNick, true); 
		return;
	}
	// We know that the trigger is "delete" since it was the only one we registered
	string PlayerName = Game.GetCurrMessage(1);	
	string Password = Game.GetCurrMessage(2);

	int ID = Game.FindPlayerByName(PlayerName);
	if (ID == -1 || !Game.IsLoggedIn(ID, PlayerNick, PlayerHost)) {
		IRCServer.SendNotice("You don't appear to be logged in as the user you want to delete.", PlayerNick);
		return;
	}

	Game.DeletePlayer(ID);
	IRCServer.SendNotice("Your player has successfully been deleted.", PlayerNick);
}

INIT_MOD(ModDelete)

