#include "stdafx.h"
#include "TradingControlWinProc.h"

TradingControlWinProc::TradingControlWinProc()
{
	//Initialize some member variables
	amountInput = new UILayout();
	msg.eventType = kMsgButtonClick;
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
	//This is where we add the input field to the main window if it has not been added
	if (WindowManager.GetMainWindow()->FindWindowByID(0x05E62A48) != nullptr
		&& WindowManager.GetMainWindow()->FindWindowByID(0x05E62A48)->IsVisible()
		&& WindowManager.GetMainWindow()->FindWindowByID(id("amountInput")) == nullptr) {
		amountInput->LoadByName(u"amountInput");
		amountInput->SetParentWindow(WindowManager.GetMainWindow()->FindWindowByID(0x04C1510F)->GetParent());
		amountInput->FindWindowByID(id("amountInput"))->SetArea({ 40, 48, 98, 67 });
		amountInput->FindWindowByID(id("amountInput"))->AddWinProc(new TradingControlWinProc());
		return true;
	}
	//To handle user input
	if (inputField) {
		if (message.IsType(kMsgMouseDown)) {
			//Disable all the buttons like "Add One", "Remove One", etc
			if (WindowManager.GetMainWindow()->FindWindowByID(0x04C1510F)->IsEnabled())
				for (auto i : WindowManager.GetMainWindow()->FindWindowByID(0x04C1510F)->GetParent()->children())
					if (object_cast<IButton>(i))
						i->SetFlag(kWinFlagEnabled, false);

			//Hide the text indicating sell/buy amount
			if (WindowManager.GetMainWindow()->FindWindowByID(0x04C1510E)->IsVisible())
				WindowManager.GetMainWindow()->FindWindowByID(0x04C1510E)->SetVisible(false);

			//Initialize the input field
			inputField->SetTextEditFlag(ITextEdit::kFlagHideCaret, false);
			inputField->SetTextEditFlag(inputField->kFlagReadOnly, false);
			return true;
		}

		if (message.IsType(kMsgKeyDown)) {
			if (message.Key.vkey == VK_RETURN) {
				if (!convert.to_bytes(inputField->GetText()).empty())
					input = convert.to_bytes(inputField->GetText());
				//Ensure input is safe otherwise it would result in a game crash
				try {
					//Only accept non-negative value
					newValue = abs(std::stoi(input));
				}
				catch (...) {
					//Any illegal input would be treated as 0
					newValue = 0;
				}
				currValue = GetCurrentValue(WindowManager.GetMainWindow()->FindWindowByID(0x04C1510E)->GetCaption());
				//Check if input exceed stack limit
				if (newValue > GetStackLimit(WindowManager.GetMainWindow()->FindWindowByID(0x04C1510E)->GetCaption()))
					newValue = GetStackLimit(WindowManager.GetMainWindow()->FindWindowByID(0x04C1510E)->GetCaption());

				//Make the game think we press "Add One"/"Remove One" button for i times by sending messages
				//Basically, we have the game spam the buttons for us
				//In my opinion, this is the simplest way but there definitely better ways can be done
				//However, this should work for every item, including custom items from other mods

				//To increase the amount
				if (newValue > currValue) {
					msg.source = WindowManager.GetMainWindow()->FindWindowByID(0x04C1510F);
					for (int i = currValue; i < newValue; i++)
						WindowManager.GetMainWindow()->FindWindowByID(0x04C1510F)->SendMsg(msg);
				}
				//To decrease the amount
				else if (newValue < currValue) {
					msg.source = WindowManager.GetMainWindow()->FindWindowByID(0x04C1510D);
					for (int i = newValue; i < currValue; i++)
						WindowManager.GetMainWindow()->FindWindowByID(0x04C1510D)->SendMsg(msg);
					inputField->SetText(u"", 0);
					inputField->SetTextEditFlag(ITextEdit::kFlagHideCaret, true);
					inputField->SetTextEditFlag(ITextEdit::kFlagReadOnly, true);
				}
				//Equal or 0 and 1
				else {
					inputField->SetText(u"", 0);
					inputField->SetTextEditFlag(ITextEdit::kFlagHideCaret, true);
					inputField->SetTextEditFlag(ITextEdit::kFlagReadOnly, true);
					WindowManager.GetMainWindow()->FindWindowByID(0x04C1510E)->SetVisible(true);
					for (auto i : WindowManager.GetMainWindow()->FindWindowByID(0x04C1510F)->GetParent()->children())
						if (object_cast<IButton>(i))
							i->SetFlag(kWinFlagEnabled, true);
				}
				//Clear the text and disable the input
				inputField->SetText(u"", 1);
				inputField->SetTextEditFlag(ITextEdit::kFlagHideCaret, true);
				inputField->SetTextEditFlag(ITextEdit::kFlagReadOnly, true);
				return true;
			}
			//Cancel
			else if (message.Key.vkey == VK_ESCAPE) {
				newValue = 0;
				inputField->SetText(u"", 0);
				inputField->SetTextEditFlag(ITextEdit::kFlagHideCaret, true);
				inputField->SetTextEditFlag(ITextEdit::kFlagReadOnly, true);
				WindowManager.GetMainWindow()->FindWindowByID(0x04C1510E)->SetVisible(true);
				for (auto i : WindowManager.GetMainWindow()->FindWindowByID(0x04C1510F)->GetParent()->children())
					if (object_cast<IButton>(i))
						i->SetFlag(kWinFlagEnabled, true);
				return true;
			}
		}
	}
	// Return true if the message was handled, and therefore no other window procedure should receive it.
	return false;
}

//A function to get item stack limit
//Currently, I haven't found any ways to get item stack properly so I have to get it with this way
int TradingControlWinProc::GetStackLimit(const char16_t *text) {
	return std::stoi(convert.to_bytes(text).substr(convert.to_bytes(text).find('/') + 1));
}

//Same as the function above
int TradingControlWinProc::GetCurrentValue(const char16_t* text) {
	return std::stoi(convert.to_bytes(text).substr(0, convert.to_bytes(text).find('/')));
}