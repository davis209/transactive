/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/IGenericBroadcastPage.h $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * IGenericBroadcastPage is interface for use by generic broadcast model
 *
 */

#if !defined(AFX_IGenericBroadcastPage_H__4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_)
#define AFX_IGenericBroadcastPage_H__4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_

class IGenericBroadcastPage
{
public:

    // Max # characters allowed to enter into filter text boxes
    // (taken from design doc)
    enum { MAX_FILTER_TEXT = 40 };

    /**
     * setTimeStartDtp
     *
     * Updates the start time controls to the new date
     *
     * @param time the new date to apply to the start time
     *
     */
    virtual void setTimeStartDtp(const CTime& time) = 0;

    /**
     * setTimeStopDtp
     *
     * Updates the stop time controls to the new date
     *
     * @param time the new date to apply to the start time
     *
     */
    virtual void setTimeStopDtp(const CTime& time) = 0;

    virtual void updateExecuteButtonEnabledState() = 0;

    virtual CWnd*       getWindow() = 0;

protected:

    /**
     * processStartStopTimeChange
     *
     * Must implement (and call) a function that validates
     * the values entered into the start and stop times of broadcast
     * dialog (relevant for DVA section only)
     */
    virtual void processStartStopTimeChange() = 0;
};

#endif // !defined(AFX_IGenericBroadcastPage_H__4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_)
