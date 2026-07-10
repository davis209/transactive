/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/MasterLiveBroadcast.h $
 * @author:  Ripple
 * @version: $Revision: #4 $
 *
 * Last modification: $DateTime: 2025/01/06 12:26:20 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#pragma once
#include "app/pa/PAAgent/src/AbstractStationBroadcast.h"
#include "app/pa/PAAgent/src/PasHelpers.h"

namespace TA_IRS_App
{
    class Table1200;
    class Table1201;
    class Table1301;
    class Table590;
    class Table302;
    class Table202;

    class MasterLiveBroadcast : public AbstractStationBroadcast
    {
    public:

        MasterLiveBroadcast(Table202& table202,
                            Table302& table302,
                            Table590& table590,
                            Table1200& table1200,
                            Table1201& table1201,
                            Table1301& table1301,
                            TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* broadcastConfigP,
                            const std::string& sessionId);
        // throws CorruptInternalStateDataException
        virtual ~MasterLiveBroadcast();

        virtual TA_Base_Bus::IPAAgentCorbaDef::BroadcastParameterSeq* getBroadcastParametersCopy();
        virtual void setBroadcastParameter(const TA_Base_Bus::IPAAgentCorbaDef::BroadcastParameter& parameter);

        virtual void validateBroadcastConfig(); //throw(TA_Base_Core::BadParameterException);

        virtual void execute();
        bool executePhaseOneAndReturnIsCompleted();
        bool executePhaseTwoAndReturnIsCompleted();

        virtual void terminate();

        virtual TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage getZoneCoverage()
        {
            TA_Base_Core::ThreadGuard guard(m_lock);
            return m_theSpecifics.zones;
        }

        // Agent-synchronised BroadcastParameter
        bool getIsConnected();
        void setIsConnected(bool value, bool sendSynchronisation = true);

        // TD17456
        virtual bool isAllFailureZones();
        // TD17456

    protected:

        virtual void runtimeSpecifics();

        void updateProgressReport(bool isRuntime = true);

    private:

        MasterLiveBroadcast(const MasterLiveBroadcast& theMasterLiveBroadcast);

    protected:

        TA_Base_Bus::IPAAgentCorbaDef::StationLiveConfig& m_theSpecifics; // For convenience

        TA_IRS_App::Table202& m_table202;
        TA_IRS_App::Table302& m_table302;
        TA_IRS_App::Table590& m_table590;
        TA_IRS_App::Table1200& m_table1200;
        TA_IRS_App::Table1201& m_table1201;
        TA_IRS_App::Table1301& m_table1301;

        bool                                            m_isConnected;
    };

    ////////////////////////
    //      INLINES
    ////////////////////////

    inline bool MasterLiveBroadcast::getIsConnected()
    {
        TA_Base_Core::ThreadGuard guard(m_lockForBroadcastParameters);
        return m_isConnected;
    }

    inline void MasterLiveBroadcast::setIsConnected(bool value, bool sendSynchronisation)
    {
        TA_Base_Core::ThreadGuard guard(m_lockForBroadcastParameters);
        m_isConnected = value;

        if (sendSynchronisation)
        {
            CORBA::Any data;
            data <<= CORBA::Any::from_boolean(value);

            PasHelpers::instance().sendPAAgentBroadcastParameterUpdate
            (TA_Base_Bus::IPAAgentCorbaDef::BROADCAST_PARAMETER_IS_CONNECTED, data, getBroadcastId().c_str());
        }
    }

    inline TA_Base_Bus::IPAAgentCorbaDef::BroadcastParameterSeq* MasterLiveBroadcast::getBroadcastParametersCopy()
    {
        TA_Base_Bus::IPAAgentCorbaDef::BroadcastParameterSeq* parameters =
            AbstractStationBroadcast::getBroadcastParametersCopy();

        int originalLength = parameters->length();
        parameters->length(originalLength + 1);

        CORBA::Any data1;
        data1 <<= CORBA::Any::from_boolean(getIsConnected());
        (*parameters)[originalLength].broadcastId = getBroadcastId().c_str();
        (*parameters)[originalLength].parameter   = TA_Base_Bus::IPAAgentCorbaDef::BROADCAST_PARAMETER_IS_CONNECTED;
        (*parameters)[originalLength].data        = data1;

        return parameters;
    }

    inline void MasterLiveBroadcast::setBroadcastParameter(const TA_Base_Bus::IPAAgentCorbaDef::BroadcastParameter& parameter)
    {
        if (parameter.parameter == TA_Base_Bus::IPAAgentCorbaDef::BROADCAST_PARAMETER_IS_CONNECTED)
        {
            bool updateData = 0;

            if (0 != (parameter.data >>= CORBA::Any::to_boolean(updateData)))
            {
                setIsConnected(updateData, false);
                LOG_INFO("BroadcastParameter update received: IsConnected=%s", updateData ? "true" : "false");
            }
        }
        else
        {
            AbstractStationBroadcast::setBroadcastParameter(parameter);
        }
    }
}
