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

#ifndef PRODVER					  //�汾��;
#define PRODVER L"1.0.0.0"
#endif

#define COMPANYNAME L"ZJU"
#define PRODUCTNAME L"RTO"
#define APPEXENAME  L"xRto.exe"
