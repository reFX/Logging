#include <ctime>

#include "refx_loggingWindow.h"

//-------------------------------------------------------------------------------------------------

namespace reFX
{

LoggingWindow::Content::Content ( LoggingWindow& o )
	: owner ( o )
{
	addAndMakeVisible ( dbc );
	dbc.setModel ( this );
	dbc.setOpaque ( true );
	dbc.setOutlineThickness ( 4 );
	dbc.setColour ( juce::ListBox::backgroundColourId, juce::Colour::fromRGB ( 41, 44, 50 ) );
	dbc.setColour ( juce::ListBox::outlineColourId, juce::Colours::transparentBlack );

	addAndMakeVisible ( clearButton );
	clearButton.setColour ( juce::TextButton::textColourOnId, juce::Colours::white );
	clearButton.setColour ( juce::TextButton::textColourOffId, juce::Colours::white );
	clearButton.onClick = [ this ]
	{
		owner.logClearedTime = std::chrono::system_clock::to_time_t ( std::chrono::system_clock::now () );
		owner.update ();
	};

	addAndMakeVisible ( saveButton );
	saveButton.setColour ( juce::TextButton::textColourOnId, juce::Colours::white );
	saveButton.setColour ( juce::TextButton::textColourOffId, juce::Colours::white );
	saveButton.onClick = [ this ]
	{
		auto f = juce::File::getSpecialLocation ( juce::File::userDesktopDirectory ).getChildFile ( owner.getName ().replace ( "logging window", "Support Info" ) + ".txt" );
		gin::overwriteWithText ( f, owner.logging.getAsString () );
	};

   #if JUCE_DEBUG || REFX_DEVELOPMENT
	addAndMakeVisible ( levelButton );
	levelButton.setButtonText ( Logging::getLogLevelName ( owner.logging.getLogLevel () ) );
	levelButton.setColour ( juce::TextButton::textColourOnId, juce::Colours::white );
	levelButton.setColour ( juce::TextButton::textColourOffId, juce::Colours::white );
	levelButton.onClick = [ this ]
	{
		juce::PopupMenu m;

		for ( int i = int ( LogLevel::error ); i >= int ( LogLevel::debuglog ); --i)
		{
			auto l = LogLevel ( i );
			m.addItem ( Logging::getLogLevelName ( l ),	true, owner.logging.getLogLevel () == l, [ this, l ]
			{
				levelButton.setButtonText ( Logging::getLogLevelName ( l ) );
				owner.logging.setLogLevel ( l );
				owner.update ();
			} );
		}

		m.showMenuAsync ( juce::PopupMenu::Options ().withDeletionCheck ( *this ).withTargetComponent ( levelButton ) );
	};
   #endif
	owner.update ();
}
//-------------------------------------------------------------------------------------------------

void LoggingWindow::Content::resized ()
{
	auto bounds = getLocalBounds ();

	auto rc = bounds.removeFromTop ( 30 );

	clearButton.setBounds ( rc.removeFromLeft ( 60 ).reduced ( 2 ) );
	saveButton.setBounds ( rc.removeFromRight ( 120 ).reduced ( 2 ) );
   #if JUCE_DEBUG || REFX_DEVELOPMENT
	rc.removeFromRight ( 4 );
	levelButton.setBounds ( rc.removeFromRight ( 120 ).reduced ( 2 ) );
   #endif

	dbc.setBounds ( bounds );
}
//-------------------------------------------------------------------------------------------------

void LoggingWindow::Content::paint ( juce::Graphics& g ) 
{
	g.setColour ( juce::Colour::fromRGB ( 82, 88, 100 ) );
	g.fillAll ();
}
//-------------------------------------------------------------------------------------------------

int LoggingWindow::Content::getNumRows ()
{
	return owner.messages.size ();
}
//-------------------------------------------------------------------------------------------------

juce::String LoggingWindow::Content::getNameForRow ( int row )
{
	if ( juce::isPositiveAndBelow ( row, owner.messages.size () ) )
	{
		const auto message = owner.messages[ row ];

		// Compose final message
		char dstTime[ 100 ] = { 0 };
		std::strftime ( dstTime, sizeof ( dstTime ), "%T", std::localtime ( &message.timeStamp ) );

		return juce::String ( dstTime ) + " - " + message.description;
	}

	return {};
}
//-------------------------------------------------------------------------------------------------

void LoggingWindow::Content::paintListBoxItem ( int row, juce::Graphics& g, int width, int height, bool /*rowIsSelected*/ )
{
	if ( juce::isPositiveAndBelow ( row, owner.messages.size () ) )
	{
		const auto message = owner.messages[ row ];

		static juce::Colour	levels[][ 2 ] = {
											{	juce::Colour ( 0xFF, 0xCC, 0x33 ), juce::Colours::black,	},	// dlog
											{	juce::Colours::transparentBlack, juce::Colours::white,		},	// log
											{   juce::Colours::green, juce::Colours::white					},	// info
											{   juce::Colours::orange, juce::Colours::black 				},	// warn
											{	juce::Colour ( 0xFF, 0x00, 0x00 ), juce::Colours::white,	},	// err
		};

		// Compose final message
		char dstTime[ 100 ] = { 0 };
		std::strftime ( dstTime, sizeof ( dstTime ), "%T", std::localtime ( &message.timeStamp ) );

		const auto	text = juce::String ( dstTime ) + " - " + message.description;

		g.setFont ( getLookAndFeel ().defaultFontWithHeight ( float ( dbc.getRowHeight () ) * 0.8f ) );

		if ( message.level != LogLevel::log )
		{
			const auto	textWidth = g.getCurrentFont ().getStringWidthFloat ( text ) + 4.0f;

			g.setColour ( levels[ ( int ) message.level ][ 0 ] );
			g.fillRoundedRectangle ( juce::Rectangle<float>{ 2.0f, 0.0f, textWidth, (float)dbc.getRowHeight () }.reduced ( 0.0f, 1.0f ), 2.0f );
		}

		g.setColour ( levels[ ( int ) message.level ][ 1 ] );

		g.drawText (	text,
						juce::Rectangle<int> ( width, height ).reduced ( 4, 0 ),
						juce::Justification::centredLeft,
						true );
	}
}
//-------------------------------------------------------------------------------------------------


LoggingWindow::LoggingWindow ( Logging& l, float scale_ )
	: juce::DocumentWindow ( "reFX logging window", juce::Colours::coral, TitleBarButtons::closeButton )
	, logging ( l )
	, scale ( scale_ )
{
	setUsingNativeTitleBar ( true );
	setLookAndFeel ( &laf );
	setContentNonOwned ( &content, false );
	setResizable ( true, true );

   #if JUCE_MAC
	juce::String settingsSubDir = "Application Support/reFX";
   #else
	juce::String settingsSubDir = "reFX";
   #endif
	juce::String appName = juce::File::getSpecialLocation ( juce::File::currentExecutableFile ).getFileNameWithoutExtension ().replace ( " ", "_" );
	settingsFile = juce::File::getSpecialLocation ( juce::File::userApplicationDataDirectory ).getChildFile ( settingsSubDir + "/logging_window_" + appName + ".json" );

	JsonFile jsonFile ( settingsFile );
	jsonFile.load ();

	juce::String pos = jsonFile.get ( "/window_pos", "" );

	//
	// Get saved window rect
	//
	auto tokens = juce::StringArray::fromTokens ( pos, false );
	tokens.removeEmptyStrings();
	tokens.trim();
	if ( tokens[ 0 ] == "fs" )
	{
		tokens.remove ( 0 );
	}

	//
	// Make sure it has a decent size
	//
	juce::Rectangle<int> newPos ( tokens[ 0 ].getIntValue (), tokens[ 1 ].getIntValue (), tokens[ 2 ].getIntValue (), tokens[ 3 ].getIntValue () );
	if ( newPos.getWidth () < 200 || newPos.getHeight () < 200 )
	{
		newPos = { 100, 100, 600, 800 };
	}

	//
	// Make sure it's on a display
	//
	std::optional<juce::Displays::Display> display;
	for ( auto d : juce::Desktop::getInstance ().getDisplays ().displays )
	{
		auto area = d.userArea.toFloat () / scale;
		if ( area.contains ( newPos.toFloat () ) )
		{
			display = d;
			break;
		}
	}

	if ( ! display.has_value () )
	{
		if ( auto d = juce::Desktop::getInstance ().getDisplays ().getPrimaryDisplay () )
		{
			display = *d;
			auto area = display->userArea.toFloat () / scale;
			newPos = juce::Rectangle<int> ( int ( area.getX () ) + 100, int ( area.getY () ) + 100, 600, 800 );
		}
	}

	//
	// Restore position
	//
	setBounds ( newPos );

	logging.setLogLevel ( ( LogLevel ) jsonFile.get ( "/log_level", ( int ) LogLevel::debuglog ) );

	update ();
}
//-------------------------------------------------------------------------------------------------

LoggingWindow::~LoggingWindow ()
{
	setLookAndFeel ( nullptr );
	if ( everShown )
	{
		JsonFile jsonFile ( settingsFile );
		jsonFile.load ();
		jsonFile.set ( juce::String ( "/window_pos" ), getWindowStateAsString () );
	   #if JUCE_DEBUG || REFX_DEVELOPMENT
		jsonFile.set ( "/log_level", ( int ) logging.getLogLevel () );
	   #endif
		jsonFile.save ();
	}
}
//-------------------------------------------------------------------------------------------------

void LoggingWindow::closeButtonPressed ()
{
	setVisible ( false );
}
//-------------------------------------------------------------------------------------------------

void LoggingWindow::visibilityChanged ()
{
	juce::DocumentWindow::visibilityChanged ();
	everShown = true;
}
//-------------------------------------------------------------------------------------------------

void LoggingWindow::update ()
{
	messages.clear ();

	for ( auto m : logging.getMessages () )
	{
		if ( m.timeStamp >= logClearedTime && m.level >= logging.getLogLevel () )
		{
			messages.add ( m );
		}
	}

	content.dbc.updateContent ();
	content.dbc.scrollToEnsureRowIsOnscreen ( messages.size () - 1 );
}
//-------------------------------------------------------------------------------------------------

float LoggingWindow::getDesktopScaleFactor () const
{
	return scale * juce::Desktop::getInstance ().getGlobalScaleFactor ();
}
//-------------------------------------------------------------------------------------------------

}
