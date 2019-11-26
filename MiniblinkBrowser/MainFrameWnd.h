#ifndef __MAINFRAMEWND__H_
#define __MAINFRAMEWND__H_

#include <windows.h>
#include <objbase.h>
#include "WkeWebkitUI.h"
#include "..\DuiLib\UIlib.h"
using namespace DuiLib;

#ifdef _DEBUG
#pragma comment(lib, "..\\bin\\Debug\\DuiLib.lib")
#else
#pragma comment(lib, "..\\bin\\Release\\DuiLib.lib")
#endif


class CMainFrameWnd : 
	public WindowImplBase,
	public IWkeEvent
{
public:
	CMainFrameWnd(void);
	~CMainFrameWnd(void);

	virtual CDuiString GetSkinFolder();
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName(void) const;
	void OnFinalMessage(HWND hWnd);
	CControlUI* CreateControl(LPCTSTR pstrClass);
protected:
	virtual void InitWindow();
	virtual void Notify(TNotifyUI& msg);

	//wke callback
	virtual bool OnWkeNavigationCallback(wkeWebView webView, wkeNavigationType navigationType, const wkeString url);
	virtual void OnWkeLoadingFinishCallback(wkeWebView webView, const wkeString url, wkeLoadingResult result, const wkeString failedReason);
	virtual void OnWkeTitleChanged(wkeWebView webView, const wkeString title);
	virtual void OnWkeUrlChanged(wkeWebView webView, const wkeString url);
private:
	CWkeWebkitUI* m_pWke;
};

#endif /*__MAINFRAMEWND__H_*/