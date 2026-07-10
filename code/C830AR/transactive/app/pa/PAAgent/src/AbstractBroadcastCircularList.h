/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/AbstractBroadcastCircularList.h $
 * @author:  Ripple
 * @version: $Revision: #5 $
 *
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#pragma once
#include "app/pa/PAAgent/src/CircularList.h"
#include "app/pa/PAAgent/src/AbstractBroadcast.h"
#include "core/exceptions/src/PasExceptions.h"
#include "core/synchronisation/src/ReEntrantThreadLockable.h"
#include <map>

namespace TA_IRS_App
{
    class AbstractTrainBroadcast;
    class TrainDVABroadcast;

    class AbstractBroadcastCircularList : public CircularList<AbstractBroadcast*>
    {
    public:

        /**
         * getInstance
         *
         * Returns an instance of the class
         *
         * @return the pointer to this singleton instance
         *
         */
        static AbstractBroadcastCircularList* getInstance();

        /**
         * removeInstance
         *
         * Removes the instance of the class (if already created) and cleanup the members.  Primarily
         * used upon program termination (e.g. from main()) so that Purify does not consider this class
         * and its members to be leaks.
         *
         */
        static void removeInstance();

        AbstractBroadcast* getBroadcast(const std::string& broadcastId);
        AbstractTrainBroadcast* getTrainBroadcast(CORBA::Octet trainId, CORBA::Octet announcementId);
        AbstractTrainBroadcast* getTrainBroadcast(CORBA::Octet announcementId);

        std::list<TrainDVABroadcast*> getAllTrainDvaBroadcasts();

        void getAllBroadcastIdTypes(TA_Base_Bus::IPAAgentCorbaDef::BroadcastIdTypeSeq& broadcastIdTypeSeq);

        bool isStationAnnounceIdUsed(CORBA::Octet announceId);

        CORBA::Octet getNextAvailableStationAnnounceId();
        //throw(TA_Base_Core::NoFreeAnnounceIdException);

        CORBA::Octet getNextAvailableTrainAnnouncementId();
        //throw(TA_Base_Core::NoFreeAnnounceIdException);

        static const unsigned char MIN_STATION_ANNOUNCE_ID;
        static const unsigned char MAX_STATION_ANNOUNCE_ID;

        static const unsigned char MIN_TRAIN_ANNOUNCEMENT_ID;
        static const unsigned char MAX_TRAIN_ANNOUNCEMENT_ID;

    protected:

        static AbstractBroadcastCircularList* m_me;

        // Synchronisation locks for the singleton access
        static TA_Base_Core::ReEntrantThreadLockable     m_lockForSingleton;
    };
}
