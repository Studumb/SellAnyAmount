// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "TradingControlWinProc.h"

void Initialize()
{
	// This method is executed when the game starts, before the user interface is shown
	// Here you can do things such as:
	//  - Add new cheats
	//  - Add new simulator classes
	//  - Add new game modes
	//  - Add new space tools
	//  - Change materials
}

void Dispose()
{
	// This method is called when the game is closing
}

// This detour is called every time player open the trading menu
// The rest is handled by winproc
member_detour(GenerateNPCStore__detour, Simulator::cSpaceTrading, void(Simulator::cPlanetRecord*, LocalizedString&)) {
	void detoured(Simulator::cPlanetRecord * planetRecord, LocalizedString & dstSpiceText) {
		original_function(this, planetRecord, dstSpiceText);
		// The trading menu
		WindowManager.GetMainWindow()->FindWindowByID(0x05E62A48)->AddWinProc(new TradingControlWinProc());
		// We only have to call this once so as soon as it is called we also detach it
		// This should prevent adding winproc to the window multiple times
		GenerateNPCStore__detour::detach();
	}
};

void AttachDetours()
{
	// Call the attach() method on any detours you want to add
	// For example: cViewer_SetRenderType_detour::attach(GetAddress(cViewer, SetRenderType));
	GenerateNPCStore__detour::attach(GetAddress(Simulator::cSpaceTrading, GenerateNPCStore));
}


// Generally, you don't need to touch any code here
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		ModAPI::AddPostInitFunction(Initialize);
		ModAPI::AddDisposeFunction(Dispose);

		PrepareDetours(hModule);
		AttachDetours();
		CommitDetours();
		break;

	case DLL_PROCESS_DETACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}

