/*
 *	Code in this file is a slightly modified version of the code
 *	available in this article :
 *
 *		http://www.drdobbs.com/cpp/logging-in-c/201804215
 *
 *	The code published in this article can be used
 *		« w/o any restrictions ».
 *	See the following mail in doc/ directory :
 *		Re: Logging In C++ Dr. Dobb's article.txt
 *
 *
 *	Modifications of the original code are pusblished
 *	under the GNU GPLv3.
 *
 */

/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2017  Fabrice Delliaux <netbox253@gmail.com>
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// http://stackoverflow.com/questions/5028302/small-logger-class


#ifndef __GLOGIK_LOG_H__
#define __GLOGIK_LOG_H__

#if !defined (UTILS_INSIDE_UTILS_H) && !defined (UTILS_COMPILATION)
#error "Only "utils/utils.h" can be included directly, this file may disappear or change contents."
#endif

#include <sstream>
#include <string>
#include <cstdio>
#include <iomanip>

#include <boost/iostreams/device/null.hpp>
#include <boost/iostreams/stream.hpp>

#include <config.h>

namespace GLogiK
{

#if DEBUGGING_ON == 0

static boost::iostreams::null_sink nullsink;
static boost::iostreams::stream<boost::iostreams::null_sink> nullout( nullsink );
#define LOG(level) nullout

#else

inline std::string NowTime();

enum TLogLevel {NONE, ERROR, WARNING, INFO, DEBUG, DEBUG1, DEBUG2, DEBUG3, DEBUG4, DEBUG5};

template <typename T>
class Log
{
public:
    Log();
    virtual ~Log();
    std::ostringstream& Get(TLogLevel level = INFO, const char* func = "");
public:
    static TLogLevel& ReportingLevel();
    static std::string ToString(TLogLevel level);
    static TLogLevel FromString(const std::string& level);
protected:
    std::ostringstream os;
private:
    Log(const Log&);
    Log& operator =(const Log&);
};

template <typename T>
Log<T>::Log()
{
}

template <typename T>
std::ostringstream& Log<T>::Get(TLogLevel level, const char* func)
{
	os << "- " << NowTime();
	os << " " << std::setfill(' ') << std::setw(32) << func;
	os << " " << std::setfill(' ') << std::setw(8) << ToString(level) << ":   ";
	os << std::string(level > DEBUG ? level - DEBUG : 0, '\t');
	return os;
}

template <typename T>
Log<T>::~Log()
{
    os << std::endl;
    T::Output(os.str());
}

template <typename T>
TLogLevel& Log<T>::ReportingLevel()
{
    static TLogLevel reportingLevel = DEBUG4;
    return reportingLevel;
}

template <typename T>
std::string Log<T>::ToString(TLogLevel level)
{
	static const char* const buffer[] = {"NONE", "ERROR", "WARNING", "INFO", "DEBUG", "DEBUG1", "DEBUG2", "DEBUG3", "DEBUG4", "DEBUG5"};
    return buffer[level];
}

template <typename T>
TLogLevel Log<T>::FromString(const std::string& level)
{
    if (level == "DEBUG5")
        return DEBUG5;
    if (level == "DEBUG4")
        return DEBUG4;
    if (level == "DEBUG3")
        return DEBUG3;
    if (level == "DEBUG2")
        return DEBUG2;
    if (level == "DEBUG1")
        return DEBUG1;
    if (level == "DEBUG")
        return DEBUG;
    if (level == "INFO")
        return INFO;
    if (level == "WARNING")
        return WARNING;
    if (level == "ERROR")
        return ERROR;
    if (level == "NONE")
        return NONE;
    Log<T>().Get(WARNING, __func__) << "Unknown logging level '" << level << "'. Using NONE level as default.";
    return NONE;
}

class LOG2FILE
{
public:
    static FILE*& Stream();
    static void Output(const std::string& msg);
};

inline FILE*& LOG2FILE::Stream()
{
    static FILE* pStream = stderr;
    return pStream;
}

inline void LOG2FILE::Output(const std::string& msg)
{   
    FILE* pStream = Stream();
    if (!pStream)
        return;
    fprintf(pStream, "%s", msg.c_str());
    fflush(pStream);
}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#   if defined (BUILDING_FILELOG_DLL)
#       define FILELOG_DECLSPEC   __declspec (dllexport)
#   elif defined (USING_FILELOG_DLL)
#       define FILELOG_DECLSPEC   __declspec (dllimport)
#   else
#       define FILELOG_DECLSPEC
#   endif // BUILDING_DBSIMPLE_DLL
#else
#   define FILELOG_DECLSPEC
#endif // _WIN32

class FILELOG_DECLSPEC FILELog : public Log<LOG2FILE> {};
//typedef Log<LOG2FILE> FILELog;

#ifndef FILELOG_MAX_LEVEL
#define FILELOG_MAX_LEVEL DEBUG4
#endif

#define LOG(level) \
    if (level > FILELOG_MAX_LEVEL) ;\
    else if (level > FILELog::ReportingLevel() || !LOG2FILE::Stream()) ; \
    else FILELog().Get(level, __func__)

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)

#include <windows.h>

inline std::string NowTime()
{
    const int MAX_LEN = 200;
    char buffer[MAX_LEN];
    if (GetTimeFormatA(LOCALE_USER_DEFAULT, 0, 0, 
            "HH':'mm':'ss", buffer, MAX_LEN) == 0)
        return "Error in NowTime()";

    char result[100] = {0};
    static DWORD first = GetTickCount();
    std::sprintf(result, "%s.%03ld", buffer, (long)(GetTickCount() - first) % 1000); 
    return result;
}

#else

#include <sys/time.h>

inline std::string NowTime()
{
    char buffer[11];
    time_t t;
    time(&t);
    tm r = {0};
    strftime(buffer, sizeof(buffer), "%X", localtime_r(&t, &r));
    struct timeval tv;
    gettimeofday(&tv, 0);
    char result[100] = {0};
    std::sprintf(result, "%s.%03ld", buffer, (long)tv.tv_usec / 1000); 
    return result;
}

#endif //WIN32

#endif // ifndef DEBUGGING_ON

} // namespace GLogiK

#endif //__GLOGIKD_LOG_H__
