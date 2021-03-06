#include <time.h>
#include <cstdlib>
#include "irc.h"
#include "game.h"

CommandClass::CommandClass(const string &trigger_, vector <string> output_)
{
	trigger = trigger_;
	output = output_;
}

void convertToLower(string &Str)
{
	for (unsigned int i = 0; i < Str.length(); ++i)
	{
		Str[i] = tolower(Str[i]);
	}
}

// Changes certain things
void Interpret(string &Str, vector <string> &Msgs, string &line, unsigned int Offset) {
	string PrevString; // Used to check for no more edits
	do {
		PrevString = Str;
		if (Str.find("%s") != string::npos) { // String to add
			int FoundAt = Str.find("%s")+2;
			int Word = atoi(Str.substr(FoundAt, 1).c_str());
			if (Msgs.size() > Word+Offset) Str.replace(FoundAt-2, 3, Msgs[Word+Offset]); // Make sure there is enough parameters
			else Str.replace(0,-1,""); // Otherwise kill the message
		}
		else if (Str.find("%c") != string::npos) { // Check for channel
			int FoundAt = Str.find("%c");
			Str.replace(FoundAt, 2, Msgs[2]);
		}
		else if (Str.find("%u") != string::npos) { // Check for user
			int FoundAt = Str.find("%u");
			Str.replace(FoundAt, 2, Msgs[0].substr(1,Msgs[0].find("!")-1));
		}
		else if (Str.find("%l") != string::npos) { // Check for user
			int FoundAt = Str.find("%l");
			Str.replace(FoundAt, 2, line);
		}
	} while (PrevString != Str); // If nothing has been edited then we are done
}

// Just a padding thing to make my life easier
string Padding(string StrToPad, char Pad)
{
	string Str;
	Str.push_back(Pad);
	Str.append(StrToPad);
	Str.push_back(Pad);
	return Str;
}

// Loads the bot's settings
void IRCClass::LoadSettings(string &Server_, int Port_, string &BotName_, string &BotRealName_, string &BotPassword_,
							string &BotVersion_, string &BotWhoAre_, string &AdminPassword_, string &Channel_, string &Admin_)
{
	Server		= Server_;
	Port		= Port_;
	BotName		= BotName_;
	BotRealName = BotRealName_;
	BotPassword = BotPassword_;
	BotVersion	= BotVersion_;
	BotWhoAre	= BotWhoAre_;
	AdminPassword =AdminPassword_;
	Channel		= Channel_;
	Admin		= Admin_;
}

// Inits the socket and port
bool IRCClass::Init()
{
	// Gotta init Winsock for windows...
#ifdef WINDOWS
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

	if (iResult != NO_ERROR) {
		cout << "WSAStartup failure.\n";
		return false;
	}
    else
		cout << "WSAStartup success.\n";
#endif

	sockfd = socket(AF_INET, SOCK_STREAM, 0); // Gets a socket

	if (sockfd < 0) { // Socket error?
		cout << "ERROR: Socket could not open.\n";
		return false;
	} else cout << "Socket success.\n";

	server = gethostbyname(Server.c_str()); // Finds the host
	if (server == NULL) { // Couldn't access site?
		cout << "ERROR: No such host: " << Server << endl;
		return false;
	} else {
		cout << "Found host.\n";
	}

	// Gets some important info
	//bzero((char *) &serv_addr, sizeof(serv_addr));
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy ((char *)&serv_addr.sin_addr.s_addr, (char *) server->h_addr, sizeof(serv_addr.sin_addr.s_addr));
	serv_addr.sin_port = htons(Port);

	// Connects to the server and port
	if (connect(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		cout << "ERROR: Could not connect.\n";
		return false;
	} else {
		cout << "Connection successful.\nReady to begin...\n";
	}

//	u_long iMode=1;
//ioctlsocket(sockfd,FIONBIO,&iMode);

	return true; // We made it!
}

// Sets the Name, Real name, and logs in (if password is present)
void IRCClass::BotInit()
{
	//if (Logs) LogOutput.open("Logs.txt", ios::app); // Opens log if we are logging

	Nick(BotName); // Sets the nick

	// User info
	stringstream Msg;
	Msg << "USER " << BotName << " 8 * :" << BotRealName << "\n\r";
	RawMsg(Msg.str()); // Sends

	while (GetMsg(true) != 9) {}

	// Password
	Msg.str("");
	Msg << "PRIVMSG nickserv :identify " << BotPassword << "\n\r";
	RawMsg(Msg.str());

	Join(Channel);
}

// Quits the server
void IRCClass::Quit(string Reason)
{
	string Msg;
	Msg.append("QUIT :");
	Msg.append(Reason);
	Msg.append("\n\r");
	send(sockfd, Msg.c_str(), Msg.length(), 0);
}

// Sends a message
void IRCClass::SendMsg(string MsgToSend, string SendTo)
{
	string Msg;
	Msg.append("PRIVMSG ");
	Msg.append(SendTo);
	Msg.append(" :");
	Msg.append(MsgToSend);
	Msg.append("\n\r");
	int Bytes = send(sockfd, Msg.c_str(), Msg.length(), 0);
	cout << endl << "REPLY: (" << Bytes << ") " << Msg << endl;
}

// Sends an action
void IRCClass::SendAction(string MsgToSend, string SendTo)
{
	stringstream Msg;
	Msg << "PRIVMSG " << SendTo << " :" << char(0x001) << "ACTION " << MsgToSend << char(0x001) << "\n\r";
	RawMsg(Msg.str());
}

// Sends a notice
void IRCClass::SendNotice(string MsgToSend, string SendTo)
{
	stringstream Msg;
	Msg << "NOTICE " << SendTo << " :" << MsgToSend << "\n\r";
	RawMsg(Msg.str());
}

// Changes a nick
void IRCClass::Nick(string NewName)
{
	string Msg;
	Msg.append("NICK ");
	Msg.append(NewName);
	Msg.append("\n\r");
	RawMsg(Msg);
}

// Joins a channel
void IRCClass::Join(string Channel)
{
	string Msg;
	Msg.append("JOIN :");
	Msg.append(Channel);
	Msg.append("\n\r");
	send(sockfd, Msg.c_str(), Msg.length(), 0);
}

void IRCClass::RawMsg(string Msg)
{
	send(sockfd, Msg.c_str(), Msg.length(), 0);
	//if (Logs) LogOutput << Msg; // Logs
	//cout << endl << "REPLY: " << Msg << endl;
}

// Parts a channel
void IRCClass::Part(string Channel, string Reason)
{
	stringstream Msg;
	Msg << "PART " << Channel << " :" << Reason << "\n\r";
	RawMsg(Msg.str());
}

// Kicks
void IRCClass::Kick(string User, string Channel, string Reason)
{
	stringstream Msg;
	Msg << "KICK " << Channel << " " << User << " :" << Reason << "\n\r";
	RawMsg(Msg.str());
}

// Gets the message
short int IRCClass::GetMsg(bool WaitFor)
{
	string Message;
	int bytesRecv;
	char recvbuf[201];
	bytesRecv = recv(sockfd, recvbuf, 200, 0);

	if (bytesRecv < -1) // Error
        return -1;
	else if (bytesRecv == -1) // Nothing wrong but nothing here
		return 0;

	recvbuf[bytesRecv] = '\0';
	Message.append(recvbuf);

	// If there is no close just add to the current message...
	CurrentMsg.append(Message);

	// If we are waiting for the server to accept us, check for a welcome message
	if (WaitFor && Message.find("001") != string::npos) return 9;

	if (Message.find("\n") == string::npos) { // Not a full message. Wait for more info
		return 1;
	}
	else if (Message.find("\n") < Message.length()-1) // Sees if we picked up more than one message
	{
		do {
			//cout << CurrentMsg;
			ProcessMsg(CurrentMsg.substr(0, CurrentMsg.find("\n")+1));
			CurrentMsg = CurrentMsg.substr(CurrentMsg.find("\n")+1, -1);
		} while (CurrentMsg.find("\n") != string::npos);
	}
	else { // Otherwise there is a perfect message :D
		//cout << CurrentMsg;
		ProcessMsg(CurrentMsg);
		CurrentMsg = ""; // Clear the current message now that we are done
	}

	return 1;
}

// Actually handles our message
void IRCClass::ProcessMsg(string Message)
{
	cout << Message;
	// Logging a message
	//if (Logs) LogOutput << Message;

	vector <string> StrSub; // For storing our string
	string StrTemp; // Just a temp string which gets shorter...

	StrSub.push_back(Message.substr(0, Message.find(" "))); // Gets the first element
	StrTemp = Message; // Puts the message into the temp

	do { // Runs through and gets the rest of the strings
		StrTemp = StrTemp.substr(StrTemp.find(" ")+1, -1); // Breaks the temp string into a smaller piece
		if (StrTemp.substr(0, StrTemp.find(" ")).length() < 1) continue; // Don't want empty spaces!

		StrSub.push_back(StrTemp.substr(0, StrTemp.find(" "))); // Adds the next part to string storing thing

		//if (StrSub.size() > 7) break; // We don't care about shit any bigger than this
	} while (StrTemp.find(" ", StrTemp.find(" ")) != string::npos); // Checks if there is another space waiting for us!

	//cout << " " << StrSub.size() << " " << Message << "\n\n";

	// Gets rid of the trailing "\n\r"
	StrSub[StrSub.size()-1] = StrSub[StrSub.size()-1].substr(0, StrSub[StrSub.size()-1].length()-2);

	// Removes color codes from the message (0x003)
	for (unsigned int i = 0; i < StrSub.size(); i++) {
		while (StrSub[i].find(char(0x003)) != string::npos && StrSub[i].length() > 3) {
			StrSub[i] = StrSub[i].replace(StrSub[i].find(char(0x003)), 3, "");
		}
	}

	if (StrSub[0] == "PING") { // Pings
		stringstream Msg;
		Msg << "PONG " << StrSub[1] << "\n\r";
		RawMsg(Msg.str()); // Send!
		return;
	}
	//:GeekBot!GeekBot@GEK-C82E6EE5.direcpc.com JOIN :#gekinzuku
	else if (StrSub[1] == "JOIN" && StrSub[0].substr(1,StrSub[0].find("!")-1) != BotName) // Someone joined (Make sure it isn't the bot :P)
	{
		SendNotice("Please type /msg GameServ register [WarriorName] [Password] [Race] [Class] {Male|Female} to join the game.", StrSub[0].substr(1,StrSub[0].find("!")-1));
		return;
	}
	// Add a !commands to list all
	else if (StrSub[1] == "PRIVMSG") { // Some kind of message!
		StrSub[3].replace(0, 1, ""); // Gets rid of the preceding ":"

		// Version
		if (StrSub[3] == Padding("VERSION", 0x001)) { // Sends bot version
			stringstream StrVersion;
			StrVersion << "NOTICE " << StrSub[0].substr(1,StrSub[0].find("!")-1) << " :" << char(0x001) << "VERSION " << BotVersion << char(0x001) << "\n\r";
			RawMsg(StrVersion.str());
		}
		else if (StrSub[3] == "admin") // Admin actions
		{
			if (StrSub.size() > 5) { // Makes sure there are enough lines
				if (StrSub[4] == AdminPassword) { // Correct login
					if (StrSub[5] == "quit") {
						RawMsg("QUIT :Going down for maintence.\n\r");
						Game.SaveToDatabase();
						Game.SaveToMySQL();
					}
					else if (StrSub[5] == "reload") {
						SendNotice("Reloading database...", StrSub[0].substr(1,StrSub[0].find("!")-1));
#ifdef USE_MYSQL
						if (Game.LoadFromMySQL()) SendNotice("Loading complete!", StrSub[0].substr(1,StrSub[0].find("!")-1));
#else
						if (Game.LoadFromDatabase()) SendNotice("Loading complete!", StrSub[0].substr(1,StrSub[0].find("!")-1));
#endif
						else SendNotice("Loading failed!", StrSub[0].substr(1,StrSub[0].find("!")-1));
					}
					else if (StrSub[5] == "reloadtext") {
						SendNotice("Reloading database...", StrSub[0].substr(1,StrSub[0].find("!")-1));
						if (Game.LoadFromDatabase()) SendNotice("Loading complete!", StrSub[0].substr(1,StrSub[0].find("!")-1));
						else SendNotice("Loading failed!", StrSub[0].substr(1,StrSub[0].find("!")-1));
					}
					else if (StrSub[5] == "reloadmysql") {
#ifdef USE_MYSQL
						SendNotice("Reloading from MySQL database...", StrSub[0].substr(1,StrSub[0].find("!")-1));
						if (Game.LoadFromMySQL()) SendNotice("Loading complete!", StrSub[0].substr(1,StrSub[0].find("!")-1));
						else SendNotice("Loading failed!", StrSub[0].substr(1,StrSub[0].find("!")-1));
#else
						SendNotice("This version of GameServ was not built with MySQL support!", StrSub[0].substr(1,StrSub[0].find("!")-1));
#endif
					}
					else if (StrSub[5] == "save") {
						SendNotice("Saving to database...", StrSub[0].substr(1,StrSub[0].find("!")-1));
#ifdef USE_MYSQL
						Game.SaveToMySQL(); // Use MySQL
#else
						Game.SaveToDatabase(); // Don't
#endif
						SendNotice("Save successful!", StrSub[0].substr(1,StrSub[0].find("!")-1));
					}
					else if (StrSub[5] == "savetext") {
						SendNotice("Saving to database...", StrSub[0].substr(1,StrSub[0].find("!")-1));
						Game.SaveToDatabase();
						SendNotice("Save successful!", StrSub[0].substr(1,StrSub[0].find("!")-1));
					}
					else if (StrSub[5] == "savemysql") {
#ifdef USE_MYSQL
						SendNotice("Saving to MySQL database...", StrSub[0].substr(1,StrSub[0].find("!")-1));
						Game.SaveToMySQL();
						SendNotice("Save successful!", StrSub[0].substr(1,StrSub[0].find("!")-1));
#else
						SendNotice("This version of GameServ was not built with MySQL support!", StrSub[0].substr(1,StrSub[0].find("!")-1));
#endif
					}
					else if (StrSub[5] == "usechan") { // Join
						if (StrSub.size() > 6) {
							Part(Game.GetChannel(), "Assigned to a new channel.");
							Join(StrSub[6]);
							Game.SetChannel(StrSub[6]);
							SendNotice("Joined!", StrSub[0].substr(1,StrSub[0].find("!")-1));
						}
						else SendNotice("You didn't specify a channel!", StrSub[0].substr(1,StrSub[0].find("!")-1));
					}
				} // Bad login
				else SendNotice("Error: Invalid login.", StrSub[0].substr(1,StrSub[0].find("!")-1));
			}
			else {
				stringstream Str;
				SendNotice("Not enough words to use a command.", StrSub[0].substr(1,StrSub[0].find("!")-1));
			}
			return;
		}

		bool Public = true;
		if (ConvertToLower(StrSub[2]) == ConvertToLower(BotName)) Public = false;

		string Speaker = StrSub[0].substr(1,StrSub[0].find("!")-1);
		string Host = StrSub[0].substr(StrSub[0].find("@")+1);
		StrSub.erase(StrSub.begin(),StrSub.begin()+3);
		Game.SetCurrMessage(Speaker, Host, StrSub, Public);
		// Go to the main game
		Game.MainLoop();
	}
	else if (StrSub[1] == "NICK") { // Keeps track of the user's nick
	    if (StrSub[2][0] == ':') StrSub[2] = StrSub[2].substr(1); // Not all ircds have the ':' for NICK remove it if it is there

	    // Looks like our nick got changed!
	    if (StrSub[0].substr(1,StrSub[0].find("!")-1) == BotName) {
	         BotName = StrSub[2];
	         return;
	    }

		Game.TrackUser(StrSub[0].substr(1,StrSub[0].find("!")-1), StrSub[2]); // Track that nick!
	}
	else if (StrSub[1] == "QUIT" || StrSub[1] == "PART") { // The user left! Log them out
		Game.UserQuit(StrSub[0].substr(1,StrSub[0].find("!")-1));
	}
	else if (StrSub[1] == "401") { // Looks like the user left and we didn't know about it! Better log them out
		Game.UserQuit(StrSub[3]);
	}
}

// Executes commands, yay
void IRCClass::Execute(int CMDNum, vector <string> &StrSub, string &Message, string UserName, string Channel)
{
	int i = CMDNum;

	if (UserName == "") UserName = StrSub[0].substr(1,StrSub[0].find("!")-1);
	if (Channel == "")	Channel = StrSub[2];

	for (unsigned int MsgID = 0; MsgID < CMDs[i].output.size(); ++MsgID) {
		if (CMDs[i].output[MsgID].find("MSG") == 0) { // Standard message
			string Response = CMDs[i].output[MsgID].substr(4,-1);
			Interpret(Response, StrSub, Message, 2);

			if (StrSub[2] == BotName) SendMsg(Response, UserName); // Private message
			else SendMsg(Response, Channel); // Channel
		}
		else if (CMDs[i].output[MsgID].find("ACT") == 0) { // An action
			string Response = CMDs[i].output[MsgID].substr(4,-1);
			Interpret(Response, StrSub, Message, 2);

			if (StrSub[2] == BotName) SendAction(Response, UserName); // Private message
			else SendAction(Response, Channel); // Channel
		}
		else if (CMDs[i].output[MsgID].find("NOTE") == 0) { // A notice
			string Response = CMDs[i].output[MsgID].substr(5,-1);
			Interpret(Response, StrSub, Message, 2);
			SendNotice(Response, UserName); // Private message
		}
		else if (CMDs[i].output[MsgID].find("PART") == 0) { // A part
			string Response = CMDs[i].output[MsgID].substr(5,-1);
			Interpret(Response, StrSub, Message, 2);
			Part(Response.substr(0, Response.find(" ")), Response.substr(Response.find(" ")+1, -1));
		}
		else if (CMDs[i].output[MsgID].find("JOIN") == 0) { // A join
			string Response = CMDs[i].output[MsgID].substr(5,-1);
			Interpret(Response, StrSub, Message, 2);
			Join(Response.substr(0, Response.find(" ")));
		}
		else if (CMDs[i].output[MsgID].find("QUIT") == 0) { // A quit
			string Response = CMDs[i].output[MsgID].substr(5,-1);
			Interpret(Response, StrSub, Message, 2);
			Quit(Response);
		}
		else if (CMDs[i].output[MsgID].find("SLEEP") == 0) { // Sleeping
			string Response = CMDs[i].output[MsgID].substr(6,-1);
			//int TicksToSleep = atoi(Response.c_str());
			//Sleep(TicksToSleep);
		}
		else if (CMDs[i].output[MsgID].find("KICK") == 0) { // kick
			string Response = CMDs[i].output[MsgID].substr(5,-1);
			Interpret(Response, StrSub, Message, 2);
			Kick(Response.substr(0, Response.find(" ")), "#gekinzuku", Response.substr(Response.find(" ")+1, -1));
		}
	}
}
