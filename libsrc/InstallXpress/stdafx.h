#pragma once
// WIN7 SP1, 2023.11.7
#define _WIN32_WINNT	0x0601

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string>

#define UILIB_API
// TODO: reference additional headers your program requires here
#include "DuiLib/UIlib.h"
using namespace DuiLib;
