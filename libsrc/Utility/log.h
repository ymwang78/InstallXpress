/*******************************************************
* 文件描叙:日志类;
* 日期:2015-09-05
*******************************************************/
#ifndef _LOG_H
#define _LOG_H

#include <stdio.h>

#define APPLOG(level)	if (level >= Clog::logLevel)  Clog::print

enum Log
{
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFOR,
    LOG_WARNI,
	LOG_ERROR,
    LOG_CRITI
};

class /*UTILITY_API*/ Clog
{
public:
	Clog();
	~Clog();

public:
	static void open(const wchar_t* company, const wchar_t*file, int level);
	static void closeLog();
	static void print(const char *fmt, ...);
	static void SetLogLevel(int level);
	static int logLevel;

private:
	static FILE *logFile;
};



#endif