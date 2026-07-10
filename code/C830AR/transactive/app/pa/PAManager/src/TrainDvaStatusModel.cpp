/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/TrainDvaStatusModel.cpp $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * TrainDvaStatusModel controls additional layer of business logic for DVA
 * specific broadcast dialog
 *
 */

#include "app/pa/PAManager/src/StdAfx.h"
#include "app/pa/PAManager/src/TrainDvaStatusModel.h"
#include "app/pa/PAManager/src/StationBroadcastModel.h"
#include "core/exceptions/src/PasExceptions.h"
#include "app/pa/PAManager/src/PaManagerModel.h"

#include <algorithm>

#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
#endif

TrainDvaStatusModel::TrainDvaStatusModel(GenericStatusPage& page,
                                         const std::string& broadcastId)
    :
    DvaStatusModel(page, broadcastId, GenericStatusModel::TRAIN),
    m_hasOverrideOption(false)
{
}

TrainDvaStatusModel::~TrainDvaStatusModel()
{
}

bool TrainDvaStatusModel::getHasOverrideOption()
{
    return m_hasOverrideOption;
}