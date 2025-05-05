#include <ctime>

#include "refx_LoggingComponent.h"

namespace reFX {

//-------------------------------------------------------------------------------------------------

LoggingComponent::LoggingComponent ()
{
	addAndMakeVisible ( dbc );
	dbc.setModel ( this );
	dbc.setOpaque ( true );
	dbc.setRowHeight ( 22 );
	dbc.setOutlineThickness ( 4 );
	dbc.setColour ( juce::ListBox::backgroundColourId, juce::Colour ( 0xff'13161B ) );
	dbc.setColour ( juce::ListBox::outlineColourId, juce::Colours::transparentBlack );

	Logging::getInstance ()->addListener ( this );
}
//-------------------------------------------------------------------------------------------------

LoggingComponent::~LoggingComponent ()
{
	Logging::getInstance ()->removeListener ( this );
}
//-------------------------------------------------------------------------------------------------

void LoggingComponent::resized ()
{
	auto bounds = getLocalBounds ();

	dbc.setBounds ( bounds );
}
//-------------------------------------------------------------------------------------------------

int LoggingComponent::getNumRows ()
{
	return messages.size ();
}
//-------------------------------------------------------------------------------------------------

juce::String LoggingComponent::getNameForRow ( int row )
{
	if ( juce::isPositiveAndBelow ( row, messages.size () ) )
	{
		const auto message = messages[ row ];

		// Compose final message
		char dstTime[ 100 ] = { 0 };
		std::strftime ( dstTime, sizeof ( dstTime ), "%T", std::localtime ( &message.timeStamp ) );

		return juce::String ( dstTime ) + " - " + message.description;
	}

	return {};
}
//-------------------------------------------------------------------------------------------------

void LoggingComponent::paintListBoxItem ( int row, juce::Graphics& g, int width, int height, bool /*rowIsSelected*/ )
{
	if ( juce::isPositiveAndBelow ( row, messages.size () ) )
	{
		const auto message = messages[ row ];

		static juce::Colour	levels[][ 2 ] = {
			{	juce::Colour ( 0xff'EBFD5A ),		juce::Colours::black	},	// dlog
			{	juce::Colours::transparentBlack,	juce::Colours::white	},	// log
			{   juce::Colour ( 0xFF'43A047 ),		juce::Colours::white	},	// info
			{   juce::Colour ( 0xff'ECBF54 ),		juce::Colours::black	},	// warn
			{	juce::Colour ( 0xff'FC5454 ),		juce::Colours::black	},	// err
		};

		// Compose final message
		char dstTime[ 100 ] = { 0 };
		std::strftime ( dstTime, sizeof ( dstTime ), "%T", std::localtime ( &message.timeStamp ) );

		const auto	text = juce::String ( dstTime ) + " - " + message.description;

		g.setFont ( juce::FontOptions () );

		const auto	msgLevel = int ( message.level );
		if ( const auto bckCol = levels[ msgLevel ][ 0 ]; ! bckCol.isTransparent () )
		{
			const auto	textWidth = juce::GlyphArrangement::getStringWidth ( g.getCurrentFont (), text );

			g.setColour ( bckCol );
			g.fillRoundedRectangle ( juce::Rectangle<float>{ textWidth + 8.0f, float ( dbc.getRowHeight () ) }.reduced ( 0.0f, 1.5f ), 3.0f );
		}

		g.setColour ( levels[ msgLevel ][ 1 ] );

		g.drawText ( text,
					 juce::Rectangle<int> ( width, height ).reduced ( 4, 0 ),
					 juce::Justification::centredLeft,
					 true );
	}
}
//-------------------------------------------------------------------------------------------------

void LoggingComponent::messageLogged ( const LogMessage& msg )
{
	if ( msg.level >= LogLevel::info )
	{
		messages.add ( msg );
		dbc.updateContent ();
		dbc.setVerticalPosition ( 1.0f );
	}
}
//-------------------------------------------------------------------------------------------------

}
