/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/main.cpp $
 * @author:  Ripple
 * @version: $Revision: #9 $
 *
 * Last modification: $DateTime: 2025/03/05 13:16:38 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "CachedConfig.h"
#include "StationPAAgent.h"
#include "MasterPAAgent.h"
#include "bus/pa/cachedmaps/src/CachedMaps.h"
#include "core/utilities/src/DebugUtilEx.h"
#include "core/utilities/src/DebugUtilOptions.h"
#include "core/utilities/src/RunParams.h"
#include "core/utilities/src/Hostname.h"
#include "core/utilities/src/NamedScopeLogger.h"
#include "core/utilities/src/GlobalInterceptors.h"
#include "core/utilities/src/stdutil/strings.h"
#include "core/utilities/src/StdUtil.h"
#include "core/exceptions/src/GenericAgentException.h"
#include "core/process_management/src/UtilityInitialiser.h"
#include "ace/OS.h"

#if defined (WIN32)
#include <crtdbg.h>
#endif

#include <sstream>

 /**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/main.cpp $
  * @author:  Sean Liew
  * @version: $Revision: #9 $
  *
  * Last modification: $DateTime: 2025/03/05 13:16:38 $
  * Last modified by:  $Author: limin.zhu $
  *
  * This is the main entry point for the application.
  *
  * Wierd stuff -
  *
  * 1. The FUNCTION_ENTRY macro uses the term "typeid(*this).name()" for log details.  In purify, this generates
  *    Invalid Pointer Reads and Writes everytime the macro is called.  It sounds a bit scary and it may be
  *    causing VC++ to crash fairly often during debugging.  Not sure if this problem occurs in the release
  *    build.
  *
  *    Solutions:
  *    a.  Remove the call to the name() method altogether.
  *    b.  Replace name() with a raw_name() call.  This does not cause a fault in purify but it means that the
  *        class name is mangled
  *
  *    Will investiage further to see if this is a real concern.
  *
  * 2. A heap of memory leaks in the code when run in purify.  Most of them caused by singletons not being
  *    explicitly removed.  Where possible, one should supply a thread-safe removeInstance() method for each
  *    singleton.  The main() function should call the removeInstance() methods for all singletons just before
  *    exiting.
  *
  * 3. Lots of problems with mismatched memory allocation/frees.  All went away when the project was explicitly
  *    set to include msvcrtd.lib and msvcprtd.lib and ignore msvcrt.lib,msvcprt.lib.
  *
  * 4. Possible issues with the Oracle oci.dll as this uses msvcrt.dll whereas everything else uses mscvrtd.dll.
  *    This issue may go way when compiled in release mode.  Tried to set all libraries to be release ones to
  *    avoid mixing debug with release libraries but got stuck with symbol linking errors when switching from
  *    mfc42d.dll to mfc42.dll.
  *
  */

using namespace TA_IRS_App;
using namespace TA_Base_Core;

int main(int argc, char* argv[])
{
    NamedScopeLoggerConfig::enable();
    LOG_SCOPE("main");
    FUNCTION_ENTRY("main");

#if defined (WIN32)
    // Remove leak checking
    _CrtSetDbgFlag(0);
#endif

    try
    {
        // Need to obtain arguments earlier than normal so we can determine
        // whether to run as a Master or Station PA Agent.
        RunParams::getInstance().parseCmdLine(argc, argv);

        if (auto hostname = RunParams::getInstance().get(RPARAM_NSHOSTNAME); hostname.empty())
        {
            RunParams::getInstance().set(RPARAM_NSHOSTNAME, TA_Base_Core::Hostname::getHostname().c_str());
        }

        //Maochun Sun++
        // #TD11155
        if (!UtilityInitialiser::initialiseUtilities(argc, argv))
        {
            return 0;
        }
        //++Maochun Sun
        // #TD11155

        {
            // ignore logs

            static const auto s_excludes =
            {
                "EventHelper.cpp",
                "CommandScheduler.cpp",
                "RealTimeEventQueue.cpp",
                "Scheduler.cpp",
                "AlarmAckStateProcessor.cpp",
                "DataPointObserver.cpp",
                "DataNodeObserver.cpp",
                "DataPointAlarmDetails.cpp",
                "DataPointBulkPollingSynchronizer.cpp",
                "DataNodeBulkPollingSynchronizer.cpp",
                "ProxyObserverNotifier.cpp",
                "ConnectionThread.cpp",
            };

            LOG_DEBUG("main(): will ignore log in files:\n%s", st::join("\n", s_excludes));

            DebugUtilOptions::enable_output_interceptor();
            GlobalInterceptors::add("DebugUtil::Impl::logMessage::ignore", [](const char* file)
            {
                return st::any_of_iequals(s_excludes, st::filename(file));
            });
        }

        AbstractPAAgentPtr agent;

        std::string agentMode = RunParams::getInstance().get("AgentMode");

        if ("Master" == agentMode)
        {
            agent = std::make_shared<MasterPAAgent>(argc, argv);
        }
        else if ("Station" == agentMode)
        {
            agent = std::make_shared<StationPAAgent>(argc, argv);
        }
        else
        {
            // Unrecognised AgentMode
            std::ostringstream buffer;
            buffer << "Invalid parameter AgentMode = " << agentMode;
            TA_THROW(TA_Base_Core::GenericAgentException(GenericAgentException::AGENT_ERROR, buffer.str().c_str()));
        }

        agent->run();
    }
    catch (const GenericAgentException& gae)
    {
        LOG_EXCEPTION("GenericAgentException", gae.what());
    }
    catch (std::exception& e)
    {
        LOG_ERROR(e.what());
    }
    catch (...)
    {
        LOG_ERROR("Caught unknown exception");
    }

#if defined (WIN32)
    // Remove leak checking
    _CrtSetDbgFlag(0);
#endif

    FUNCTION_EXIT;
    return 0;
}

/*

  Things to worry about later:

  . unsigned long v.s. int implicit conversions.  See sun compiler output to track down.
  . Implement "ToDo"s
  . Unrem "PutMeBack"s
  . All entity configuration parameters need to be onlineupdatable.
  . Container safe auto_ptr (boost) needs to be used when accessing DataPointNobjects in
     StationAgentsController::setStationControlModeDP
  . Need to revise agent synchronisation model...
  . a. New MasterMode A
    b. Emergency condition is automatically gone.
    c. Since Current Master Mode is 1002 (Normal) the alarm for MasterMode A is still around.
       Should remove the alarms.
  . Need to request updates of all ATS states on startup.
  . Remove "DEMO" blocks
  . Remove "LOGOUT" debug info
  . Table startup and destruction states?
  . TCP/IP socket enabling/disabling?

  Consitency checks:

  2. Verify that all station managed PA Zone DataPoints are used as decision components.

*/
