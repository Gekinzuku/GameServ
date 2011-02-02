#include <iostream>
#include <string>
#include "/home/jamie/Programming/C++/GameBot/irc.h"
#include "/home/jamie/Programming/C++/GameBot/game.h"
#include "/home/jamie/Programming/C++/GameBot/modules.h"

using namespace std;

class ModRegister : public Module
{
public:
	ModRegister(int ID);
	void OnMessage();
};

/****************************************************************
*This module is responsible for letting users make new accounts.*
****************************************************************/

// Init
ModRegister::ModRegister(int ID)
{
	// Don't forget to register triggers, otherwise nothing will happen!
	Modules.Register("register", ID);
}

// The entry point of the program, this gets called whenever one of the triggers is activated
void ModRegister::OnMessage()
{
	string PlayerNick = Game.GetCurrSpeaker();
	string Command = ConvertToLower(Game.GetCurrMessage(0)); // Determine which trigger was hit

	if (Game.GetCurrMessageSize() < 6) {
		Game.Help("register", PlayerNick, true); 
		return;
	}
	// We know that the trigger is "register" since it was the only one we registered
	string PlayerName = Game.GetCurrMessage(1);	
	string Password = Game.GetCurrMessage(2);
	string RaceName = Game.GetCurrMessage(3);	
	string ClassName = Game.GetCurrMessage(4);	
	string Gender = Game.GetCurrMessage(5);	

	// Makes sure we aren't already logged in as someone else
	int ID = Game.FindPlayerByNick(PlayerNick);
	if (ID != -1) {
		IRCServer.SendNotice("Sorry, but you are already logged in as someone else. Please logout or ghost first.", PlayerNick);
		return;
	}

	// Make sure the name isn't too long
	if (PlayerName.length() > 18 || PlayerName.length() < 3) {
		IRCServer.SendNotice("Sorry, but your warrior's name must be between 3 and 20 characters", PlayerNick);
		return;
	}

	// Make sure our password isn't too short or long!
	if (Password.length() < 5 || Password.length() > 20) {
		IRCServer.SendNotice("Sorry, but your password must be between five and twenty characters long.", PlayerNick);
		return;
	}

	// Makes sure this is a valid name with no bad characters
	for (unsigned int i = 0; i < PlayerName.length(); ++i)
	{
		if (!isalnum(PlayerName[i])) {
			stringstream Msg;
			Msg << "Sorry, but " << PlayerName << " is not a valid name. Please only use alphanumeric letters.";
			IRCServer.SendNotice(Msg.str(), PlayerNick);
			return;
		}
	}

	// Checks if that name is already registered
	ID = Game.FindPlayerByName(PlayerName);
	// Is this nick already registered?
	if (ID != -1) {
		stringstream Msg;
		Msg << "Sorry, but " << PlayerName << " is already registered.";
		IRCServer.SendNotice(Msg.str(), PlayerNick);
		return;
	}

	// Checks for a valid gender
	char CGen;
	if (ConvertToLower(Gender) == "male") CGen = 'M';
	else if (ConvertToLower(Gender) == "female") CGen = 'F';
	else {
	    stringstream Msg;
		Msg << "Sorry, but '" << Gender << "' is not a valid gender. Please use either Male or Female.";
		IRCServer.SendNotice(Msg.str(), PlayerNick);
		return;
	}

	// Finds the ID of the race
	int RaceID = Game.FindRaceByName(RaceName);

	// Checks if the race exists!
	if (RaceID == -1) {
	    stringstream Msg;
		Msg << "Sorry, but '" << RaceName << "' is not a valid race.";
		IRCServer.SendNotice(Msg.str(), PlayerNick);
		return;
	}

	// Make sure it isn't oper only
	if (Game.GetRace(RaceID).OperOnly) {
	    stringstream Msg;
		Msg << "Sorry, but '" << Game.GetRace(RaceID).Name << "' is a restricted race. This means you cannot become a member of it unless an oper puts you in it.";
		IRCServer.SendNotice(Msg.str(), PlayerNick);
		return;
	}

	// Finds the ID of the class
	int ClassID = Game.FindClassByName(ClassName);

	// Checks if the class exists!
	if (ClassID == -1) {
	    stringstream Msg;
		Msg << "Sorry, but '" << ClassName << "' is not a valid class.";
		IRCServer.SendNotice(Msg.str(), PlayerNick);
		return;
	}

	// Make sure it isn't oper only
	if (Game.GetClass(ClassID).OperOnly) {
	    stringstream Msg;
		Msg << "Sorry, but '" << Game.GetClass(ClassID).Name << "' is a restricted class. This means you cannot become a member of it unless an oper puts you in it.";
		IRCServer.SendNotice(Msg.str(), PlayerNick);
		return;
	}

	// It's alright to register this name!
	PlayerStruct NewPlayer;
	NewPlayer.Name = PlayerName; NewPlayer.Nick = PlayerNick; NewPlayer.Password = Password;
	NewPlayer.Level = 1;	
	NewPlayer.Gold = NewPlayer.Health = 30;
	NewPlayer.Exp = 0;
	NewPlayer.Gender = CGen;
	NewPlayer.Race = RaceID;
	NewPlayer.Class = ClassID;
	NewPlayer.Host = Game.GetCurrHost();
	NewPlayer.Current = NewPlayer.Equip[0] = NewPlayer.Equip[1] = NewPlayer.Equip[2] = -1;
	Game.AddPlayer(NewPlayer);
	stringstream Msg;
	Msg << "Thank you for registering! Your warrior, " << PlayerName << ", is now ready to fight!";
	IRCServer.SendNotice(Msg.str(), PlayerNick);
}

INIT_MOD(ModRegister)

