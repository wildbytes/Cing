// Precompiled headers
#include "Cing-Precompiled.h"

#include "LogHistory.h"


namespace Cing
{

//-----------------------------------------------------------------------------------------------
LogHistory::LogHistory()
	: m_totalLogs(0)
	, m_maxLogs(0)
	, m_minLogLevel(LogManager::LOG_NORMAL)
{
	init();
}

//-----------------------------------------------------------------------------------------------
LogHistory::~LogHistory()
{
	end();
}

//-----------------------------------------------------------------------------------------------
void LogHistory::init()
{
	// if already initialized return
	if (m_maxLogs > 0)
		return;

	// initialize the variables
	m_maxLogs = DefaultMaxLogs;
	m_totalLogs = 0;
	m_logs.reserve(m_maxLogs);
}

//-----------------------------------------------------------------------------------------------
void LogHistory::end()
{
	if (m_maxLogs > 0)
	{
		// shut down
		m_maxLogs = 0;
		m_totalLogs = 0;
		m_logs.clear();
	}
}

//-----------------------------------------------------------------------------------------------
void LogHistory::setMaxLogs(int maxLogs)
{
	if (maxLogs > 0)
	{
		// assign the value
		m_maxLogs = maxLogs;

		// resize or reserve room in the vector
		if (m_logs.size() > (size_t)maxLogs)
		{
			m_logs.resize(maxLogs);
		}

		// make sure there's enough space
		m_logs.reserve(maxLogs);
	}
}

//-----------------------------------------------------------------------------------------------
void LogHistory::addLog(LogManager::LogMessageLevel level, std::string const& msg)
{
	if ((int)level >= (int)m_minLogLevel)
	{
		if (m_logs.size() >= (size_t)m_maxLogs)
		{
			// remove the first one to make room for this log
			m_logs.erase(m_logs.begin());
		}

		// add the new log
		LogInfo log;
		log.level = level;
		log.message = msg;
		m_logs.push_back(log);
	}

	// increase the total number of logs
	++m_totalLogs;
}

}
