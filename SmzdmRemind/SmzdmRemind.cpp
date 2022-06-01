// SmzdmRemind.cpp : 定义应用程序的入口点。
//
#ifdef _WIN64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#include "framework.h"
#include "SmzdmRemind.h"
#include "shellapi.h"
#include "commctrl.h"
#include "psapi.h"

//typedef BOOL (WINAPI* pfnShowToast)(WCHAR* szTitle, WCHAR* szBody, WCHAR* szImagePath, WCHAR* szLink);
//pfnShowToast ShowToast;
#include "wintoastlib.h"
using namespace WinToastLib;
class CustomHandler : public IWinToastHandler {
public:
	WCHAR szLink[128];
	void toastActivated() const {
		ShellExecute(NULL, L"open", szLink, NULL, NULL, SW_SHOW);
		delete this;
	}
	void toastActivated(int actionIndex) const {
		//click button
		int t = actionIndex;
		if (t == 0)
			ShellExecute(NULL, L"open", szLink, NULL, NULL, SW_SHOW);
	}

	void toastDismissed(WinToastDismissalReason state) const {
		//switch state
	}

	void toastFailed() const {
	}
};
//初始化模板
void ShowToast(WCHAR * szTitle, WCHAR * szBody, WCHAR * szImagePath, WCHAR * szLink)
{
	WinToastTemplate templ(WinToastTemplate::ImageAndText01);
	templ.setTextField(szTitle, WinToastTemplate::FirstLine);
	templ.setAttributionText(szBody);
	templ.setAudioOption(WinToastTemplate::Default);
	templ.setImagePath(szImagePath);
	//    templ.setExpiration(3000);
	CustomHandler* nch = new CustomHandler();
	lstrcpy(nch->szLink, szLink);
	templ.addAction(L"打开");
	templ.addAction(L"关闭");
	WinToast::instance()->showToast(templ, nch);
	//	return templ;
}

wchar_t* lstrstr(const wchar_t* str, const wchar_t* sub)
{
	int i = 0;
	int j = 0;
	while (str[i] && sub[j])
	{
		if (str[i] == sub[j])//如果相等
		{
			++i;
			++j;
		}
		else		     //如果不等
		{
			i = i - j + 1;
			j = 0;
		}
	}
	if (!sub[j])
	{
		return (wchar_t*)&str[i - lstrlen(sub)];
	}
	else
	{
		return (wchar_t*)0;
	}
}
int winhttpDownload(WCHAR* wUrl, WCHAR* wFile)
{
    HANDLE hFile = CreateFile(wFile,  // creates a new file
        FILE_APPEND_DATA,         // open for writing
        0,          // allow multiple readers
        NULL,                     // no security
        CREATE_ALWAYS,            // creates a new file, always.
        FILE_ATTRIBUTE_NORMAL,    // normal file
        NULL);                    // no attr. template        
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return 2;
    }
	DWORD dwSize = 0;
	DWORD dwSumSize = 0;
	DWORD dwDownloaded = 0;
	DWORD dwBuffer = 0,
		dwBufferLength = sizeof(DWORD),
		dwIndex = 0;
	LPSTR pszOutBuffer;
    BOOL  bResults = FALSE;
    HINTERNET  hSession = NULL,
        hConnect = NULL,
        hRequest = NULL;
    // Use WinHttpOpen to obtain a session handle.
    hSession = WinHttpOpen(L"WinHTTP_Download Example/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);

    // Specify an HTTP server.
    //INTERNET_PORT nPort = (pGetRequest->fUseSSL) ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;
	WCHAR* wHost = lstrstr(wUrl, L":");
    if (wHost)
        wHost += 2;
    else
        wHost = wUrl;
    WCHAR* wUrlPath = lstrstr(wHost, L"/");    
    WCHAR szHost[128];
    lstrcpyn(szHost, wHost, wUrlPath - wHost+1);
    if (hSession)
        hConnect = WinHttpConnect(hSession, szHost,
            //hConnect = WinHttpConnect(hSession, L"avatar.csdn.net",
            INTERNET_DEFAULT_HTTPS_PORT, 0);
    // Create an HTTP request handle.
    if (hConnect)
        hRequest = WinHttpOpenRequest(hConnect, L"GET", wUrlPath,
            L"HTTP/1.1", WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            WINHTTP_FLAG_SECURE);

    // Send a request.
    if (hRequest)
        bResults = WinHttpSendRequest(hRequest,
            WINHTTP_NO_ADDITIONAL_HEADERS,
            0, WINHTTP_NO_REQUEST_DATA, 0,
            0, 0);

    // End the request.
    if (bResults)
        bResults = WinHttpReceiveResponse(hRequest, NULL);
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER, NULL, &dwBuffer, &dwBufferLength, &dwIndex);
    // Continue to verify data until there is nothing left.
    if (bResults)
    {
        do
        {
            // Verify available data.
            dwSize = 0;
            WinHttpQueryDataAvailable(hRequest, &dwSize);
            // Allocate space for the buffer.
            pszOutBuffer = new char[dwSize + (size_t)1];
            if (!pszOutBuffer)
            {
                dwSize = 0;
            }
            else
            {
                dwSumSize += dwSize;
                // Read the Data.
                ZeroMemory(pszOutBuffer, dwSize + (size_t)1);

                if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer,
                    dwSize, &dwDownloaded)) {
                }
                else {
                    WriteFile(hFile, pszOutBuffer, dwDownloaded, &dwDownloaded, NULL);
                }
                // Free the memory allocated to the buffer.
                delete[] pszOutBuffer;
            }

        } while (dwSize > 0);

        // Close files.
        CloseHandle(hFile);

        // Close open handles.
        if (hRequest) WinHttpCloseHandle(hRequest);
        if (hConnect) WinHttpCloseHandle(hConnect);
        if (hSession) WinHttpCloseHandle(hSession);
    }
    return 0;
}
#define NETPAGESIZE 2097152
#define MAX_LOADSTRING 100
#define WM_IAWENTRAY WM_USER+199
// 全局变量:
RTL_OSVERSIONINFOW rovi;//WIN系统版本号
WCHAR szAppName[] = L"SmzdmRemind";
HINSTANCE hInst;                                // 当前实例
HANDLE hMutex = NULL;
HMODULE hWintoast;
HWND hMain;
HWND hSetting;
HWND hList;
HWND hListRemind;
HWND hCombo;
HWND hComboTime;
HWND hComboPage;
HANDLE hMap = NULL;
HANDLE hGetDataThread;
NOTIFYICONDATA nid;
HICON iMain;
//HICON iTray;
BOOL bPost = FALSE;//社区文章
BOOL bOpen = FALSE;//程序第一次获取数据
BOOL bExit = FALSE;//退出线程
BOOL bResetTime = FALSE;//重新设置时间
BOOL bGetData = FALSE;//是否获取数据中
BOOL bNewTrayTips = FALSE;//新的通知样式
WCHAR szWxPusherToken[] = L"AT_YGOXF3ZtPSkz5lkxhFUZ5ZkHOgrkKSdG";
WCHAR szReminSave[] = L"SmzdmRemind.sav";
DWORD iTimes[] = { 1,3,5,10,15,30 };
WCHAR szTimes[][5] = {L"1分钟",L"3分钟",L"5分钟",L"10分钟",L"15分钟",L"30分钟"};
int mIDs[24] =          { 0,  183 ,   20057,  3949,       2537,       247,        241,      6753,               2897,       243,       8645,      257,       8912,        239,        4031,       20155,      269,           4033,           5108,       3981,        20383,      167 ,      153,    6255 };
WCHAR szBus[24][9] =    { L"" , L"京东",L"京喜",L"京东国际",L"天猫超市",L"天猫精选",L"聚划算",L"天猫国际官方直营",L"天猫国际",L"淘宝精选",L"拼多多",L"唯品会", L"小米有品", L"苏宁易购",L"苏宁国际", L"抖音电商",L"亚马逊中国",L"亚马逊海外购",L"网易严选", L"考拉海购",L"微信小程序",L"真快乐",L"当当",L"什么值得买" };
WCHAR szPage[9][2] = {L"1",L"2",L"3",L"4",L"5",L"6",L"7",L"8",L"9"};
// 此代码模块中包含的函数的前向声明:
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    MainProc(HWND, UINT, WPARAM, LPARAM);
typedef struct _REMINDITEM
{
    BOOL bNotUse;
    WCHAR szKey[128];
    WCHAR szFilter[128];
    UINT uMinPrice;
    UINT uMaxPrice;
    int iBusiness;
    UINT uid;//商品ID    
    FILETIME ft;
    WCHAR szID[10];
    BYTE dReserve[256-20];
}REMINDITEM;
REMINDITEM* lpRemindItem = NULL;
typedef struct _REMINDDATA
{
	BOOL bExit;
    WCHAR szWeChatToken[512];
}REMINDDATA;
REMINDDATA* lpRemindData;
DWORD riSize = 0;
typedef struct _REMINDSAVE
{
    BOOL bDirectly;
    BOOL bTips;
    BOOL bWxPusher;    
    BOOL bWeChat;
    int iTime;
    WCHAR szWxPusherUID[64];    
    WCHAR szWeChatAgentId[8];
    WCHAR szWeChatID[24];    
    WCHAR szWeChatSecret[48];
    WCHAR szWeChatUserID[64];
    int iPage;
    BOOL bDingDing;
    WCHAR szDingDingToken[80];
    BOOL bLastSort;//最新排序
    BOOL bPost;//社区文章
    BYTE dReserve[512-sizeof BOOL*3-160];
}REMINDSAVE;
REMINDSAVE RemindSave = {
    TRUE,
    FALSE,
    FALSE,
    FALSE,
    1,
    L"",
    L"",
    L"",
    L"",
    L"@all",
    1,
    FALSE,
    {0},
    FALSE,
    {0}
};
void SetToCurrentPath()////////////////////////////////////设置当前程序为当前目录
{
	WCHAR szDir[MAX_PATH];
	GetModuleFileName(NULL, szDir, MAX_PATH);
	int len = lstrlen(szDir);
	for (int i = len - 1; i > 0; i--)
	{
		if (szDir[i] == '\\')
		{
			szDir[i] = 0;
			SetCurrentDirectory(szDir);
			break;
		}
	}
}
BOOL bSort = TRUE;
int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    if (lParamSort == 2 || lParamSort == 4|| lParamSort == 5|| lParamSort == 6 || lParamSort == 7)
    {
		WCHAR sz1[129], sz2[129];
		ListView_GetItemText(hList, lParam1, lParamSort, sz1, 128);
		ListView_GetItemText(hList, lParam2, lParamSort, sz2, 128);
        float l1 = _wtof(sz1);
        float l2 = _wtof(sz2);
        if (l1 == l2)
            return 0;
        else if (l1 > l2)
        {
            if (bSort)
                return 1;
            else
                return -1;
        }
        else
        {
            if (bSort)
                return -1;
            else
                return 1;
        }
    }
    else
    {
        WCHAR sz1[129],sz2[129];
        ListView_GetItemText(hList, lParam1, lParamSort, sz1, 128);
        ListView_GetItemText(hList, lParam2, lParamSort, sz2, 128);
        int s = lstrcmp(sz1, sz2);
        if (bSort&&s!=0)
            s = -s;
        return s;
    }
}
BOOL AutoRun(BOOL GetSet, BOOL bAutoRun, const WCHAR* szName)//读取、设置开机启动、关闭开机启动
{
    BOOL ret = FALSE;
    WCHAR sFileName[MAX_PATH];
    sFileName[0] = L'\"';
    GetModuleFileName(NULL, &sFileName[1], MAX_PATH);
    int sLen = lstrlen(sFileName);
    sFileName[sLen] = L'\"';
    sFileName[sLen + 1] = L' ';
    sFileName[sLen + 2] = L't';
    sFileName[sLen + 3] = L'\0';
    HKEY pKey;
    RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", NULL, KEY_ALL_ACCESS, &pKey);
    if (pKey)
    {
        if (GetSet)
        {
            if (bAutoRun)
            {
                RegSetValueEx(pKey, szName, NULL, REG_SZ, (BYTE*)sFileName, (DWORD)lstrlen(sFileName) * 2);
            }
            else
            {
                RegDeleteValue(pKey, szName);
            }
            ret = TRUE;
        }
        else
        {
            WCHAR nFileName[MAX_PATH];
            DWORD cbData = MAX_PATH * sizeof WCHAR;
            DWORD dType = REG_SZ;
            if (RegQueryValueEx(pKey, szName, NULL, &dType, (LPBYTE)nFileName, &cbData) == ERROR_SUCCESS)
            {
                if (lstrcmp(sFileName, nFileName) == 0)
                    ret = TRUE;
                else
                    ret = FALSE;
            }
        }
        RegCloseKey(pKey);
    }
    return ret;
}
void UrlUTF8(WCHAR* wstr,WCHAR * wout)
{
    char szUtf8[1024] = { 0 };
    char szout[1024]={0};
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, szUtf8, 1024, NULL, NULL);
	int len = strlen(szUtf8);
	for (int i = 0; i < len; i++)
	{
		if (isalnum((BYTE)szUtf8[i])) //判断字符中是否有数组或者英文
		{
			char tempbuff[2] = { 0 };
			wsprintfA(tempbuff, "%c", (BYTE)szUtf8[i]);
			strcat_s(szout,tempbuff);
		}
		else if (isspace((BYTE)szUtf8[i]))
		{
			strcat_s(szout,"+");
		}
		else
		{
			char tempbuff[4];
			wsprintfA(tempbuff, "%%%X%X", ((BYTE)szUtf8[i]) >> 4, ((BYTE)szUtf8[i]) % 16);
			strcat_s(szout,tempbuff);
		}
	}
    ::MultiByteToWideChar(CP_UTF8, NULL, szout, 1024, wout, 1024);
}
BOOL SetForeground(HWND hWnd)//激活窗口为前台
{
    bool bResult = false;
    bool bHung = IsHungAppWindow(hWnd) != 0;
    DWORD dwCurrentThreadId = 0, dwTargetThreadId = 0;
    DWORD dwTimeout = 0;

    dwCurrentThreadId = GetCurrentThreadId();
    dwTargetThreadId = GetWindowThreadProcessId(hWnd, NULL);

    if (IsIconic(hWnd)) {
        //		ShowWindow(hWnd,SW_RESTORE);
        SendMessage(hWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
    }

    if (!bHung) {
        for (int i = 0; i < 10 && hWnd != GetForegroundWindow(); i++) {
            dwCurrentThreadId = GetCurrentThreadId();
            dwTargetThreadId = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
            AttachThreadInput(dwCurrentThreadId, dwTargetThreadId, true);
            SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            BringWindowToTop(hWnd);
            AllowSetForegroundWindow(ASFW_ANY);
            bResult = SetForegroundWindow(hWnd) != 0;
            AttachThreadInput(dwCurrentThreadId, dwTargetThreadId, false);
            Sleep(10);
        }
    }
    else {
        BringWindowToTop(hWnd);
        bResult = SetForegroundWindow(hWnd) != 0;
    }
    return bResult;
    /*
        int tIdCur = GetWindowThreadProcessId(GetForegroundWindow(), NULL);//获取当前窗口句柄的线程ID
        int tIdCurProgram = GetWindowThreadProcessId(hWnd,NULL);//获取当前运行程序线程ID
        BOOL ret=AttachThreadInput(tIdCur, tIdCurProgram, 1);//是否能成功和当前自身进程所附加的输入上下文有关;
        SetForegroundWindow(hWnd);
        AttachThreadInput(tIdCur, tIdCurProgram, 0);
        return ret;
    */
}
BOOL RunProcess(LPTSTR szExe, const WCHAR* szCommandLine, HANDLE* pProcess)/////////////////////////////////运行程序
{
	BOOL ret = FALSE;
	STARTUPINFO StartInfo;
	PROCESS_INFORMATION procStruct;
	memset(&StartInfo, 0, sizeof(STARTUPINFO));
	StartInfo.cb = sizeof(STARTUPINFO);
	WCHAR* sz;
	WCHAR szName[MAX_PATH];
	if (szExe == (LPTSTR)1)
		sz = NULL;
	else if (szExe)
		sz = szExe;
	else
	{
		GetModuleFileName(NULL, szName, MAX_PATH);
		sz = szName;
	}
	WCHAR szLine[MAX_PATH];
	szLine[0] = L'\0';
	if (szCommandLine)
		lstrcpy(szLine, szCommandLine);
	ret = CreateProcess(sz,// RUN_TEST.bat位于工程所在目录下
		szLine,
		NULL,
		NULL,
		FALSE,
		NULL,// 这里不为该进程创建一个控制台窗口
		NULL,
		NULL,
		&StartInfo, &procStruct);
	if (pProcess == NULL)
		CloseHandle(procStruct.hProcess);
	else
		*pProcess = procStruct.hProcess;
	CloseHandle(procStruct.hThread);
	//	SetTimer(hMain, 11, 1000, NULL);
	return ret;
}
void EmptyProcessMemory(DWORD pID)
{
	HANDLE hProcess;
	if (pID == NULL)
		hProcess = GetCurrentProcess();
	else
	{
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pID);
	}
	SetProcessWorkingSetSize(hProcess, -1, -1);
	EmptyWorkingSet(hProcess);
}
void ReadSet()
{
	SetToCurrentPath();
	HANDLE hFile = CreateFile(szReminSave, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE, NULL);
    if (hFile!= INVALID_HANDLE_VALUE)
    {
        DWORD dwBytes;
        ReadFile(hFile, &RemindSave, sizeof RemindSave, &dwBytes, NULL);

        riSize = GetFileSize(hFile, NULL) - sizeof RemindSave;
        if (riSize)
        {            
            if (lpRemindItem != NULL)
                delete[]lpRemindItem;
            lpRemindItem = (REMINDITEM*)new BYTE[riSize];
            ReadFile(hFile, lpRemindItem, riSize, &dwBytes, NULL);
        }
        CloseHandle(hFile);
    }

}
void WriteSet(REMINDITEM*lpRI)
{
	SetToCurrentPath();
    if (lpRI)
    {
        HANDLE hFile = CreateFile(szReminSave, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
			DWORD dwBytes = NULL;
			WriteFile(hFile, &RemindSave, sizeof RemindSave, &dwBytes, NULL);
            SetFilePointer(hFile, 0, NULL, FILE_END);
            WriteFile(hFile, lpRI, sizeof REMINDITEM, &dwBytes, NULL);
            CloseHandle(hFile);
        }
    }
    else
    {
        HANDLE hFile = CreateFile(szReminSave, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
        if (hFile!= INVALID_HANDLE_VALUE)
        {
            DWORD dwBytes = NULL;
            WriteFile(hFile, &RemindSave, sizeof RemindSave, &dwBytes, NULL);
            int n = riSize / sizeof REMINDITEM;
            for (int i = 0; i < n; i++)
            {
                if (lpRemindItem[i].szKey[0] != L'\0')
                    WriteFile(hFile, &lpRemindItem[i], sizeof REMINDITEM, &dwBytes, NULL);
            }
            CloseHandle(hFile);

        }
    }
}
/*

BOOL SendServerJ(wchar_t* SendKey, wchar_t* szTitle, wchar_t* szContent, wchar_t* szUrl, float fPrice, wchar_t* szBusiness, wchar_t* szImg)
{
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    BOOL  bResults = FALSE;
    HINTERNET  hSession = NULL, hConnect = NULL, hRequest = NULL;
    // Use WinHttpOpen to obtain a session handle.
    hSession = WinHttpOpen(L"WxPusher", WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, NULL);
    // Specify an HTTP server.
    if (hSession)
        hConnect = WinHttpConnect(hSession, L"sctapi.ftqq.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
    WCHAR szGet[1024] = L"/";
    lstrcat(szGet,L"SCT152372TQlJTExCGuU63HYmj8Uargtjb");
    lstrcat(szGet,L".send");
    // Create an HTTP request handle.
    if (hConnect)
        hRequest = WinHttpOpenRequest(hConnect, L"POST", szGet, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

    // Send a request.
    if (hRequest)
    {
        LPCWSTR header = L"Content-type: application/x-www-form-urlencoded/r/n";
        DWORD ret = WinHttpAddRequestHeaders(hRequest, header, lstrlen(header), WINHTTP_ADDREQ_FLAG_ADD);
        WCHAR szBody[2048] = L"{\"title\":\"3333\"\n}";
        DWORD dwByte = 0;
        char szUTF8[2048]={0};
        ::WideCharToMultiByte(CP_UTF8, NULL, szBody, int(wcslen(szBody)), szUTF8, 2048, NULL, NULL);
        bResults = WinHttpSendRequest(hRequest, 0, 0, szUTF8, strlen(szUTF8), strlen(szUTF8), 0);
    }
    // End the request.
    if (bResults)
        bResults = WinHttpReceiveResponse(hRequest, NULL);
	char pszOutBuffer[2048];
	int i = 0;
	if (bResults)
	{
		do
		{
			dwSize = 0;
			WinHttpQueryDataAvailable(hRequest, &dwSize);
			if (!dwSize)
				break;
			if (i + dwSize > 2048)
				dwSize = 2048 - i;
			if (WinHttpReadData(hRequest, (LPVOID)&pszOutBuffer[i], dwSize, &dwDownloaded))
			{
				i = strlen(pszOutBuffer);
			}
			if (!dwDownloaded)
				break;
		} while (dwSize != 0);
	}
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);
    return TRUE;
}
BOOL SendIYUU(wchar_t* szTOKEN, wchar_t* szTitle, wchar_t* szContent, wchar_t* szUrl, float fPrice, wchar_t* szBusiness, wchar_t* szImg)
{
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    BOOL  bResults = FALSE;
    HINTERNET  hSession = NULL, hConnect = NULL, hRequest = NULL;
    // Use WinHttpOpen to obtain a session handle.
    hSession = WinHttpOpen(L"IYUU", WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, NULL);
    // Specify an HTTP server.
    if (hSession)
        hConnect = WinHttpConnect(hSession, L"iyuu.cn", INTERNET_DEFAULT_HTTPS_PORT, 0);
    WCHAR szGet[1024]=L"/";
    lstrcat(szGet, szTOKEN);
    lstrcat(szGet, L".send");
    / *
        lstrcat(szGet, L"?appToken=AT_YGOXF3ZtPSkz5lkxhFUZ5ZkHOgrkKSdG&content=");
        lstrcat(szGet, szTitle);
        lstrcat(szGet, L"&uid=UID_XdNax8pCAEVtiAYu0ZmHNGq9r1Ma&url=");
        lstrcat(szGet, szUrl);
    * /
    // Create an HTTP request handle.
    if (hConnect)
        hRequest = WinHttpOpenRequest(hConnect, L"POST", szGet, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

    // Send a request.
    if (hRequest)
    {
        LPCWSTR header = L"Content-type: application/x-www-form-urlencoded";
        DWORD ret = WinHttpAddRequestHeaders(hRequest, header, lstrlen(header), WINHTTP_ADDREQ_FLAG_ADD);
        WCHAR szBody[2048] = L"{\n \"text\"= \"33333\",\"desp\"= \"666\"\n}";
		DWORD dwByte = 0;
		char szUTF8[2048]={0};
		::WideCharToMultiByte(CP_UTF8, NULL, szBody, int(wcslen(szBody)), szUTF8, 2048, NULL, NULL);
		bResults = WinHttpSendRequest(hRequest, 0, 0, szUTF8, strlen(szUTF8), strlen(szUTF8), 0);

    }
    // End the request.
    if (bResults)
        bResults = WinHttpReceiveResponse(hRequest, NULL);
	char pszOutBuffer[2048];
	int i = 0;
	if (bResults)
	{
		do
		{
			dwSize = 0;
			WinHttpQueryDataAvailable(hRequest, &dwSize);
			if (!dwSize)
				break;
			if (i + dwSize > 2048)
				dwSize = 2048 - i;
			if (WinHttpReadData(hRequest, (LPVOID)&pszOutBuffer[i], dwSize, &dwDownloaded))
			{
				i = strlen(pszOutBuffer);
			}
			if (!dwDownloaded)
				break;
		} while (dwSize != 0);
	}
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);
    return TRUE;
}
*/
BOOL GetWeChatToken(wchar_t* corpid, wchar_t *corpsecret,wchar_t * access_token)
{
    access_token[0] = L'~';
	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	BOOL  bResults = FALSE;
	HINTERNET  hSession = NULL, hConnect = NULL, hRequest = NULL;
	// Use WinHttpOpen to obtain a session handle.
	hSession = WinHttpOpen(L"WeChatPusher", WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, NULL);
	// Specify an HTTP server.
	if (hSession)
		hConnect = WinHttpConnect(hSession, L"qyapi.weixin.qq.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
	WCHAR szGet[1024] = L"/cgi-bin/gettoken?corpid=";
	lstrcat(szGet, corpid);
    lstrcat(szGet, L"&corpsecret=");
    lstrcat(szGet, corpsecret);
	// Create an HTTP request handle.
	if (hConnect)
		hRequest = WinHttpOpenRequest(hConnect, L"GET", szGet, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
	// Send a request.
	if (hRequest)
	{
        bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
	}

	// End the request.
	if (bResults)
		bResults = WinHttpReceiveResponse(hRequest, NULL);
	char pszOutBuffer[2048];
	int i = 0;
	if (bResults)
	{
		do
		{
			dwSize = 0;
			WinHttpQueryDataAvailable(hRequest, &dwSize);
			if (!dwSize)
				break;
			if (i + dwSize > 2048)
				dwSize = 2048 - i;
			if (WinHttpReadData(hRequest, (LPVOID)&pszOutBuffer[i], dwSize, &dwDownloaded))
			{
				i = strlen(pszOutBuffer);
			}
			if (!dwDownloaded)
				break;
		} while (dwSize != 0);
        WCHAR szOutBuffer[2048] = { 0 };
		MultiByteToWideChar(CP_UTF8, 0, pszOutBuffer, -1, szOutBuffer, 2048);
        WCHAR *cToken = lstrstr(szOutBuffer, L"access_token");
        if (cToken)
        {
            WCHAR* cTokenLeft = lstrstr(cToken + 14, L"\"");
            if (cTokenLeft)
            {
                cTokenLeft += 1;
                WCHAR* cTokenRight = lstrstr(cTokenLeft, L"\"");
                if (cTokenRight)
                {
                    cTokenRight[0] = L'\0';
                    lstrcpy(access_token, cTokenLeft);
                }
            }
        }
	}
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);
	return TRUE;
}

BOOL SendWeChatPusher(wchar_t* uid, wchar_t* szTitle, wchar_t* szContent, wchar_t* szUrl,  wchar_t* szImg)
{
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    BOOL  bResults = FALSE;
    HINTERNET  hSession = NULL, hConnect = NULL, hRequest = NULL;
    // Use WinHttpOpen to obtain a session handle.
    hSession = WinHttpOpen(L"DingDing", WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, NULL);
    // Specify an HTTP server.
    if (hSession)
        hConnect = WinHttpConnect(hSession, L"qyapi.weixin.qq.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
    WCHAR szGet[1024] = L"/cgi-bin/message/send?access_token=";
    lstrcat(szGet, lpRemindData->szWeChatToken);
    // Create an HTTP request handle.
    if (hConnect)
        hRequest = WinHttpOpenRequest(hConnect, L"POST", szGet, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    // Send a request.
    if (hRequest)
    {
        LPCWSTR header = L"Content-Type:application/json";
        DWORD ret = WinHttpAddRequestHeaders(hRequest, header, lstrlen(header), WINHTTP_ADDREQ_FLAG_ADD);
		WCHAR szBody[2048] = L"{\n\"touser\": \"";
        lstrcat(szBody, RemindSave.szWeChatUserID);
        lstrcat(szBody, L"\",\n\"msgtype\": \"news\",\n	\"agentid\": ");
        lstrcat(szBody, RemindSave.szWeChatAgentId);
        lstrcat(szBody, L",\n	\"news\" : {\n	\"articles\": [{\n	\"title\": \"");
        lstrcat(szBody, szTitle);
        lstrcat(szBody, L"\",\n \"description\": \"");
        lstrcat(szBody, szContent);
        lstrcat(szBody, L"\",\n \"url\": \"");
//        WCHAR surl[] = L"https://app.smzdm.com/xiazai/?json={%22url%22:%22https://m.smzdm.com/p/54084912/%22,%22channel_name%22:%22youhui%22,%22linkVal%22:%2254084912%22,%22article_id%22:%2254084912%22,%22open_from%22:%22youshangXZ%22,%22frompage%22:%22taoxi01%22,%22targetpage%22:%22youhui%22,%22zdm_cp%22:%22H5|%22}";
        lstrcat(szBody, szUrl);
        lstrcat(szBody, L"\",\n \"picurl\": \"https://");
        lstrcat(szBody, szImg);
		lstrcat(szBody, L"\"\n }]\n},\n}");
        DWORD dwByte = 0;
        char szUTF8[2048] = { 0 };
        ::WideCharToMultiByte(CP_UTF8, NULL, szBody, int(lstrlen(szBody)), szUTF8, 2048, NULL, NULL);
        bResults = WinHttpSendRequest(hRequest, 0, 0, szUTF8, strlen(szUTF8), strlen(szUTF8), 0);
    }

    // End the request.
    if (bResults)
        bResults = WinHttpReceiveResponse(hRequest, NULL);
    char pszOutBuffer[2048];
    int i = 0;
    if (bResults)
    {
        do
        {
            dwSize = 0;
            WinHttpQueryDataAvailable(hRequest, &dwSize);
            if (!dwSize)
                break;
            if (i + dwSize > 2048)
                dwSize = 2048 - i;
            if (WinHttpReadData(hRequest, (LPVOID)&pszOutBuffer[i], dwSize, &dwDownloaded))
            {
                i = strlen(pszOutBuffer);
            }
            if (!dwDownloaded)
                break;
        } while (dwSize != 0);
    }
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);
    if (strstr(pszOutBuffer, "access_token"))
    {
        if (lpRemindData->szWeChatToken[0] != L'~')
        {
            GetWeChatToken(RemindSave.szWeChatID, RemindSave.szWeChatSecret, lpRemindData->szWeChatToken);
            SendWeChatPusher(uid, szTitle, szContent, szUrl,szImg);
        }
        return FALSE;
    }
    else
        return TRUE;
}
BOOL SendDingDing(wchar_t* access_token, wchar_t* szTitle, wchar_t* szContent, wchar_t* szUrl,wchar_t* szImg)
{
	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	BOOL  bResults = FALSE;
	HINTERNET  hSession = NULL, hConnect = NULL, hRequest = NULL;
	// Use WinHttpOpen to obtain a session handle.
	hSession = WinHttpOpen(L"DingDing", WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, NULL);
	// Specify an HTTP server.
	if (hSession)
		hConnect = WinHttpConnect(hSession, L"oapi.dingtalk.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
	WCHAR szGet[1024] = L"/robot/send?access_token=";
	lstrcat(szGet, access_token);
	// Create an HTTP request handle.
	if (hConnect)
		hRequest = WinHttpOpenRequest(hConnect, L"POST", szGet, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
	// Send a request.
	if (hRequest)
	{
		LPCWSTR header = L"Content-Type:application/json";
		DWORD ret = WinHttpAddRequestHeaders(hRequest, header, lstrlen(header), WINHTTP_ADDREQ_FLAG_ADD);
		WCHAR szBody[2048] = L"{\n \"msgtype\": \"link\",\n \"link\": {\n \"text\": \"";
		lstrcat(szBody, szContent);
        lstrcat(szBody, L"\",\n \"title\": \"");
		lstrcat(szBody, szTitle);
        lstrcat(szBody, L"\",\n \"picUrl\": \"https://");
        lstrcat(szBody, szImg);
		lstrcat(szBody, L"\",\n \"messageUrl\": \"");
		//        WCHAR surl[] = L"https://app.smzdm.com/xiazai/?json={%22url%22:%22https://m.smzdm.com/p/54084912/%22,%22channel_name%22:%22youhui%22,%22linkVal%22:%2254084912%22,%22article_id%22:%2254084912%22,%22open_from%22:%22youshangXZ%22,%22frompage%22:%22taoxi01%22,%22targetpage%22:%22youhui%22,%22zdm_cp%22:%22H5|%22}";
		lstrcat(szBody, szUrl);				
		lstrcat(szBody, L"\"\n }\n}");
		DWORD dwByte = 0;
		char szUTF8[2048]={0};
		::WideCharToMultiByte(CP_UTF8, NULL, szBody, int(lstrlen(szBody)), szUTF8, 2048, NULL, NULL);
		bResults = WinHttpSendRequest(hRequest, 0, 0, szUTF8, strlen(szUTF8), strlen(szUTF8), 0);
	}
	// End the request.
	if (bResults)
		bResults = WinHttpReceiveResponse(hRequest, NULL);
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);
	return TRUE;
}

BOOL SendWxPusher(wchar_t* uid, wchar_t* szTitle, wchar_t* szContent,wchar_t *szUrl,wchar_t *szImg)
{
	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	BOOL  bResults = FALSE;
	HINTERNET  hSession = NULL, hConnect = NULL, hRequest = NULL;
	// Use WinHttpOpen to obtain a session handle.
	hSession = WinHttpOpen(L"WxPusher", WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, NULL);
	// Specify an HTTP server.
	if (hSession)
		hConnect = WinHttpConnect(hSession, L"wxpusher.zjiecode.com",INTERNET_DEFAULT_HTTPS_PORT, 0);
    WCHAR szGet[1024] = L"/api/send/message";
/*
    lstrcat(szGet, L"?appToken=AT_YGOXF3ZtPSkz5lkxhFUZ5ZkHOgrkKSdG&content=");
    lstrcat(szGet, szTitle);
    lstrcat(szGet, L"&uid=UID_XdNax8pCAEVtiAYu0ZmHNGq9r1Ma&url=");
    lstrcat(szGet, szUrl);
*/
	// Create an HTTP request handle.
	if (hConnect)
		hRequest = WinHttpOpenRequest(hConnect, L"POST", szGet, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

	// Send a request.
    if (hRequest)
    {
        LPCWSTR header = L"Content-Type:application/json";
        DWORD ret =WinHttpAddRequestHeaders(hRequest, header, lstrlen(header), WINHTTP_ADDREQ_FLAG_ADD);
		WCHAR szBody[2048] = L"\n{\n\"appToken\":\"";
		lstrcat(szBody, szWxPusherToken);
        lstrcat(szBody, L"\",\n\"content\":\"");
        lstrcat(szBody,L"<img src=https://");
        lstrcat(szBody, szImg);
        lstrcat(szBody, L" /> <font size=4> <br /> ");
		lstrcat(szBody, szContent);
		lstrcat(szBody, L" </font> \",\n\"summary\":\"");
		lstrcat(szBody, szTitle);        
        lstrcat(szBody, L"\",\n\"contentType\":2,\n\"topicIds\":[123],\n\"uids\":[\"");
        lstrcat(szBody, uid);
        lstrcat(szBody, L"\"],\n\"url\":\"");
		lstrcat(szBody, szUrl);
		lstrcat(szBody, L"\"\n}\n");
        DWORD dwByte=0;
        char szUTF8[2048] = { 0 };
        ::WideCharToMultiByte(CP_UTF8, NULL, szBody, int(lstrlen(szBody)), szUTF8, 2048, NULL, NULL);
        bResults = WinHttpSendRequest(hRequest, 0, 0, szUTF8, strlen(szUTF8), strlen(szUTF8), 0);
    }

	// End the request.
	if (bResults)
		bResults = WinHttpReceiveResponse(hRequest, NULL);
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);
    return TRUE;
}
BOOL SearchSMZDM(REMINDITEM * lpRI,BOOL bList,int iPage,BOOL bSmzdmSearch)
{
	WCHAR szGet[1024] = L"/?s=";
	WCHAR szUrlCode[1024];
	UrlUTF8(lpRI->szKey, szUrlCode);
	//	lstrcat(szGet, lpRI->szKey);
	lstrcat(szGet, szUrlCode);
    if (!bList || !bPost)
        lstrcat(szGet, L"&c=faxian");
    else
        lstrcat(szGet, L"&c=post&f_c=post");
    if (lpRI->iBusiness&&(!bPost||!bList))
    {
        lstrcat(szGet, L"&mall_id=");
        WCHAR sz[16];
        wsprintf(sz, L"%d", mIDs[lpRI->iBusiness]);
        lstrcat(szGet, sz);
    }
    if(RemindSave.bLastSort|| !bList)
	    lstrcat(szGet, L"&order=time&v=b");
    else
        lstrcat(szGet, L"&order=score&v=b");
    if ((lpRI->uMaxPrice != 0||lpRI->uMinPrice!=0)&&(!bPost||!bList))
    {
        lstrcat(szGet, L"&min_price=");
        if (lpRI->uMinPrice != 0)
        {
            WCHAR szPrice[16];
            int p = lpRI->uMinPrice * 100;
            wsprintf(szPrice, L"%d.%2.2d", p/100, p % 100);
            lstrcat(szGet, szPrice);
        }
        lstrcat(szGet, L"&max_price=");
        if (lpRI->uMaxPrice != 0)
        {
            WCHAR szPrice[16];
            int p = lpRI->uMaxPrice * 100;
            wsprintf(szPrice, L"%d.%2.2d", p / 100, p % 100);
            lstrcat(szGet, szPrice);
        }
    }
    if (bSmzdmSearch)
    {
        WCHAR szUrl[2048]=L"https://search.smzdm.com";
        lstrcat(szUrl, szGet);
        ShellExecute(NULL, L"open", szUrl, NULL, NULL, SW_SHOW);
        return TRUE;
    }
    if (iPage)
    {
        WCHAR sz[8];
        wsprintf(sz, L"&p=%d",iPage);
        lstrcat(szGet, sz);
    }
	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	char* pszOutBuffer = new char[NETPAGESIZE];
	BOOL  bResults = FALSE;
	HINTERNET  hSession = NULL, hConnect = NULL, hRequest = NULL;
	// Use WinHttpOpen to obtain a session handle.
	hSession = WinHttpOpen(L"Remind", WINHTTP_ACCESS_TYPE_NO_PROXY, NULL, NULL, NULL);
	if (rovi.dwMajorVersion == 6 && rovi.dwMinorVersion == 1)//WIN 7 开启TLS1.2
	{
		DWORD flags = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
		WinHttpSetOption(hSession, WINHTTP_OPTION_SECURE_PROTOCOLS, &flags, sizeof(flags));
	}
	// Specify an HTTP server.
	if (hSession)
		hConnect = WinHttpConnect(hSession, L"search.smzdm.com",
			INTERNET_DEFAULT_HTTPS_PORT, 0);
	// Create an HTTP request handle.
	if (hConnect)
		hRequest = WinHttpOpenRequest(hConnect, L"GET", szGet, NULL, L"", WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (rovi.dwMajorVersion == 6 && rovi.dwMinorVersion == 1)//WIN 7 开启TLS1.2
    {
        DWORD dwSecFlag = SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
            SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
            SECURITY_FLAG_IGNORE_UNKNOWN_CA |
            SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;
        WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &dwSecFlag, sizeof(dwSecFlag));
    }
	// Send a request.
    if (hRequest)
    {
        bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    }

	// End the request.
	if (bResults)
		bResults = WinHttpReceiveResponse(hRequest, NULL);
    DWORD dErr=GetLastError();
	// Keep checking for data until there is nothing left.
	size_t i = 0;
    ZeroMemory(pszOutBuffer, NETPAGESIZE);
    if (bResults)
    {
        do
        {
            dwSize = 0;
            WinHttpQueryDataAvailable(hRequest, &dwSize);
            if (!dwSize)
                break;
            if (i + dwSize > NETPAGESIZE)
                dwSize = NETPAGESIZE - i;
            if (WinHttpReadData(hRequest, (LPVOID)&pszOutBuffer[i], dwSize, &dwDownloaded))
            {
                i = strlen(pszOutBuffer);
            }
            if (!dwDownloaded)
                break;
        } while (dwSize != 0);
        WCHAR* szOutBuffer = new WCHAR[NETPAGESIZE];
        MultiByteToWideChar(CP_UTF8, 0, pszOutBuffer, -1, szOutBuffer, NETPAGESIZE);
        DWORD sl=strlen(pszOutBuffer);
        UINT iID = 0;
        WCHAR wID[10]={0};
        WCHAR szLink[64];
        WCHAR szImg[128];
        szImg[0] = L'\0';
        WCHAR szTitle[128];
        float fPrice = 0;
        //    WCHAR szPrice[8];
        WCHAR szDescripe[512];
        szDescripe[0] = L'\0';
        UINT lZhi = 0;
        UINT lBuZhi=0;
        UINT lStar=0;
        UINT lTalk=0;
        WCHAR szGoPath[512];
        szGoPath[0] = L'\0';
        //    WCHAR szTime[6];
        SYSTEMTIME st;
        ULONGLONG ft1, ft2;
        GetLocalTime(&st);
        SystemTimeToFileTime(&st, (LPFILETIME)&ft1);
        if (bOpen == FALSE)
        {
            ft1 -= (ULONGLONG)3600 * 10000000;
        }
        else
        {
            ft2 = iTimes[RemindSave.iTime];
            ft1 -= ft2 * 60 * 10000000;
            ft1 -= 60 * 10000000;
        }
        SYSTEMTIME st2;
        FileTimeToSystemTime((FILETIME*)&ft1, &st2);
        WCHAR szBusiness[17];
        WCHAR* cStart = lstrstr(szOutBuffer, L"feed-row-wide");
        if (bList)
        {
            SendMessage(hList, WM_SETREDRAW, FALSE, FALSE);
            if (iPage==0)
            {
                ListView_DeleteAllItems(hList);
                ListView_SetItemCount(hList, 81);
            }
        }
//        	WCHAR szIYUU[] = L"IYUU12087Tbb1266ff7043661f3c1aafadf8952e8d406b9af6";
//        	SendIYUU(szIYUU, szTitle, szDescripe, szLink, fPrice, szBusiness, szImg);
        while (cStart)
        {
            GetLocalTime(&st);
            UINT tid = 0;
            WCHAR* cGray = lstrstr(cStart, L"feed-row-grey");
            if (cGray > cStart + 32 || cGray == NULL||bList)//不是灰色的继续
            {
                cStart += 12;
                WCHAR* cID = lstrstr(cStart, L"article_id");
                if (cID)//////第一个商品ID
                {
                    WCHAR* cIDLeft = lstrstr(cID, L":");
                    if (cIDLeft)
					{
                        cIDLeft += 2;
                        tid = _wtoi(cIDLeft);
                        WCHAR* cIDRight = lstrstr(cIDLeft, L"'");
                        if (cIDRight)
                        {
                            if(cIDRight-cIDLeft<10)
                                lstrcpyn(wID, cIDLeft, cIDRight - cIDLeft);
                        }
                        if (iID == 0)
                            iID = tid;
                        if (tid == lpRI->uid && lpRI->uid != 0 && !bList)
                            break;
                    }
                }
                WCHAR* cLink = lstrstr(cStart, L"href=");
                if (cLink)
                {
                    WCHAR* cLinkLeft = lstrstr(cLink, L"\"");
                    if (cLinkLeft)
                    {
                        cLinkLeft += 1;
                        WCHAR* cLinkRight = lstrstr(cLinkLeft, L"\"");
                        if (cLinkRight)
                        {
                            __int64 n = cLinkRight - cLinkLeft;
                            if (n > 63)
                                n = 63;
                            if (cLinkLeft[0] == L'/')
                            {
                                if (n == 63)
                                    n -= 6;
                                lstrcpy(szLink, L"https:");                                
                                lstrcpyn(szLink+6, cLinkLeft,n);
                            }
                            else
                                lstrcpyn(szLink, cLinkLeft, n);
                        }
                    }
                }
                WCHAR* cImg = lstrstr(cStart, L"img src");
                if (cImg)
                {
                    WCHAR* cImgLeft = lstrstr(cImg, L"//");
                    if (cImgLeft)
                    {
                        cImgLeft += 2;
                        WCHAR* cImgRight = lstrstr(cImgLeft, L"\"");
                        if (cImgRight)
                        {
                            __int64 n = cImgRight - cImgLeft;
                            if (n > 127)
                                n = 127;
                            lstrcpyn(szImg, cImgLeft, n);
                        }
                    }
                }
                BOOL bBreak = FALSE;
                WCHAR* cTitle = lstrstr(cStart, L"alt=");
                if (cTitle)
                {
                    WCHAR* cTitleLeft = lstrstr(cTitle, L"\"");
                    if (cTitleLeft)
                    {
                        cTitleLeft += 1;
                        WCHAR* cTitleRight = lstrstr(cTitleLeft, L"\"");
                        if (cTitleRight)
                        {
                            __int64 n = cTitleRight - cTitleLeft;
                            if (n > 127)
                                n = 127;
                            lstrcpyn(szTitle, cTitleLeft, n);
                            if (lstrlen(lpRI->szFilter) != 0)
                            {                                
                                WCHAR* cFilterLeft = lpRI->szFilter;
                                while (cFilterLeft)
                                {
                                    WCHAR* cFilterRight = lstrstr(cFilterLeft, L" ");
                                    if (cFilterRight)
                                        cFilterRight[0] = L'\0';
                                    if (lstrstr(szTitle, cFilterLeft))
                                    {
                                        if (cFilterRight)
                                            cFilterRight[0] = L' ';
                                        bBreak = TRUE;
                                        break;
                                    }
                                    if (cFilterRight)
                                    {
                                        cFilterRight[0] = L' ';
                                        cFilterLeft = cFilterRight + 1;
                                        while (cFilterLeft[0] == L' ')
                                        {
                                            cFilterLeft++;
                                        }
                                    }
                                    else
                                        cFilterLeft = 0;
                                }
                            }
                        }
                    }
                }
                if (bBreak)
                {
                    cStart = lstrstr(cStart, L"feed-row-wide");
                    continue;
                }
				WCHAR* cTalk = lstrstr(cStart, L"z-icon-talk-o-thin");
				if (cTalk)
				{
					WCHAR* cTalkLeft = lstrstr(cTalk, L"i>");
					if (cTalkLeft)
					{
						cTalkLeft += 2;
						lTalk = _wtoi(cTalkLeft);
					}
				}
                if (!bPost || !bList)
                {
                    WCHAR* cPrice = lstrstr(cStart, L"z-highlight");
                    if (cPrice)
                    {
                        WCHAR* cPriceLeft = lstrstr(cPrice, L">");
                        if (cPriceLeft)
                        {
                            cPriceLeft += 1;
                            fPrice = _wtof(cPriceLeft);
                        }
                    }

                    WCHAR* cDescripe = lstrstr(cStart, L"feed-block-descripe-top");
                    if (cDescripe)
                    {
                        WCHAR* cDescripeLeft = lstrstr(cDescripe, L">");
                        if (cDescripeLeft)
                        {
                            cDescripeLeft += 1;
                            while (cDescripeLeft[0] == L' ' || cDescripeLeft[0] == L'\r' || cDescripeLeft[0] == L'\n')
                            {
                                cDescripeLeft += 1;
                            }
                            WCHAR* cDescripeRight = lstrstr(cDescripeLeft, L"</div");
                            if (cDescripeRight)
                            {
                                size_t sCount = cDescripeRight - cDescripeLeft;
                                if (sCount >= 512)
                                    sCount = 511;
                                lstrcpyn(szDescripe, cDescripeLeft, sCount);
                            }
                        }
                    }

                    WCHAR* cZhi = lstrstr(cStart, L"z-icon-zhi-o-thin");
                    if (cZhi)
                    {
                        WCHAR* cZhiLeft = lstrstr(cZhi, L"span");
                        if (cZhiLeft)
                        {
                            cZhiLeft += 5;
                            lZhi = _wtoi(cZhiLeft);
                        }
                    }

                    WCHAR* cBuZhi = lstrstr(cStart, L"z-icon-buzhi-o-thin");
                    if (cBuZhi)
                    {
                        WCHAR* cBuZhiLeft = lstrstr(cBuZhi, L"span");
                        if (cBuZhiLeft)
                        {
                            cBuZhiLeft += 5;
                            lBuZhi = _wtoi(cBuZhiLeft);
                        }
                    }

                    WCHAR* cStar = lstrstr(cStart, L"z-icon-star-o-thin");
                    if (cStar)
                    {
                        WCHAR* cStarLeft = lstrstr(cStar, L"i>");
                        if (cStarLeft)
                        {
                            cStarLeft += 2;
                            lStar = _wtoi(cStarLeft);
                        }
                    }
					WCHAR* cGoPath = lstrstr(cStart, L"go_path");
					if (cGoPath)
					{
						WCHAR* cGoPathLeft = lstrstr(cGoPath, L":");
						if (cGoPathLeft)
						{
							cGoPathLeft += 2;
							WCHAR* cGoPathRight = lstrstr(cGoPathLeft, L"\'");
							if (cGoPathRight)
							{
								__int64 n = cGoPathRight - cGoPathLeft;
								if (n > 511)
									n = 511;
                                lstrcpyn(szGoPath, cGoPathLeft, n);
							}
						}
					}
                }
                else
                {
					WCHAR* cZhi = lstrstr(cStart, L"z-icon-thumb-up-o-thin");
					if (cZhi)
					{
						WCHAR* cZhiLeft = lstrstr(cZhi, L"number\">");
						if (cZhiLeft)
						{
							cZhiLeft += 8;
							lZhi = _wtoi(cZhiLeft);
						}
					}
					WCHAR* cStar = lstrstr(cStart, L"z-icon-star-o-thin");
					if (cStar)
					{
						WCHAR* cStarLeft = lstrstr(cStar, L"span>");
						if (cStarLeft)
						{
							cStarLeft += 5;
							lStar = _wtoi(cStarLeft);
						}
					}
                    WCHAR* cBusiness = lstrstr(cStart, L"z-avatar-name");
                    if (cBusiness)
                    {
                        WCHAR* cBusinessLeft = lstrstr(cBusiness, L">");
                        if (cBusinessLeft)
                        {
                            cBusinessLeft += 1;
                            WCHAR* cBusinessRight = lstrstr(cBusinessLeft, L"</");
                            if (cBusinessRight)
                            {
                                __int64 n = cBusinessRight - cBusinessLeft;
                                if (n > 16)
                                    n = 16;
                                lstrcpyn(szBusiness, cBusinessLeft, n);
                            }
                        }
                    }
                }
                WCHAR* cTime = lstrstr(cStart, L"feed-block-extras");
                if (cTime)
                {
                    cTime = lstrstr(cTime, L">");
                    if (cTime)
                    {
                        WCHAR* cDateLeft = lstrstr(cTime, L"-");
                        WCHAR* cTimeLeft = lstrstr(cTime, L":");
                        if (cDateLeft && cDateLeft < cTimeLeft)
                        {
                            if (cDateLeft[3] == L'-')
                            {
                                st.wYear = _wtoi(cDateLeft - 4);
                                cDateLeft += 3;
                                cTimeLeft = NULL;
                                st.wHour = 0;
                                st.wMinute = 0;
                            }
                            WCHAR* cDateRight = cDateLeft + 1;
                            cDateLeft -= 2;
                            st.wMonth = _wtoi(cDateLeft);
                            st.wDay = _wtoi(cDateRight);
                        }
                        if (cTimeLeft)
                        {
                            WCHAR* cTimeRight = cTimeLeft + 1;
                            cTimeLeft -= 2;
                            st.wHour = _wtoi(cTimeLeft);
                            st.wMinute = _wtoi(cTimeRight);
                            /*
                                            while (cTimeRight[0] != L' ')
                                            {
                                                cTimeRight += 1;
                                            }
                                            lstrcpyn(szTime, cTimeLeft, cTimeRight - cTimeLeft);
                            */
                        }
                        SystemTimeToFileTime(&st, (FILETIME*)&ft2);
                        if (ft1 > ft2 && !bList&& lpRI->uid != 0)
                            break;
                        if (lpRI->uid == 0)
                            lpRI->uid = iID;
                        if (!bPost || !bList)
                        {
                            WCHAR* cBusiness = lstrstr(cTime, L"<span");
                            if (cBusiness)
                            {
                                WCHAR* cBusinessLeft = lstrstr(cBusiness, L">");
                                if (cBusinessLeft)
                                {
                                    cBusinessLeft += 1;
                                    while (cBusinessLeft[0] == L' ' || cBusinessLeft[0] == L'\r' || cBusinessLeft[0] == L'\n')
                                    {
                                        cBusinessLeft += 1;
                                    }
                                    WCHAR* cBusinessRight = cBusinessLeft;
                                    while (cBusinessRight[0] != L' ')
                                    {
                                        cBusinessRight += 1;
                                    }
                                    __int64 n = cBusinessRight - cBusinessLeft;
                                    if (n > 16)
                                        n = 16;
                                    lstrcpyn(szBusiness, cBusinessLeft, n);
                                }
                            }
                        }
                    }
                }
                if (bList)
                {
                    WCHAR sz[64];
                    LVITEM li = { 0 };
                    int iSub = 0;
                    li.mask = LVIF_TEXT;
                    li.pszText = szTitle;
                    li.iSubItem = iSub++;
                    li.iItem = ListView_GetItemCount(hList);
                    li.iItem = ListView_InsertItem(hList, &li);
                    li.pszText = szDescripe;
                    li.iSubItem = iSub++;
//                    ListView_SetItem(hList, &li);
                    int p = fPrice * 100;
                    wsprintf(sz, L"%d.%2.2d", p / 100, p % 100);
                    li.pszText = sz;
                    li.iSubItem = iSub++;
                    ListView_SetItem(hList, &li);
                    li.pszText = szImg;
                    li.iSubItem = iSub++;
//                    ListView_SetItem(hList, &li);
                    wsprintf(sz, L"%d", lZhi);
                    li.pszText = sz;
                    li.iSubItem = iSub++;
                    ListView_SetItem(hList, &li);
                    wsprintf(sz, L"%d", lBuZhi);
                    li.pszText = sz;
                    li.iSubItem = iSub++;
                    ListView_SetItem(hList, &li);
					wsprintf(sz, L"%d", lStar);
					li.pszText = sz;
					li.iSubItem = iSub++;
					ListView_SetItem(hList, &li);
                    wsprintf(sz, L"%d", lTalk);
                    li.pszText = sz;
                    li.iSubItem = iSub++;
                    ListView_SetItem(hList, &li);
                    wsprintf(sz, L"%d-%2.2d-%2.2d %2.2d:%2.2d",st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
                    li.pszText = sz;
                    li.iSubItem = iSub++;
                    ListView_SetItem(hList, &li);
                    li.pszText = szBusiness;
                    li.iSubItem = iSub++;
                    ListView_SetItem(hList, &li);
                    li.pszText = szLink;
                    li.iSubItem = iSub++;
                    ListView_SetItem(hList, &li);
                    li.pszText = szGoPath;
                    li.iSubItem = iSub++;
                    ListView_SetItem(hList, &li);     
//                    WCHAR szToken[] = L"SCT152372TQlJTExCGuU63HYmj8Uargtjb";
//                    SendServerJ(szToken, szTitle, szDescripe, szLink, fPrice, szBusiness, szImg);
                    
                }
                else
                {
                    WCHAR wTitle[192],wDescripe[666];
                    int p = fPrice * 100;
                    wsprintf(wTitle, L"%d.%2.2d元 %s %s", p / 100, p % 100, szTitle, szBusiness);
                    wsprintf(wDescripe, L"~%s~ %s", lpRI->szKey, szDescripe);
                    if (RemindSave.bTips)
                    {
                        if (bNewTrayTips)
                        {
                            CreateDirectory(L"cache", NULL);
                            WCHAR wImg[MAX_PATH];
                            GetCurrentDirectory(MAX_PATH, wImg);
                            lstrcat(wImg, L"\\cache\\");
                            WCHAR* wFileLeft = lstrstr(szImg+2, L"/");;
                            while (true)
                            {
                                wFileLeft += 1;
                                WCHAR * wFileRight = lstrstr(wFileLeft, L"/");
                                if (!wFileRight)
                                    break;
                                else
                                    wFileLeft = wFileRight;
                            }
                            lstrcat(wImg, wFileLeft);
                            winhttpDownload(szImg, wImg);
                            ShowToast(wTitle, wDescripe, wImg, szLink);
                        }
                        else
                        {

                            nid.uFlags =NIF_MESSAGE | NIF_INFO|NIF_TIP|NIF_ICON;
//                            nid.dwState = NIS_SHAREDICON;
//                            nid.dwStateMask = NIS_SHAREDICON;
//							nid.uVersion = NOTIFYICON_VERSION_4;
//							nid.hBalloonIcon = iMain;
							nid.dwInfoFlags = NIIF_NONE;
//                            nid.uID = tid;
                            nid.uTimeout = tid;
                            lstrcpyn(nid.szInfoTitle, wTitle, 63);
                            lstrcpyn(nid.szInfo, wDescripe, 255);
                            Shell_NotifyIcon(NIM_MODIFY, &nid);
//                            Shell_NotifyIcon(NIM_SETVERSION, &nid);
                        }
                    }
                    if (RemindSave.bDingDing)
                    {
                        SendDingDing(RemindSave.szDingDingToken, wTitle, wDescripe, szLink, szImg);
                    }
                    if (RemindSave.bWeChat)
                    {
                        SendWeChatPusher(RemindSave.szWeChatUserID, wTitle, wDescripe, szLink, szImg);
                    }
                    if (RemindSave.bWxPusher)
                    {
                        SendWxPusher(RemindSave.szWxPusherUID, wTitle, wDescripe, szLink, szImg);
                    }
                    if (RemindSave.bDirectly)
                        ShellExecute(NULL, L"open", szLink, NULL, NULL, SW_SHOWNOACTIVATE);

                }
            }
            else
                cStart += 2;
            cStart = lstrstr(cStart, L"feed-row-wide");
        }
        if (bList)
            SendMessage(hList, WM_SETREDRAW, TRUE, FALSE);
        if (iID != 0)
            lpRI->uid = iID;
        delete[] szOutBuffer;
    }
    delete[] pszOutBuffer;
	// Close any open handles.
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);
	return bResults;
}
int iReset = 11;
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
#ifdef NDEBUG
	if (OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, szAppName) == NULL)/////////////////////////创建守护进程
	{
		HANDLE hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(BOOL), szAppName);
		if (hMap)
		{
			lpRemindData = (REMINDDATA*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(REMINDDATA));
			ZeroMemory(lpRemindData, sizeof(REMINDDATA));
			while (lpRemindData->bExit == FALSE&&iReset!=0)
			{
				HANDLE hProcess;
				RunProcess(0, 0, &hProcess);
                EmptyProcessMemory(NULL);;
				WaitForSingleObject(hProcess, INFINITE);
				CloseHandle(hProcess);
                iReset--;
			}
			UnmapViewOfFile(lpRemindData);
			CloseHandle(hMap);
			ExitProcess(0);
			return 0;
		}
	}
#endif
	hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, szAppName);
	if (hMap)
	{
        lpRemindData = (REMINDDATA*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(REMINDDATA));
	}
#ifdef NDEBUG
#else
	else
	{
		hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(BOOL), szAppName);
        lpRemindData = (REMINDDATA*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(REMINDDATA));
		ZeroMemory(lpRemindData, sizeof(REMINDDATA));
	}
#endif // !DAEMON
	typedef WINUSERAPI DWORD WINAPI RTLGETVERSION(PRTL_OSVERSIONINFOW  lpVersionInformation);
	rovi.dwOSVersionInfoSize = sizeof(rovi);
	RTLGETVERSION* RtlGetVersion = (RTLGETVERSION*)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion");
	if (RtlGetVersion)
		RtlGetVersion(&rovi);
	hMutex = CreateMutex(NULL, TRUE, L"_SmzdmRemind_");
    if (hMutex != NULL)
    {
        if (ERROR_ALREADY_EXISTS != GetLastError())
        {
/*
            INITCOMMONCONTROLSEX icce;
            icce.dwSize = sizeof INITCOMMONCONTROLSEX;
            icce.dwICC = ICC_LISTVIEW_CLASSES;
            InitCommonControlsEx(&icce);
*/
            // 执行应用程序初始化:
            if (!InitInstance(hInstance, nCmdShow))
            {
                return FALSE;
            }


            MSG msg;

            // 主消息循环:
            while (GetMessage(&msg, nullptr, 0, 0))
            {
                if (!IsDialogMessage(hMain, &msg))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
            bExit = TRUE;
            CloseHandle(hGetDataThread);
            Shell_NotifyIcon(NIM_DELETE, &nid);
            DestroyIcon(iMain);
//            DestroyIcon(iTray);
            CloseHandle(hMap);
            hMap = NULL;
			if (hMutex)
				CloseHandle(hMutex);
            return (int)msg.wParam;
        }
        else
        {
            LoadString(hInstance, IDS_TIPS, nid.szTip, 88);
            HWND hWnd = FindWindow(NULL, nid.szTip);
            if (hWnd)
            {
                ShowWindow(hWnd, SW_SHOW);
                SetForeground(hWnd);
            }
        }
    }
	if (hMap)
		CloseHandle(hMap);
    ExitProcess(0);
}
DWORD WINAPI GetDataThreadProc(PVOID pParam)//获取网站数据线程
{
	while (!bExit)
	{
		DWORD dStart = GetTickCount();
        if (!bOpen)
        {
            dStart -= iTimes[RemindSave.iTime] * 60000;
            dStart += 8888;
        }
		while (true)
		{
			DWORD dTime = GetTickCount() - dStart;
			if (dTime > iTimes[RemindSave.iTime] * 60000)
				break;
			else
				Sleep(988);
            WCHAR sz[16];
            wsprintf(sz, L"%d秒后获取", (iTimes[RemindSave.iTime] * 60000 - dTime) / 1000);
            SetDlgItemText(hMain, IDC_STATIC_COUNTDOWN, sz);
			if (bResetTime)
			{
                dStart = GetTickCount();
				dStart -= iTimes[RemindSave.iTime] * 60000;
				dStart += 8888;
				bResetTime = FALSE;
			}
		}
        bGetData = TRUE;
		SetWindowText(hMain, L"正在从网站获取并处理数据请稍后...");
		int n = riSize / sizeof REMINDITEM;
		for (int i = 0; i < n; i++)
		{
			if (!lpRemindItem[i].bNotUse)
				SearchSMZDM(&lpRemindItem[i], FALSE, FALSE,FALSE);
		}
		WriteSet(NULL);
		SetWindowText(hMain, nid.szTip);
        bOpen = TRUE;
        bGetData = FALSE;
	}
	return TRUE;
}
//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL bInit = TRUE;
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 将实例句柄存储在全局变量中    
    hMain = ::CreateDialog(hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, (DLGPROC)MainProc);
    if (!hMain)
    {
        return FALSE;
    }
    iMain = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SMZDMREMIND));
//    iTray = LoadIcon(hInst, MAKEINTRESOURCE(IDI_TRAY));
    SetClassLongPtr(hMain, -14, (LONG_PTR)iMain);
    SetClassLongPtr(hMain, -34, (LONG_PTR)iMain);
    //////////////////////////////////////////////////////////////////////////////////设置通知栏图标
    nid.cbSize = sizeof NOTIFYICONDATA;
    nid.uID = WM_IAWENTRAY;
    nid.hWnd = hMain;
    nid.hIcon = iMain;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_IAWENTRAY;    
    LoadString(hInst, IDS_TIPS, nid.szTip, 88);
    Shell_NotifyIcon(NIM_ADD, &nid);    
    Shell_NotifyIcon(NIM_SETVERSION, &nid);
	hList = GetDlgItem(hMain, IDC_LIST);
    hListRemind = GetDlgItem(hMain, IDC_LIST_REMIND);
    ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT |LVS_EX_GRIDLINES|LVS_EX_INFOTIP);
    ListView_SetExtendedListViewStyle(hListRemind, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES|LVS_EX_CHECKBOXES);
	LVCOLUMN lc;
	lc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_FMT;
	WCHAR szTitle[][6] = { L"标题" ,L"描述" ,L"价格",L"图片",L"值/赞",L"不值",L"收藏",L"评论",L"时间",L"平台/作者",L"链接",L"直达"};
	int iTitle[] = { 505,0,66,0,45,45,45,45,123,138,238,68 };
	for (int i = 0; i < 12; i++)
	{
		lc.cx = iTitle[i];
        if(i==2|| i == 4 || i == 5 || i == 6 || i == 7)
    		lc.fmt = LVCFMT_RIGHT;
        else if(i==1 || i == 3 || i == 10 || i == 11)
            lc.fmt = LVCFMT_LEFT;
        else
            lc.fmt = LVCFMT_CENTER;
		lc.pszText = szTitle[i];
		lc.iSubItem = i;
		ListView_InsertColumn(hList, i, &lc);
	}
    WCHAR szKey[][5] = { L"关键词",L"过滤词",L"最小价格",L"最大价格",L"平台"};
    int iKey[] = { 150,100,60,60,90};
    for (int i=0;i<5;i++)
    {
        lc.cx = iKey[i];
        lc.fmt = LVCFMT_CENTER;
        lc.pszText = szKey[i];
        lc.iSubItem = i;
        ListView_InsertColumn(hListRemind, i, &lc);
    }
    hCombo = GetDlgItem(hMain, IDC_COMBO);
    for (int i=0;i<24;i++)
    {
        SendMessage(hCombo, CB_ADDSTRING, NULL, (LPARAM)szBus[i]);
    }
    hComboTime = GetDlgItem(hMain, IDC_COMBO_TIME);
    for (int i=0;i<6;i++)
    {
        SendMessage(hComboTime, CB_ADDSTRING, NULL, (LPARAM)szTimes[i]);
    }
    hComboPage = GetDlgItem(hMain, IDC_COMBO_PAGE);
	for (int i = 0; i < 9; i++)
	{
		SendMessage(hComboPage, CB_ADDSTRING, NULL, (LPARAM)szPage[i]);
	}    
    ReadSet();
    SendMessage(hComboTime, CB_SETCURSEL, RemindSave.iTime, NULL);
    SendMessage(hComboPage, CB_SETCURSEL, RemindSave.iPage, NULL);
    CheckDlgButton(hMain, IDC_CHECK_OPEN_LINK, RemindSave.bDirectly);
    CheckDlgButton(hMain, IDC_CHECK_WXPUSHER, RemindSave.bWxPusher);
    CheckDlgButton(hMain, IDC_CHECK_TIPS, RemindSave.bTips);
    SetDlgItemText(hMain,IDC_UID, RemindSave.szWxPusherUID);
    CheckDlgButton(hMain, IDC_CHECK_WECHAT, RemindSave.bWeChat);
    SetDlgItemText(hMain, IDC_WECHAT_AID, RemindSave.szWeChatAgentId);
    SetDlgItemText(hMain, IDC_WECHAT_CORPID, RemindSave.szWeChatID);
    SetDlgItemText(hMain, IDC_WECHAT_SECRET,RemindSave.szWeChatSecret);
    SetDlgItemText(hMain, IDC_WECHAT_UID, RemindSave.szWeChatUserID);
    CheckDlgButton(hMain, IDC_CHECK_DINGDING, RemindSave.bDingDing);
    SetDlgItemText(hMain, IDC_ACCESS_TOKEN, RemindSave.szDingDingToken);
    CheckDlgButton(hMain, IDC_RADIO_LAST_SORT, RemindSave.bLastSort);
    CheckDlgButton(hMain, IDC_RADIO_SCORE_SORT, !RemindSave.bLastSort);
    CheckDlgButton(hMain, IDC_CHECK_AUTORUN, AutoRun(FALSE, FALSE, szAppName));
    if (riSize)
    {
		//	ListView_DeleteAllItems(hListRemind);
		int n = riSize / sizeof REMINDITEM;
		for (int i = 0; i < n; i++)
		{
			WCHAR sz[128];
			LVITEM li = { 0 };
			int iSub = 0;
			li.mask = LVIF_TEXT;
			li.pszText = lpRemindItem[i].szKey;
			li.iSubItem = iSub++;
			li.iItem = ListView_GetItemCount(hListRemind);
			li.iItem = ListView_InsertItem(hListRemind, &li);
            ListView_SetCheckState(hListRemind, li.iItem, !lpRemindItem[i].bNotUse);
			li.pszText = lpRemindItem[i].szFilter;
			li.iSubItem = iSub++;
			ListView_SetItem(hListRemind, &li);
			wsprintf(sz, L"%d", lpRemindItem[i].uMinPrice);
			li.pszText = sz;
			li.iSubItem = iSub++;
			ListView_SetItem(hListRemind, &li);
			wsprintf(sz, L"%d", lpRemindItem[i].uMaxPrice);
			li.pszText = sz;
			li.iSubItem = iSub++;
			ListView_SetItem(hListRemind, &li);
			li.pszText = szBus[lpRemindItem[i].iBusiness];
			li.iSubItem = iSub++;
			ListView_SetItem(hListRemind, &li);
/*
			wsprintf(sz, L"%d", lpRemindItem[i].iBusiness);
			li.pszText = sz;
			li.iSubItem = iSub++;
			ListView_SetItem(hListRemind, &li);
*/
		}        
//        SetTimer(hMain, 3, iTimes[RemindSave.iTime] * 60000, NULL);
//        SetTimer(hMain, 6, 8888, NULL);
    }
    else
        ShowWindow(hMain, SW_SHOW);
    hGetDataThread = CreateThread(NULL, 0, GetDataThreadProc, 0, 0, 0);    
/*
    HMODULE hWintoast = LoadLibrary(L"WinToast.dll");
    if (hWintoast)
    {
        typedef BOOL(WINAPI* pfnInit)(WCHAR* szAppName);
        pfnInit Init =(pfnInit)GetProcAddress(hWintoast, "Init");        
        if (Init)
        {
            if (Init(szAppName))
            {
                ShowToast = (pfnShowToast)GetProcAddress(hWintoast, "ShowToast");
                bNewTrayTips = TRUE;
            }
        }
    }
    if (bNewTrayTips == FALSE)
        FreeLibrary(hWintoast);
*/
	WinToast::isCompatible();
	WinToast::instance()->setAppName(szAppName);
	WinToast::instance()->setAppUserModelId(szAppName);
	bNewTrayTips = WinToast::instance()->initialize();
    bInit = FALSE;
    return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK MainProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    {
    }
    return TRUE;
/*
    case WM_TIMER:
        if (wParam==3||wParam==6)
        {
			if (wParam == 6)
				KillTimer(hWnd, wParam);
            SetWindowText(hMain, L"正在从网站获取并处理数据请稍后...");
            if(IsWindowVisible(hMain))
                SetCursor(LoadCursor(NULL,IDC_WAIT));
            int n = riSize / sizeof REMINDITEM;
            for (int i=0;i<n;i++)
            {
                if(!lpRemindItem[i].bNotUse)
                    SearchSMZDM(&lpRemindItem[i], FALSE,FALSE);
            }
            WriteSet(NULL);
            SetWindowText(hMain,nid.szTip);
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            bOpen = TRUE;
            return TRUE;
        }
        break;
*/
    case WM_IAWENTRAY:
//        if (HIWORD(lParam) == nid.uID)
        {
            if (LOWORD(lParam) == WM_LBUTTONDOWN || LOWORD(lParam) == WM_RBUTTONDOWN)
            {
                SetFocus(GetDlgItem(hMain, IDC_KEY));
                ShowWindow(hMain, SW_SHOW);
                SetForeground(hMain);
            }
            else if (LOWORD(lParam) == NIN_BALLOONUSERCLICK && !RemindSave.bDirectly)
            {
                WCHAR sz[128];
                wsprintf(sz, L"https://www.smzdm.com/p/%d", nid.uID);
                ShellExecute(NULL, L"open", sz, NULL, NULL, SW_SHOW);
            }
        }
        return TRUE;
    case WM_NOTIFY:
    {
        if (bInit)
            return FALSE;
/*
        if (wParam == IDC_SYSLINK1||wParam==IDC_SYSLINK2)
        {
            LPNMHDR lpnh = (LPNMHDR)lParam;
            if (lpnh->code == NM_CLICK || lpnh->code == NM_RETURN)
            {
                CloseHandle(ShellExecute(NULL, L"open", L"http://810619.xyz:888/index.php?share/folder&user=1&sid=Zk2Ecwbt", NULL, NULL, SW_SHOW));
            }
        }
*/
        if (wParam == IDC_LIST)
        {
            LPNMITEMACTIVATE lnia = (LPNMITEMACTIVATE)lParam;
            if (lnia->iItem != -1)
            {
                if (lnia->hdr.code == NM_RCLICK)
                {
                    WCHAR sz[256];
                    if (lnia->iSubItem == 11)
                    {
                        ListView_GetItemText(hList, lnia->iItem, 11, sz, 256);
                    }
                    else
                    {
                        ListView_GetItemText(hList, lnia->iItem, 10, sz, 256);
                    }
                    ShellExecute(NULL, L"open", sz, NULL, NULL, SW_SHOW);
                }
            }
            else
            {
				if (lnia->hdr.code == LVN_COLUMNCLICK)
				{
                    if (lnia->iSubItem == 4 || lnia->iSubItem == 6 || lnia->iSubItem == 7)
                        bSort = FALSE;
                    ListView_SortItemsEx(hList, CompareFunc, lnia->iSubItem);
                    bSort = !bSort;
//					ListView_SortItems(hList, CompareFunc, lnia->iSubItem);
				}
            }
        }
        else if (wParam == IDC_LIST_REMIND)
        {
            LPNMITEMACTIVATE lnia = (LPNMITEMACTIVATE)lParam;
            if (lnia->iItem == -1 || lnia->iItem > riSize / sizeof REMINDITEM)
                return FALSE;
            if (lnia->hdr.code == LVN_ITEMCHANGED)
			{
                
                DWORD o = lnia->uOldState & 0x2000;
                DWORD n = lnia->uNewState & 0x2000;
                if (o!=n)
                {
                    BOOL bCheck = ListView_GetCheckState(hListRemind, lnia->iItem);
                    if (lpRemindItem[lnia->iItem].bNotUse == bCheck)
                    {
                        lpRemindItem[lnia->iItem].bNotUse = !bCheck;
                        WriteSet(NULL);
                    }
                }
            }			
            if (lnia->hdr.code == NM_RCLICK)
            {
                if (!bGetData)
                {
                    if (MessageBox(hMain, L"确定要删除此项？", L"提示", MB_OKCANCEL | MB_ICONWARNING) == IDOK)
                    {
                        lpRemindItem[lnia->iItem].szKey[0] = L'\0';
                        ListView_DeleteItem(hListRemind, lnia->iItem);
                        WriteSet(NULL);
                        ReadSet();
                    }
                }
            }
            else if (lnia->hdr.code == NM_CLICK)
            {
                WCHAR sz[128];
                SetDlgItemText(hMain, IDC_KEY, lpRemindItem[lnia->iItem].szKey);
                SetDlgItemText(hMain, IDC_FILTER, lpRemindItem[lnia->iItem].szFilter);
                ListView_GetItemText(hListRemind, lnia->iItem, 2, sz, 32);
                SetDlgItemText(hMain, IDC_EDIT_MIN_PRICE, sz);
                ListView_GetItemText(hListRemind, lnia->iItem, 3, sz, 32);
                SetDlgItemText(hMain, IDC_EDIT_MAX_PRICE, sz);
                SendMessage(hCombo, CB_SETCURSEL, lpRemindItem[lnia->iItem].iBusiness, NULL);
            }
        }
    }
    break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDC_BUTTON_LINK1:
            case IDC_BUTTON_LINK2:
            case IDC_BUTTON_LINK3:
            case IDC_BUTTON_LINK4:
            case IDC_BUTTON_LINK5:
            case IDC_BUTTON_LINK6:
            case IDC_BUTTON_LINK7:
            case IDC_BUTTON_LINK8:
            {
                WCHAR szLink[][64] = { L"https://faxian.smzdm.com/9kuai9/h4s0t0f0p1/#filter-block",L"https://faxian.smzdm.com/h2s0t0f0c0p1/#filter-block",L"https://faxian.smzdm.com/h3s0t0f0c0p1/#filter-block",L"https://faxian.smzdm.com/h4s0t0f0c0p1/#filter-block",L"https://post.smzdm.com/",L"https://post.smzdm.com/hot_7/",L"https://test.smzdm.com/",L"https://duihuan.smzdm.com/"};
                ShellExecute(NULL, L"open", szLink[wmId-IDC_BUTTON_LINK1], NULL, NULL, SW_SHOW);
            }
                break;
            case IDC_REMIND:
            case IDC_SEARCH:
            case IDC_SMZDM_SEARCH:
            {
                bInit = TRUE;
                REMINDITEM ri={0};
                WCHAR sz[128];
                GetDlgItemText(hWnd, IDC_KEY, sz, 128);
                WCHAR *szKey = sz;
				while (szKey[0] == L' ')
				{
					szKey++;
				}
				if (lstrlen(szKey) == 0)
					return FALSE;
                lstrcpy(ri.szKey, szKey);
                GetDlgItemText(hWnd, IDC_FILTER, sz, 128);
                WCHAR* szFilter = sz;
                while (szFilter[0]==L' ')
                {
                    szFilter++;
                }
                lstrcpy(ri.szFilter, sz);
                ri.iBusiness = (int)SendMessage(hCombo, CB_GETCURSEL, NULL, NULL);
                bPost = IsDlgButtonChecked(hWnd, IDC_CHECK_POST);
                RemindSave.bLastSort = IsDlgButtonChecked(hWnd, IDC_RADIO_LAST_SORT);
                if (ri.iBusiness == -1)
                    ri.iBusiness = 0;
                ri.uMinPrice = GetDlgItemInt(hWnd, IDC_EDIT_MIN_PRICE, NULL, TRUE);
                ri.uMaxPrice = GetDlgItemInt(hWnd, IDC_EDIT_MAX_PRICE, NULL, TRUE);
                if (wmId == IDC_SMZDM_SEARCH)
                {
                    SearchSMZDM(&ri, TRUE, 0, TRUE);
                }
                else if (wmId == IDC_SEARCH)
                {
                    SearchSMZDM(&ri, TRUE, 0,FALSE);
                    int n = (int)SendMessage(hComboPage, CB_GETCURSEL, NULL, NULL);
                    for (int i = 1; i <= n; i++)
                    {
                        SearchSMZDM(&ri, TRUE, i + 1,FALSE);
                    }
                }
                else if (!bGetData)
                {
                    LVITEM li = { 0 };
                    li.mask = LVIF_TEXT;
                    BOOL bOur = FALSE;
                    int cur = ListView_GetNextItem(hListRemind, -1, LVNI_SELECTED);
                    int iSub = 0;
                    if (cur != -1)
                    {
                        ListView_GetItemText(hListRemind, cur, 0, sz, 127);

                        if (lstrcmp(ri.szKey, sz) == 0)
                            bOur = TRUE;
                    }
                    if (bOur)
                    {
                        li.iItem = cur;
                    }
                    else
                    {
                        li.pszText = ri.szKey;
                        li.iSubItem = iSub;
                        li.iItem = ListView_GetItemCount(hListRemind);
                        li.iItem = ListView_InsertItem(hListRemind, &li);
                        ListView_SetCheckState(hListRemind, li.iItem, TRUE);
                    }
                    iSub++;
                    li.pszText = ri.szFilter;
                    li.iSubItem = iSub++;
                    ListView_SetItem(hListRemind, &li);
                    wsprintf(sz, L"%d", ri.uMinPrice);
                    li.pszText = sz;
                    li.iSubItem = iSub++;
                    ListView_SetItem(hListRemind, &li);
                    wsprintf(sz, L"%d", ri.uMaxPrice);
                    li.pszText = sz;
                    li.iSubItem = iSub++;
                    ListView_SetItem(hListRemind, &li);
                    li.pszText = szBus[ri.iBusiness];
                    li.iSubItem = iSub++;
                    ListView_SetItem(hListRemind, &li);
                    if (bOur)
                    {
                        ri.uid = lpRemindItem[cur].uid;
                        ri.bNotUse = lpRemindItem[cur].bNotUse;
                        lpRemindItem[cur] = ri;
                        WriteSet(NULL);
                    }
                    else
                    {
                        WriteSet(&ri);
                        ListView_SetItemState(hListRemind, li.iItem, LVIS_SELECTED, LVIS_SELECTED);
                    }
                    ReadSet();
                }
                else
                    MessageBox(hMain,L"请稍等正在获取数据，等获取好了请再试一次！", L"提示", MB_OK);
                bInit = FALSE;
            }
            break;
            case IDC_SAVE:
                RemindSave.bDirectly = IsDlgButtonChecked(hWnd, IDC_CHECK_OPEN_LINK);
                RemindSave.bWxPusher = IsDlgButtonChecked(hWnd, IDC_CHECK_WXPUSHER);
                RemindSave.bTips = IsDlgButtonChecked(hWnd, IDC_CHECK_TIPS);
                GetDlgItemText(hWnd, IDC_UID, RemindSave.szWxPusherUID, 63);
                RemindSave.bWeChat = IsDlgButtonChecked(hWnd, IDC_CHECK_WECHAT);
                GetDlgItemText(hWnd, IDC_WECHAT_AID, RemindSave.szWeChatAgentId, 8);
                GetDlgItemText(hWnd, IDC_WECHAT_CORPID, RemindSave.szWeChatID, 24);
                GetDlgItemText(hWnd, IDC_WECHAT_SECRET, RemindSave.szWeChatSecret, 48);
                GetDlgItemText(hWnd, IDC_WECHAT_UID, RemindSave.szWeChatUserID, 64);
                RemindSave.bDingDing = IsDlgButtonChecked(hWnd, IDC_CHECK_DINGDING);
                GetDlgItemText(hWnd, IDC_ACCESS_TOKEN, RemindSave.szDingDingToken, 80);
                RemindSave.bLastSort = IsDlgButtonChecked(hWnd, IDC_RADIO_LAST_SORT);                
                RemindSave.iTime = (int)SendMessage(hComboTime, CB_GETCURSEL, NULL, NULL);
                RemindSave.iPage = (int)SendMessage(hComboPage, CB_GETCURSEL, NULL, NULL);
                WriteSet(NULL);
/*
                KillTimer(hMain, 3);
                SetTimer(hMain, 3, iTimes[RemindSave.iTime] * 60000, NULL);
                SetTimer(hMain, 6, 8888, NULL);
*/
                bResetTime = TRUE;
                break;
            case IDC_CHECK_AUTORUN:
                AutoRun(TRUE, IsDlgButtonChecked(hWnd, IDC_CHECK_AUTORUN), szAppName);
                break;
            case IDC_EXIT:
                if (MessageBox(hMain, L"确定要退出？退出后将无法推送！", L"提示", MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
                {
                    DestroyWindow(hWnd);
                    lpRemindData->bExit = TRUE;
                }
                break;
            case IDCLOSE:
            case IDCANCEL:
                EmptyProcessMemory(NULL);
                ListView_DeleteAllItems(hList);
                ShowWindow(hWnd, SW_HIDE);
                break;
            default:
                return FALSE;
            }
        }
        return TRUE;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return (INT_PTR)FALSE;
}
