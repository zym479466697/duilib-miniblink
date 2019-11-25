#include "MainFrameWnd.h"

// js call c++
jsValue JS_CALL jsCallCpp(jsExecState es)
{
	const wchar_t *szParam = jsToTempStringW(es, jsArg(es, 0));
	int intParam = jsToInt(es, jsArg(es, 1));
	ShellExecute(NULL, _T("open"), szParam, NULL, NULL, SW_SHOW);
	//MessageBox(NULL, szParam, _T("js调用c++"), MB_OK);
	return jsUndefined();
}

CMainFrameWnd::CMainFrameWnd(void)
{
	m_pWke = NULL;
}


CMainFrameWnd::~CMainFrameWnd(void)
{
}


CDuiString CMainFrameWnd::GetSkinFolder()
{
	return _T("");
}

CDuiString CMainFrameWnd::GetSkinFile()
{
	return _T("skin.xml");
}

LPCTSTR CMainFrameWnd::GetWindowClassName(void) const
{
	return _T("MainFrameWnd");
}

CControlUI* CMainFrameWnd::CreateControl(LPCTSTR pstrClass)
{
	if( _tcscmp(pstrClass, _T("wkeWebKit")) == 0 ) 
		return new CWkeWebKitUI;
	
	return NULL;
}

void CMainFrameWnd::OnFinalMessage(HWND hWnd)
{
	WindowImplBase::OnFinalMessage(hWnd);
	delete this;
}

std::wstring GetExeDir()     
{      
	TCHAR exeFullPath[MAX_PATH]; // Full path   
	std::wstring strPath = L"";   
	GetModuleFileName(NULL, exeFullPath, MAX_PATH);   
	strPath = (std::wstring)exeFullPath;    // Get full path of the file   
	int pos = strPath.find_last_of(L'\\', strPath.length());   
	return strPath.substr(0, pos);  // Return the directory without the file name   
}

void CMainFrameWnd::InitWindow()
{
	//js调用c++
	jsBindFunction("jsCallCpp", jsCallCpp, 2);

	m_pWke = static_cast<CWkeWebKitUI*>(m_PaintManager.FindControl(_T("wke")));
	m_pWke->SetWkeEvent(this);
	//js调用c++测试
	TCHAR exePath[1024] = {0};
	wsprintf(exePath, _T("file:///%s/html/jsTest.html"), GetExeDir().c_str());
	m_pWke->LoadFile(exePath);
}

void CMainFrameWnd::Notify(TNotifyUI& msg)
{
	if( msg.sType == _T("click") ) {
		CDuiString name = msg.pSender->GetName();
		if( name == _T("closebtn")) {
			PostQuitMessage(0);
			return; 
		}
		else if( name == _T("minbtn")) {
			SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0); 
			return; 
		}
		else if( name == _T("GoBackBtn")) {
			m_pWke->GoBack();
			return; 
		}
		else if( name == _T("GoForwardBtn")) {
			m_pWke->GoForward();
			return; 
		}
		else if( name == _T("RefreshBtn")) {
			m_pWke->GetWebView()->reload();
			return; 
		}
		else if( name == _T("HomeBtn")) {
			m_pWke->LoadUrl(_T("http://www.flrhbz.com"));
			return; 
		}
		else if( name == _T("GoBtn")) {
			CEditUI* pEditUrl = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("EditUrl")));
			if(pEditUrl)
			{
				m_pWke->LoadUrl(pEditUrl->GetText());
			}
			return; 
		}
	}
	WindowImplBase::Notify(msg);
}


bool CMainFrameWnd::OnWkeNavigationCallback(wkeWebView webView, wkeNavigationType navigationType, const wkeString url)
{
	return true;
}

void CMainFrameWnd::OnWkeLoadingFinishCallback(wkeWebView webView, const wkeString url, wkeLoadingResult result, const wkeString failedReason)
{
	if(m_pWke->GetWebView() != (IWebView*)webView) return;
	switch (result)
	{
	case WKE_LOADING_SUCCEEDED:
		{
			//C++调用js
			m_pWke->RunJs(_T("cppCallJs();"));
		}
		break;
	case WKE_LOADING_FAILED:
	case WKE_LOADING_CANCELED:
		break;
	}
}

void CMainFrameWnd::OnWkeTitleChanged(wkeWebView webView, const wkeString title)
{

}

void CMainFrameWnd::OnWkeUrlChanged(wkeWebView webView, const wkeString url)
{
	if(m_pWke->GetWebView() == (IWebView*)webView)
	{
		CEditUI* pEditUrl = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("EditUrl")));
		pEditUrl->SetText(wkeGetStringW(url));
	}
}
