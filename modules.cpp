#include "modules.h"
#include "irc.h"
#include "game.h"

ModulesClass Modules;
vector<Module*> ModuleList;

bool ModulesClass::LoadModule(string FileName_)
{
	FileName_.insert(0, STR_MOD_PATH);
	if (FileName.size() > 0) {
		for (int i = 0; i < FileName.size(); ++i) {
			if (FileName_ == FileName[i]) {
				cout << "This module is already loaded.\n";
				return false;
			}
		}
	}

	LibHandle.push_back(dlopen(FileName_.c_str(), RTLD_LAZY));
	if (!LibHandle[LibHandle.size()-1])
	{
		IRCServer.SendNotice(dlerror(), "GeekyLink");
		LibHandle.pop_back();
		return false;
	}	
	// This runs the Init(); function
	typedef void* InitFunc(int);
	InitFunc * InitPoint = (InitFunc*) dlsym(LibHandle[LibHandle.size()-1], "InitModule");
	if (!InitPoint) {
		IRCServer.SendNotice(dlerror(), "GeekyLink");
		LibHandle.pop_back();
		return false;
	}
	
	ModuleList.push_back((Module *) (*InitPoint)(ModuleList.size()));
	FileName.push_back(FileName_);

	return true;
}

// Unload the module, and remove its vector entries
bool ModulesClass::UnloadModule(int ID)
{
	dlclose(LibHandle[ID]);
	LibHandle.erase(LibHandle.begin()+ID);
	FileName.erase(FileName.begin()+ID);
	ModuleList.erase(ModuleList.begin()+ID);
	for (int i = 0; i < Trigger.size();) {
		if (Trigger[i].Module == ID) {
			Trigger.erase(Trigger.begin()+i);
			continue;
		}
		/* We got to move any triggers for modules loaded after this module back one */
		if (Trigger[i].Module > ID) --Trigger[i].Module;
		++i;
	}
	return true;
}

// Find the module from the filename and then unload it
bool ModulesClass::UnloadModule(string FileName_)
{
	FileName_.insert(0, STR_MOD_PATH);
	if (FileName.size() > 0) {
		for (int i = 0; i < FileName.size(); ++i) {
			if (FileName[i] == FileName_) return UnloadModule(i);
		}
	}
	return false;
}

//
void ModulesClass::Register(string Trigger_, int ModID)
{
	TriggerS TempTrigger;
	TempTrigger.Word = Trigger_;
	TempTrigger.Module = ModID;
	Trigger.push_back(TempTrigger);
	cout << "Registering " << Trigger_ << " with mod " << ModID << endl;
}

