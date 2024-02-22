#include "StdAfx.h"
#include "UILabelIcon.h"

#include <atlconv.h>
namespace DuiLib
{
	CLabelIconUI::CLabelIconUI(void)
	{
		m_hIcon = NULL;
		m_strForegImage = _T("");
	}

	CLabelIconUI::~CLabelIconUI(void)
	{
		if (m_hIcon)
		{
			::DestroyIcon(m_hIcon);
		}
	}

	LPCTSTR CLabelIconUI::GetClass() const
	{
		return _T("LabelIconUI");
	}

	LPVOID CLabelIconUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcscmp(pstrName, _T("LabelIcon")) == 0) return this;
		return CLabelUI::GetInterface(pstrName);
	}

	void CLabelIconUI::PaintBkImage(HDC hDC)
	{
		if (!m_pManager) return;

		if (m_strForegImage.IsEmpty() == false)
		{
			if (!DrawImage(hDC, (LPCTSTR)m_strForegImage)) m_strForegImage.Empty();
			return;
		}

		if (m_sIconImage.IsEmpty())
			return;

		if (!m_hIcon)
		{			
			m_hIcon = LoadIconImage(m_sIconImage, m_rcItem.right - m_rcItem.left, m_rcItem.bottom - m_rcItem.top);
		}
		if (m_hIcon)
		{
			DrawIconEx(hDC, m_rcItem.left, m_rcItem.top, m_hIcon, m_rcItem.right - m_rcItem.left,
				m_rcItem.bottom - m_rcItem.top, 0, NULL, DI_NORMAL);
		}
	}

	void CLabelIconUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if (_tcscmp(pstrName, _T("bkimage")) == 0) SetBkImage(pstrValue);
		else CLabelUI::SetAttribute(pstrName, pstrValue);
	}

	void CLabelIconUI::SetBkImage(LPCTSTR szImage)
	{
		m_sIconImage = szImage;
		Invalidate();
	}

	void CLabelIconUI::SetForegImage(LPCTSTR szImage)
	{
		m_strForegImage = szImage;
		Invalidate();
	}

	CDuiString CLabelIconUI::GetBkImage()const
	{
		return m_sIconImage;
	}

	HICON CLabelIconUI::LoadIconImage(LPCTSTR bitmap, int cx, int cy)
	{
		LPBYTE pData = NULL;
		DWORD dwSize = 0;
		CDuiString bitmapfile = ParsingIconImageString(bitmap);
		do
		{
			CDuiString sFile = CPaintManagerUI::GetResourcePath();
			if (CPaintManagerUI::GetResourceZip().IsEmpty())
			{
				sFile += bitmapfile;
				if (FileExist(sFile))
				{
					HICON icobitmap = (HICON)::LoadImage(NULL, sFile, IMAGE_ICON, cx, cy, LR_LOADFROMFILE);
					return icobitmap;
				}
				else
				{
					break;
				}
				
			}
			
		} while (0);

		if (FileExist(bitmapfile))
		{
			HICON icobitmap = (HICON)::LoadImage(NULL, bitmapfile, IMAGE_ICON, cx, cy, LR_LOADFROMFILE);
			return icobitmap;
		}
		return NULL;
	}

	CDuiString CLabelIconUI::ParsingIconImageString(LPCTSTR pStrImage)
	{
		CDuiString sImageName = pStrImage;
		CDuiString sItem;
		CDuiString sValue;
		LPTSTR pstr = NULL;
		while (*pStrImage != _T('\0'))
		{
			sItem.Empty();
			sValue.Empty();
			while (*pStrImage > _T('\0') && *pStrImage <= _T(' ')) pStrImage = ::CharNext(pStrImage);
			while (*pStrImage != _T('\0') && *pStrImage != _T('=') && *pStrImage > _T(' '))
			{
				LPTSTR pstrTemp = ::CharNext(pStrImage);
				while (pStrImage < pstrTemp)
				{
					sItem += *pStrImage++;
				}
			}
			while (*pStrImage > _T('\0') && *pStrImage <= _T(' ')) pStrImage = ::CharNext(pStrImage);
			if (*pStrImage++ != _T('=')) break;
			while (*pStrImage > _T('\0') && *pStrImage <= _T(' ')) pStrImage = ::CharNext(pStrImage);
			if (*pStrImage++ != _T('\'')) break;
			while (*pStrImage != _T('\0') && *pStrImage != _T('\''))
			{
				LPTSTR pstrTemp = ::CharNext(pStrImage);
				while (pStrImage < pstrTemp)
				{
					sValue += *pStrImage++;
				}
			}
			if (*pStrImage++ != _T('\'')) break;
			if (!sValue.IsEmpty())
			{
				if (sItem == _T("file") || sItem == _T("res"))
				{
					sImageName = sValue;
				}
			}
			if (*pStrImage++ != _T(' ')) break;
		}

		return sImageName;
	}

	bool CLabelIconUI::FileExist(LPCTSTR strPath)
	{
		WIN32_FIND_DATA wfd;
		bool rValue = false;
		HANDLE hFind = FindFirstFile(strPath, &wfd);
		if ((hFind != INVALID_HANDLE_VALUE))
		{
			rValue = true;
		}
		FindClose(hFind);
		return rValue;
	}
}