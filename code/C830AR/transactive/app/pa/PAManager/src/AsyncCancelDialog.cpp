/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/AsyncCancelDialog.cpp $
 * @author:  Ripple
 * @version: $Revision: #2 $
 *
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by:  $Author: limin.zhu $
 *
 */
#include "StdAfx.h"
#include "app/pa/PAManager/src/AsyncCancelDialog.h"
#include "app/pa/PAManager/src/IAsyncCancelCondition.h"

enum { TIMER_ID = 0 };

AsyncCancelDialog::AsyncCancelDialog(CWnd* pParent, DWORD interval, IAsyncCancelCondition* callback)
    :
    CDialog(IDD_DIALOGOVERWRITING_BROADCAST, pParent),
    m_hTimer(0),
    m_interval(interval),
    m_callback(callback)
{
}

AsyncCancelDialog::~AsyncCancelDialog()
{
    // Close this window if still open
    if (0 != m_hWnd)
    {
        DestroyWindow();
    }
}

BEGIN_MESSAGE_MAP(AsyncCancelDialog, CDialog)
    //{{AFX_MSG_MAP(AsyncCancelDialog)
    ON_WM_TIMER()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL AsyncCancelDialog::OnInitDialog()
{
    m_hTimer = SetTimer(TIMER_ID, m_interval, NULL);

    // If we couldn't initialise timer (0 returned), means user must cancel manually..
    ASSERT(m_hTimer);

    return CDialog::OnInitDialog();
}

void AsyncCancelDialog::OnTimer( UINT nIDEvent )
{
    if (TIMER_ID == nIDEvent)
    {
        // Check to see if dialog should be closed
        if (0 != m_callback)
        {
            // The class passed into constructor is indicating
            // dialog may terminate early
            if (m_callback->shouldCancel())
            {
                OnCancel();
            }
            else
            {
                updateProgress();
            }
        }
    }
    else
    {
        TA_ASSERT(FALSE, "Unexpected timer ID");
    }

    return;
}

void AsyncCancelDialog::updateProgress()
{
    // Continue waiting - indicate progress by updating '...' on main text
    CWnd* mainText = GetDlgItem(IDC_MAINTEXT);

    if (0 != mainText)
    {
        CString rString;
        mainText->GetWindowText(rString);

        if (0 == m_originalText.length())
        {
            m_originalText = rString.GetBuffer(0);
        }

        enum { DOT_COUNT = 2 };

        // Continue appending up to 'DOT_COUNT' dots (one at a time), once
        // exceeded, revert the text back to 'm_originalText'
        if (rString.GetLength() < (int)(m_originalText.length() + DOT_COUNT))
        {
            rString += ".";
        }
        else
        {
            rString = m_originalText.c_str();
        }

        // Update window text with updated progress string
        mainText->SetWindowText(rString);
    }
}
