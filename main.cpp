/***********************************
** IRC bot coded by GeekyLink ( geekylink@gekinzuku.com )
** This mess of code is functional but... inefficient and all
** http://gekinzuku.com
************************************/
#include <iostream>
#include <cstdlib>
#include <time.h>
using namespace std;

#include "defines.h"

// Stores our IRCClass
#include "irc.h"

IRCClass IRCServer;

string ConvertToLower(string Str)
{
	for (unsigned int i = 0; i < Str.length(); ++i)
	{
		Str[i] = tolower(Str[i]);
	}
	return Str;
}

int main() // IRC bot, by GeekLink. Kinda shitty but who cares?
{
    srand(time(NULL));
	cout << "Loading settings..\n";


	LoadSettingsFromFile(IRCServer);

	// Starts up our bot.
	if (IRCServer.Init() == false) {
		cout << "Could not start the IRC bot...\n"; // Failure :(
		return 0;
	}
	IRCServer.BotInit(); // Sets the nick

	cout << "Made it to the main loop\n";
	while(1) {
		// Loop and loop forever
		if (IRCServer.GetMsg(false) == -1) cout << "There was an error in the message loop!";
	}

	// Like it'll ever get here anyway :|
#ifdef WINDOWS
	WSACleanup();
#endif
	return 0;
}

void LoadSettingsFromFile(IRCClass &irc)
{
	// Temp vars for holding data from the settings file
	string Server, BotName, BotRealName, BotPassword, BotVersion, BotWhoAre, cmdfile, Admin, AdminPassword, Channel;
	int Port = 0;
	bool Logs = false;

	// Loads the settings
	ifstream File("settings.conf", ios::in);
	while (!File.eof()) {
		string line;
		getline(File, line);
		if (line[0] == '#') continue; // Skip this line... it is a comment

		if (line.find("Server=") != string::npos) GetDataFromLine(line, Server, "Server=",1); // Server
		else if (line.find("Port=") != string::npos) Port = atoi(line.substr(line.find("Port=")+strlen("Port="),-1).c_str()); // Port
		else if (line.find("BotName=") != string::npos) GetDataFromLine(line, BotName, "BotName=",1); // Bot Name
		else if (line.find("BotRealName=") != string::npos) GetDataFromLine(line, BotRealName, "BotRealName=",2); // Bot Real Name
		else if (line.find("BotPassword=") != string::npos) GetDataFromLine(line, BotPassword, "BotPassword=",1); // Bot's password
		else if (line.find("BotVersion=") != string::npos) GetDataFromLine(line, BotVersion, "BotVersion=",2); // Bot's version
		else if (line.find("BotWhoAre=") != string::npos) GetDataFromLine(line, BotWhoAre, "BotWhoAre=",2); // Bot's whoare
		else if (line.find("AdminPassword=") != string::npos) GetDataFromLine(line, AdminPassword, "AdminPassword=",1); // Admin password
		else if (line.find("Channel=") != string::npos) GetDataFromLine(line, Channel, "Channel=",1); // Channel
		else if (line.find("Admin=") != string::npos) GetDataFromLine(line, Admin, "Admin=",1); // Channel
		else if (line.find("LogOn") != string::npos) Logs = true;
	}
	File.close(); // DONE!

	// Checks if anything important is missing
	if (Server == "") Error("Error: Missing Server=. Please fix your settings.conf file.");
	if (Port == 0) Error("Error: Missing Port=. Please fix your settings.conf file.");
	if (BotName == "") Error("Error: Missing BotName=. Please fix your settings.conf file.");
	if (BotRealName == "") Error("Error: Missing BotRealName=. Please fix your settings.conf file.");
	if (BotVersion == "") Error("Error: Missing BotVersion=. Please fix your settings.conf file.");
	if (BotWhoAre == "") Error("Error: Missing BotWhoAre=. Please fix your settings.conf file.");
	if (AdminPassword == "") Error("Error: Missing AdminPassword=. Please fix your settings.conf file.");
	if (Admin == "") Error("Error: Missing Admin=. Please fix your settings.conf file.");
	if (Channel == "") Error("Error: Missing Channel=. Please fix your settings.conf file.");

	// Lists the loaded settings
	cout << "Server: " << Server << "\n";
	cout << "Port: " << Port << "\n";
	cout << "BotName: " << BotName << "\n";
	cout << "BotName: " << BotRealName << "\n";
	if (BotPassword == "") cout << "Not using password.\n";
	else cout << "Using password.\n";

	if (!Logs) cout << "Not logging messages.\n";
	else cout << "Logging messages.\n";
	cout << "Bot Version: " << BotVersion << "\n";
	cout << "BotWhoAre: " << BotWhoAre << "\n";
	cout << "Channel: " << Channel << "\n\n";

	// Loads the settings
	irc.LoadSettings(Server, Port, BotName, BotRealName, BotPassword, BotVersion, BotWhoAre, AdminPassword, Channel, Admin);
	irc.UseLogs(Logs);
}

// Gets data out of the line
void GetDataFromLine(string &line, string &Put, const char * check, unsigned int ClearSpaces)
{
	Put = line.substr(line.find(check)+strlen(check),-1);
	// If ClearSpaces is 0 then we don't want to do this.
	if (ClearSpaces == 1)
		RemoveSpaces(Put, true);
	else if (ClearSpaces == 2)
		RemoveSpaces(Put, false);
}

// Removes spaces from a string
void RemoveSpaces(string &str, bool all)
{
	if (all) { // Get rid of all spaces
		while (str.find(" ") != string::npos) {
			str.replace(str.find(" "), 1, "");
		}
	}
	else { // Only the spaces at the front and end

	}
}

void Error(string ErrorStr)
{
	cout << ErrorStr << endl;
	exit(0);
}

void FatalError(string ErrorStr)
{
    cout << "************\nFATAL ERROR: " << ErrorStr << "\n************\n";
    stringstream Msg;
    Msg << "************ FATAL ERROR: " << ErrorStr << " ************";
    IRCServer.SendNotice(Msg.str(), IRCServer.Admin);
}
