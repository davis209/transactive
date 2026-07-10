/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/WindowStub.h $
  * @author:  Jade Lee
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  * Provides a class allowing for unit testing of classes that post messages
  * to windows.  Creates a hidden window internally, and buffers all messages received
  *
  */

#pragma once

#include <windows.h>
#include <TestCase.h>
#include <TestFramework.h>

#include <map>

class WindowStub
{
public:
    struct Message
    {
        HWND hwnd;
        UINT message;
        WPARAM wParam;
        LPARAM lParam;
    };
    typedef std::vector<Message> MessageQueue;

    /// @exception std::exception if unable to create window
    WindowStub();
    /// Creates a window with custom type (ie a list control)
    WindowStub(LPCTSTR classIdentifier);
    virtual ~WindowStub();

    HWND    getHWnd() { return m_hWnd; }

    // Creates a CWnd object with specified ID - responsibility of client to delete this
    static CWnd* createMFCWindow(UINT nID = 1);

    ////////////////////////
    // Message buffer accessor functions - all automatically invoke processMsgQueue
    // in order to update m_received buffer (process windows message queue)

        /// @return all received messages
        const MessageQueue& getReceived() const;
        /// Searches for specified message, if found stores result in pResult   
        /// @return true if found the specified message
        bool    receivedMessage(UINT message, Message* pResult = 0) const;

        /// Resets / clears the message queue
        void reset();

    ////////////////////////
    ////////////////////////

    
    /// This needs to be called to grab all waiting messages (and place into receive buffer)
    /// (a PostMessage to our window will not be processed/received until this is called)
    static void    processMsgQueue();

private:

    void addToQueue(Message& msg) {     m_received.push_back(msg);  }

    /// Registers the window class that will be used to create our stub window
    static BOOL registerTestWindowClass();

    HWND m_hWnd;

    static LRESULT WINAPI WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    MessageQueue    m_received;

    // Map of WindowStub instances to their relative window handle
    static std::map<HWND, WindowStub*> s_instances;
    static bool s_wndClassRegistered;
};