/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/PALayoutHelper.h $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by: $Author: limin.zhu $
 *
 * PaLayoutHelper is used to allow centralize the formatting of specific controls
 * so that code is not duplicated throughout the PA Manager Module
 *
 */

#if !defined(PaLayoutHelper_BB23A710_0466_469f_952A_E7A090EFB3CD__INCLUDED_)
#define PaLayoutHelper_BB23A710_0466_469f_952A_E7A090EFB3CD__INCLUDED_

#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"
#include "app/pa/PAManager/src/DvaListCtrl.h"

class PaLayoutHelper
{
public:

    /**
     * convertFromUTC converts a time defined as an absolute (UTC) value into
     *      a local time (which is time zone dependent)
     */
    static time_t convertFromUTC(time_t utcTime);

    /**
     * convertToUTC converts a time defined as a local (time zone dependent) time into
     *  an absolute (UTC) time
     */
    static time_t convertToUTC(time_t localTime);

    enum EConversion
    {
        FROM_UTC,
        TO_UTC
    };
    /// Helper class for convertFromUTC / convertToUTC
    static time_t convertTime(time_t time, const EConversion& conversion);

    /**
     * formatSelectedMessageList
     *
     * Applies the default formatting / header setup to specified DVA list control
     *
     * @param listToUpdate the list control to apply the formatting to
     *
     * @param messageLabel the label to give to the first 'message' column
     *
     * @param styleFlags (optional) if the user wants to override the default 'style' of list control
     *
     * @param accountForScrollBar true if we want to account for the width of the scroll
     *          bar when determing the appropriate widths of the colummns (such that
     *          no scroll bar appears on the bottom of list, when the vertical scroll bar appears)
     *
     */
    static void formatDvaMessageList(CListCtrl& listToUpdate,
                                     const std::string& selectedMessageLabel,
                                     bool accountForScrollBar = true,
                                     const DWORD styleFlags = LVS_EX_FULLROWSELECT);

    /**
     * setUpDateTimeControls
     *
     * Helper function to set suitable formatting values for date/time combination,
     * These formats are applied to the input parameters - any of the arguments
     *  may be null, in which case no formatting will be applied
     *
     * @param date - control for setting the day/month date
     *
     * @param timeOfDay - control for setting the time of day
     *
     */
    static void     setUpDateTimeControls(CDateTimeCtrl* date,
                                          CDateTimeCtrl* timeOfDay);

    /**
     * getSecondsFromControls
     *
     * Returns the number of seconds contained in the specified controls
     * (collectively) - if no pDateControl parameter provided (ie is null),
     * then todays date is used - any of the input parameters may be null.
     * may be NULL to indicate not to consider that parameter in the
     * calculation (hence use of pointers for arguments).
     * The reason we split hours, minutes and seconds into separate parameters is
     *  because the control stores arbitrary values in, for example, hours field, for
     *  a control who is only supposed to be defining seconds (and so on).
     * Typical usage is to have pHourControl==pMinuteControl==pSecondControl
     *
     * @param pDateControl the date (day / month / year) value
     *
     * @param pHourControl format "H", the hour of day **24 hour clock, 0-23**
     *
     * @param pMinuteControl format "mm", the current minute, 0-59
     *
     * @param pSecondControl format "ss", the current second, 0-59
     *
     * @return the number of seconds which can be used to generate a valid CTime object
     *          in the current time zone (using the CTime constructor)
     *
     */
    static time_t getSecondsFromControls(const CDateTimeCtrl* pDateControl,
                                         const CDateTimeCtrl* pHourControl,
                                         const CDateTimeCtrl* pMinuteControl,
                                         const CDateTimeCtrl* pSecondControl);

    /**
     * fetchStringFromControl
     *
     * Returns string contained by input control
     *
     * @param sourceControl the control which the string returned is taken from
     *
     * @returns the string contained by the input control
     *
     */
    static std::string  fetchStringFromControl(const CEdit& sourceControl);

    /**
     * getListHeaderWidth
     *
     * @param targetList the list we want to calculate header width of
     *
     * @param accountForScrollBar true if want to subtract with of default scroll bar for result
     *
     * @return total width of the list header
     *
     */
    static int getListHeaderWidth(CListCtrl& targetList, bool accountForScrollbar);

    /**
     * appendStringToStatusWindow
     *
     * Appends the string defined by newLine to the specified
     *  listbox, ensuring the listbox automatically scrolls
     *  down to show the latest update (at the bottom of
     *  the view)
     *
     * @param listBox the list box to send the updates to
     *
     * @param newLine the new line to add to the end of the control
     *      If the string contains a newline character at the end, this
     *      is stripped off (not required for listboxes)
     *
     */
    static void appendStringToStatusWindow(CListBox& listControl,
                                           const std::string& newLine);

private:

    PaLayoutHelper();
    virtual ~PaLayoutHelper();

    // Copy constructor not used so made private
    PaLayoutHelper(const PaLayoutHelper&);
    // Assignment not used so it is made private
    PaLayoutHelper& operator=(const PaLayoutHelper&);
};

#endif // !defined(PaLayoutHelper_BB23A710_0466_469f_952A_E7A090EFB3CD__INCLUDED_)
