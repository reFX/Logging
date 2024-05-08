#include <ctime>

#include "refx_loggingWindow.h"

//-------------------------------------------------------------------------------------------------

namespace reFX
{
JUCE_IMPLEMENT_SINGLETON ( Logging )

bool isCurrentUserAdmin ()
{
   #if JUCE_MAC
	juce::ChildProcess cp;
	cp.start ( { "id", "-u" } );
	cp.waitForProcessToFinish ( 100 );

	auto output = cp.readAllProcessOutput ().trim ();
	return output == "0";
   #elif JUCE_WINDOWS
	// Get authority information
	SID_IDENTIFIER_AUTHORITY	NtAuthority = SECURITY_NT_AUTHORITY;
	PSID						AdministratorsGroup;

	if ( AllocateAndInitializeSid ( &NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup ) )
	{
		BOOL	isMember = false;

		CheckTokenMembership ( nullptr, AdministratorsGroup, &isMember );

		FreeSid ( AdministratorsGroup );

		return isMember;
	}
	return false;
   #else
    #error "Unknown platform!"
   #endif
}
//-------------------------------------------------------------------------------------------------

juce::String Logging::Message::toString ()
{
	// Compose final message
	char dstTime[ 100 ] = { 0 };
	std::strftime ( dstTime, sizeof ( dstTime ), "%T", std::localtime ( &timeStamp ) );

	juce::String code;
	switch ( level )
	{
		case LogLevel::log: 		code = "LOG "; break;
		case LogLevel::info: 		code = "INFO"; break;
		case LogLevel::warning: 	code = "WARN"; break;
		case LogLevel::error: 		code = "ERR "; break;
		case LogLevel::debuglog: 	code = "DLOG"; break;
		default: jassertfalse; 		code = "   "; break;
	}

	return juce::String ( dstTime ) + ": " + code + " - " + description;
}
//-------------------------------------------------------------------------------------------------

Logging::~Logging ()
{
	clearSingletonInstance ();
}
//-------------------------------------------------------------------------------------------------

void Logging::logMessage ( const juce::String& messageText, const LogLevel msgLevel )
{
	auto self = Logging::getInstance ();
	juce::ScopedLock sl ( self->lock );

	Message msg = { messageText, msgLevel };

	self->messages.add ( msg );
	self->triggerAsyncUpdate ();

	if ( self->logStream != nullptr )
	{
		self->logStream->writeText ( msg.toString () + "\r\n", false, false, nullptr );
		self->logStream->flush ();
	}

	outputDebugString ( msg.toString () );
}
//-------------------------------------------------------------------------------------------------

void Logging::handleAsyncUpdate ()
{
	if ( loggingWindow )
	{
		loggingWindow->update ();
	}
}
//-------------------------------------------------------------------------------------------------

juce::Array<Logging::Message> Logging::getMessages ()
{
	juce::ScopedLock sl ( lock );

	return messages;
}
//-------------------------------------------------------------------------------------------------

void Logging::closeLoggingWindow ()
{
	loggingWindow = nullptr;
}
//-------------------------------------------------------------------------------------------------

LoggingWindow& Logging::getLoggingWindow ( float scale )
{
	if ( loggingWindow == nullptr )
	{
		loggingWindow = std::make_unique<LoggingWindow> (*this, scale );
	}

	return *loggingWindow;
}
//-------------------------------------------------------------------------------------------------

bool Logging::isLoggingWindowVisible ()
{
	if ( loggingWindow == nullptr )
	{
		return false;
	}
	return loggingWindow->isVisible ();
}
//-------------------------------------------------------------------------------------------------

void Logging::setLogFolder ( const juce::File& f )
{
	juce::ScopedLock sl ( lock );

	if ( f != juce::File () )
	{
		f.createDirectory ();

		auto files = f.findChildFiles ( juce::File::findFiles, true, "*", juce::File::FollowSymlinks::noCycles );
		std::sort ( files.begin (), files.end (), [] (const auto& lhs, const auto& rhs) { return lhs.getCreationTime () < rhs.getCreationTime (); });

		while ( files.size () > 3 )
		{
			files.removeAndReturn ( 0 ).deleteFile ();
		}

		juce::File logFile = f.getChildFile ( juce::Time::getCurrentTime ().toISO8601 ( false ) + ".txt" );
		logStream = std::make_unique<juce::FileOutputStream> (logFile);
	}
	else
	{
		logStream = nullptr;
	}

	logFolder = f;
}
//-------------------------------------------------------------------------------------------------

juce::String Logging::getLogLevelName ( LogLevel l )
{
	switch ( l )
	{
		case LogLevel::error:		return "Error";
		case LogLevel::warning:		return "Warning";
		case LogLevel::info:		return "Info";
		case LogLevel::log:			return "Log";
		case LogLevel::debuglog:	return "Debug Log";
		default:
			jassertfalse;
			return {};
	}
}
//-------------------------------------------------------------------------------------------------

juce::String Logging::getSystemStats ()
{
	juce::String text;

	text += "Creator:   " + creatorString + "\r\n";
	text += "Location:  " + juce::File::getSpecialLocation ( juce::File::currentApplicationFile ).getFullPathName () + "\r\n";
	text += "Timestamp: " + juce::Time::getCurrentTime ().toString ( true, true, true, true ) + "\r\n\r\n";

	//
	// Computer specific
	//
	text += "Computer:  " + juce::SystemStats::getComputerName () + "\r\n";
	text += "OS:        " + juce::SystemStats::getOperatingSystemName () + "\r\n";
	text += "Device:    " + ( juce::SystemStats::getDeviceManufacturer () + " " + juce::SystemStats::getDeviceDescription () ).trim () + "\r\n";
	text += "Admin:     " + juce::String ( isCurrentUserAdmin () ? "Yes" : "No" ) + "\r\n\r\n";

	//
	// CPU specific
	//
	text += "CPU:       " + juce::SystemStats::getCpuVendor () + " " + juce::SystemStats::getCpuModel () + " " + juce::String ( juce::SystemStats::getCpuSpeedInMegahertz () ) + " MHz\r\n";
	text += "Cores:     " + juce::String ( juce::SystemStats::getNumPhysicalCpus () ) + " / " + juce::String ( juce::SystemStats::getNumCpus () ) + "\r\n";
	text += "Memory:    " + juce::String ( juce::roundToInt ( juce::SystemStats::getMemorySizeInMegabytes () / 1024.0 ) ) + " GB" + "\r\n";
	#if JUCE_INTEL
	text += "SSE41:     " + juce::String ( juce::SystemStats::hasSSE41 () ? "Yes" : "No" ) + "\r\n";
	text += "SSE42:     " + juce::String ( juce::SystemStats::hasSSE42 () ? "Yes" : "No" ) + "\r\n";
	#endif

	//
	// Displays
	//
	for ( auto d : juce::Desktop::getInstance().getDisplays ().displays )
	{
		const auto	physRect = ( d.totalArea.toDouble () * d.scale ).toNearestIntEdges ();

		text += juce::String::formatted ( "Display:   %d x %d @ %d%%\r\n", physRect.getWidth (), physRect.getHeight (), juce::roundToInt ( d.scale * 100.0 ) );
	}

	if ( additionalSystemStats )
	{
		text += additionalSystemStats ();
	}

	text += "------------------------------------------------------------------------------\r\n\r\n";

	return text;
}
//-------------------------------------------------------------------------------------------------

juce::String Logging::getAsString ()
{
	auto text = getSystemStats ();

	if ( logStream != nullptr )
	{
		text += mergeLogFiles ();
	}

	for ( auto& msg : messages )
	{
		text += msg.toString () + "\r\n";
	}

	return text;
}

juce::String Logging::mergeLogFiles ()
{
	juce::String text;

	auto files = logFolder.findChildFiles ( juce::File::findFiles, false );
	std::sort ( files.begin (), files.end (), [] (const auto& lhs, const auto& rhs) { return lhs.getCreationTime () < rhs.getCreationTime (); });

	// Get all the files except the current one, get that one from memory
	for ( int i = 0; i < files.size() - 1; i++)
	{
		auto f = files[i];

		text += f.loadFileAsString ();
		text += "------------------------------------------------------------------------------\r\n\r\n";
	}

	return text;
}

}
