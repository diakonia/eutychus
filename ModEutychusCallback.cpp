/**********************************************************************

Audacity: A Digital Audio Editor

ModEutychusCallback.cpp

James Crook

Audacity is free software.
This file is licensed under the wxWidgets license, see License.txt

********************************************************************//**

\class ModEutychusCallback
\brief ModEutychusCallback is a class containing all the callback 
functions for the second generation Eutychus.  These functions are 
added into the standard Audacity Project Menus.

*//*****************************************************************//**

\class ModEutychusCommandFunctor
\brief We create one of these functors for each menu item or 
command which we register with the Command Manager.  These take the 
click from the menu into the actual function to be called.

*//********************************************************************/

#include <wx/wx.h>
#include "ModEutychusCallback.h"
#include "Audacity.h"
#include "ShuttleGui.h"
#include "Project.h"
#include "LoadModules.h"
#include "Registrar.h"
#include "EutychusTrackPanel.h"

/*
There are several functions that can be used in a GUI module.

//#define versionFnName   "GetVersionString"
If the version is wrong, the module will be rejected.
That is it will be loaded and then unloaded.

//#define ModuleDispatchName "ModuleDispatch"
The most useful function.  See the example in this 
file.  It has several cases/options in it.

//#define scriptFnName    "RegScriptServerFunc"
This function is run from a non gui thread.  It was originally 
created for the benefit of mod-script-pipe.

//#define mainPanelFnName "MainPanelFunc"
This function is the hijacking function, to take over Audacity
and replace the main project window with our own wxFrame.

*/

// The machinery here is somewhat overkill for what we need.
// It allows us to add lots of menu and other actions into Audacity.
// We need to jump through these hoops even if only adding
// two menu items into Audacity.

// The OnFunc functrions are functions which can be invoked 
// by Audacity.  Mostly they are for menu items.  They could
// be for buttons.  They could be for commands invokable by
// script (but no examples of that yet).
class ModEutychusCallback
{
public:
	void OnFuncShowEutychusPanel();
};

typedef void (ModEutychusCallback::*ModEutychusCommandFunction)();

// We have an instance of this CommandFunctor for each 
// instance of a command we attach to Audacity.
// Although the commands have void argument,
// they do receive an instance of ModEutychusCallback as a 'this', 
// so if we want to, we can pass data to each function instance.
class ModEutychusCommandFunctor:public CommandFunctor
{
public:
	ModEutychusCommandFunctor(ModEutychusCallback *pData,
		ModEutychusCommandFunction pFunction);
	virtual void operator()(int index = 0);
public:
	ModEutychusCallback * mpData;
	ModEutychusCommandFunction mpFunction;
};

// If pData is NULL we will later be passing a NULL 'this' to pFunction.
// This may be quite OK if the class function is written as if it
// could be static.
ModEutychusCommandFunctor::ModEutychusCommandFunctor(ModEutychusCallback *pData,
																   ModEutychusCommandFunction pFunction)
{
	mpData = pData;
	mpFunction = pFunction;
}

// The dispatching function in the command functor.
void ModEutychusCommandFunctor::operator()(int index )
{
	(mpData->*(mpFunction))();
}

#define ModEutychusFN(X) new ModEutychusCommandFunctor(pModEutychusCallback, \
	(ModEutychusCommandFunction)(&ModEutychusCallback::X)) 


void ModEutychusCallback::OnFuncShowEutychusPanel()
{
	int k=3;
	Registrar::ShowNewPanel();
}

// Oooh look, we're using a NULL object, and hence a NULL 'this'.
// That's OK.
ModEutychusCallback * pModEutychusCallback=NULL;

//This is the DLL related machinery that actually gets called by Audacity
//as prt of loading and using a DLL.
//It is MUCH simpler to use C for this interface because then the
//function names are not 'mangled'.
//The function names are important, because they are what Audacity looks
//for.  Change the name and they won't be found.
//Change the signature, e.g. return type, and you probably have a crash.
extern "C" {
	// GetVersionString
	// REQUIRED for the module to be accepted by Audacity.
	// Without it Audacity will see a version number mismatch.
	MOD_EUTYCHUS_SERVICE_RECORDER_DLL_API wxChar * GetVersionString()
	{
		// Make sure that this version of the module requires the version 
		// of Audacity it is built with. 
		// For now, the versions must match exactly for Audacity to 
		// agree to load the module.
		return AUDACITY_VERSION_STRING;
	}

	// This is the function that connects us to Audacity.
	MOD_EUTYCHUS_SERVICE_RECORDER_DLL_API int ModuleDispatch(ModuleDispatchTypes type)
	{
		switch (type)
		{
		case ModuleInitialize:
			TrackPanel::FactoryFunction = EutychusTrackPanelFactory;
			Registrar::Start();
			break;
		case AppInitialized:
			break;
		case AppQuiting:
			Registrar::Finish();
			break;
		case ProjectInitialized:
			Registrar::ShowNewPanel();
			break;
		case MenusRebuilt:
			break;
		default:
			break;
		}

		return 1;
	}

	//Example code commented out.
#if 0
	// This is an example function to hijack the main panel
	int MOD_EUTYCHUS_SERVICE_RECORDER_DLL_API MainPanelFunc(int ix)
	{
		ix=ix;//compiler food
		// If we wanted to hide Audacity's Project, 
		// we'd create a new wxFrame right here and return a pointer to it
		// as our return result.

		// Don't hijack the main panel, just return a NULL;
		return NULL;
	}
#endif


} // End extern "C"
