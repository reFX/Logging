#pragma once

#include <chrono>

namespace reFX
{
//-------------------------------------------------------------------------------------------------

class LoggingWindow
	: public juce::DocumentWindow
{
public:
	LoggingWindow ( Logging&, float scale );
	~LoggingWindow () override;

	void update ();
	void visibilityChanged () override;

private:
	void closeButtonPressed () override;
	float getDesktopScaleFactor () const override;

	//-------------------------------------------------------------------------------------------------

	class Content
		: public juce::Component
		, public juce::ListBoxModel
	{
	public:
		Content ( LoggingWindow& owner );

		int getNumRows () override;
		void paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
		void resized () override;
		void paint ( juce::Graphics& g ) override;
		juce::String getNameForRow ( int row ) override;

		LoggingWindow&		owner;

		juce::ListBox		dbc;
		juce::TextButton	clearButton { "Clear" };
		juce::TextButton	saveButton { "Save to Desktop" };
	   #if JUCE_DEBUG || REFX_DEVELOPMENT
		juce::TextButton	levelButton { "Level" };
	   #endif
	};

	Logging&			logging;
	juce::File			settingsFile;
	std::unique_ptr<juce::LookAndFeel>	laf;
	juce::Array<Logging::Message> messages;

	Content				content { *this };

	time_t 				logClearedTime = 0;
	float				scale = 1.0f;
	bool				everShown = false;
	juce::Font			font { 12.0f };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LoggingWindow)
};
//-------------------------------------------------------------------------------------------------
}
