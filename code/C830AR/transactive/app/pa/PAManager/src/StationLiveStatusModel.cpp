/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/StationLiveStatusModel.cpp $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * StationLiveStatusModel controls additional layer of business logic for Live
 * broadcast dialog
 *
 */

#include "app/pa/PAManager/src/StdAfx.h"
#include "app/pa/PAManager/src/StationLiveStatusModel.h"

#include <algorithm>

#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
#endif

StationLiveStatusModel::StationLiveStatusModel(GenericStatusPage& page,
                                               const std::string& broadcastId)
    :
    LiveStatusModel(page, broadcastId, GenericStatusModel::STATION)
{
}

StationLiveStatusModel::~StationLiveStatusModel()
{
}
