/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/GenericStatusPageStub.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#pragma once

#include "app/pa/pamanager/src/GenericStatusPage.h"
#include "WindowStub.h"

class GenericStatusPageStub : public GenericStatusPage
{
public:
    
    GenericStatusPageStub() 
    : 
    GenericStatusPage(0, 0) 
    {
        m_list.Attach(m_listWindowStub.getHWnd());
        m_button.Attach(m_buttonWindowStub.getHWnd());
    }

    virtual ~GenericStatusPageStub()
    {
        m_list.Detach();
        m_button.Detach();
    }
    
    virtual HWND    getHWnd() { return m_windowStub.getHWnd(); }
    virtual afx_msg void OnCancel() {}
    virtual void onBroadcastStateChanged(const TA_IRS_Bus::IPAAgentCorbaDef::EBroadcastState& newState) {}
    virtual CListCtrl& getFailuresListControl() { return m_list; }
    virtual CListCtrl& getProgressListControl() { return m_list; }

    GenericStatusModel&     getStatusModel() { return *m_statusModel; }
    virtual GenericStatusModel*     getStatusModelPointer() { return m_statusModel; }
    
    virtual void    destroyStatusModel() { }
    virtual CButton& getRetryAllButton() { return m_button; }
    virtual CButton& getRetryFailedButton() { return m_button; }
    WindowStub m_windowStub;
    WindowStub m_listWindowStub;
    WindowStub m_buttonWindowStub;

    CListCtrl m_list;
    CButton m_button;

    // Expect this to be set externally
    GenericStatusModel* m_statusModel;
};
