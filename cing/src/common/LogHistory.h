#ifndef _CingLogHistory_h_
#define _CingLogHistory_h_


#include "CommonPrereqs.h"
#include "Singleton.h"

#include "LogManager.h"

#include <string>
#include <vector>


namespace Cing
{

//-----------------------------------------------------------------------------------------------
// This class keeps a history of the logs of the app. Useful
class LogHistory : public SingletonStatic< LogHistory >
{
public:
	// Structure holding the information we store for the logs
	struct LogInfo
	{
		LogManager::LogMessageLevel level;
		std::string					message;
	};

	// Singleton needs
	friend class SingletonStatic< LogManager >;

	// Constants
	const static int DefaultMaxLogs = 100;

public:
	LogHistory();
	~LogHistory();

	void init();
	void end();

	// adds the given log to the history of logs
	void addLog(LogManager::LogMessageLevel level, std::string const& msg);

	// accessors
	int getTotalLogs() const			{ return m_totalLogs; }
	int getMaxLogs() const				{ return m_maxLogs; }
	int getNumLogsInHistory() const		{ return (int)m_logs.size(); }
	LogInfo const&				getLog(int i) const			{ return m_logs[i]; }
	LogManager::LogMessageLevel	getMinLogLevel() const		{ return m_minLogLevel; }

	void setMaxLogs(int maxLogs);
	void setMinLogLevel(LogManager::LogMessageLevel level);

private:
	int m_totalLogs;				//< the total number of messages that have been logged
	int m_maxLogs;					//< the maximum number of logs to be stored in the history
	LogManager::LogMessageLevel m_minLogLevel;
	std::vector<LogInfo> m_logs;	//< the vector of logs
};

}

#endif
