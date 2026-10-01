#include "app/signs/TISBridgeAgent/src/TISBridgeAgent.h"

#include "core/exceptions/src/GenericAgentException.h"
#include "core/exceptions/src/TransactiveException.h"
#include "core/utilities/src/DebugUtil.h"

int main(int argc, char* argv[])
{
    try
    {
        TA_IRS_App::TISBridgeAgent bridgeAgent(argc, argv);
        bridgeAgent.startTISBridgeAgent();
    }
    catch (const TA_Base_Core::GenericAgentException& e)
    {
        LOG_EXCEPTION_CATCH(SourceInfo, "GenericAgentException", e.what());
    }
    catch (const TA_Base_Core::TransactiveException& e)
    {
        LOG_EXCEPTION_CATCH(SourceInfo, "TransactiveException", e.what());
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
