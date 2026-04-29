#pragma once
#include <vector>
#include <string>
#include <mutex>

namespace dae
{
    class AudioLogger
    {
    public:
        static void Log(const std::string& message);
        static std::vector<std::string> GetLogs();
        static void Clear();

    private:
        static std::vector<std::string> m_Logs;
        static std::mutex m_Mutex;
        static const size_t m_MaxLogs = 100;
    };
}
