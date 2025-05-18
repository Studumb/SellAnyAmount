#include "stdafx.h"
#include "TradingControlWinProc.h"

TradingControlWinProc::TradingControlWinProc()
{
	// Initialize some member variables
	amountInput = new UILayout();
	msg.eventType = kMsgButtonClick;
	mainWindow = WindowManager.GetMainWindow();
	inputField = object_cast<ITextEdit>(WindowManager.GetMainWindow()->FindWindowByID(id("amountInput")));
}

TradingControlWinProc::~TradingControlWinProc() {}

// For internal use, do not modify.
int TradingControlWinProc::AddRef()
{
	return DefaultRefCounted::AddRef();
}

// For internal use, do not modify.
int TradingControlWinProc::Release()
{
	return DefaultRefCounted::Release();
}

// You can extend this function to return any other types your class implements.
void* TradingControlWinProc::Cast(uint32_t type) const
{
	CLASS_CAST(Object);
	CLASS_CAST(IWinProc);
	CLASS_CAST(TradingControlWinProc);
	return nullptr;
}

// This method returns a combinations of values in UTFWin::EventFlags.
// The combination determines what types of events (messages) this window procedure receives.
// By default, it receives mouse/keyboard input and advanced messages.
int TradingControlWinProc::GetEventFlags() const
{
	return kEventFlagBasicInput | kEventFlagAdvanced;
}

// The method that receives the message. The first thing you should do is probably
// checking what kind of message was sent...
bool TradingControlWinProc::HandleUIMessage(IWindow* window, const Message& message)
{
	// This is where we add the input field to the main window if it has not been added
	if (mainWindow->FindWindowByID(0x05E62A48) != nullptr
		&& mainWindow->FindWindowByID(0x05E62A48)->IsVisible()
		&& mainWindow->FindWindowByID(id("amountInput")) == nullptr) {
		amountInput->LoadByName(u"amountInput");
		amountInput->SetParentWindow(mainWindow->FindWindowByID(0x04C1510F)->GetParent());
		amountInput->FindWindowByID(id("amountInput"))->SetArea({ 40, 48, 98, 67 });
		amountInput->FindWindowByID(id("amountInput"))->AddWinProc(new TradingControlWinProc());
		return true;
	}

	// To handle user input
	if (inputField) {
		// Detect what kind of key event
		switch (message.eventType) {
		// When user click to edit the input field
		case kMsgMouseDown:	
			// Disable all the buttons like "Add One", "Remove One", etc
			if (mainWindow->FindWindowByID(0x04C1510F)->IsEnabled())
				for (auto i : mainWindow->FindWindowByID(0x04C1510F)->GetParent()->children())
					if (object_cast<IButton>(i))
						i->SetFlag(kWinFlagEnabled, false);

			// Hide the text box indicating sell/buy amount
			if (mainWindow->FindWindowByID(0x04C1510E)->IsVisible())
				mainWindow->FindWindowByID(0x04C1510E)->SetVisible(false);

			// Initialize the input field
			inputField->SetTextEditFlag(ITextEdit::kFlagHideCaret, false);
			inputField->SetTextEditFlag(inputField->kFlagReadOnly, false);
			return true;
		case kMsgKeyDown:
			// Enter to confirm
			if (message.Key.vkey == VK_RETURN) {
				if (!convert.to_bytes(inputField->GetText()).empty())
					input = convert.to_bytes(inputField->GetText());
				// Ensure input is safe otherwise it would result in a game crash
				try {
					// Only accept non-negative value
					newValue = abs(std::stoi(input));
				}
				catch (...) {
					// Any illegal input would be treated as 0
					newValue = 0;
				}

				currValue = GetCurrentValue(mainWindow->FindWindowByID(0x04C1510E)->GetCaption());

				// Check if input exceed stack limit
				if (newValue > GetStackLimit(mainWindow->FindWindowByID(0x04C1510E)->GetCaption()))
					newValue = GetStackLimit(mainWindow->FindWindowByID(0x04C1510E)->GetCaption());

				// Trick the game to think that we press "Add One"/"Remove One" button for i times by sending messages
				// Basically, we have the game spam the buttons for us
				// In my opinion, this is the simplest way but there definitely better ways can be done
				// However, this should work for every item, including custom items from other mods

				// To increase the amount
				if (newValue > currValue) {
					// Assign the message source to "Add One" button
					msg.source = mainWindow->FindWindowByID(0x04C1510F);
					for (int i = currValue; i < newValue; i++)
						mainWindow->FindWindowByID(0x04C1510F)->SendMsg(msg);
				}
				// To decrease the amount
				else if (newValue < currValue) {
					// Assign the message source to "Remove One" button
					msg.source = mainWindow->FindWindowByID(0x04C1510D);
					for (int i = newValue; i < currValue; i++)
						mainWindow->FindWindowByID(0x04C1510D)->SendMsg(msg);
				}
				//Equal or 0 and 1
				else
					inputField->SetText(u"", 0);

				// After that disable the input field
				inputField->SetText(u"", 0);
				inputField->SetTextEditFlag(ITextEdit::kFlagHideCaret, true);
				inputField->SetTextEditFlag(ITextEdit::kFlagReadOnly, true);
				// Enable, make hidden window visible
				mainWindow->FindWindowByID(0x04C1510E)->SetVisible(true);
				for (auto i : mainWindow->FindWindowByID(0x04C1510F)->GetParent()->children())
					if (object_cast<IButton>(i))
						i->SetFlag(kWinFlagEnabled, true);
				return true;
			}
			// Cancel when hit ESC
			else if (message.Key.vkey == VK_ESCAPE) {
				newValue = 0;
				inputField->SetTextEditFlag(ITextEdit::kFlagHideCaret, true);
				inputField->SetTextEditFlag(ITextEdit::kFlagReadOnly, true);
				mainWindow->FindWindowByID(0x04C1510E)->SetVisible(true);
				for (auto i : mainWindow->FindWindowByID(0x04C1510F)->GetParent()->children())
					if (object_cast<IButton>(i))
						i->SetFlag(kWinFlagEnabled, true);
				return true;
			}
			break;
		}
	}
	// Return true if the message was handled, and therefore no other window procedure should receive it.
	return false;
}

// A function to get item stack limit
// Currently, I haven't found any ways to get item stack properly so I have to get it with this way
int TradingControlWinProc::GetStackLimit(const char16_t* text) {
	std::string value = convert.to_bytes(text);
	return std::stoi(value.substr(value.find('/') + 1));
}

// Same as the function above but to get the current number
int TradingControlWinProc::GetCurrentValue(const char16_t* text) {
	std::string value = convert.to_bytes(text);
	return std::stoi(value.substr(0, value.find('/')));
}
