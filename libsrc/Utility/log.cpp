#include "log.h"
#include "StdAfx.h"
#include "SyncMutex.h"
#include <time.h>   
#include <stdarg.h>
#include <string>
#include <tchar.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <wchar.h>
#include <stdio.h>
#pragma comment(lib,"shlwapi.lib")

using namespace std;

FILE * Clog::logFile = NULL;
int Clog::logLevel = SUCCESSLOG;

Clog::Clog()
{
}


Clog::~Clog()
{
}


void Clog::open(const wchar_t *file, int level)
{
	AutoMutex automutex;

	if (logFile)
		closeLog();

	wchar_t path[MAX_PATH] = { 0 };
	SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path);
	wstring strappdatadir = path;

    if (!PathIsDirectory(strappdatadir.c_str()))
        ::CreateDirectory(strappdatadir.c_str(), NULL);

	strappdatadir += file;

	logFile = _wfopen(strappdatadir.c_str(), L"wb+");

	if (NULL == logFile) return;

	logLevel = level;
}

void Clog::closeLog()
{
	AutoMutex automutex;

	if (logFile)
		fclose(logFile); //关闭文件;
	logFile = NULL;
}

void Clog::SetLogLevel(int level)
{
	logLevel = level;
}

void Clog::print(const char *fmt, ...)
{
	AutoMutex automutex;

	if (NULL == logFile) return;

	char log_text[2048];//日志内容;
	memset(log_text, 0, sizeof(log_text));

	va_list v1;
	va_start(v1, fmt);
	_vsnprintf(log_text, sizeof(log_text), fmt, v1);
	va_end(v1);

	char log_data[128] = { 0 }; //日期;
	SYSTEMTIME st;
	GetLocalTime(&st);
	sprintf(log_data,"%d/%d/%d %02d:%02d:%02d", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond);
	
	char log_content[4096];
	memset(log_content, 0, sizeof(log_content));
	sprintf(log_content, " %s ---%s \n", log_data, log_text);
	if (logFile)
	{
		fwrite(log_content, 1, strlen(log_content), logFile);
		fflush(logFile);
	}
}