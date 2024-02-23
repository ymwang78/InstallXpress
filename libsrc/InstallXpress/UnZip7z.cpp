#include "stdafx.h"
#include "UnZip7z.h"
#include <Shlwapi.h>
#include "Utility/TypeConvertUtil.h"
#include "Utility/log.h"
#include "7ZLookInStream.h"
#include "Utility/ResourceHandler.h"
#include <shellapi.h>
#pragma comment(lib, "shell32.lib")

extern "C"
{
#include "7Z/7z.h"
#include "7Z/7zAlloc.h"
#include "7Z/7zCrc.h"
#include "7Z/7zFile.h"
#include "7Z/7zVersion.h"
}

extern "C" void ExecuteProcess(const wchar_t* cmd, bool hidden, int wait_second);

CUnZip7z::CUnZip7z()
{
}


CUnZip7z::~CUnZip7z()
{
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 

int CUnZip7z::cat_path(LPTSTR lpszPath, LPCWSTR lpSubPath)
{
	PathAppend(lpszPath, lpSubPath);
	return 0;
}

int CUnZip7z::save_file(LPCTSTR lpszFile, const void * lpBuf, DWORD dwSize)
{
	int nRet = 0;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	if (0 == nRet)
	{
		hFile = CreateFile(lpszFile,GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			nRet = 1;
		}
	}
	if (0 == nRet)
	{
		DWORD dwWrite = 0;

		WriteFile(hFile, lpBuf, dwSize, &dwWrite, 0);

		if (dwWrite != dwSize)
		{
			nRet = 1;
		}
	}
	if (INVALID_HANDLE_VALUE != hFile)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
	return nRet;
}

int CUnZip7z::getunzipfilenum(ResourceHandler* resHandler)
{
	int nsize = 0;
	CSzArEx db;
	SRes res;
	ISzAlloc allocImp;
	ISzAlloc allocTempImp;
	UInt16 *temp = NULL;
	size_t tempSize = 0;
	CMem7zLookInStream lookIn{ 0 };

	InitMem7zLookInStream(&lookIn, (const BYTE*)resHandler->GetData(), resHandler->GetSize());

	////////////////////////////////////////////////////////////  
	// init  
	allocImp.Alloc = SzAlloc;
	allocImp.Free = SzFree;

	allocTempImp.Alloc = SzAllocTemp;
	allocTempImp.Free = SzFreeTemp;

	CrcGenerateTable();

	SzArEx_Init(&db);

	res = SzArEx_Open(&db, &lookIn.vtbl, &allocImp, &allocTempImp);

	// 如果没有打开成功,就不会有文件; 
	if (SZ_OK == res) {
		nsize = db.NumFiles;
	}

	SzArEx_Free(&db, &allocImp);
	SzFree(NULL, temp);

	return nsize;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
int CUnZip7z::unzip_7z_file(ResourceHandler* resHandler, std::wstring &mUnPackPath, HWND callback, UINT Msg)
{
	int nRet = 0;
	LPCTSTR lpszOutputPath = mUnPackPath.c_str();
	//如果解压缩的路径不存在 试图创建它 ;
	if (!FolderExist(mUnPackPath))
	{
		//解压后存放的文件夹不存在 创建它 ;
		//@todo 请求提权
		if (FALSE == CreatedMultipleDirectory(mUnPackPath))
		{
			APPLOG(Log::ERRORLOG)("\n---unzip_7z_file: %s create faile ---\n", WtoS(mUnPackPath).c_str());
			//创建目录失败 ;
			return FALSE;
		}
	}

	CSzArEx db;
	SRes res;
	ISzAlloc allocImp;
	ISzAlloc allocTempImp;
	UInt16 *temp = NULL;
	size_t tempSize = 0;

    CMem7zLookInStream lookIn{ 0 };

    InitMem7zLookInStream(&lookIn, (const BYTE*)resHandler->GetData(), resHandler->GetSize());

	////////////////////////////////////////////////////////////  
	// init  
	allocImp.Alloc = SzAlloc;
	allocImp.Free = SzFree;

	allocTempImp.Alloc = SzAllocTemp;
	allocTempImp.Free = SzFreeTemp;

	CrcGenerateTable();

	SzArEx_Init(&db);

	res = SzArEx_Open(&db, &lookIn.vtbl, &allocImp, &allocTempImp);

	// 如果没有打开成功,就不会有文件; 
	if (SZ_OK == nRet)
	{	
		UInt32 blockIndex = 0xFFFFFFFF; // it can have any value before first call (if outBuffer = 0)   
		Byte *outBuffer = 0;            // it must be 0 before first call for each new archive.   
		size_t outBufferSize = 0;       // it can have any value before first call (if outBuffer = 0)   

		for (unsigned int i = 0; i < db.NumFiles; ++i)
		{
			size_t offset = 0;
			size_t outSizeProcessed = 0;
			size_t len;

			unsigned isDir = SzArEx_IsDir(&db, i);
			len = SzArEx_GetFileNameUtf16(&db, i, NULL);

			if (len > tempSize)
			{
				SzFree(NULL, temp);
				tempSize = len;
				temp = (UInt16 *)SzAlloc(NULL, tempSize * sizeof(temp[0]));
				if (!temp)
				{
					APPLOG(Log::ERRORLOG)("\n---unzip_7z_file: mem error ---\n");
					res = SZ_ERROR_MEM;
					break;
				}
			}
			SzArEx_GetFileNameUtf16(&db, i, temp);

			if (!isDir)
			{
				res = SzArEx_Extract(&db, &lookIn.vtbl, i, &blockIndex, &outBuffer, &outBufferSize,
					&offset, &outSizeProcessed, &allocImp, &allocTempImp);
				if (res != SZ_OK)
				{
					APPLOG(Log::ERRORLOG)("\n---unzip_7z_file: SzArEx_Extract error,error code : %d ---\n", res);
					break;
				}				
			}

			TCHAR szFile[MAX_PATH] = { 0 };
			_tcscpy_s(szFile, lpszOutputPath);

			CSzFile outFile;
			size_t processedSize;
			PathAppend(szFile, (LPCWSTR)temp);

			for (size_t j = 0; szFile[j] != 0; j++)
			{
				if ((szFile[j] == '/') || (szFile[j] == '\\'))
				{
					szFile[j] = 0;
					CreateDirectory(szFile, 0);
					szFile[j] = CHAR_PATH_SEPARATOR;
				}
			}
			if (isDir)
			{
				CreateDirectory(szFile, 0);
				PostMessage(callback, Msg, i, db.NumFiles);
				continue;
			}
			else 
			{
                //if (PathFileExists(szFile)) {
                //    BOOL bRet = DeleteFile(szFile);
                //    if (!bRet) {
                //        APPLOG(Log::ERRORLOG)("\n---unzip_7z_file: DeleteFile error, filename : %s ,last error:%lu---\n", WtoS(szFile).c_str(), GetLastError());
                //    }
                //}

				do {
					res = 0;
					//OutputDebugString(szFile);
					//OutputDebugString(L"\n");
                    DWORD dLastError = OutFile_OpenW(&outFile, szFile);
                    if (dLastError) {
						if (dLastError == 5) {
							//HASP会拒绝其他进程访问，先用SHELL去删除
                            CDuiString delCommand;
                            //delCommand.Format(L"del /f /s /q %s", szFile);
                            //_wsystem(delCommand);
							delCommand.Format(L"cmd /c del /F /Q %s", szFile);
                            ExecuteProcess(delCommand, true, -1);
							dLastError = OutFile_OpenW(&outFile, szFile);
						}
						if (dLastError) {
                            res = SZ_ERROR_FAIL;
                            APPLOG(Log::ERRORLOG)("\n---unzip_7z_file: OutFile_OpenW error, filename : %s ,last error:%lu---\n", WtoS(szFile).c_str(), dLastError);
                            CDuiString msg;
                            msg.Format(_T("写入文件:<%s>失败"), szFile);
                            int ret = MessageBox(NULL, msg, _T("错误提示"), MB_ABORTRETRYIGNORE);
                            if (ret == IDRETRY)
                            {
                                continue;
                            }
                            else if (ret == IDIGNORE) {
                                break;
                            }
                            else {
                                exit(0);
                            }
						}
                    }
				} while (res == SZ_ERROR_FAIL);
				if (res == SZ_ERROR_FAIL) {
                    processedSize = outSizeProcessed;
                    continue;
				}
			}
			processedSize = outSizeProcessed;

			if (File_Write(&outFile, outBuffer + offset, &processedSize) != 0 || processedSize != outSizeProcessed)
			{
				res = SZ_ERROR_FAIL;
				APPLOG(Log::ERRORLOG)("\n---unzip_7z_file: File_Write error --\n");
				break;
			}

			if (File_Close(&outFile))
			{
				res = SZ_ERROR_FAIL;
				APPLOG(Log::ERRORLOG)("\n---unzip_7z_file: File_Close error --\n");
				break;
			}

#ifdef USE_WINDOWS_FILE
			if (SzBitWithVals_Check(&db.Attribs, i))
				SetFileAttributesW(szFile, db.Attribs.Vals[i]);
#endif
			PostMessage(callback, Msg, i, db.NumFiles);
		}

		IAlloc_Free(&allocImp, outBuffer);
	}
	else
	{
		APPLOG(Log::ERRORLOG)("\n---unzip_7z_file: %u open zip faile ---\n", res);
	}
	SzArEx_Free(&db, &allocImp);
	SzFree(NULL, temp);

	return res;
}

///////////////////////////////////////////////////////////////////////////// 
// 函数说明: 检查指定的文件夹是否存在 ;
// 参数说明: [in]：strPath 检查的文件夹 (此方法会主动向路径末尾添加*.*) ;
// 返回值:BOOL类型,存在返回TRUE,否则为FALSE ;
///////////////////////////////////////////////////////////////////////////// 
BOOL CUnZip7z::FolderExist(std::wstring& strPath)
{
	std::wstring sCheckPath = strPath;

	if (sCheckPath[sCheckPath.length() - 1] != '\\')
	{
		sCheckPath.append(1, '\\');
	}
	sCheckPath += L"*.*";

	WIN32_FIND_DATA wfd;
	BOOL rValue = FALSE;

	HANDLE hFind = FindFirstFile(sCheckPath.c_str(), &wfd);

	if ((hFind != INVALID_HANDLE_VALUE) &&
		(wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) || (wfd.dwFileAttributes&FILE_ATTRIBUTE_ARCHIVE))
	{
		//如果存在并类型是文件夹 ;
		rValue = TRUE;
	}

	FindClose(hFind);
	return rValue;
}

///////////////////////////////////////////////////////////////////////////// 
// 函数说明: 创建多级目录 ;
// 参数说明: [in]： 路径字符串 ;
// 返回值: BOOL 成功True 失败False ;///////////////////////////////////////////////////////////////////////////// 
BOOL CUnZip7z::CreatedMultipleDirectory(std::wstring Directoryname)
{
	if (Directoryname[Directoryname.length() - 1] != '\\')
	{
		Directoryname.append(1, '\\');
	}
	std::vector< std::wstring> vpath;
	std::wstring strtemp;
	BOOL  bSuccess = FALSE;
	for (size_t i = 0; i < Directoryname.length(); i++)
	{
		if (Directoryname[i] != '\\')
		{
			strtemp.append(1, Directoryname[i]);
		}
		else
		{
			vpath.push_back(strtemp);
			strtemp.append(1, '\\');
		}
	}
	std::vector<std::wstring>::const_iterator vIter;
	for (vIter = vpath.begin();vIter != vpath.end(); vIter++)
	{
		bSuccess = CreateDirectory(vIter->c_str(), NULL) ? TRUE : FALSE;
	}

	return bSuccess;
}
