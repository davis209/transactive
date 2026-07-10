/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/PaManagerStub.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
// Provides a stub class to prevent linker errors
// (we don't want to include PAManager.cpp in the build as it
// attempts to create the PA Manager application in its entirety)
#include "StdAfx.h"
#include "app/pa/PAManager/src/PAManager.h"
#include "app/pa/PAManager/src/PAManagerGUI.h"
#include "bus/pa/cachedmaps/src/CachedMaps.h"
#include "app/pa/PAManager/src/CachedConfig.h"
#include "app/pa/PAManager/src/PaRightsManager.h"
#include "app/pa/PAManager/src/GraphworxComms.h"
#include "app/pa/PAManager/src/PaManagerDlg.h"
#include "core/data_access_interface/pa_4669/src/PaAtsTriggeredBroadcastAccessFactory.h"
#include "core/data_access_interface/pa_4669/src/PaDvaMessageAccessFactory.h"
#include "core/data_access_interface/pa_4669/src/PaDvaMessageVersionAccessFactory.h"
#include "core/data_access_interface/pa_4669/src/PaTrainDvaMessageAccessFactory.h"
#include "core/data_access_interface/pa_4669/src/PaZoneAccessFactory.h"
#include "core/data_access_interface/pa_4669/src/PaZoneGroupAccessFactory.h"
#include "bus/generic_gui/src/IGUIApplication.h"
#include "app/pa/PAManager/src/PaStatusNameChangeSubscriber.h"
#include "bus/generic_gui/src/AppLauncher.h"
#include "bus/application_types/src/apptypes.h"

void PAManagerApp::cleanupSingletons()
{
    // When we're shutting down, shut down all the graphworx displays also
    TA_App::GraphworxComms::getInstance().resetGraphworxDisplaysToInactive();

    PaStatusNameChangeSubscriber::removeInstance();
    TA_IRS_Bus::CachedMaps::removeInstance();
    PaRightsManager::removeInstance();
    CachedConfig::removeInstance();

    TA_IRS_Core::PaAtsTriggeredBroadcastAccessFactory::removeInstance();
    TA_IRS_Core::PaDvaMessageAccessFactory::removeInstance();
    TA_IRS_Core::PaDvaMessageVersionAccessFactory::removeInstance();
    TA_IRS_Core::PaTrainDvaMessageAccessFactory::removeInstance();
    TA_IRS_Core::PaZoneAccessFactory::removeInstance();
    TA_IRS_Core::PaZoneGroupAccessFactory::removeInstance();    
}
