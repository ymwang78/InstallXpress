// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO: reference additional headers your program requires here
#define	 UILIB_API
#include "DuiLib/UIlib.h"
using namespace DuiLib;

#include <string>
using namespace std;

#ifndef PRODVER					  //版本号;
#define PRODVER L"4.1.0.0"
#endif

#define COMPANYNAME L"TaijiControl"	//公司名称;
#define PRODUCTNAME L"TaiJiMPC"	//产品名称;
#define APPEXENAME  L"TaiJiMPC.exe" //主程序名称;