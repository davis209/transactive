/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/GenericBroadcastModel.cpp $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * GenericBroadcastModel contains shared functionality between broadcast models
 *
 */

#include "app/pa/PAManager/src/stdafx.h"

#include "app/pa/PAManager/src/GenericBroadcastModel.h"
#include "app/pa/PAManager/src/IGenericBroadcastPage.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
#endif

GenericBroadcastModel::GenericBroadcastModel(IGenericBroadcastPage& page)
    :
    m_broadcastPage(page)
{
    FUNCTION_ENTRY( "GenericBroadcastModel" );
    FUNCTION_EXIT;
}

GenericBroadcastModel::~GenericBroadcastModel()
{
    FUNCTION_ENTRY( "~GenericBroadcastModel" );
    FUNCTION_EXIT;
}
