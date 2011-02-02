#ifndef IRC_H
#define IRC_H
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <fstream>
using namespace std;

#include "defines.h"

#ifdef LINUX
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>
#elif defined WINDOWS
	#include <winsock2.h>
#endif

class CommandClass {
public:
	CommandClass(const string &trigger_, vector <string> output_);
	string trigger;
	vector <string> output;
};


class IRCClass {
public:
	IRCClass() { Port = 0; }

	// Sending shit
	void LoadSettings(string &Server_, int Port_, string &BotName_, string &BotRealName_, string &BotPassword_,
		string &BotVersion_, string &BotWhoAre_, string &AdminPassword_, string &Channel_, string &Admin);
	bool Init(); // Inits the socket, connects, etc
	void BotInit(); // Sets nick, user info, logs in

	void Quit(string Reason);
	void SendMsg(string MsgToSend, string SendTo);
	void SendAction(string MsgToSend, string SendTo);
	void SendNotice(string MsgToSend, string SendTo);
	void Nick(string NewName);
	void Join(string Channel);
	void RawMsg(string Msg);
	void Part(string Channel, string Reason);
	void Kick(string User, string Channel, string Reason);

	void Execute(int CMDNum, vector <string> &StrSub, string &Message, string UserName, string Channel);

	// Get message
	short int GetMsg(bool WaitFor); // Gets messages from the server, Waits till we have a full message before sending it to be processed
	void ProcessMsg(string Message); // Proccess the message

	// Stores the commands
	vector <CommandClass> CMDs;
	void UseLogs(bool Logs_) { Logs = Logs_; }

//private:
	int sockfd; // Socket, yay
	sockaddr_in serv_addr;
	hostent *server;

	// Settings pretty much
	string Server, BotName, BotRealName, BotPassword, BotVersion, BotWhoAre, AdminPassword, Admin, Channel;
	int Port;
	bool Logs;

//	ofstream LogOutput;

	//string BotName; // Stores the bot name, so it knows when someone is talking to it, etc
	string CurrentMsg; // Stores the current message
};

extern IRCClass IRCServer;

// Functions
void LoadSettingsFromFile(IRCClass &irc);
void RemoveSpaces(string &str, bool All);
void GetDataFromLine(string &line, string &Put, const char * check, unsigned int ClearSpaces);
void Error(string ErrorStr);
void FatalError(string ErrorStr);
string ConvertToLower(string Str);
#endif
