#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// #include <KDebug>

using namespace std;

std::string Logger::m_fileName  = "";
std::string Logger::m_lastError = "";
int Logger::m_lastErrorCount = 0;

FILE* Logger::m_logger = 0;

// #include <sstream>
// static int numberOfColorSupported()
// {
//  FILE* fp;
//  char path[1035];
//
//  /* Open the command for reading. */
//  fp = popen("tput colors", "r");
//  if (fp == 0)
//      return 0;
//
//  /* Read the output */
//  char* result = fgets(path, sizeof(path) - 1, fp);
//
//  /* close */
//  pclose(fp);
//
//  if (!result)
//      return 0;
//
//  if (istringstream(path))
//      return atoi(path);
//
//  return 0;
// }

static const char* getVar(const char* name)
{
    const char* value = ::getenv(name);
    return value ? value : "";
}

static bool ENABLE_DEBUG = (strcasecmp(getVar("DEBUG"), "1") == 0);
static bool ENABLE_COLOR = (strcasecmp(getVar("ENABLE_COLOR"), "0") != 0);
// (numberOfColorSupported() >= 8);
// ::getenv("TERM") != 0; -> enough for kdevelop, but skip the logs, and .xsessionerrors

const char* Logger::LogToStr(ELOG level)
{
    switch (level)
    {
    case ERROR:
        return ENABLE_COLOR ? "\e[1;31mERROR\e[0m " : "ERROR "; // red
    case DEBUG:
        return ENABLE_COLOR ? "\e[1;32mDEBUG\e[0m " : "DEBUG "; // green
    case INFO:
        return ENABLE_COLOR ? "\e[1;34mINFO\e[0m  " : "INFO  "; // blue
    default:
        return "";
    }
}

bool Logger::SetDestination(const string& fileName)
{
    if (m_fileName == fileName)
        return true;

    if (!m_fileName.empty())
        Close();

    m_fileName = fileName;
    if (!fileName.empty())
        return Open();

    return false;
}

bool Logger::Open()
{
    m_logger = fopen(m_fileName.c_str(), "a+");

    if (!m_logger)
    {
        m_fileName = "";
        return false;
    }
    return true;
}

void Logger::Close()
{
    if (!m_logger || m_fileName.empty()) return;

    fclose(m_logger);
    m_logger = 0;
    m_fileName = "";
}

void Logger::Log(ELOG level, const char* format, ...)
{
    if (level == DEBUG && !ENABLE_DEBUG)
        return;

    int size = 100;
    std::string str;
    va_list ap;
    while (1)
    {
        str.resize(size);
        va_start(ap, format);
        int n = vsnprintf((char*)str.c_str(), size, format, ap);
        va_end(ap);
        if (n > -1 && n < size)
        {
            str.resize(n);
            break;
            //          return str;
        }
        if (n > -1)
            size = n + 1;
        else
            size *= 2;
    }

    if (m_lastError == str)
    {
        m_lastErrorCount++;
        return;
    }
    else if (m_lastErrorCount)
    {
        if (m_logger)
            fprintf(m_logger, "Last message repeated %d times\n", m_lastErrorCount);
        else
            printf("Last message repeated %d times\n", m_lastErrorCount);
        m_lastErrorCount=0;
    }

    m_lastError = str;

    if (level != CAT)
    {
        char heure[12];
        heure[0] = 0;
        time_t time_of_day = time(0);
        strftime(heure, 12, "%H:%M:%S ", localtime(&time_of_day));

        str = string(heure) + LogToStr(level) + str + '\n';
    }

//  if (level == DEBUG && !ENABLE_DEBUG)
//      kDebug() << QString("X10 - %1").arg(str.c_str());
//  else

    if (m_logger)
        fprintf(m_logger, "X10 - %s", str.c_str());
    else
        printf("X10 - %s", str.c_str());

    if (level != CAT)
        m_logger ? fflush(m_logger) : fflush(stdout);

//     if (level == ERROR)
//         SetLastError(str);
}

std::string Logger::string_format(const std::string& fmt, ...)
{
    int size = 100;
    std::string str;
    va_list ap;
    while (1)
    {
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf((char*)str.c_str(), size, fmt.c_str(), ap);
        va_end(ap);
        if (n > -1 && n < size)
        {
            str.resize(n);
            return str;
        }
        if (n > -1)
            size = n + 1;
        else
            size *= 2;
    }
}
