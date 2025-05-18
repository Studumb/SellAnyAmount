#pragma once

#include <Spore\BasicIncludes.h>
#include <codecvt>
#include <cctype>
#include <string>

#define TradingControlWinProcPtr intrusive_ptr<TradingControlWinProc>

// To avoid repeating UTFWin:: all the time.
using namespace UTFWin;

class TradingControlWinProc 
	: public IWinProc
	, public DefaultRefCounted
{
public:
	static const uint32_t TYPE = id("TradingControlWinProc");
	ITextEditPtr inputField;
	UILayoutPtr amountInput;
	IWindowPtr mainWindow;
	// The current set number of item
	int currValue{ 1 };
	// The new amount set by player input
	int newValue{ 0 };
	// For converting from string to number
	Message msg;

	std::string input;
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;

	TradingControlWinProc();
	~TradingControlWinProc();

	int AddRef() override;
	int Release() override;
	void* Cast(uint32_t type) const override;
	
	int GetEventFlags() const override;

	// This is the function you have to implement, called when a window you added this winproc to received an event
	bool HandleUIMessage(IWindow* pWindow, const Message& message) override;
	
	int GetStackLimit(const char16_t* text);
	int GetCurrentValue(const char16_t* text);
};
