#include "AudioLogger.h"

namespace dae
{
    std::vector<std::string> AudioLogger::m_Logs;
    std::mutex AudioLogger::m_Mutex;

    void AudioLogger::Log(const std::string& message)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Logs.push_back(message);
        if (m_Logs.size() > m_MaxLogs)
        {
            m_Logs.erase(m_Logs.begin());
        }
    }

    std::vector<std::string> AudioLogger::GetLogs()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_Logs;
    }

    void AudioLogger::Clear()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Logs.clear();
    }
}
