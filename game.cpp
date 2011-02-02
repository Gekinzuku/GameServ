#include <sstream>
#include <fstream>
#include <cstdlib>
#include <math.h>
#ifdef USE_MYSQL
#include <mysql/mysql.h>
#endif
#include "game.h"
#include "modules.h"
GameClass Game;

// Constructor
GameClass::GameClass()
{ 
	Channel = "#gekinzuku"; 
#ifdef USE_MYSQL
	LoadFromMySQL(); 
#else
	LoadFromDatabase();
#endif
	LoadHelp();
}

// Saves everything to the MySQL Database
bool GameClass::SaveToMySQL()
{
#ifdef USE_MYSQL
	MYSQL conn;
	MYSQL_ROW Row;
	MYSQL_RES *Result;
	stringstream Query;

	mysql_init(&conn);

	// Connects
	if (!mysql_real_connect(&conn, "localhost", "GameServ", "3d656fd40f981de0b98b835c2c6ed772", "GameServ", 3306, NULL, 0)) {
		mysql_close(&conn);
		return false;
	}

	// Truncates all the data! :o
	if (mysql_query(&conn, "TRUNCATE Players") != 0 || mysql_query(&conn, "TRUNCATE Items") != 0 ||
	    mysql_query(&conn, "TRUNCATE Classes") != 0 || mysql_query(&conn, "TRUNCATE Races") != 0) {
		mysql_close(&conn);
		return false;
	}

	// Saves the players
	for (int ID = 0; ID < Player.size(); ++ID)
	{
		Query.str(""); // Get rid of previous query
		Query << "INSERT INTO Players (UserName, Password, Level, Exp, Gold, Health, Race, Class, Equip, Current, Gender, Items, Nick, Host) VALUES ";
		Query << "(\"" << Player[ID].Name << "\", \"" << Player[ID].Password << "\", " << Player[ID].Level << ", " << Player[ID].Exp << ", " << Player[ID].Gold << ", " << Player[ID].Health << ", " << Player[ID].Race << ", " << Player[ID].Class << ", \"";
		for (int i = 0; i < 3; ++i) Query << Player[ID].Equip[i] << ","; // Puts equiped items in
		Query << "\", " << Player[ID].Current << ", \"" << Player[ID].Gender << "\", \"";
		for (int i = 0; i < Player[ID].Item.size(); ++i) Query << Player[ID].Item[i].ID << " " << Player[ID].Item[i].Amount << ",";
		Query << "\", \"" << Player[ID].Nick << "\", \"" << Player[ID].Host << "\")";
		mysql_query(&conn, Query.str().c_str()); // Add the data!
	}

	// Saves the items
	for (int ID = 0; ID < Item.size(); ++ID)
	{
		Query.str(""); // Clears the query
		Query << "INSERT INTO Items (Name, Type, Requires, Power, Strength, UseUp, Description) VALUES ";
		Query << "(\"" << Item[ID].Name << "\", " << Item[ID].Type << ", " << Item[ID].Requires << ", " << Item[ID].Power << ", " << Item[ID].Strength << ", " << Item[ID].Use << ", \"" << Item[ID].Description << "\")";
		mysql_query(&conn, Query.str().c_str());
	}

	// Saves the Races
	for (int ID = 0; ID < Race.size(); ++ID)
	{
		Query.str("");
		Query << "INSERT INTO Races (Name, Burn, Critical, Energy, Freeze, Immunity, Magic, Money, Poison, Shock, OperOnly) VALUES ";
		Query << "(\"" << Race[ID].Name << "\", " << Race[ID].Burn << ", " << Race[ID].Critical << ", " << Race[ID].Energy << ", " << Race[ID].Freeze << ", " << Race[ID].Immunity << ", " << Race[ID].Magic << ", " << Race[ID].Money << ", " << Race[ID].Poison << ", " << Race[ID].Shock << ", " << Race[ID].OperOnly << ")";
		mysql_query(&conn, Query.str().c_str());
	}

	// Saves the Classes
	for (int ID = 0; ID < Class.size(); ++ID)
	{
		Query.str("");
		Query << "INSERT INTO Classes (Name, Endurance, ExpRate, Luck, Magic, Speed, Stealth, Strength, Wit, OperOnly)  VALUES ";
		Query << "(\"" << Class[ID].Name << "\", " << Class[ID].Endurance << ", " << Class[ID].ExpRate << ", " << Class[ID].Luck << ", " << Class[ID].Magic << ", " << Class[ID].Speed << ", " << Class[ID].Stealth << ", " << Class[ID].Strength << ", " << Class[ID].Wit << ", " << Class[ID].OperOnly << ")";
		mysql_query(&conn, Query.str().c_str());
	}

	mysql_close(&conn);
	return true;	
#else
	return false;
#endif
}

// Reloads everything from the MySQL database
bool GameClass::LoadFromMySQL()
{
#ifdef USE_MYSQL
	MYSQL conn;
	MYSQL_ROW Row;
	MYSQL_RES *Result;
	string TempStr;

	mysql_init(&conn);
	// Connects to the database
	if (!mysql_real_connect(&conn, "localhost", "GameServ", "3d656fd40f981de0b98b835c2c6ed772", "GameServ", 3306, NULL, 0)) {
		mysql_close(&conn);
		return false;
	}

	// Gets the players
	if (mysql_query(&conn, "SELECT * FROM Players") != 0) {
		mysql_close(&conn);
		return false;
	}
	Result = mysql_store_result(&conn);

	// Make sure there are the right number of fields!
	if (mysql_num_fields(Result) != 15) {
		mysql_close(&conn);
		return false;
	}
	Player.clear();
	PlayerStruct TempPlayer;
	ItemIDStruct TempItemID;
	while (Row = mysql_fetch_row(Result))
	{
		TempPlayer.Name = Row[1];
		TempPlayer.Password = Row[2];
		TempPlayer.Level = atoi(Row[3]);
		TempPlayer.Exp = atoi(Row[4]);
		TempPlayer.Gold = atoi(Row[5]);
		TempPlayer.Health = atoi(Row[6]);
		TempPlayer.Race = atoi(Row[7]);
		TempPlayer.Class = atoi(Row[8]);
		TempStr = Row[9];
		for (int EquipID = 0; TempStr.length() != 0; ++EquipID) {
			TempPlayer.Equip[EquipID] = atoi(TempStr.substr(0,TempStr.find(",")).c_str());
			TempStr = TempStr.substr(TempStr.find(",")+1);
		}
		TempPlayer.Current = atoi(Row[10]);
		TempPlayer.Gender = Row[11][0];

		TempStr = Row[12];
		TempPlayer.Item.clear();
		for (int ItemID = 0; TempStr.length() != 0; ++ItemID) {
			TempItemID.ID = atoi(TempStr.substr(0,TempStr.find(" ")).c_str()); TempStr = TempStr.substr(TempStr.find(" ")+1);
			TempItemID.Amount = atoi(TempStr.substr(0,TempStr.find(",")).c_str()); TempStr = TempStr.substr(TempStr.find(",")+1);
			TempPlayer.Item.push_back(TempItemID);
		}

		TempPlayer.Nick = Row[13];
		TempPlayer.Host = Row[14];
		Player.push_back(TempPlayer);
	}

	// Gets the Items
	if (mysql_query(&conn, "SELECT * FROM Items") != 0) {
		mysql_close(&conn);
		return false;
	}
	Result = mysql_store_result(&conn);

	// Make sure there are the right number of fields!
	if (mysql_num_fields(Result) != 8) {
		mysql_close(&conn);
		return false;
	}
	Item.clear();
	ItemStruct TempItem;
	while (Row = mysql_fetch_row(Result))
	{
		TempItem.Name = Row[1];
		TempItem.Type = atoi(Row[2]);
		TempItem.Requires = atoi(Row[3]);
		TempItem.Power = atoi(Row[4]);
		TempItem.Strength = atoi(Row[5]);
		TempItem.Use = atoi(Row[6]);
		TempItem.Description = Row[7];
		Item.push_back(TempItem);
	}

	// Gets the Classes
	if (mysql_query(&conn, "SELECT * FROM Classes") != 0) {
		mysql_close(&conn);
		return false;
	}
	Result = mysql_store_result(&conn);

	// Make sure there are the right number of fields!
	if (mysql_num_fields(Result) != 11) {
		mysql_close(&conn);
		return false;
	}
	Class.clear();
	ClassStruct TempClass;
	while (Row = mysql_fetch_row(Result))
	{
		TempClass.Name = Row[1];
		TempClass.Endurance = atof(Row[2]);
		TempClass.ExpRate = atof(Row[3]);
		TempClass.Luck = atof(Row[4]);
		TempClass.Magic = atof(Row[5]);
		TempClass.Speed = atof(Row[6]);
		TempClass.Stealth = atof(Row[7]);
		TempClass.Strength = atof(Row[8]);
		TempClass.Wit = atof(Row[9]);
		TempClass.OperOnly = atoi(Row[10]);
		Class.push_back(TempClass);
	}
	// Gets the Classes
	if (mysql_query(&conn, "SELECT * FROM Races") != 0) {
		mysql_close(&conn);	
		return false;
	}
	Result = mysql_store_result(&conn);

	// Make sure there are the right number of fields!
	if (mysql_num_fields(Result) != 12) {
		mysql_close(&conn);
		return false;
	}
	Race.clear();
	RaceStruct TempRace;
	while (Row = mysql_fetch_row(Result))
	{
		TempRace.Name = Row[1];
		TempRace.Burn = atof(Row[2]);
		TempRace.Critical = atof(Row[3]);
		TempRace.Energy = atof(Row[4]);
		TempRace.Freeze = atof(Row[5]);
		TempRace.Immunity = atof(Row[6]);
		TempRace.Magic = atof(Row[7]);
		TempRace.Money = atof(Row[8]);
		TempRace.Poison = atof(Row[9]);
		TempRace.Shock = atof(Row[10]);
		TempRace.OperOnly = atoi(Row[11]);
		Race.push_back(TempRace);
	}

	mysql_close(&conn);
	return LoadHelp();
#else
	return false;
#endif
}

// Saves the data to a file
void GameClass::SaveToDatabase()
{
	// Save the players
	ofstream File("data/players.db");
	for (unsigned int i = 0; i < Player.size(); ++i)
	{
		File << Player[i].Name << " " << Player[i].Password << " " << Player[i].Level << " " << Player[i].Exp << " " << Player[i].Gold << " " << Player[i].Health << " " << Player[i].Race << " " << Player[i].Class << " ";

		// Saves the equiped items
		File << Player[i].Equip[WEP_MELEE] << " " << Player[i].Equip[WEP_RANGED] << " " << Player[i].Equip[WEP_AMMO] << " ";

		File << Player[i].Current << " " << Player[i].Gender << "!";

		// Saves the items!
		for (unsigned int ItemNum = 0; ItemNum < Player[i].Item.size(); ++ItemNum) File << Player[i].Item[ItemNum].ID << " " << Player[i].Item[ItemNum].Amount << ",";

		// Saves if the user is logged in or not
		if (Player[i].Nick != "" && Player[i].Host != "") // User is logged in
           		File << Player[i].Nick << " " << Player[i].Host;
		File << "\n";
	}
	File.close();

	// Save the classes
	File.open("data/classes.db");
	for (unsigned int i = 0; i < Class.size(); ++i)
		File << Class[i].Name << " " << Class[i].Endurance << " " << Class[i].ExpRate << " " << Class[i].Luck << " " << Class[i].Magic << " " << Class[i].Speed << " " << Class[i].Stealth << " " << Class[i].Strength << " " << Class[i].Wit << " " << Class[i].OperOnly << "\n";
	File.close();

	// Save the races
	File.open("data/races.db");
	for (unsigned int i = 0; i < Race.size(); ++i)
		File << Race[i].Name << " " << Race[i].Burn << " " << Race[i].Critical << " " << Race[i].Energy << " " << Race[i].Freeze << " " << Race[i].Immunity << " " << Race[i].Magic << " " << Race[i].Money << " " << Race[i].Poison << " " << Race[i].Shock << " " << Race[i].OperOnly << "\n";
	File.close();

	// Save the item database
	File.open("data/items.db");
	for (unsigned int i = 0; i < Item.size(); ++i)
        File << Item[i].Name << "!" << Item[i].Type << " " << Item[i].Requires << " " << Item[i].Power << " " << Item[i].Strength << " " << Item[i].Use << " " << Item[i].Description << "\n";
	File.close();
}

// Loads the data to a file
bool GameClass::LoadFromDatabase()
{
	ifstream File("data/players.db");
	if (File.is_open()) // Make sure file is open
	{
		Player.clear(); // Delete our current records
		string Line; // Used for storing the data while reading from file

		// Get the number of lines
	        int Lines = 0;
		for (;!File.eof(); Lines++) getline(File, Line);
		File.clear(); File.seekg(0); // Starts file over
		Player.reserve(Lines); // Reserves the number of lines needed ahead of time

		PlayerStruct TempPlayer; // Temp player to hold the data
		ItemIDStruct ItemIdTemp;
		while (getline(File, Line)) { // Add the new records
			if (Line == "") break;
			TempPlayer.Name = Line.substr(0,Line.find(" ")); Line = Line.substr(Line.find(" ")+1);
			TempPlayer.Password = Line.substr(0,Line.find(" ")); Line = Line.substr(Line.find(" ")+1);
			TempPlayer.Level = atoi(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempPlayer.Exp = atoi(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempPlayer.Gold = atoi(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempPlayer.Health = atoi(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempPlayer.Race = atoi(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempPlayer.Class = atoi(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempPlayer.Equip[0] = atoi(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempPlayer.Equip[1] = atoi(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempPlayer.Equip[2] = atoi(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempPlayer.Current = atoi(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempPlayer.Gender = Line[0]; Line = Line.substr(Line.find("!")+1);

            // Gets the items
            TempPlayer.Item.clear();
            for (int i = 0; Line.find(",") != string::npos; ++i) {
                ItemIdTemp.ID = atoi(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
                ItemIdTemp.Amount = atoi(Line.substr(0,Line.find(",")).c_str()); Line = Line.substr(Line.find(",")+1);
                TempPlayer.Item.push_back(ItemIdTemp); // Push it back!
            }

            if (Line != "") // User is logged in!
			{
                TempPlayer.Nick = Line.substr(0,Line.find(" ")); Line = Line.substr(Line.find(" ")+1);
                TempPlayer.Host = Line.substr(0,Line.find("\n"));
			} // Not logged in
            else TempPlayer.Nick = TempPlayer.Host = "";

			Player.push_back(TempPlayer);
		}
	}
	else { // Failure? :(
		File.close();
		return false;
	}
	File.close();

	File.open("data/classes.db");
	if (File.is_open()) // Make sure file is open
	{
		Class.clear(); // Delete our current records
		string Line; // Used for storing the data while reading from file

		// Get the number of lines
        int Lines = 0;
		for (;!File.eof(); Lines++) getline(File, Line);
		File.clear(); File.seekg(0); // Starts file over
		Class.reserve(Lines); // Reserves the number of lines needed ahead of time

		ClassStruct TempClass; // Temp Class to hold the data
		while (getline(File, Line)) { // Add the new records
			if (Line == "") break;
			TempClass.Name = Line.substr(0,Line.find(" ")); Line = Line.substr(Line.find(" ")+1);
			TempClass.Endurance = atof(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempClass.ExpRate = atof(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempClass.Luck = atof(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempClass.Magic = atof(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempClass.Speed = atof(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempClass.Stealth = atof(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempClass.Strength = atof(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempClass.Wit = atof(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempClass.OperOnly = atoi(Line.substr(0,Line.find("\n")).c_str());
			Class.push_back(TempClass);
		}
	}
	else { // Failure? :(
		File.close();
		return false;
	}
	File.close();

	File.open("data/races.db");
	if (File.is_open()) // Make sure file is open
	{
		Race.clear(); // Delete our current records
		string Line; // Used for storing the data while reading from file

	        // Get the number of lines
        	int Lines = 0;
		for (;!File.eof(); Lines++) getline(File, Line);
		File.clear(); File.seekg(0); // Starts file over
		Race.reserve(Lines); // Reserves the number of lines needed ahead of time

		RaceStruct TempRace; // Temp Race to hold the data
		while (getline(File, Line)) { // Add the new records
			if (Line == "") break;
			TempRace.Name = Line.substr(0,Line.find(" ")); Line = Line.substr(Line.find(" ")+1);
			TempRace.Burn = atof(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempRace.Critical = atof(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempRace.Energy = atof(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempRace.Freeze = atof(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempRace.Immunity = atof(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempRace.Magic = atof(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempRace.Money = atof(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempRace.Poison = atof(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempRace.Shock = atof(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempRace.OperOnly = atoi(Line.substr(0,Line.find("\n")).c_str());
			Race.push_back(TempRace);
		}
	}
	else { // Failure? :(
		File.close();
		return false;
	}
	File.close();

    	// Time to load the items
	File.open("data/items.db");
	if (File.is_open())
	{
        Item.clear(); // Clear all previous help
        string Line;
        // Get the number of lines
        int Lines = 0;
		for (;!File.eof(); Lines++) getline(File, Line);
		File.clear(); File.seekg(0); // Starts file over
		Item.reserve(Lines); // Reserves the number of lines needed ahead of time

		ItemStruct TempItem; // Temp to push onto the item object once we have all the info we need
		while (getline(File, Line)) { // Add the new records
			if (Line == "") break;
			TempItem.Name = Line.substr(0,Line.find("!")); Line = Line.substr(Line.find("!")+1);
            		TempItem.Type = atoi(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempItem.Requires = atoi(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempItem.Power = atoi(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
            		TempItem.Strength = atoi(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
            		TempItem.Use = atoi(Line.substr(0,Line.find(" ")).c_str()); Line = Line.substr(Line.find(" ")+1);
			TempItem.Description = Line.substr(0,Line.find("\n"));
			Item.push_back(TempItem);
		}
	}
    	else {
        	File.close();
	        return false;
	}
	File.close();

	return LoadHelp();
}

// Loads the help database
bool GameClass::LoadHelp()
{
	ifstream File;
	// Time to load the help!
	File.open("data/help.db");
	if (File.is_open())
	{
        	HelpObj.clear(); // Clear all previous help
	        string Line;

        	HelpStruct HelpTemp; // Temp to push onto the help object once we have all the info we need
	        while (getline(File,Line)) {
        	    if (Line.find("help=") != string::npos) { // Looking for the start of help command
                	HelpTemp.Output.clear(); // Clear out the old one!
	                GetDataFromLine(Line, HelpTemp.Name, "help=",2); // Get the name
        	        HelpTemp.Name = ConvertToLower(HelpTemp.Name); // Covert it to lowercase to save us time in the actual program!
                	getline(File,Line);
	                while (Line != "endhelp") { // Check for the end of the help!
        	            if (Line != "") HelpTemp.Output.push_back(Line);
                	    if(!getline(File,Line)) { // If the user omitted a 'endhelp'
                        	File.close();
	                        return false; // error
        	            }
                	}
	                HelpObj.push_back(HelpTemp); // Push the data onto the actual vector!
        	    }
	        }
	}
    	else {
        	File.close();
        	return false;
    	}
    	File.close();	
}

// Gets race from nick
string GameClass::GetRaceFromName(string &Name)
{
    for (unsigned int i = 0; i < Player.size(); ++i)
    {
        if (ConvertToLower(Player[i].Name) == ConvertToLower(Name)) return Race[Player[i].Race].Name;
    }

    return "";
}

// Gets the ID from the nick
int GameClass::GetIDFromNick(string &Nick)
{
    for (unsigned int i = 0; i < Player.size(); ++i)
    {
        if (ConvertToLower(Player[i].Nick) == ConvertToLower(Nick)) return i;
    }
    return -1;
}

// Gets the player's name from nick
string GameClass::GetNameFromNick(string &Nick)
{
    for (unsigned int i = 0; i < Player.size(); ++i)
    {
        if (ConvertToLower(Player[i].Nick) == ConvertToLower(Nick)) return Player[i].Name;
    }
    return "";
}

// Gets an item's ID from it's name
int GameClass::GetItemIDFromItemName(string &Name)
{
	for (unsigned int i = 0; i < Item.size(); ++i)
	{
		if (ConvertToLower(Item[i].Name) == ConvertToLower(Name)) return i;
	}
	return -1;
}

// Returns the item slot if a player has the item, otherwise returns -1
int GameClass::DoesPlayerHaveItem(int PlayerID, int ItemID)
{
	for (unsigned int i = 0; i < Player[PlayerID].Item.size(); ++i)
	{
		if (Player[PlayerID].Item[i].ID == ItemID) return i;
	}
	return -1;
}

// Sends the Name
void GameClass::ShowStats(string Name, string Recipient)
{
	for (unsigned int i = 0; i < Player.size(); ++i)
	{
		if (ConvertToLower(Player[i].Name) == ConvertToLower(Name)) {
			stringstream Msg;
			Msg << Player[i].Name << " (" << (Player[i].Gender == 'M' ? "Male" : "Female") << ") the " << Race[Player[i].Race].Name << " " << Class[Player[i].Class].Name << " is level " << Player[i].Level << " and has " << Player[i].Exp << " EXP. Health: " << Player[i].Health << ". GekinMonies: " << Player[i].Gold;
			IRCServer.SendNotice(Msg.str(), Recipient);
			return;
		}
	}

	// Not found!
	IRCServer.SendNotice("No warrior with that name found.", Recipient);
}

// Uses up an item
void GameClass::UseItem(int PlayerID, int ItemID)
{
    	// Avoid seg faults
    	if (PlayerID > Player.size()-1) {
        	FatalError("A playerID is larger than the vector in UseItem()!");
        	return;
    	}
    	if (ItemID > Item.size()-1) {
        	FatalError("A ItemID is larger than the vector in UseItem()!");
        	return;
	}

	int ID = Player[PlayerID].Item[ItemID].ID;
	int Amount = Player[PlayerID].Item[ItemID].Amount;

	// Are we using a ranged weapon?
	if (Player[PlayerID].Equip[WEP_RANGED] == ItemID) {
		int AmmoID = Player[PlayerID].Equip[WEP_AMMO];
		// Is the ammo supposed to get used up?
		if (Item[Player[PlayerID].Item[AmmoID].ID].Use) {
        		if (Player[PlayerID].Item[AmmoID].Amount > 1) { // If we have more than 1 just subtract one
            			--Player[PlayerID].Item[AmmoID].Amount;
            			cout << "Subtracting one from the amount list.\n";
	        	}
        		else { // Otherwise we gotta delete the item and unequip it!
				Player[PlayerID].Equip[WEP_AMMO] = -1;
				Player[PlayerID].Current = -1;
            			Player[PlayerID].Item.erase(Player[PlayerID].Item.begin()+AmmoID);
            			cout << "Deleting the item!\n";
	        	}
    		}
	}

	// Is the item supposed to get used up?
	if (Item[ID].Use) {
        	if (Amount > 1) { // If we have more than 1 just subtract one
            		--Player[PlayerID].Item[ItemID].Amount;
            		cout << "Subtracting one from the amount list.\n";
        	}
        	else { // Otherwise we gotta delete the item and unequip it!
            		if (Player[PlayerID].Equip[WEP_MELEE] == ItemID)
                		Player[PlayerID].Equip[WEP_MELEE] = -1;
			else if (Player[PlayerID].Equip[WEP_RANGED] == ItemID)
				Player[PlayerID].Equip[WEP_RANGED] = Player[PlayerID].Equip[WEP_AMMO] = -1;
			Player[PlayerID].Current = -1;
            		Player[PlayerID].Item.erase(Player[PlayerID].Item.begin()+ItemID);
            		cout << "Deleting the item!\n";
        	}
    	}
}

// Lists things
void GameClass::List(string Stat, string Recipient)
{
    if (ConvertToLower(Stat) == "races") { // Shows all the races
        stringstream Msg;
        Msg << "Races: ";
        for (unsigned int i = 0; i < Race.size(); ++i)
        {
			Msg << Race[i].Name;
			if (i != Race.size()-1) Msg << ", ";
        }
        IRCServer.SendNotice(Msg.str(), Recipient);
		return;
    }
    else if (ConvertToLower(Stat) == "classes") { // Shows all the classes
        stringstream Msg;
        Msg << "Classes: ";
        for (unsigned int i = 0; i < Class.size(); ++i)
        {
			Msg << Class[i].Name;
			if (i != Class.size()-1) Msg << ", ";
        }
        IRCServer.SendNotice(Msg.str(), Recipient);
		return;
    }
    else if (ConvertToLower(Stat) == "usersonline") { // Shows who's online
        stringstream Msg;
        Msg << "Users Online: ";
        bool FirstName = true;
        for (unsigned int i = 0; i < Player.size(); ++i)
        {   // Don't show users who aren't online
            if (Player[i].Nick == "") continue;

            if (FirstName) {
                Msg << Player[i].Name;
                FirstName = false;
            }
			else Msg << ", " << Player[i].Name;
        }
        IRCServer.SendNotice(Msg.str(), Recipient);
		return;
    }
    else if (ConvertToLower(Stat) == "items") { // Shows items
        stringstream Msg;
        Msg << "Items: ";
        for (unsigned int i = 0; i < Item.size(); ++i)
        {
            Msg << Item[i].Name;
			if (i != Item.size()-1) Msg << ", ";
        }
        IRCServer.SendNotice(Msg.str(), Channel);
		return;
    }
    else if (ConvertToLower(Stat) == "myitems") { // Shows what we have
        stringstream Msg;
        Msg << "You have: ";

        int PlayerNum = GetIDFromNick(Recipient);
        if (PlayerNum == -1) {
            stringstream Msg;
            Msg << "Sorry, but you don't seem to be logged in!";
            IRCServer.SendNotice(Msg.str(), Recipient);
            return;
        }

        for (unsigned int i = 0; i < Player[PlayerNum].Item.size(); ++i)
        {
            Msg << Item[Player[PlayerNum].Item[i].ID].Name << " (" << Player[PlayerNum].Item[i].Amount << ")";
			if (i != Player[PlayerNum].Item.size()-1) Msg << ", ";
        }
        // If this player has nothing
        if (Player[PlayerNum].Item.size() == 0) Msg << "nothing.";

        IRCServer.SendNotice(Msg.str(), Recipient);
		return;
    }
    else if (ConvertToLower(Stat) == "equip")
    {
        stringstream Msg;
        int ID = GetIDFromNick(Recipient);
        if (ID == -1) return;
	Msg << "You are using your ";

	if (Player[ID].Current == WEP_MELEE) Msg << "melee weapon";
	else if (Player[ID].Current == WEP_RANGED) Msg << "ranged weapon";
	else if (Player[ID].Current == -1) Msg << "none";
	Msg << ", and you have equiped: ";

        if (Player[ID].Equip[WEP_MELEE] != -1) Msg << " Melee: " << Item[Player[ID].Item[Player[ID].Equip[WEP_MELEE]].ID].Name;
        if (Player[ID].Equip[WEP_RANGED] != -1) Msg << " Ranged: " << Item[Player[ID].Item[Player[ID].Equip[WEP_RANGED]].ID].Name;
        if (Player[ID].Equip[WEP_AMMO] != -1) Msg << " Ammo: " << Item[Player[ID].Item[Player[ID].Equip[WEP_AMMO]].ID].Name;
        if (Msg.str() == "") Msg << "You have nothing! :D:D:D:D:D:D:D:D";
        IRCServer.SendNotice(Msg.str(), Recipient);
        return;
    }

    // Not found!
    stringstream Msg;
    Msg << "Unable to list '" << Stat << "'.";
	IRCServer.SendNotice(Msg.str(), Recipient);
}

// Attacking
void GameClass::Attack(string &PlayerNick, string &Victim, string &Recipient, string &Host)
{
	// Sees if Player actually exists
	int PlayerNum = -1, VictimNum = -1;
	for (unsigned int i = 0; i < Player.size(); ++i)
	{
		if (ConvertToLower(Player[i].Nick) == ConvertToLower(PlayerNick)) PlayerNum = i;
		if (ConvertToLower(Player[i].Name) == ConvertToLower(Victim)) VictimNum = i;
	}

	// No player with this name?
	if (PlayerNum == -1) {
		IRCServer.SendNotice(ERROR_BADLOGIN, Recipient);
		return;
	}

	// No victim with that name?
	if (VictimNum == -1) {
		IRCServer.SendNotice("There is no warrior with that name.", Recipient);
		return;
	}

	// Attacking themseleves?
	if (PlayerNum == VictimNum) {
		IRCServer.SendNotice("You can't attack yourself!", Recipient);
		return;
	}

	// We know we have a proper PlayerNum and VictimNum. But we need to verify the user is who they say they are still!
	if (Player[PlayerNum].Host != Host)
	{
		IRCServer.SendNotice(ERROR_BADHOST, Recipient);
		return;
	}

	// Is this player dead?
	if (Player[PlayerNum].Health == 0) {
        	IRCServer.SendNotice("You are dead!", Recipient);
        	return;
	}

	// Is the enemy dead?
	if (Player[VictimNum].Health == 0) {
		stringstream Msg;
        	Msg << Player[VictimNum].Name << " is already dead!";
        	IRCServer.SendNotice(Msg.str(), Recipient);
        	return;
	}

	// Chance of hitting
    if (rand()%1500 + (100*Class[Player[PlayerNum].Class].Speed*2) < (100*Class[Player[VictimNum].Class].Luck*0.75) + (100*Class[Player[VictimNum].Class].Speed*3)) {
        stringstream Msg;
        Msg << Player[VictimNum].Name << " dodged the attack!";
        IRCServer.SendNotice(Msg.str(), Recipient);
        if (Player[VictimNum].Nick != "") {
            Msg.str("");
            Msg << Player[PlayerNum].Name << " tried to attack you!";
            IRCServer.SendNotice(Msg.str(), Player[VictimNum].Nick);
        }
        return;
    }

	// Weapon power
	int PlayerWep = 0, VictimWep = 0;
	// Melee weapons
	if (Player[PlayerNum].Current == WEP_MELEE && Player[PlayerNum].Equip[WEP_MELEE] != -1) {
        	PlayerWep = Item[Player[PlayerNum].Item[Player[PlayerNum].Equip[WEP_MELEE]].ID].Power;
		UseItem(PlayerNum, Player[PlayerNum].Equip[WEP_MELEE]);
	} // Victim
	if (Player[VictimNum].Current == WEP_MELEE && Player[VictimNum].Equip[WEP_MELEE] != -1) {
        	VictimWep = Item[Player[VictimNum].Item[Player[VictimNum].Equip[WEP_MELEE]].ID].Power;
        	UseItem(VictimNum, Player[VictimNum].Equip[WEP_MELEE]);
	}

	// Ranged weapons	
	if (Player[PlayerNum].Current == WEP_RANGED && Player[PlayerNum].Equip[WEP_RANGED] != -1 && Player[PlayerNum].Equip[WEP_AMMO] != -1) {
        	PlayerWep = Item[Player[PlayerNum].Item[Player[PlayerNum].Equip[WEP_RANGED]].ID].Power + Item[Player[PlayerNum].Item[Player[PlayerNum].Equip[WEP_AMMO]].ID].Power;
		UseItem(PlayerNum, Player[PlayerNum].Equip[WEP_RANGED]);
	} // Victim
	if (Player[VictimNum].Current == WEP_RANGED && Player[VictimNum].Equip[WEP_RANGED] != -1 && Player[VictimNum].Equip[WEP_AMMO] != -1) {
        	VictimWep = Item[Player[VictimNum].Item[Player[VictimNum].Equip[WEP_RANGED]].ID].Power + Item[Player[VictimNum].Item[Player[VictimNum].Equip[WEP_AMMO]].ID].Power;
		UseItem(VictimNum, Player[VictimNum].Equip[WEP_RANGED]);
	}

    // How much damage is dealt to each player
	float DamageV = rand()%4+PlayerWep + 1, DamageP = rand()%3+VictimWep + 1;

    // Adds the damage modifiers
    DamageV += (Player[PlayerNum].Level * Class[Player[PlayerNum].Class].Wit * (Class[Player[PlayerNum].Class].Strength*2))*0.3 - (Player[VictimNum].Level * Class[Player[VictimNum].Class].Wit * (Class[Player[VictimNum].Class].Endurance*2))*0.1;
    DamageP += (Player[VictimNum].Level * Class[Player[VictimNum].Class].Wit * (Class[Player[VictimNum].Class].Strength*2))*0.3 - (Player[PlayerNum].Level * Class[Player[PlayerNum].Class].Wit * (Class[Player[PlayerNum].Class].Endurance*2))*0.1;

    // Determines chance of critical hit
	if (rand()%1500 < (100*Race[Player[PlayerNum].Race].Critical) + (100*Class[Player[PlayerNum].Class].Wit*0.5)) DamageV *= 1.5;

    // We don't want negative or '0' damage
    if (DamageV < 1) DamageV = 1; if (DamageP < 1) DamageP = 1;

    // We don't want negative health!
    if (DamageV > Player[VictimNum].Health) DamageV = Player[VictimNum].Health;
    if (DamageP > Player[PlayerNum].Health) DamageP = Player[PlayerNum].Health;

    // Actually subtracts the health
	Player[VictimNum].Health -= round(DamageV);
	Player[PlayerNum].Health -= round(DamageP);

    // Round the damage cause all that's left is the stats
    DamageP = round(DamageP); DamageV = round(DamageV);

	stringstream MsgP, MsgV;

    MsgV << "You were attacked by " << Player[PlayerNum].Name << " for " << DamageV << " damage, and you countered for " << DamageP << " damage";
    MsgP << "You attacked " << Player[VictimNum].Name << " for " << DamageV << " damage, and received " << DamageP << " damage";

	if (Player[PlayerNum].Health > 0) {
		// The attacker always gets cash unless they died in the prcoess! Amount they get depends on how well they fought.
		int Ratio = 1;
		if (Player[PlayerNum].Level > Player[VictimNum].Level) Ratio = (Player[PlayerNum].Level-Player[VictimNum].Level);
		if (Ratio < 1) Ratio = 1;
		if (Ratio > 10) Ratio = 10;

		int Gold = (Player[VictimNum].Gold)*((Class[Player[PlayerNum].Class].Stealth*Ratio)/50)+ rand()%3;
		if (Gold < 1) Gold = 1;
		Player[PlayerNum].Gold += Gold;
		Player[VictimNum].Gold -= Gold;
		
	    	// Exp - (Race.ExpRate * 23 * EnemyLevel)/7
        	int Exp = (Class[Player[PlayerNum].Class].ExpRate * 23 * Player[VictimNum].Level)/7;
		Player[PlayerNum].Exp += Exp;
		if (Exp < 1) Exp = 1; // Always give at least one EXP!
		MsgP << " gaining " << Exp << " EXP and stealing " << Gold << " GekinMonies.";
		MsgV << " You lost " << Gold << " GekinMonies in the fight";
		if (CheckLevelUp(PlayerNum)) MsgP << " You grew to level " << Player[PlayerNum].Level << "!";
	}
	else {
		MsgV << ". " << Player[PlayerNum].Name << " died in combat.";
		MsgP << ". You died in combat.";

		// If the attack died and the victim didn't the victim gets a boost of gold!
		if (Player[VictimNum].Health > 0) {
			int Ratio = 1;
			if (Player[VictimNum].Level > Player[PlayerNum].Level) Ratio = (Player[VictimNum].Level-Player[PlayerNum].Level);
			if (Ratio < 1) Ratio = 1;
			if (Ratio > 10) Ratio = 10;

			int Gold = (Player[PlayerNum].Gold)*((Class[Player[VictimNum].Class].Stealth*Ratio)/47)+ rand()%4;
			if (Gold < 1) Gold = 1;
			Player[VictimNum].Gold += Gold;
			Player[PlayerNum].Gold -= Gold;
			MsgV << " You looted " << Gold << " GekinMonies as well";
			MsgP << " You lost " << Gold << " GekinMonies!";
		}
	}

	if (Player[VictimNum].Health > 0) {
	    	// Exp - (Race.ExpRate * 23 * EnemyLevel)/7
		int Exp = (Class[Player[VictimNum].Class].ExpRate * 23 * Player[PlayerNum].Level)/21;
	    	Player[VictimNum].Exp += Exp;
		if (Exp < 1) Exp = 1; // Always give at least one EXP!
	        MsgV << " and you gained " << Exp << " EXP.";
        	if (CheckLevelUp(VictimNum)) MsgV << " You grew to level " << Player[VictimNum].Level << "!";
	}
	else {
        	MsgV << " and you were slain!";
		MsgP << " " << Player[VictimNum].Name << " has been slain.";
	}

	// Sends the player's the info of the battle
	IRCServer.SendNotice(MsgP.str(), Player[PlayerNum].Nick);
	if (Player[VictimNum].Nick != "") IRCServer.SendNotice(MsgV.str(), Player[VictimNum].Nick);
}

// Logs the user out regardless of their host
void GameClass::Ghost(string &PlayerName, string &Password, string &Speaker)
{
// Sees if Player actually exists
	int PlayerNum = -1;
	for (unsigned int i = 0; i < Player.size(); ++i)
	{
		if (ConvertToLower(Player[i].Name) == ConvertToLower(PlayerName) && Player[i].Password == Password) PlayerNum = i;
	}

	// No player with this name?
	if (PlayerNum == -1) {
		IRCServer.SendNotice("Your ghosting credentials are invalid.", Speaker);
		return;
	}

	// We can logout!
	Player[PlayerNum].Nick = Player[PlayerNum].Host = "";
	IRCServer.SendNotice("You have successfully logged out.", Speaker);
}

// Logs in, if they have the right password!
void GameClass::LogIn(string &PlayerName, string &PlayerNick, string &Password, string &Host)
{
	// Makes sure we aren't already logged in as someone else
	for (unsigned int i = 0; i < Player.size(); ++i)
	{
		if (ConvertToLower(Player[i].Nick) == ConvertToLower(PlayerNick)) {
		    // Breaks if the names match
		    if (ConvertToLower(PlayerName) == ConvertToLower(Player[i].Name)) break;
			stringstream Msg;
			Msg << "Error: You are already logged in as " << Player[i].Name << ". If you want to login as " << PlayerName << " instead, please /msg GameServ logout first.";
			IRCServer.SendNotice(Msg.str(), PlayerNick);
			return;
		}
	}

	// Loop through the players
	for (unsigned int i = 0; i < Player.size(); ++i)
	{
		// Successful login?
		if (ConvertToLower(Player[i].Name) == ConvertToLower(PlayerName) && Player[i].Password == Password) {
			Player[i].Host = Host;
			Player[i].Nick = PlayerNick;
			stringstream Msg;
			Msg << "You are now logged in as " << Player[i].Name;
			IRCServer.SendNotice(Msg.str(), PlayerNick);
			ShowStats(Player[i].Name, PlayerNick);
			return;
		}
	}
	IRCServer.SendNotice("Your login credentials are invalid.", PlayerNick);
}

// Logs out the player. (Assuming they are logged in!)
void GameClass::LogOut(string &PlayerNick, string &Host)
{
	// Sees if Player actually exists
	int PlayerNum = -1;
	for (unsigned int i = 0; i < Player.size(); ++i)
	{
		if (ConvertToLower(Player[i].Nick) == ConvertToLower(PlayerNick)) PlayerNum = i;
	}

	// No player with this name?
	if (PlayerNum == -1) {
		IRCServer.SendNotice(ERROR_BADLOGIN, PlayerNick);
		return;
	}

	// We know we have a proper PlayerNum and VictimNum. But we need to verify the user is who they say they are still!
	if (Player[PlayerNum].Host != Host)
	{
		IRCServer.SendNotice(ERROR_BADHOST, PlayerNick);
		return;
	}

	// We can logout!
	Player[PlayerNum].Nick = Player[PlayerNum].Host = "";
	IRCServer.SendNotice("You have successfully logged out.", PlayerNick);
}

// Levels up a player if they have enough EXP!
bool GameClass::CheckLevelUp(int PlayerID)
{
    int PrevLvl = Player[PlayerID].Level;
    int Exp = Player[PlayerID].Exp;
    int NextLevel = (2 * pow((Player[PlayerID].Level+2), 3))/3;
    do {
        if (NextLevel < Exp) ++Player[PlayerID].Level;
        NextLevel = (2 * pow((Player[PlayerID].Level+2), 3))/3;
    } while (NextLevel < Exp);

    if (PrevLvl < Player[PlayerID].Level) return true;

    return false;
}

// Updates the player's nick
void GameClass::TrackUser(string OldNick, string NewNick)
{
	int PlayerNum = GetIDFromNick(OldNick);
	if (PlayerNum == -1) return; // Person isn't playing game

	// Update the nick
	Player[PlayerNum].Nick = NewNick;
}

// Logs a user out if they left
void GameClass::UserQuit(string Nick)
{
	int PlayerNum = GetIDFromNick(Nick);
	if (PlayerNum == -1) return; // Person isn't playing game

	// Log them out
	Player[PlayerNum].Nick = Player[PlayerNum].Host = "";
}

// Equips an item
void GameClass::Equip(string &Type, string &ItemName, string &Speaker, string &Host)
{
	int PlayerID = GetIDFromNick(Speaker);
	if (PlayerID == -1) {
        	IRCServer.SendNotice(ERROR_BADLOGIN, Speaker);
        	return;
	}
	if (Player[PlayerID].Host != Host) {
		IRCServer.SendNotice(ERROR_BADHOST,Speaker);
		return;
	}

	int ItemID = GetItemIDFromItemName(ItemName); // Gets ID

	// Not an item?
	if (ItemID == -1) {
		IRCServer.SendNotice(ERROR_NOTANITEM, Speaker);
		return;
	}

	int SlotID = DoesPlayerHaveItem(PlayerID, ItemID);
	
	// Hey, they don't have that item!
	if (SlotID == -1) {
		IRCServer.SendNotice(ERROR_DONOTHAVE, Speaker);
		return;
	}

	// Equip a melee item
	if (ConvertToLower(Type) == "melee") { // Melee weapons
		if (Item[Player[PlayerID].Item[SlotID].ID].Type != WEP_MELEE) {
			IRCServer.SendNotice(ERROR_WRONGTYPE, Speaker);
			return;
		}
		Player[PlayerID].Equip[WEP_MELEE] = SlotID;
        	stringstream Msg;
        	Msg << "You are now using a " << Item[ItemID].Name << " as a melee weapon.";
        	IRCServer.SendNotice(Msg.str(), Speaker);
    	}
	else if (ConvertToLower(Type) == "ranged") { // Ranged items
		if (Item[Player[PlayerID].Item[SlotID].ID].Type != WEP_RANGED) {
			IRCServer.SendNotice(ERROR_WRONGTYPE, Speaker);
			return;
		}
		Player[PlayerID].Equip[WEP_RANGED] = SlotID;
        	stringstream Msg;
        	Msg << "You are now using a " << Item[ItemID].Name << " as a ranged weapon.";
        	IRCServer.SendNotice(Msg.str(), Speaker);
    	}
	else if (ConvertToLower(Type) == "ammo") {
		if (Item[Player[PlayerID].Item[SlotID].ID].Type != WEP_AMMO) {
			IRCServer.SendNotice(ERROR_WRONGTYPE, Speaker);
			return;
		}
		Player[PlayerID].Equip[WEP_AMMO] = SlotID;
        	stringstream Msg;
        	Msg << "You are now using a " << Item[ItemID].Name << " as ammo.";
        	IRCServer.SendNotice(Msg.str(), Speaker);
    	}
    	else IRCServer.SendNotice(ERROR_INVALIDTYPE, Speaker);
}

// Which weapon of our equiped ones do we want to actually use?
void GameClass::SetUse(string &Type, string &Speaker, string &Host)
{
	int PlayerID = GetIDFromNick(Speaker);
	if (PlayerID == -1) {
        	IRCServer.SendNotice(ERROR_BADLOGIN, Speaker);
        	return;
	}
	if (Player[PlayerID].Host != Host) {
		IRCServer.SendNotice(ERROR_BADHOST,Speaker);
		return;
	}

	if (Type == "melee") {
		if (Player[PlayerID].Equip[WEP_MELEE] == -1) {
			IRCServer.SendNotice("You don't have a melee weapon equiped!", Speaker);
			return;
		}
		Player[PlayerID].Current = WEP_MELEE;
		IRCServer.SendNotice("You are now using your melee weapon.", Speaker);
	}
	else if (Type == "ranged") {
		if (Player[PlayerID].Equip[WEP_RANGED] == -1) {
			IRCServer.SendNotice("You don't have a ranged weapon equiped!", Speaker);
			return;
		}
		if (Player[PlayerID].Equip[WEP_AMMO] == -1) {
			IRCServer.SendNotice("You don't have any ammo for your ranged weapon.", Speaker);
			return;
		}
		Player[PlayerID].Current = WEP_RANGED;
		IRCServer.SendNotice("You are now using your ranged weapon.", Speaker);
	}
	else if (Type == "none") {
		Player[PlayerID].Current = -1;
		IRCServer.SendNotice("You are now using just your hands.", Speaker);
	}
	else IRCServer.SendNotice(ERROR_INVALIDTYPE, Speaker);
}

// Sets the current message
void GameClass::SetCurrMessage(string &speaker, string &host, vector <string> &StrSub, bool Public)
{
	CurrMessage.Speaker = speaker;
	CurrMessage.Host = host;	
	CurrMessage.Public = Public;
	CurrMessage.StrSub = StrSub;
}

// Regular game loop
//void GameClass::MainLoop(string &Speaker, vector <string> &StrSub, string &Host, bool Public)
void GameClass::MainLoop()
{
	// This really should be changed
	string Speaker = CurrMessage.Speaker;
	string Host = CurrMessage.Host;
	vector <string> StrSub = CurrMessage.StrSub;
	bool Public = CurrMessage.Public;
	/* If public == true then the message is on the channel
	** If public == false then the message was privately sent to the gamebot
	** StrSub is a vector of strings containing each word the Speaker said.
	** Example: register username mypass
	** StrSub[0] = register, StrSub[1] = username, StrSub[2] = mypass
	*/

	// Everything must be private for now
	if (Public) return;

	cout << Modules.GetTriggerCount() << endl;
	for (int i = 0; i < Modules.GetTriggerCount(); ++i) {
		if (Modules.GetTriggerWord(i) == ConvertToLower(StrSub[0])) {
			cout << "Trigger called: " << Modules.GetTriggerWord(i) << endl;
			ModuleList[Modules.GetTriggerModule(i)]->OnMessage();
			return;
		} 
	}

	// Joins
	if (ConvertToLower(StrSub[0]) == "loadmodule") // Stats
	{
		if (StrSub.size() >= 2) {
			Modules.LoadModule(StrSub[1]);
		}
	}
	else if (ConvertToLower(StrSub[0]) == "unloadmodule") // Stats
	{
		if (StrSub.size() >= 2) {
			Modules.UnloadModule(StrSub[1]);
		}
	}
	else if (ConvertToLower(StrSub[0]) == "stats") // Stats
	{
		if (StrSub.size() >= 2) {
			ShowStats(StrSub[1], Speaker);
		}
		else {
			ShowStats(GetNameFromNick(Speaker), Speaker);
		}
	}
	else if (ConvertToLower(StrSub[0]) == "attack") // Attack
	{
		if (StrSub.size() >= 2) {
			Attack(Speaker, StrSub[1], Speaker, Host);
		}
		else Help("attack", Speaker, true);
	}
	else if (ConvertToLower(StrSub[0]) == "list") // Lists everything in a group
	{
		if (StrSub.size() >= 2) {
			List(StrSub[1], Speaker);
		}
		else Help("list", Speaker, true);
	}
	else if (ConvertToLower(StrSub[0]) == "equip")
	{
		// We want the full item name even if it has spaces!
		string ItemName = "";
		for (int i = 2; i < StrSub.size(); ++i)
		{
			ItemName += StrSub[i];
			if (i < StrSub.size()-1) ItemName += " ";
		}
		if (StrSub.size() >= 3) Equip(StrSub[1], ItemName, Speaker, Host);
		else Help("equip", Speaker, true);
	}
	else if (ConvertToLower(StrSub[0]) == "use")
	{
		if (StrSub.size() >= 2) {
			StrSub[1] = ConvertToLower(StrSub[1]);
			SetUse(StrSub[1], Speaker, Host);	
		}
		else Help("use", Speaker, true);
	}
	else if (ConvertToLower(StrSub[0]) == "heal")
	{
		int PlayerNum = GetIDFromNick(Speaker);
		Player[PlayerNum].Health = 50;
	}
	else if (ConvertToLower(StrSub[0]) == "help")
	{
		if (StrSub.size() >= 2) Help(StrSub[1], Speaker, false);
		else Help("help", Speaker, false);
	}
	else { // Otherwise they fail
	    IRCServer.SendNotice("Sorry, but that command is not recognized.", Speaker);
	}
}

void GameClass::Help(string Command, string &Recipient, bool Incorrect)
{
    // Loop through all possible help commands
    bool GotHelp = false;
	for (unsigned int HelpNum = 0; HelpNum < HelpObj.size(); ++HelpNum)
	{
	    if (HelpObj[HelpNum].Name == ConvertToLower(Command)) {
            for (unsigned int CmdNum = 0; CmdNum < HelpObj[HelpNum].Output.size(); ++CmdNum) {
                if(HelpObj[HelpNum].Output[CmdNum][0] == '!') {
                    if (!Incorrect) IRCServer.SendNotice(HelpObj[HelpNum].Output[CmdNum].substr(1), Recipient);
                }
                else IRCServer.SendNotice(HelpObj[HelpNum].Output[CmdNum], Recipient);
            }
            GotHelp = true;
	    }
	}

	if (!GotHelp) { // User asked for something that doesn't exist!
		stringstream Msg;
		Msg << "Sorry, no help seems to be available for '" << Command << "'.";
		IRCServer.SendNotice(Msg.str(), Recipient);
		return;
	}

	// Let them know about help in case they need it!
	if (Incorrect) {
		stringstream Msg;
		Msg << "For more help use: /msg GameServ help " << Command;
		IRCServer.SendNotice(Msg.str(), Recipient);
	}
}

