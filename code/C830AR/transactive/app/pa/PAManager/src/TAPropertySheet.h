/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/TAPropertySheet.h $
 * @author:  Ripple
 * @version: $Revision: #2 $
 *
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by:  $Author: limin.zhu $
 *
 */
#if !defined(AFX_TAPROPERTYSHEET_H__F59F5AFF_C911_4D4E_B0CF_58AEAFE5B8EC__INCLUDED_)
#define AFX_TAPROPERTYSHEET_H__F59F5AFF_C911_4D4E_B0CF_58AEAFE5B8EC__INCLUDED_

#if _MSC_VER > 1000
    #pragma once
#endif // _MSC_VER > 1000
// TApropertysheet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// TAPropertySheet

class TAPropertySheet : public CPropertySheet
{
protected:

    //	void BuildPropPageArray();
    RECT m_rctPage;
    CFont m_fntPage;

    DECLARE_DYNAMIC(TAPropertySheet)

    // Construction

public:

    TAPropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
    TAPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

    // Attributes

public:

    /**
     * setParentToReceiveNotifyMessages (simple accessor)
     */
    void setParentToReceiveNotifyMessages(bool bSet) {
        m_sendNotifyToParent = bSet;
    }

    // Operations

public:

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(TAPropertySheet)

public:

    virtual BOOL OnInitDialog();

protected:

    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    //}}AFX_VIRTUAL

    // Implementation

public:

    virtual ~TAPropertySheet();

    // Generated message map functions

protected:

    virtual void BuildPropPageArray ();
    //{{AFX_MSG(TAPropertySheet)
    //}}AFX_MSG
    afx_msg LONG OnResizePage (UINT, LONG);
    DECLARE_MESSAGE_MAP()

private:

    // Set this to true to send all notify messages to our parent window (also)
    bool    m_sendNotifyToParent;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TAPROPERTYSHEET_H__F59F5AFF_C911_4D4E_B0CF_58AEAFE5B8EC__INCLUDED_)
