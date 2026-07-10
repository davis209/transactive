/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution in any form.
 *
 * Source:    $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/MasterPAAgent.h $
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
    class MasterPAAgent : public virtual AbstractPAAgent
    {
    public:

        MasterPAAgent(int argc, char* argv[]);
        virtual ~MasterPAAgent();

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
        MasterPAAgent(const MasterPAAgent& theAgent);
        MasterPAAgent& operator=(const MasterPAAgent&);
    };
}
