#pragma once

#include <chrono>

#define	Z_ERR(_m)		{ juce::String zTempDbgBuf; zTempDbgBuf << _m; ::reFX::Logging::logMessage ( zTempDbgBuf, ::reFX::LogLevel::error ); }

#define	Z_WARN(_m)		{ juce::String zTempDbgBuf; zTempDbgBuf << _m; ::reFX::Logging::logMessage ( zTempDbgBuf, ::reFX::LogLevel::warning ); }

#define	Z_INFO(_m)		{ juce::String zTempDbgBuf; zTempDbgBuf << _m; ::reFX::Logging::logMessage ( zTempDbgBuf, ::reFX::LogLevel::info ); }

#if REFX_DEVELOPMENT || _DEBUG
	#define Z_LOG(_m)	{ juce::String zTempDbgBuf; zTempDbgBuf << _m; ::reFX::Logging::logMessage ( zTempDbgBuf, ::reFX::LogLevel::log ); }
#else
	#define Z_LOG(_m)
#endif

#ifdef _DEBUG
	#define Z_DLOG(_m)	{ juce::String zTempDbgBuf; zTempDbgBuf << _m; ::reFX::Logging::logMessage ( zTempDbgBuf, ::reFX::LogLevel::debuglog ); }
#else
	#define Z_DLOG(_m)
#endif

namespace reFX
{
//-------------------------------------------------------------------------------------------------

class LoggingWindow;

enum class LogLevel : int
{
	error		= 4,
	warning		= 3,
	info		= 2,
	log			= 1,
	debuglog	= 0,
};
//-------------------------------------------------------------------------------------------------

struct LoggingOptions
{
	float 		scale = 1.0f;
	int			rowHeight = 22;
	juce::Font	font = juce::FontOptions ();

	std::function<std::unique_ptr<juce::LookAndFeel>()>	lookAndFeelFactory;
};
//-------------------------------------------------------------------------------------------------

class Logging
	: public juce::AsyncUpdater
	, public juce::DeletedAtShutdown
	, public juce::Logger
{
public:
	Logging () = default;
	~Logging () override;

	juce::String					creatorString;
	std::function<juce::String ()>	additionalSystemStats;

	void setLogFolder ( const juce::File& );

	LogLevel getLogLevel ()				{ return level; }
	void setLogLevel ( LogLevel l )		{ level = l;	}

	static juce::String getLogLevelName ( LogLevel l );

	void closeLoggingWindow ();
	LoggingWindow& getLoggingWindow ( const LoggingOptions& );
	bool isLoggingWindowVisible ();

	static void logMessage ( const juce::String& message, const LogLevel level );

	juce::String getAsString ();

	//-------------------------------------------------------------------------------------------------
	JUCE_DECLARE_SINGLETON (Logging, false)

private:
	friend class LoggingWindow;

	void logMessage ( const juce::String& message ) override
	{
		logMessage ( message, LogLevel::info );
	}

    struct Message
    {
		Message ( const juce::String& d = "", const LogLevel l = LogLevel::debuglog )
			: description ( d ), level ( l ) {}

		juce::String toString ();

		const time_t		timeStamp = std::chrono::system_clock::to_time_t (std::chrono::system_clock::now ());
		const juce::String	description;
        const LogLevel 		level = LogLevel::debuglog;
    };

	juce::Array<Message> getMessages ();

	void handleAsyncUpdate () override;

	juce::String getSystemStats ();
	juce::String mergeLogFiles ();

	juce::CriticalSection 	lock;
	juce::Array<Message>	messages;
	LogLevel				level = LogLevel::debuglog;

	juce::File								logFolder;
	std::unique_ptr<juce::FileOutputStream>	logStream;
	std::unique_ptr<LoggingWindow> 			loggingWindow;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ( Logging )
};
//-------------------------------------------------------------------------------------------------
}
