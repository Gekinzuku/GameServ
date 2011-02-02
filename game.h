#include <string.h>
#include <vector>
#include <dlfcn.h>
#include "irc.h"
using namespace std;

enum { WEP_MELEE, WEP_RANGED, WEP_AMMO };

// Stores the ID and how many the player has of it
struct MessageStruct
{
	string Speaker;
	string Host;
	vector <string> StrSub; 
	bool Public;
};

struct ItemIDStruct
{
    unsigned int ID;
    short Amount;
};

// Actually holds the item info
struct ItemStruct
{
    string Name, Description;
    short Type;
    int Power, Strength, Requires;
    bool Use;
};

// Player
struct PlayerStruct
{
	string Name, Nick, Password, Host;
	int Level, Exp, Health, Gold, Race, Class;
	char Gender; // M = male, F = female
	short Equip[3], Current;
	vector <ItemIDStruct> Item;
};

// Races
struct RaceStruct
{
	string Name; // Name of the race
	float Burn, Critical, Energy, Freeze, Immunity, Magic, Money, Poison, Shock; // Stats
	bool OperOnly; // Locked races
};

// Classes
struct ClassStruct
{
	string Name; // Name of the race
	float Endurance, ExpRate, Luck, Magic, Speed, Stealth, Strength, Wit; // Stats
	bool OperOnly; // Locked classes
};

// The help struct
struct HelpStruct
{
  string Name;
  vector <string> Output;
};

class GameClass
{
public:
	GameClass();
	void MainLoop();
	void TrackUser(string OldNick, string NewNick);
	void UserQuit(string Nick);

	bool SaveToMySQL();
	bool LoadFromMySQL();
	void SaveToDatabase();
	bool LoadFromDatabase();
	bool LoadHelp();

	// Get various things
	string GetRaceFromName(string &Name);
	string GetNameFromNick(string &Nick);
	int GetIDFromNick(string &Name);
	int GetItemIDFromItemName(string &Name);
	int DoesPlayerHaveItem(int PlayerID, int ItemID);
	void SetChannel(string &Channel_) { Channel = Channel_; };
	string GetChannel() { return Channel; }

	// Relating to the current message
	void SetCurrMessage(string &speaker, string &host, vector <string> &StrSub, bool Public);
	string GetCurrSpeaker() { return CurrMessage.Speaker; }
	string GetCurrHost() { return CurrMessage.Host; }
	string GetCurrMessage(int Word) { return CurrMessage.StrSub[Word]; }
	unsigned int GetCurrMessageSize() { return CurrMessage.StrSub.size(); }
	bool GetCurrPublic() { return CurrMessage.Public; }

	// Get/set things relating to the player
	unsigned int GetPlayerSize() { return Player.size(); }
	PlayerStruct GetPlayer(int ID) { return Player[ID]; }
	void SetPlayerNick(int ID, string Nick) { Player[ID].Nick = Nick; }
	void SetPlayerHost(int ID, string Host) { Player[ID].Host = Host; }
	unsigned int FindPlayerByName(string Name) { Name = ConvertToLower(Name); for (unsigned int i = 0; i < Player.size(); ++i) { if (ConvertToLower(Player[i].Name) == Name) return i; } return -1; }
	unsigned int FindPlayerByNick(string Nick) { Nick = ConvertToLower(Nick); for (unsigned int i = 0; i < Player.size(); ++i) { if (ConvertToLower(Player[i].Nick) == Nick) return i; } return -1; }
	void AddPlayer(PlayerStruct &NewPlayer) { Player.push_back(NewPlayer); };
	void DeletePlayer(int ID) { Player.erase(Player.begin()+ID); }
	bool IsLoggedIn(int ID, string Nick, string Host) { if (ConvertToLower(Player[ID].Nick) == ConvertToLower(Nick) && Player[ID].Host == Host) return true; else return false; }

	// Get/set things relating to the race
	unsigned int GetRaceSize() { return Race.size(); }
	RaceStruct GetRace(int ID) { return Race[ID]; }
	unsigned int FindRaceByName(string Name) { Name = ConvertToLower(Name); for (unsigned int i = 0; i < Race.size(); ++i) { if (ConvertToLower(Race[i].Name) == Name) return i; } return -1; }

	// Get/set things relating to the class
	unsigned int GetClassSize() { return Class.size(); }
	ClassStruct GetClass(int ID) { return Class[ID]; }
	unsigned int FindClassByName(string Name) { Name = ConvertToLower(Name); for (unsigned int i = 0; i < Class.size(); ++i) { if (ConvertToLower(Class[i].Name) == Name) return i; } return -1; }

	void Help(string Command, string &Recipient, bool Incorrect); // Sends the user help about a command
private:

	void UseItem(int Player, int Item);
	void Equip(string &Type, string &Item, string &Speaker, string &Host);
	void SetUse(string &Type, string &Speaker, string &Host);
	bool CheckLevelUp(int Player);

	void ShowStats(string Name, string Recipient);
	void List(string Stat, string Recipient);
	void Attack(string &PlayerNick, string &Victim, string &Recpient, string &Host);

	void LogIn(string &PlayerName, string &PlayerNick, string &Password, string &Host);
	void LogOut(string &PlayerNick, string &Host);
	void Ghost(string &PlayerName, string &Password, string &Speaker);


	/**********************
	******* Variables *****
	**********************/
	string Channel;
	MessageStruct CurrMessage;

	vector <PlayerStruct> Player;
	vector <RaceStruct> Race;
	vector <ClassStruct> Class;
	vector <ItemStruct> Item;

    vector <HelpStruct> HelpObj;
};

extern GameClass Game;

#define ERROR_BADLOGIN 		"Sorry, you don't appear to be logged in."
#define ERROR_BADHOST		"Sorry, but your username and host don't match. Use either the \'ghost\' command or \'login\' again."
#define ERROR_NOTANITEM		"Sorry, that is not a valid item."
#define ERROR_DONOTHAVE 	"Sorry, but you do not have that item."
#define ERROR_WRONGTYPE 	"Sorry, but that item cannot be equiped to that slot." 
#define ERROR_ALREADYEQUIP	"That item is already equiped to that slot!"
#define ERROR_INVALIDTYPE	"That weapon type does not exist."

