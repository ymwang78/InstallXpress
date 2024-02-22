/*******************************************************
* 文件描叙:日志类;
* 日期:2015-09-05
*******************************************************/
#ifndef _LOG_H
#define _LOG_H

#include <stdio.h>

#define APPLOG(level)	if (level <= Clog::logLevel)  Clog::print

enum Log
{
	ERRORLOG = 1,
	GRENALLOG,
	SUCCESSLOG,
};

class /*UTILITY_API*/ Clog
{
public:
	Clog();
	~Clog();

public:
	static void open(const wchar_t*file, int level);
	static void closeLog();
	static void print(const char *fmt, ...);
	static void SetLogLevel(int level);
	static int logLevel;

private:
	static FILE *logFile;
};



#endif