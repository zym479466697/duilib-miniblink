#ifndef __WKEWEBKIT__H_
#define __WKEWEBKIT__H_
#include <queue>
#include <Windows.h>
#include <objbase.h>
#include "..\DuiLib\UIlib.h"
using namespace DuiLib;

#include "wke.h"
using namespace wke;

void LogFormat(const TCHAR* format, ...);

class IWkeEvent
{
public:
	//wke navigation callback
	virtual bool OnWkeNavigationCallback(wkeWebView webView, wkeNavigationType navigationType, const wkeString url) = 0;
	//wke loading callback
	virtual void OnWkeLoadingFinishCallback(wkeWebView webView, const wkeString url, wkeLoadingResult result, const wkeString failedReason) = 0;
	//wke title changed
	virtual void OnWkeTitleChanged(wkeWebView webView, const wkeString title) = 0;
	//wke url changed
	virtual void OnWkeUrlChanged(wkeWebView webView, const wkeString url) = 0;
};


class CWkeWebkitUI : 
	public CControlUI, 
	public IMessageFilterUI
{
public:
	CWkeWebkitUI(void);
	virtual ~CWkeWebkitUI(void);

	//init wke
	static void WkeWebkit_Init();
	//shutdown wke
	static void WkeWebkit_Shutdown();

	static void  OnWkeTitleChanged(wkeWebView webView, void* param, const wkeString title);
	static void  OnWkeUrlChanged(wkeWebView webView, void* param, const wkeString url);
	static void  OnWkeLoadingFinishCallback(wkeWebView webView, void* param, const wkeString url, wkeLoadingResult result, const wkeString failedReason);
	static bool  OnWkeNavigationCallback(wkeWebView webView, void* param, wkeNavigationType navigationType, const wkeString url);
	static void  OnWkePaintUpdatedCallback(wkeWebView webView, void* param, const HDC hdc, int x, int y, int cx, int cy);

	IWebView* GetWebView() { return m_pWebView;}
	//load url
	void LoadUrl(LPCTSTR pstrUrl);
	//load file
	void LoadFile(LPCTSTR pstrFile);
	//set wke event callback
	void SetWkeEvent(IWkeEvent* pWkeEvent);
	//run js
	jsValue RunJs(LPCTSTR pstrJs);
	//go back
	bool GoBack();
	//go forward
	bool GoForward();
protected:
	LPCTSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);
	void SetPos(RECT rc, bool bNeedInvalidate = true);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	bool DoPaint(HDC hDC, const RECT& rcPaint, CControlUI* pStopControl);
	virtual void DoInit();
	virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
	
	bool OnMouseEvent( UINT message, WPARAM wParam,LPARAM lParam);
	bool OnKeyDown( UINT uMsg, WPARAM wParam,LPARAM lParam);
	bool OnKeyUp( UINT uMsg, WPARAM wParam,LPARAM lParam);
	bool OnMouseWheel( UINT uMsg, WPARAM wParam,LPARAM lParam);
	bool OnChar( UINT uMsg, WPARAM wParam,LPARAM lParam);
	bool OnContextMenu( UINT uMsg, WPARAM wParam,LPARAM lParam);
	bool OnImeStartComposition( UINT uMsg, WPARAM wParam,LPARAM lParam);
	bool OnSetCursor();

	//wke need paint
	void OnPaintUpdatedCallback(const HDC hdc, int x, int y, int cx, int cy);
private:
	IWebView* m_pWebView;
	DWORD m_byAlpha;
	IWkeEvent* m_pWkeEvent;
};

#endif /*__WKEWEBKIT__H_*/