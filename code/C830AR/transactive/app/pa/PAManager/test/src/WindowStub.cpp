/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/WindowStub.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#include "StdAfx.h"
#include "WindowStub.h"
#include <exception>

const std::string TEST_WINDOW_CLASS = "TestWindowClass";
const std::string TEST_WINDOW_TITLE = "WindowStub";

std::map<HWND, WindowStub*> WindowStub::s_instances;
bool WindowStub::s_wndClassRegistered = false;

WindowStub::WindowStub()
:
m_hWnd(0)
{
    if (registerTestWindowClass())
    {
        DWORD style = 0;

        m_hWnd = CreateWindowExA(0, 
                            TEST_WINDOW_CLASS.c_str(), 
                            TEST_WINDOW_TITLE.c_str(), 
                            style,
                            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
                            0, 
                            0, 
                            0,
                            NULL);
    }

    if (0 == m_hWnd)
    {
        throw std::exception("Unable to create window stub");
    }

    s_instances[m_hWnd] = this;
}

WindowStub::WindowStub(LPCTSTR classIdentifier)
:
m_hWnd(0)
{
    static bool ccInitialised = false;

    // Will fail if this has never been called
    if (!ccInitialised)
    {
        InitCommonControls();
        ccInitialised = true;
    }

    DWORD style = 0;

    m_hWnd = CreateWindowExA(0, 
                        classIdentifier, 
                        TEST_WINDOW_TITLE.c_str(), 
                        style,
                        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
                        0, 
                        0, 
                        0,
                        NULL);   

    if (0 == m_hWnd)
    {
        throw std::exception("Unable to create window stub");
    }

    s_instances[m_hWnd] = this;
}


CWnd* WindowStub::createMFCWindow(UINT nID)
{
    // This doesn't work properly as is, need a way to initialise AFX..
    /*if (registerTestWindowClass())
    {
        DWORD exStyle = WS_EX_APPWINDOW;
        DWORD style = 0;
        RECT rc = {0, 0, 1, 1};
        CWnd* window = new CWnd();
        window->CreateEx(exStyle, TEST_WINDOW_CLASS.c_str(), TEST_WINDOW_TITLE.c_str(), style, rc, NULL, nID);

        return window;
    }
    else*/
    {
        return 0;
    }
}


WindowStub::~WindowStub()
{
    std::map<HWND, WindowStub*>::iterator itr = s_instances.find(m_hWnd);

    if (itr != s_instances.end())
    {
        s_instances.erase(itr);
    }

    DestroyWindow(m_hWnd);
}


void WindowStub::reset()
{
    processMsgQueue();
    m_received.clear();
}


const WindowStub::MessageQueue& WindowStub::getReceived() const
{
    processMsgQueue();
    return m_received;
}


bool WindowStub::receivedMessage(UINT message, Message* pResult) const
{
    processMsgQueue();

    for (MessageQueue::const_iterator itr = m_received.begin();
            itr != m_received.end();
            itr ++)
    {
        if (itr->message == message)
        {
            if (0 != pResult)
            {
                *pResult = (*itr);
            }
            return true;
        }
    }

    return false;
}


void WindowStub::processMsgQueue()
{    
    MSG msg;
	::ZeroMemory(&msg, sizeof(msg));

    // Purge all waiting messages 
	while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
    {
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);		
	}
}


BOOL WindowStub::registerTestWindowClass()
{
    if (s_wndClassRegistered)
    {
        // Only need to register the class once
        return TRUE;
    }
    WNDCLASSA cls;

    cls.style = 0;
    cls.lpfnWndProc = WinProc;
    cls.cbClsExtra = 0;
    cls.cbWndExtra = 0;
    cls.hInstance = GetModuleHandleA(0);
    cls.hIcon = 0;
    cls.hCursor = LoadCursorA(0, (LPSTR)IDC_ARROW);
    cls.hbrBackground = (HBRUSH)::GetStockObject(WHITE_BRUSH);
    cls.lpszMenuName = NULL;
    cls.lpszClassName = TEST_WINDOW_CLASS.c_str();

    if(!RegisterClassA(&cls))
    {   
        return FALSE;
    }

    s_wndClassRegistered = true;
        
    return TRUE;
}


LRESULT WINAPI WindowStub::WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Message msg;
    msg.hwnd = hWnd;
    msg.message = message;
    msg.wParam = wParam;
    msg.lParam = lParam;

    // Catch the message and send to relevant stub
    // (on creation we miss some of the initial messages - as
    // the window hasn't been registered..could fix, but doesn't seem necessary)
    std::map<HWND, WindowStub*>::iterator itr = s_instances.find(hWnd);

    if (itr != s_instances.end())
    {
        itr->second->addToQueue(msg);
    }

    return DefWindowProcA(hWnd, message, wParam, lParam);
}


