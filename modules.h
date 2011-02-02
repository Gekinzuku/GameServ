#include <iostream>
#include <vector>
#include <string>
using namespace std; 

typedef void* func_t(void);

struct TriggerS
{
	string Word;
	int Module;
};

class Module {
public:
	virtual void OnAnyMessage() {}
	virtual void OnMessage() {}
};

extern vector<Module*> ModuleList;

class ModulesClass {
public:
	bool LoadModule(string FileName_);
	bool UnloadModule(int ID);
	bool UnloadModule(string FileName_);
	void Register(string Trigger_, int ModID);

	int GetTriggerCount() { return Trigger.size(); }
	string GetTriggerWord(int ID) { return Trigger[ID].Word; }
	int GetTriggerModule(int ID) { return Trigger[ID].Module; }
private:
	vector <string> FileName;
	vector <void *> LibHandle; 
	vector <TriggerS> Trigger;
};

extern ModulesClass Modules;

#define INIT_MOD(y) \
	extern "C" Module * InitModule(int id) \
	{ \
		return new y(id); \
	}

#define STR_MOD_PATH "modules/"

