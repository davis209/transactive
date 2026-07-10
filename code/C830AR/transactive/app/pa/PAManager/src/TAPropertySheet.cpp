/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/TAPropertySheet.cpp $
 * @author:  Ripple
 * @version: $Revision: #2 $
 *
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by:  $Author: limin.zhu $
 *
 */
// TApropertysheet.cpp : implementation file
//

#include <afxpriv.h>
#include "app/pa/PAManager/src/stdafx.h"
#include "app/pa/PAManager/src/TApropertysheet.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
#endif

#define WM_RESIZEPAGE WM_APP+1
static const double TNE_WIDTH_ADJUSTMENT = (double)20 / 18;

enum { CDF_CENTER, CDF_TOPLEFT, CDF_NONE };

// helper function which sets the font for a window and all its children
// and also resizes everything according to the new font
void ChangeDialogFont(CWnd* pWnd, CFont* pFont, int nFlag)
{
    CRect windowRect;

    // grab old and new text metrics
    TEXTMETRIC tmOld, tmNew;
    CDC * pDC = pWnd->GetDC();
    CFont * pSavedFont = pDC->SelectObject(pWnd->GetFont());
    pDC->GetTextMetrics(&tmOld);
    pDC->SelectObject(pFont);
    pDC->GetTextMetrics(&tmNew);
    pDC->SelectObject(pSavedFont);
    pWnd->ReleaseDC(pDC);

    long oldHeight = tmOld.tmHeight + tmOld.tmExternalLeading;
    long newHeight = tmNew.tmHeight + tmNew.tmExternalLeading;

    if (nFlag != CDF_NONE)
    {
        // calculate new dialog window rectangle
        CRect clientRect, newClientRect, newWindowRect;

        pWnd->GetWindowRect(windowRect);
        pWnd->GetClientRect(clientRect);
        long xDiff = windowRect.Width() - clientRect.Width();
        long yDiff = windowRect.Height() - clientRect.Height();

        newClientRect.left = newClientRect.top = 0;
        newClientRect.right = (long)
                              ((double) clientRect.right * TNE_WIDTH_ADJUSTMENT * tmNew.tmAveCharWidth / tmOld.tmAveCharWidth);
        newClientRect.bottom = clientRect.bottom * newHeight / oldHeight;

        if (nFlag == CDF_TOPLEFT) // resize with origin at top/left of window
        {
            newWindowRect.left = windowRect.left;
            newWindowRect.top = windowRect.top;
            newWindowRect.right = windowRect.left + newClientRect.right + xDiff;
            newWindowRect.bottom = windowRect.top + newClientRect.bottom + yDiff;
        }
        else if (nFlag == CDF_CENTER) // resize with origin at center of window
        {
            newWindowRect.left = windowRect.left -
                                 (newClientRect.right - clientRect.right) / 2;
            newWindowRect.top = windowRect.top -
                                (newClientRect.bottom - clientRect.bottom) / 2;
            newWindowRect.right = newWindowRect.left + newClientRect.right + xDiff;
            newWindowRect.bottom = newWindowRect.top + newClientRect.bottom + yDiff;
        }

        pWnd->MoveWindow(newWindowRect);
    }

    pWnd->SetFont(pFont);

    // iterate through and move all child windows and change their font.
    CWnd* pChildWnd = pWnd->GetWindow(GW_CHILD);

    while (pChildWnd)
    {
        pChildWnd->SetFont(pFont);
        pChildWnd->GetWindowRect(windowRect);

        CString strClass;
        ::GetClassName(pChildWnd->m_hWnd, strClass.GetBufferSetLength(32), 31);
        strClass.MakeUpper();

        if (strClass == _T("COMBOBOX"))
        {
            CRect rect;
            pChildWnd->SendMessage(CB_GETDROPPEDCONTROLRECT, 0, (LPARAM) &rect);
            windowRect.right = rect.right;
            windowRect.bottom = rect.bottom;
        }

        pWnd->ScreenToClient(windowRect);
        windowRect.left = (long)
                          ((double) windowRect.left * TNE_WIDTH_ADJUSTMENT * tmNew.tmAveCharWidth / tmOld.tmAveCharWidth);
        windowRect.right = (long)
                           ((double) windowRect.right * TNE_WIDTH_ADJUSTMENT * tmNew.tmAveCharWidth / tmOld.tmAveCharWidth);
        windowRect.top = windowRect.top * newHeight / oldHeight;
        windowRect.bottom = windowRect.bottom * newHeight / oldHeight;
        pChildWnd->MoveWindow(windowRect);

        pChildWnd = pChildWnd->GetWindow(GW_HWNDNEXT);
    }
}

/////////////////////////////////////////////////////////////////////////////
// TAPropertySheet

IMPLEMENT_DYNAMIC(TAPropertySheet, CPropertySheet)

TAPropertySheet::TAPropertySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
    : CPropertySheet(nIDCaption, pParentWnd, iSelectPage),
      m_sendNotifyToParent(false)
{
}

TAPropertySheet::TAPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
    : CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}

TAPropertySheet::~TAPropertySheet()
{
    if (m_fntPage.m_hObject)
        VERIFY (m_fntPage.DeleteObject ());
}

BEGIN_MESSAGE_MAP(TAPropertySheet, CPropertySheet)
    //{{AFX_MSG_MAP(TAPropertySheet)
    //}}AFX_MSG_MAP
    ON_MESSAGE (WM_RESIZEPAGE, OnResizePage)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TAPropertySheet message handlers

void TAPropertySheet::BuildPropPageArray()
{
    CPropertySheet::BuildPropPageArray();

    // get first page
    CPropertyPage* pPage = GetPage (0);
    ASSERT (pPage);

    // dialog template class in afxpriv.h
    CDialogTemplate dlgtemp;
    // load the dialog template
    VERIFY (dlgtemp.Load (pPage->m_psp.pszTemplate));
    // get the font information
    CString strFace;
    WORD	wSize;
    VERIFY (dlgtemp.GetFont (strFace, wSize));

    if (m_fntPage.m_hObject)
        VERIFY (m_fntPage.DeleteObject ());

    // create a font using the info from first page
    VERIFY (m_fntPage.CreatePointFont (wSize * 10, strFace));
}

BOOL TAPropertySheet::OnInitDialog()
{
    ModifyStyleEx (0, WS_EX_CONTROLPARENT);

    // TODO fix the following so it only does this if not modal!
    HWND hWnd = (HWND)::GetDlgItem(m_hWnd, AFX_IDC_TAB_CONTROL);
    CRect rectOld;
    ::GetWindowRect(hWnd, &rectOld);
    ScreenToClient(rectOld);
    int nCx = rectOld.Width() / 1;
    int nCy = rectOld.Height() / 1;
    rectOld.right -= nCx;
    rectOld.bottom -= nCy;
    // move tab control
    ::SetWindowPos(hWnd, NULL, 0, 0, rectOld.Width(), rectOld.Height(),
                   SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    GetWindowRect(&rectOld);
    rectOld.right -= nCx;
    rectOld.bottom -= nCy;
    SetWindowPos(NULL, 0, 0, rectOld.Width(), rectOld.Height(),
                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

    CPropertySheet::OnInitDialog();

    // get the font for the first active page
    CPropertyPage* pPage = GetActivePage ();
    ASSERT (pPage);

    // change the font for the sheet
    ChangeDialogFont (this, &m_fntPage, CDF_CENTER);

    // change the font for each page
    for (int iCntr = 0; iCntr < GetPageCount (); iCntr++)
    {
        VERIFY (SetActivePage (iCntr));
        CPropertyPage* pPage = GetActivePage ();
        ASSERT (pPage);
        ChangeDialogFont (pPage, &m_fntPage, CDF_CENTER);
    }

    VERIFY (SetActivePage (pPage));

    // set and save the size of the page
    CTabCtrl* pTab = GetTabControl ();
    ASSERT (pTab);

    if (m_psh.dwFlags & PSH_WIZARD)
    {
        pTab->ShowWindow (SW_HIDE);
        GetClientRect (&m_rctPage);

        CWnd* pButton = GetDlgItem (ID_WIZBACK);
        ASSERT (pButton);
        CRect rc;
        pButton->GetWindowRect (&rc);
        ScreenToClient (&rc);
        m_rctPage.bottom = rc.top - 2;
    }
    else
    {
        pTab->GetWindowRect (&m_rctPage);
        ScreenToClient (&m_rctPage);
        pTab->AdjustRect (FALSE, &m_rctPage);
    }

    // resize the page
    pPage->MoveWindow (&m_rctPage);

    return TRUE;
}

BOOL TAPropertySheet::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
    BOOL result = CPropertySheet::OnNotify(wParam, lParam, pResult);
    NMHDR* pnmh = (LPNMHDR) lParam;

    // the sheet resizes the page whenever it is activated so we need to size it correctly
    if (TCN_SELCHANGE == pnmh->code)
    {
        PostMessage (WM_RESIZEPAGE);

        // Allow parent to receive this notification
        if (m_sendNotifyToParent)
        {
            TA_ASSERT(0 != GetParent(), "Parent invalid");
            // So lParam doesn't get blown away before parent gets message
            // wait until parent processes the message
            GetParent()->SendNotifyMessage(WM_NOTIFY, (WPARAM)(this), lParam);
        }
    }

    return result;
}

LONG TAPropertySheet::OnResizePage (UINT, LONG)
{
    // resize the page
    CPropertyPage* pPage = GetActivePage ();
    ASSERT (pPage);
    pPage->MoveWindow (&m_rctPage);

    return 0;
}

BOOL TAPropertySheet::OnCommand(WPARAM wParam, LPARAM lParam)
{
    // the sheet resizes the page whenever the Apply button is clicked so we need to size it correctly
    if (ID_APPLY_NOW == wParam ||
    ID_WIZNEXT == wParam ||
    ID_WIZBACK == wParam)
    {
        PostMessage (WM_RESIZEPAGE);
    }

    return CPropertySheet::OnCommand(wParam, lParam);
}
