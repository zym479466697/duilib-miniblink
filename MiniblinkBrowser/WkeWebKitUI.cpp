#include "WkeWebKitUI.h"
#include <Imm.h>
#include <assert.h>
#pragma comment(lib,"imm32.lib")
#pragma comment(lib,"msimg32.lib")

void LogFormat(const TCHAR* format, ...)
{
	TCHAR szLog[4096] = {0};
	va_list arglist;
	va_start(arglist, format);
	_vsnwprintf(szLog, 4096, format, arglist);
	va_end(arglist);
	OutputDebugString(szLog);
	OutputDebugString(_T("\r\n"));
}

CWkeWebKitUI::CWkeWebKitUI(void) : m_byAlpha(255)
{
	m_pWebView = NULL;
	m_pWkeEvent = NULL;
}


CWkeWebKitUI::~CWkeWebKitUI(void)
{
	if(m_pWebView)
	{
		wkeDestroyWebView((wkeWebView)m_pWebView);
		m_pWebView = NULL;
	}
}

void CWkeWebKitUI::WkeWebKit_Init()
{
	wkeInit();
}

void CWkeWebKitUI::WkeWebKit_Shutdown()
{
	wkeShutdown();
}


LPCTSTR CWkeWebKitUI::GetClass() const
{
	return _T("wkeWebKit");
}

LPVOID CWkeWebKitUI::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("wkeWebKit")) == 0 ) return static_cast<CWkeWebKitUI*>(this);
	return CControlUI::GetInterface(pstrName);
}

void CWkeWebKitUI::DoInit()
{
	CControlUI::DoInit();
	m_pWebView = (IWebView*)wkeCreateWebView();
	//wkeSetHandle((wkeWebView)m_pWebView, GetManager()->GetPaintWindow());
	//wkeSetTransparent((wkeWebView)m_pWebView, true);
	wkeOnTitleChanged((wkeWebView)m_pWebView, OnWkeTitleChanged, this);
	wkeOnURLChanged((wkeWebView)m_pWebView, OnWkeUrlChanged, this);
	wkeOnLoadingFinish((wkeWebView)m_pWebView,OnWkeLoadingFinishCallback,this);
	wkeOnNavigation((wkeWebView)m_pWebView, OnWkeNavigationCallback,this);
	wkeOnPaintUpdated((wkeWebView)m_pWebView, OnWkePaintUpdatedCallback,this);
	wkeSetUserAgent((wkeWebView)m_pWebView, "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/69.0.2228.0 Safari/537.36");

	GetManager()->AddMessageFilter(this);
}

void CWkeWebKitUI::SetPos(RECT rc, bool bNeedInvalidate)
{
	CControlUI::SetPos(rc, bNeedInvalidate);

	if(m_pWebView) {
		m_pWebView->resize(rc.right - rc.left, rc.bottom - rc.top);
		//wkeRepaintIfNeeded((wkeWebView)m_pWebView);
	}
}


void CWkeWebKitUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if( _tcscmp(pstrName, _T("alpha")) == 0 ) {
		LPTSTR pstr = NULL;
		m_byAlpha = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
	}
	else
	{
		CControlUI::SetAttribute(pstrName, pstrValue);
	}
}

bool CWkeWebKitUI::DoPaint(HDC hDC, const RECT& rcPaint, CControlUI* pStopControl)
{
	if(!m_pWebView)
	{
		return CControlUI::DoPaint(hDC, rcPaint, pStopControl);
	}
	
	RECT rcClip;
	::GetClipBox(hDC, &rcClip);
	RECT rcClient = GetClientPos();
	RECT rcInvalid;
	::IntersectRect(&rcInvalid, &rcClip,&rcClient);
	int invalidWidth = rcInvalid.right - rcInvalid.left;
	int invalidHeight = rcInvalid.bottom - rcInvalid.top;

	HDC hdcWke = wkeGetViewDC((wkeWebView)m_pWebView);
	BOOL ret = false;
	if(m_byAlpha != 0xff)
	{
		BLENDFUNCTION bf = {AC_SRC_OVER, 0, m_byAlpha, AC_SRC_ALPHA };
		ret = AlphaBlend(hDC, rcInvalid.left, rcInvalid.top, invalidWidth, invalidHeight,
			hdcWke, rcInvalid.left-rcClient.left, rcInvalid.top-rcClient.top, invalidWidth, invalidHeight, bf);
	}
	else
	{
		ret = BitBlt(hDC, rcInvalid.left, rcInvalid.top, invalidWidth, invalidHeight,
			hdcWke, rcInvalid.left-rcClient.left, rcInvalid.top-rcClient.top,SRCCOPY);
	}
	return ret;
}


LRESULT CWkeWebKitUI::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	bHandled = false;
	switch(uMsg)
	{
	case WM_SETFOCUS:
		{
			if(m_pWebView) m_pWebView->setFocus();
		}break;
	case WM_KILLFOCUS:
		{
			if(m_pWebView) m_pWebView->killFocus();
		}break;
	case WM_KEYDOWN:
		{
			if(!IsFocused()) return false;
			bHandled = OnKeyDown(uMsg, wParam, lParam);
			return true;
		}break;
	case WM_KEYUP:
		{
			if(!IsFocused()) return false;
			bHandled = OnKeyUp(uMsg, wParam, lParam);
			return true;
		}break;
	case WM_CHAR:
		{
			if(!IsFocused()) return false;
			bHandled = OnChar(uMsg, wParam, lParam);
			return true;
		}break;
	case WM_IME_STARTCOMPOSITION:
		{
			bHandled = OnImeStartComposition(uMsg, wParam, lParam);
			return true;
		}break;
	case WM_CONTEXTMENU:
		{
			bHandled = OnContextMenu(uMsg, wParam, lParam);
			return true;
		}
		break;
	default:
		{
			if(uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST)
			{
				if(uMsg == WM_MOUSEHWHEEL || uMsg == WM_MOUSEWHEEL)
				{
					if(!IsFocused()) return false;
					bHandled = OnMouseWheel(uMsg, wParam, lParam);
				}
				else
				{
					bHandled = OnMouseEvent(uMsg, wParam, lParam);
				}
				return true;
			}
		}break;
	}
	return false;
}

void CWkeWebKitUI::LoadUrl(LPCTSTR pstrUrl)
{
	m_pWebView->loadURL(pstrUrl);
}

void CWkeWebKitUI::LoadFile(LPCTSTR pstrFile)
{
	m_pWebView->loadFile(pstrFile);
}

void CWkeWebKitUI::SetWkeEvent(IWkeEvent* pWkeEvent)
{
	m_pWkeEvent = pWkeEvent;
}

jsValue CWkeWebKitUI::RunJs(LPCTSTR pstrJs)
{
	return m_pWebView->runJS(pstrJs);
}

bool CWkeWebKitUI::GoBack()
{
	return m_pWebView->goBack();
}

bool CWkeWebKitUI::GoForward()
{
	return m_pWebView->goForward();
}

bool CWkeWebKitUI::OnMouseEvent( UINT message, WPARAM wParam,LPARAM lParam)
{
	RECT rcClient = GetClientPos();
	POINT point = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
	if(!PtInRect(&rcClient, point))
	{
		if (message == WM_LBUTTONDOWN 
			|| message == WM_MBUTTONDOWN 
			|| message == WM_RBUTTONDOWN)
		{
			m_bFocused = false;
		}
		return false;
	}
	
	int x = GET_X_LPARAM(lParam)-rcClient.left;
	int y = GET_Y_LPARAM(lParam)-rcClient.top;

	if (message == WM_LBUTTONDOWN || message == WM_MBUTTONDOWN || message == WM_RBUTTONDOWN)
	{
		SetFocus();
		SetCapture(GetManager()->GetPaintWindow());
	}
	else if (message == WM_LBUTTONUP || message == WM_MBUTTONUP || message == WM_RBUTTONUP)
	{
		ReleaseCapture();
	}
	
	unsigned int flags = 0;

	if (wParam & MK_CONTROL)
		flags |= WKE_CONTROL;
	if (wParam & MK_SHIFT)
		flags |= WKE_SHIFT;

	if (wParam & MK_LBUTTON)
		flags |= WKE_LBUTTON;
	if (wParam & MK_MBUTTON)
		flags |= WKE_MBUTTON;
	if (wParam & MK_RBUTTON)
		flags |= WKE_RBUTTON;

	bool bHandled = m_pWebView->fireMouseEvent(message, x, y, flags);

	OnSetCursor();
	return bHandled;
}

bool CWkeWebKitUI::OnKeyDown( UINT uMsg, WPARAM wParam,LPARAM lParam )
{
	unsigned int flags = 0;
	if (HIWORD(lParam) & KF_REPEAT)
		flags |= WKE_REPEAT;
	if (HIWORD(lParam) & KF_EXTENDED)
		flags |= WKE_EXTENDED;

	bool bHandled = m_pWebView->fireKeyDownEvent(wParam, flags, false);
	return bHandled;
}

bool CWkeWebKitUI::OnKeyUp( UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	unsigned int flags = 0;
	if (HIWORD(lParam) & KF_REPEAT)
		flags |= WKE_REPEAT;
	if (HIWORD(lParam) & KF_EXTENDED)
		flags |= WKE_EXTENDED;

	bool bHandled = m_pWebView->fireKeyUpEvent(wParam, flags, false);
	return bHandled;
}

bool CWkeWebKitUI::OnMouseWheel( UINT uMsg, WPARAM wParam,LPARAM lParam )
{
	POINT pt;
	pt.x = GET_X_LPARAM(lParam);
	pt.y = GET_Y_LPARAM(lParam);

	RECT rc = GetClientPos();
	pt.x -= rc.left;
	pt.y -= rc.top;

	int delta = GET_WHEEL_DELTA_WPARAM(wParam);

	unsigned int flags = 0;

	if (wParam & MK_CONTROL)
		flags |= WKE_CONTROL;
	if (wParam & MK_SHIFT)
		flags |= WKE_SHIFT;

	if (wParam & MK_LBUTTON)
		flags |= WKE_LBUTTON;
	if (wParam & MK_MBUTTON)
		flags |= WKE_MBUTTON;
	if (wParam & MK_RBUTTON)
		flags |= WKE_RBUTTON;

	//flags = wParam;

	bool bHandled = m_pWebView->fireMouseWheelEvent(pt.x, pt.y, delta, flags);
	return bHandled;
}

bool CWkeWebKitUI::OnChar( UINT uMsg, WPARAM wParam,LPARAM lParam )
{
	unsigned int charCode = wParam;
	unsigned int flags = 0;
	if (HIWORD(lParam) & KF_REPEAT)
		flags |= WKE_REPEAT;
	if (HIWORD(lParam) & KF_EXTENDED)
		flags |= WKE_EXTENDED;

	//flags = HIWORD(lParam);

	bool bHandled = m_pWebView->fireKeyPressEvent(wParam, flags, false);
	return bHandled;
}

bool CWkeWebKitUI::OnContextMenu( UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	POINT pt;
	pt.x = GET_X_LPARAM(lParam);
	pt.y = GET_Y_LPARAM(lParam);

	if (pt.x != -1 && pt.y != -1)
		ScreenToClient(GetManager()->GetPaintWindow(), &pt);

	unsigned int flags = 0;
	if (wParam & MK_CONTROL)
		flags |= WKE_CONTROL;
	if (wParam & MK_SHIFT)
		flags |= WKE_SHIFT;

	if (wParam & MK_LBUTTON)
		flags |= WKE_LBUTTON;
	if (wParam & MK_MBUTTON)
		flags |= WKE_MBUTTON;
	if (wParam & MK_RBUTTON)
		flags |= WKE_RBUTTON;
	bool bHandled = m_pWebView->fireContextMenuEvent(pt.x, pt.y, flags);
	return bHandled;
}

bool CWkeWebKitUI::OnImeStartComposition( UINT uMsg, WPARAM wParam,LPARAM lParam )
{
	wkeRect caret = m_pWebView->getCaret();

	RECT rcClient = GetClientPos();

	CANDIDATEFORM form;
	form.dwIndex = 0;
	form.dwStyle = CFS_EXCLUDE;
	form.ptCurrentPos.x = caret.x + rcClient.left;
	form.ptCurrentPos.y = caret.y + caret.h + rcClient.top;
	form.rcArea.top = caret.y + rcClient.top;
	form.rcArea.bottom = caret.y + caret.h + rcClient.top;
	form.rcArea.left = caret.x + rcClient.left;
	form.rcArea.right = caret.x + caret.w + rcClient.left;
	COMPOSITIONFORM compForm;
	compForm.ptCurrentPos=form.ptCurrentPos;
	compForm.rcArea=form.rcArea;
	compForm.dwStyle=CFS_POINT;

	HWND hWnd=GetManager()->GetPaintWindow();
	HIMC hIMC = ImmGetContext(hWnd);
	ImmSetCandidateWindow(hIMC, &form);
	ImmSetCompositionWindow(hIMC,&compForm);
	ImmReleaseContext(hWnd, hIMC);
	return true;
}

bool CWkeWebKitUI::OnSetCursor()
{
	int cursorInfoType = wkeGetCursorInfoType((wkeWebView)m_pWebView);
	HCURSOR hCur = NULL;
	switch (cursorInfoType) {
	case WkeCursorInfoPointer:
		hCur = ::LoadCursor(NULL, IDC_ARROW);
		break;
	case WkeCursorInfoIBeam:
		hCur = ::LoadCursor(NULL, IDC_IBEAM);
		break;
	case WkeCursorInfoHand:
		hCur = ::LoadCursor(NULL, IDC_HAND);
		break;
	case WkeCursorInfoWait:
		hCur = ::LoadCursor(NULL, IDC_WAIT);
		break;
	case WkeCursorInfoHelp:
		hCur = ::LoadCursor(NULL, IDC_HELP);
		break;
	case WkeCursorInfoEastResize:
		hCur = ::LoadCursor(NULL, IDC_SIZEWE);
		break;
	case WkeCursorInfoNorthResize:
		hCur = ::LoadCursor(NULL, IDC_SIZENS);
		break;
	case WkeCursorInfoSouthWestResize:
	case WkeCursorInfoNorthEastResize:
		hCur = ::LoadCursor(NULL, IDC_SIZENESW);
		break;
	case WkeCursorInfoSouthResize:
	case WkeCursorInfoNorthSouthResize:
		hCur = ::LoadCursor(NULL, IDC_SIZENS);
		break;
	case WkeCursorInfoNorthWestResize:
	case WkeCursorInfoSouthEastResize:
		hCur = ::LoadCursor(NULL, IDC_SIZENWSE);
		break;
	case WkeCursorInfoWestResize:
	case WkeCursorInfoEastWestResize:
		hCur = ::LoadCursor(NULL, IDC_SIZEWE);
		break;
	case WkeCursorInfoNorthEastSouthWestResize:
	case WkeCursorInfoNorthWestSouthEastResize:
		hCur = ::LoadCursor(NULL, IDC_SIZEALL);
		break;
	default:
		hCur = ::LoadCursor(NULL, IDC_ARROW);
		break;
	}

	if (hCur) {
		::SetCursor(hCur);
		return true;
	}
	return false;
}


void CWkeWebKitUI::OnWkeTitleChanged(wkeWebView webView, void* param, const wkeString title)
{
	CWkeWebKitUI* pWkeUI =(CWkeWebKitUI*)param;
	IWebView* pWebView = (IWebView*)webView;
	if(pWkeUI && pWkeUI->m_pWkeEvent)
	{
		pWkeUI->m_pWkeEvent->OnWkeTitleChanged(webView, title);
	}
}

void CWkeWebKitUI::OnWkeUrlChanged(wkeWebView webView, void* param, const wkeString url)
{
	CWkeWebKitUI* pWkeUI =(CWkeWebKitUI*)param;
	IWebView* pWebView = (IWebView*)webView;
	if(pWkeUI && pWkeUI->m_pWkeEvent)
	{
		pWkeUI->m_pWkeEvent->OnWkeUrlChanged(webView, url);
	}
}

void CWkeWebKitUI::OnWkeLoadingFinishCallback(wkeWebView webView, void* param, const wkeString url, wkeLoadingResult result, const wkeString failedReason)
{
	CWkeWebKitUI* pWkeUI =(CWkeWebKitUI*)param;
	IWebView* pWebView = (IWebView*)webView;
	if(pWkeUI && pWkeUI->m_pWkeEvent)
	{
		pWkeUI->m_pWkeEvent->OnWkeLoadingFinishCallback(webView, url, result, failedReason);
	}
}


bool  CWkeWebKitUI::OnWkeNavigationCallback(wkeWebView webView, void* param, wkeNavigationType navigationType, const wkeString url)
{
	CWkeWebKitUI* pWkeUI =(CWkeWebKitUI*)param;
	IWebView* pWebView = (IWebView*)webView;
	if(pWkeUI && pWkeUI->m_pWkeEvent)
	{
		return pWkeUI->m_pWkeEvent->OnWkeNavigationCallback(webView, navigationType, url);
	}
	return true;
}


void CWkeWebKitUI::OnWkePaintUpdatedCallback(wkeWebView webView, void* param, const HDC hdc, int x, int y, int cx, int cy)
{
	CWkeWebKitUI* pWkeUI =(CWkeWebKitUI*)param;
	IWebView* pWebView = (IWebView*)webView;
	if(pWkeUI && pWebView == pWkeUI->m_pWebView)
	{
		pWkeUI->OnPaintUpdatedCallback(hdc, x,y,cx,cy);
	}
}

void CWkeWebKitUI::OnPaintUpdatedCallback(const HDC hdc, int x, int y, int cx, int cy)
{
	//RECT invalidateRc = {x, y, cx, cy};
	//if( m_pManager != NULL ) m_pManager->Invalidate(invalidateRc);
	Invalidate();
}