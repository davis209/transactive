#include "app/pa/PABridgeAgent/src/PaBridgeAgent.h"

#include "core/exceptions/src/GenericAgentException.h"
#include "core/exceptions/src/TransactiveException.h"
#include "core/utilities/src/DebugUtil.h"

int main(int argc, char* argv[])
{
    try
    {
        TA_IRS_App::PaBridgeAgent bridgeAgent(argc, argv);
        bridgeAgent.startPaBridgeAgent();
    }
    catch (const TA_Base_Core::GenericAgentException& gae)
    {
        LOG_EXCEPTION_CATCH(SourceInfo, "GenericAgentException", gae.what());
    }
    catch (const TA_Base_Core::TransactiveException& te)
    {
        LOG_EXCEPTION_CATCH(SourceInfo, "TransactiveException", te.what());
    }
    catch (const std::exception& e)
    {
        LOG_EXCEPTION_CATCH(SourceInfo, "std::exception", e.what());
    }
    catch (...)
    {
        LOG_EXCEPTION_CATCH(SourceInfo, "Unknown Exception", "Caught unknown exception");
    }

    return 1;
}
