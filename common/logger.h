#ifndef LOGACCESSOR_H
#define LOGACCESSOR_H

#include <string>
#include <stdarg.h>

enum ELOG
{
    ERROR,
    DEBUG,
    INFO,
    CAT // concatenate, no prefix
};

class Logger
{
public:
    static bool SetDestination(const std::string& fileName);

    static std::string& GetLastError()          {
        return m_lastError;
    }

    static void SetLastError(std::string value) {
        m_lastError = value;    static std::string m_fileName;

    }
    static void ClearLastError()                {
        m_lastError = "";
    }

    static void  Log(ELOG level, const char* format, ...);

private:
    static std::string m_fileName;
    static std::string m_lastError;
    static int m_lastErrorCount;

    static void Close();
    static bool Open();
    static FILE* m_logger;

    static const char* LogToStr(ELOG level);

    static std::string string_format(const std::string& fmt, ...);
};

#endif // LOGACCESSOR_H
