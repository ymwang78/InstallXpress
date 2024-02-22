#ifndef UTILITY_H_
#define UTILITY_H_

#if defined(UTILITY_EXPORTS)
# define UTILITY_API __declspec(dllexport)
#else
# define UTILITY_API __declspec(dllimport)
#endif

#define MARKUP_STL

#include "DirUtility.h"
#include "log.h"
#include "Markup.h"
#include "TypeConvertUtil.h"
#include "RegOperation.h"
#include "IGuidWindow.h"
#include "IniOperation.h"
#include "Md5.h"
#include "GeneralFormat.h"
#include "ShareMemory.h"
#include "Chinesetopy/ChineseToPy.h"
#include "ComputerInfo.h"
#endif
