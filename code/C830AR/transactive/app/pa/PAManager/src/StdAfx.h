/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/StdAfx.h $
 * @author:  Ripple
 * @version: $Revision: #4 $
 *
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by:  $Author: limin.zhu $
 *
 */
// stdafx.h : include file for standard system include files (for PAManager App)
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__A42DB050_CE42_4317_833D_19C8503C691E__INCLUDED_)
#define AFX_STDAFX_H__A42DB050_CE42_4317_833D_19C8503C691E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#pragma warning( disable : 4786 )

// "warning C4290: C++ Exception Specification ignored"
#pragma warning( disable : 4290 )

#pragma warning(disable:4284)   // For std::set warnings

#include "core/utilities/src/DebugUtilEx.h"

#include <list>
#include <string>
#include <algorithm>

#include "core/utilities/src/DebugUtil.h"
#include "core/utilities/src/TAAssert.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"
#include "core/naming/src/NamedObject.h"
#include "app/pa/PAManager/src/resource.h"		// main symbols
#include "app/pa/PAManager/src/PaErrorHandler.h"

// wParam defines zone key
#define WM_PA_STN_ZONE_UPDATE			    ( WM_APP + 0  )
// wParam defines group key (use WM_PA_STN_HMI_ZONE_SEL_CHANGE for HMI group)
#define WM_PA_STN_ZONE_GROUP_UPDATE         ( WM_APP + 1  )
#define WM_PA_STN_HMI_ZONE_SEL_CHANGE		( WM_APP + 2  )
#define WM_PA_PRIORITY_SCHEME_UPDATE	    ( WM_APP + 3  )
#define WM_PA_STN_STATUS_CONFIG_UPDATE		( WM_APP + 4  )
#define WM_PA_STN_STATUS_REPORT_UPDATE		( WM_APP + 5  )
#define WM_PA_STN_STATUS_NAME_UPDATE        ( WM_APP + 6  )
#define WM_PA_DVA_MESSAGE_UPDATE            ( WM_APP + 7  )
#define WM_PA_DVA_VERSION_UPDATE            ( WM_APP + 8  )
#define WM_PA_TRAIN_INBOUND_LIST_UPDATE     ( WM_APP + 9  )
#define WM_PA_TRAIN_OUTBOUND_LIST_UPDATE    ( WM_APP + 10 )
#define WM_PA_TRAIN_DYN_LIST_UPDATE         ( WM_APP + 11 )
#define WM_PA_ERROR_MESSAGE                 ( WM_APP + 12 )
#define WM_PA_HMI_DISPLAYSEL_CHANGED        ( WM_APP + 13 )
// When change in PTT state (wParam indicates 0 for ceased, 1 for begin talking)
#define WM_PA_TRAIN_PTT_CHANGE              ( WM_APP + 14 )
#define WM_PA_TRAIN_DVA_MESSAGE_UPDATE      ( WM_APP + 15 )
// Indicates there was a change in some right (non-specific)
#define WM_PA_RIGHTS_CHANGE                 ( WM_APP + 16 )
#define WM_PA_ATTEMPT_APPLICATION_RESTART   ( WM_APP + 17 )
#define WM_PA_TRAIN_GLOBAL_VERSION_UPDATE   ( WM_APP + 18 )
#define WM_PA_TRAIN_VERSION_UPDATE   ( WM_APP + 19 )

// 18611 libo
#define WM_PA_MANAGER_ACTIVE   ( WM_APP + 20)
// 18611 libo

typedef std::set<unsigned long> DvaMessageUpdateData;
typedef DvaMessageUpdateData::iterator DvaMessageUpdateDataIt;

/// TD #3152
/// Defines origins of broadcasts within PA Manager application
/// - important so we know which broadcasts we are allowed to terminate
/// even if the privelege isn't present (for example, live broadcasts,
/// dva playbacks / recording broadcasts all need to be terminated
/// locally, even if priveleges aren't present..)
enum EBroadcastOrigin
{
    BO_RECORDING_BROADCAST,
    BO_LIVE_BROADCAST,
    BO_PLAYBACK_BROADCAST,
    BO_UNKNOWN      // Valid for general purpose use, when we don't know where originated
};

const std::string   STATION_PA_AGENT_ENTITY_TYPE_NAME = "StationPAAgent";
const std::string   MASTER_PA_AGENT_ENTITY_TYPE_NAME = "MasterPAAgent";

#ifndef STUBBED_PA_AGENT

    // Unit test classes define an agent stub class
typedef TA_Base_Core::NamedObject<TA_Base_Bus::IPAAgentCorbaDef,
    TA_Base_Bus::IPAAgentCorbaDef_ptr,
    TA_Base_Bus::IPAAgentCorbaDef_var> PAAgentNamedObject;

#else

    // Otherwise we're building the test code, so include stub directly
#include "app/pa/PAManager/test/src/PAAgentNamedObjectStub.h"

#endif

/**
 * castPointerToReference
 *
 * Casts the input pointer to an equivalent reference defined by template specification
 *
 * For exception Apple& apple = castPointerToReference<Apple>(pFruit);
 *
 * @param input the input pointer to perform the cast on
 *
 * @return reference to the casted object
 *
 * @exception bad_cast if the input pointer is null, or the cast is invalid
 *
 */
template <class TYPE, class ANY>
TYPE& castPointerToReference(ANY* input)
{
    if (0 == input)
    {
        //TA_THROW(std::bad_cast("Attempting to cast null pointer to reference"));
        TA_THROW(TA_Base_Core::TransactiveException("Attempting to cast null pointer to reference"));
    }

    TYPE* result = dynamic_cast<TYPE*>(input);

    if (0 == input)
    {
        //TA_THROW(std::bad_cast("Invalid type cast"));
        TA_THROW(TA_Base_Core::TransactiveException("Invalid type cast"));
    }

    return *result;
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A42DB050_CE42_4317_833D_19C8503C691E__INCLUDED_)
