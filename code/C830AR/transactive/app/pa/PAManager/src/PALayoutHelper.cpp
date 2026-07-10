/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/PALayoutHelper.cpp $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * PaLayoutHelper is used to allow centralize the formatting of specific controls
 * so that code is not duplicated throughout the PA Manager Module
 *
 */

#include "app/pa/PAManager/src/stdafx.h"
#include "app/pa/PAManager/src/PaLayoutHelper.h"
#include "app/pa/PAManager/src/pamanager.h"
#include "app/pa/PAManager/src/StationBroadcastModel.h"
#include "app/pa/PAManager/src/CachedConfig.h"
#include "app/pa/PAManager/src/IStationBroadcastPage.h"

#include <winbase.h>

#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
#endif

using namespace TA_Base_Core;

PaLayoutHelper::PaLayoutHelper()
{
}

PaLayoutHelper::~PaLayoutHelper()
{
}

void PaLayoutHelper::formatDvaMessageList(CListCtrl& listToUpdate,
                                          const std::string& selectedMessageLabel,
                                          bool accountForScrollBar,
                                          const DWORD styleFlags)
{
    static const DWORD ID_COL_WIDTH_PIXELS = 35;

    // ++AZ TES 9613, tooltips set in ListCtrlSelNoFocus
    //listToUpdate.SetExtendedStyle(styleFlags | LVS_EX_INFOTIP);
    // AZ++

    int headerTotalWidth = getListHeaderWidth(listToUpdate, accountForScrollBar);

    // The primary column resizes to fill out width of columns
    listToUpdate.InsertColumn(0,
                              selectedMessageLabel.c_str(),
                              LVCFMT_LEFT,
                              headerTotalWidth);

    //listToUpdate.InsertColumn(1, "ID", LVCFMT_LEFT, ID_COL_WIDTH_PIXELS);
}

void PaLayoutHelper::setUpDateTimeControls(CDateTimeCtrl* date,
                                           CDateTimeCtrl* timeOfDay)
{
    if (0 != date)
    {
        date->SetFormat("ddd, dd MMMM");
    }

    if (0 != timeOfDay)
    {
        timeOfDay->SetFormat("H:mm:ss");
    }
}

time_t PaLayoutHelper::getSecondsFromControls(const CDateTimeCtrl* pDateControl,
                                              const CDateTimeCtrl* pHourControl,
                                              const CDateTimeCtrl* pMinuteControl,
                                              const CDateTimeCtrl* pSecondControl)
{
    time_t result = 0;
    CString str;

    SYSTEMTIME time;

    // Must always have a date element in order to create a valid
    // CDate object
    if (0 == pDateControl)
    {
        // Set to today if no date field provided
        GetSystemTime(&time);
    }
    else
    {
        pDateControl->GetTime(&time);
    }

    // Careful how we handle this part - we must strip out all the
    // time of day information
    // Strip out (zero out) all the time of day components
    time.wHour = time.wMinute = time.wSecond = time.wMilliseconds = 0;

    // Create object represented time rounded to a whole day
    CTime roundedTime(time);

    // The result from GetTime() is in seconds, no conversion required
    result += roundedTime.GetTime();

    SYSTEMTIME sysTime;

    if (pHourControl)
    {
        pHourControl->GetTime(&sysTime);
        // 3600 seconds / hour
        result += (3600 * sysTime.wHour);
    }

    if (pMinuteControl)
    {
        pMinuteControl->GetTime(&sysTime);
        result += (60 * sysTime.wMinute);
    }

    if (pSecondControl)
    {
        pSecondControl->GetTime(&sysTime);
        result += sysTime.wSecond;
    }

    return result;
}

std::string PaLayoutHelper::fetchStringFromControl(const CEdit& sourceControl)
{
    int lineLength = sourceControl.LineLength(0);

    if (0 == lineLength)
    {
        // Must perform this special check, otherwise GetLine call passed to CEdit
        // causes memory corruption (if used with a lineLength of 0)
        // Probably because WORD is 2 bytes, being stored in a 1 byte memory space..
        return "";
    }

    CString rString;
    sourceControl.GetWindowText(rString);
    std::string str(rString.GetBuffer(0));

    return str;
}

int PaLayoutHelper::getListHeaderWidth(CListCtrl& targetList, bool accountForScrollbar)
{
    CRect rc;
    targetList.GetClientRect(rc);
    int headerTotalWidth = rc.Width();

    if (accountForScrollbar)
    {
        headerTotalWidth -= GetSystemMetrics(SM_CXVSCROLL);
    }

    return headerTotalWidth;
}

void PaLayoutHelper::appendStringToStatusWindow(
    CListBox& listBox,
    const std::string& newLine)
{
    if (newLine.length() == 0)
    {
        return;
    }

    // Strip out any trailing carriage returns
    std::string strippedLine = newLine.substr(0, newLine.find("\r\n"));

    listBox.AddString(strippedLine.c_str());

    // Scroll to bottom of control automatically
    BOOL hr = listBox.SetTopIndex(listBox.GetCount() - 1);
    TA_ASSERT(hr != LB_ERR, "Auto scroll failed");
}

time_t PaLayoutHelper::convertFromUTC(time_t utcTime)
{
    return convertTime(utcTime, FROM_UTC);
}

time_t PaLayoutHelper::convertToUTC(time_t localTime)
{
    return convertTime(localTime, TO_UTC);
}

time_t PaLayoutHelper::convertTime(time_t time, const EConversion& conversion)
{
    // Special case - invalid times of zero return zero as adjusted time..
    if (0 == time)
    {
        return time;
    }

    TIME_ZONE_INFORMATION timeZone;

    if (TIME_ZONE_ID_INVALID == GetTimeZoneInformation(&timeZone))
    {
        LOG_ERROR("convertTime: Unable to determine time zone, defaulting to GMT time");

        PaErrorHandler::displayError(PaErrorHandler::UNSPECIFIED_ERROR);
        return time;
    }

    SYSTEMTIME system_inputTime;

    if (!CTime(time).GetAsSystemTime(system_inputTime))
    {
        LOG_ERROR("convertTime: GetAsSystemTime failed, defaulting to GMT time");

        PaErrorHandler::displayError(PaErrorHandler::UNSPECIFIED_ERROR);
        return time;
    }

    // Have to convert to file time as intermediary step in order
    // to access time conversion routines
    FILETIME file_inputTime, file_convertedTime;

    if (!SystemTimeToFileTime(&system_inputTime, &file_inputTime))
    {
        LOG_ERROR("convertTime: SystemTimeToFileTime failed, defaulting to GMT time");

        PaErrorHandler::displayError(PaErrorHandler::UNSPECIFIED_ERROR);
        return time;
    }

    BOOL result = false;

    switch (conversion)
    {
        case FROM_UTC:
            // Convert from UTC to local
            result = FileTimeToLocalFileTime(&file_inputTime, &file_convertedTime);

            //result = SystemTimeToTzSpecificLocalTime(&timeZone, &inputTime, &convertedTime);
            break;

        case TO_UTC:
            // Convert from local to UTC
            result = LocalFileTimeToFileTime(&file_inputTime, &file_convertedTime);

            // Had to use file time methods, as TzSpecificLocalTimeToSystemTime doesn't exist in headers??
            //result = TzSpecificLocalTimeToSystemTime(&timeZone, &inputTime, &convertedTime);
            break;

        default:
            TA_ASSERT(FALSE, "Invalid conversion specified");
    }

    if (!result)
    {
        LOG_ERROR("convertTime: Unable to convertTime time, defaulting to GMT time");

        PaErrorHandler::displayError(PaErrorHandler::UNSPECIFIED_ERROR);
        return time;
    }

    // Convert from file time to system time again
    SYSTEMTIME system_convertedTime;
    FileTimeToSystemTime(&file_convertedTime, &system_convertedTime);

    // Finally convert back to time_t
    return CTime(system_convertedTime).GetTime();
}
