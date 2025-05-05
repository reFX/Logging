#pragma once

namespace reFX
{
//-------------------------------------------------------------------------------------------------

class LoggingComponent : public juce::Component
					   , public juce::ListBoxModel
					   , private Logging::Listener
{
public:
	LoggingComponent ();
	~LoggingComponent () override;

	int getNumRows () override;
	void paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
	void resized () override;
	juce::String getNameForRow ( int row ) override;

	void messageLogged ( const LogMessage& ) override;

private:
	juce::Array<LogMessage>	messages;
	juce::ListBox			dbc;
};

}
