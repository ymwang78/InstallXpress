#include "stdafx.h"
#include "IGuidWindow.h"

#include <atlbase.h>
#include <map>
using namespace std;

map<unsigned, IWindow*> m_vAllManger;
////////////////////////////////////////////////////////////////////////////////////////////////////
IGuidWindow::IGuidWindow(unsigned int uWindowId, unsigned int uUin, unsigned belongid)
	: m_belongid(belongid)
	, m_uWindowTypeId(uWindowId)
	, m_uUin(uUin)
{
	map<unsigned, IWindow*>::iterator it= m_vAllManger.find(m_belongid);
	if (it != m_vAllManger.end())
	{
		IWindow* pWindow = it->second;
		if (pWindow) pWindow->m_WindowList.push_back(this);
	}
}


IGuidWindow::~IGuidWindow()
{
	map<unsigned, IWindow*>::iterator it = m_vAllManger.find(m_belongid);
	if (it != m_vAllManger.end())
	{
		IWindow* pWindow = it->second;
		if (pWindow) pWindow->m_WindowList.remove(this);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
IWindow::IWindow(unsigned belongid)
	:m_belongid(belongid)
{
	m_WindowList.clear();	
	m_vAllManger.insert(make_pair(m_belongid, this));
}

IWindow::~IWindow()
{
	map<unsigned, IWindow*>::iterator it = m_vAllManger.find(m_belongid);
	if (it != m_vAllManger.end())
	{
		m_vAllManger.erase(it);
	}
}

IGuidWindow* IWindow::FindDuiWindow(unsigned int uWindowId, unsigned int uUin)
{
	for (PtrList::iterator it = m_WindowList.begin(); it != m_WindowList.end(); ++it)
	{
		IGuidWindow *pWindow = (IGuidWindow *)*it;
		if (ATL::AtlIsValidAddress(pWindow, sizeof(pWindow)))
		{
			if (pWindow->m_uWindowTypeId == uWindowId && (uUin == 0 || pWindow->m_uUin == uUin))
				return pWindow;
		}
		else
		{
			m_WindowList.erase(it);
		}			
	}
	return NULL;
}