/***********************************************
* 文件描叙: 窗口管理;
* 文件名: IGuidWindow.h;
* 时间:   20015-09-06;
***********************************************/
#ifndef IGUIDWINDOW_H_
#define IGUIDWINDOW_H_

#include <list>
using namespace std;

typedef list<void *>	PtrList;

class IGuidWindow
{
public:
	IGuidWindow(unsigned int uWindowId,unsigned int uUin, unsigned guid);
	virtual ~IGuidWindow();

public:
	unsigned int m_uWindowTypeId;
	unsigned int m_uUin;
	unsigned int m_belongid;
};

class IWindow
{
public:
	IWindow(unsigned belongid);
	virtual ~IWindow();

public:
	IGuidWindow* FindDuiWindow(unsigned int uWindowId, unsigned int uUin = 0);
		
public:
	PtrList m_WindowList;
	unsigned m_belongid;
};

#endif // IGUIDWINDOW_H_