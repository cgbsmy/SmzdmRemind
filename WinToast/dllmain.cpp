// dllmain.cpp : 定义 DLL 应用程序的入口点。
#ifdef WINTOAST_EXPORTS
#define WINTOAST_API __declspec(dllexport)
#else
#define WINTOAST_API __declspec(dllimport)
#endif


#include "pch.h"
#include "shellapi.h"
#include "wintoastlib.h"
//WCHAR szAppName[] = L"什么值得买提醒";

//using namespace std; 
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
extern "C" WINTOAST_API void ShowToast(WCHAR* szTitle, WCHAR* szBody, WCHAR* szImagePath, WCHAR* szLink)
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
extern "C" WINTOAST_API BOOL Init(WCHAR * szAppName)
{
	WinToast::isCompatible();
	WinToast::instance()->setAppName(szAppName);
	WinToast::instance()->setAppUserModelId(szAppName);
	return WinToast::instance()->initialize();
}
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

