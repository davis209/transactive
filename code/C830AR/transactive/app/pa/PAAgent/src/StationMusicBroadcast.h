/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/StationMusicBroadcast.h $
 * @author:  Ripple
 * @version: $Revision: #2 $
 *
 * Last modification: $DateTime: 2024/12/10 11:18:50 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#pragma once
#include "app/pa/PAAgent/src/AbstractStationBroadcast.h"
#include "app/pa/PAAgent/src/PasHelpers.h"

namespace TA_IRS_App
{
    class Table201;
    class Table202;
    class Table301;
    class Table302;

    class StationMusicBroadcast : public virtual AbstractStationBroadcast
    {
    public:

        StationMusicBroadcast(Table201& table201,
                              Table202& table202,
                              Table301& table301,
                              Table302& table302,
                              TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* broadcastConfigP,
                              const std::string& sessionId);
        // throws CorruptInternalStateDataException

        virtual ~StationMusicBroadcast();

        virtual void validateBroadcastConfig(); //throw(TA_Base_Core::BadParameterException);

        virtual void execute();

        virtual void terminate();

        virtual TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage getZoneCoverage()
        {
            // Shouldn't retry this broadcast - just return nothing
            return TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage();
        }

    protected:

        virtual void runtimeSpecifics();

    private:

        StationMusicBroadcast(const StationMusicBroadcast& theStationMusicBroadcast);

    protected:

        TA_Base_Bus::IPAAgentCorbaDef::StationMusicConfig& m_theSpecifics; // For convenience

        TA_IRS_App::Table201& m_table201;
        TA_IRS_App::Table202& m_table202;
        TA_IRS_App::Table301& m_table301;
        TA_IRS_App::Table302& m_table302;
    };
}
