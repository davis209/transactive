/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/IStationBroadcastPage.h $
 * @author Sean Liew
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 */

#if !defined(AFX_IStationBroadcastPage_H__B3711608_2C1B_4371_9411_7B156F95E8B9__INCLUDED_)
#define AFX_IStationBroadcastPage_H__B3711608_2C1B_4371_9411_7B156F95E8B9__INCLUDED_

#if _MSC_VER > 1000
    #pragma once
#endif // _MSC_VER > 1000

#include "app/pa/PAManager/src/IGenericBroadcastPage.h"

class PAManagerDlg;
class DvaMessageChangeSubscriber;

class IStationBroadcastPage : public IGenericBroadcastPage
{
public:

    virtual void updateNormalMessageList(TA_IRS_Bus::StationDvaMessageRecordPointers& records) = 0;
    virtual void updateEmergencyMessageList(TA_IRS_Bus::StationDvaMessageRecordPointers& records) = 0;
    virtual void updateAdhocMessageList(TA_IRS_Bus::StationDvaMessageRecordPointers& records) = 0;
    virtual void updateSelectedMessagesList(TA_IRS_Bus::StationDvaMessageRecordPointers& records) = 0;

    /**
     * displayDvaStatusDialog
     *
     * Pops up a DVA status dialog window
     *
     * @param broadcastId the identifier that defines what information is shown in status dialog
     *
     */
    virtual void    displayDvaStatusDialog(const std::string& broadcastId) = 0;

    /**
     * displayLiveStatusDialog
     *
     * Pops up a live status dialog window
     *
     * @param broadcastId the identifier that defines what information is shown in status dialog
     *
     */
    virtual void    displayLiveStatusDialog(const std::string& broadcastId) = 0;

    virtual DvaMessageChangeSubscriber& getDvaMessageChangeSubscriber() = 0;
};

#endif // !defined(AFX_IStationBroadcastPage_H__B3711608_2C1B_4371_9411_7B156F95E8B9__INCLUDED_)
