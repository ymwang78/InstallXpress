// stdafx.cpp : source file that includes just the standard includes
// Install.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
#pragma comment(lib, "duilib.lib")
#pragma comment(lib, "Utility.lib")
#pragma comment(lib, "lib7z.lib")
#pragma comment(lib, "liblua.lib")
#pragma comment(lib, "InstallXpress.lib")
#if SUPPORT_WEBP
#pragma comment(lib, "libwebp.lib")
#endif
