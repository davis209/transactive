/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/LiveStatusPage.h $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * LiveStatusPage controls shared view functionality for live status pages (views)
 *
 */

#if !defined(AFX_LiveStatusPage_B3711608_2C1B_4371_9411_7B156F95E8B9__INCLUDED_)
#define AFX_LiveStatusPage_B3711608_2C1B_4371_9411_7B156F95E8B9__INCLUDED_

#include "app/pa/PAManager/src/GenericStatusPage.h"

class LiveStatusModel;

class LiveStatusPage : public GenericStatusPage
{
public:

    /**
     * LiveStatusPage (constructor)
     *
     */
    LiveStatusPage(
        // song toan++
        // TD 10316
        bool isOCC,
        // song toan++
        // TD 10316
        UINT nIDTemplate, CWnd* pParentWnd = NULL);

protected:

    /**
     * OnInitDialog (extention of implementation)
     *
     */
    virtual BOOL    OnInitDialog();

    // Generated message map functions
    //{{AFX_MSG(LiveStatusPage)
    afx_msg void OnDestroy();
    //}}AFX_MSG

    // Required to allow use of MFC message map macros in source file
    DECLARE_MESSAGE_MAP()

    enum EPAAgentOperation
    {
        RETRY_FAILURES,
        RETRY_ALL
    };

    /**
     * attemptOperation
     *
     * Performs the specified agent operation, the operations are centralized
     * in this function to maintain shared exception handling (because all
     * functions are handled in the same manner)
     *
     * @param op the type of operation requested
     *
     */
    void    attemptOperation(const EPAAgentOperation op);

private:

    /**
     * getLiveStatusModel
     *
     * @return status model for this page
     *
     * @exception bad_cast if model doesn't exist / incorrect type (internal error)
     *
     */
    LiveStatusModel&    getLiveStatusModel();
};

#endif // !defined(AFX_LiveStatusPage_B3711608_2C1B_4371_9411_7B156F95E8B9__INCLUDED_)
