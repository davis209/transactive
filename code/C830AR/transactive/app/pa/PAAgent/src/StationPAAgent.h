/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/StationPAAgent.h $
 * @author:  Ripple
 * @version: $Revision: #2 $
 *
 * Last modification: $DateTime: 2024/12/10 11:18:50 $
 * Last modified by:  $Author: limin.zhu $
 *
 */
/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution in any form.
 *
 * Source:    $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/StationPAAgent.h $
 * @author:   Andy Parker
 * @version:  $Revision: #2 $
 *
 * Last modification: $DateTime: 2024/12/10 11:18:50 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#pragma once
#include "app/pa/PAAgent/src/AbstractPAAgent.h"

namespace TA_IRS_App
{
    class AtsAgentSubscriber;

    class StationPAAgent : public virtual AbstractPAAgent
    {
    public:

        StationPAAgent(int argc, char* argv[]);
        virtual ~StationPAAgent();

    protected:

        virtual void processSpecificConfiguration();
        //throw(TA_Base_Core::InvalidPasConfigurationException, TA_Base_Core::DatabaseException, TA_Base_Core::DataException);

        virtual void initialiseAndValidateCachedMaps();
        //throw(TA_Base_Core::InvalidPasConfigurationException, TA_Base_Core::CachedMappingNotFoundException, TA_Base_Core::DatabaseException, TA_Base_Core::DataException);

        virtual void createSpecificComponents();
        //throw(TA_Base_Core::GenericAgentException, TA_Base_Core::InvalidPasConfigurationException, TA_Base_Core::CachedMappingNotFoundException);
        virtual void destroySpecificComponents();
        virtual void startSpecificComponents();
        virtual void stopSpecificComponents();

        virtual void agentSetControlSpecifics();
        virtual void agentSetMonitorSpecifics();

        // Disable copy constructor and assignment operator
        StationPAAgent(const StationPAAgent& theAgent);
        StationPAAgent& operator=(const StationPAAgent&);

        FireCountdownAckMonitor* m_fireCountdownAckMonitor;
        AtsAgentSubscriber* m_atsAgentSubscriber;
    };
}
