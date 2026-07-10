/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/TrainLiveStatusModel.h $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * TrainLiveStatusModel controls additional layer of business logic for Live
 * broadcast dialog
 *
 */

#if !defined(TrainLiveStatusModel_4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_)
#define TrainLiveStatusModel_4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_

#include "app/pa/PAManager/src/LiveStatusModel.h"
#include "app/pa/PAManager/src/LiveStatusPage.h"

#include <deque>

class TrainLiveStatusModel
    : public LiveStatusModel
{
    friend class TrainLiveStatusModelTestCases;

public:

    struct TrainContinueState
    {
        int trainId;
        bool continueRequired;
    };

    /**
     * TrainLiveStatusModel (constructor)
     *
     * @param page the view associated with this model
     *
     * @param broadcastId the broadcastId to be associated with this status model
     *
     */
    TrainLiveStatusModel(GenericStatusPage& page, const std::string& broadcastId);

    virtual ~TrainLiveStatusModel();

    /**
     * setHasOverrideOption (simple set function)
     */
    void    setHasOverrideOption(bool hasOption) {
        m_hasOverrideOption = hasOption;
    }

    /**
     * doesNewContinueRequiredTrainExist
     *
     * @return true if there is one or more new (updated) states for some trains
     *      with respect to its continue required state
     *
     * @see popFrontNewContinueRequiredTrain
     *
     */
    bool doesNewContinueRequiredTrainExist();

    /**
     * popFrontNewContinueRequiredTrain
     *
     * @return the oldest element in the continue required train state change FIFO queue
     *
     * @pre doesContinueRequiredTrainExist returns true
     *
     */
    TrainContinueState popFrontNewContinueRequiredTrain();

    /**
     * requestContinueOnTrain
     *
     * @param trainId the identifier of the train to perform the continue request for
     *
     * @exception CORBA::Exception upon comms error
     *
     * @exception BadParameterException from PAAgent
     *
     * @exception BroadcastIdNotFound on internal error (from PAAgent)
     *
     */
    void requestContinueOnTrain();

    /**
     * endLiveAnnouncement
     *
     * Makes a request to the PA Agent to end the live announcement
     *
     * @exception CORBA::Exception upon comms error
     *
     * @exception BadParameterException from PAAgent
     *
     * @exception BroadcastIdNotFound on internal error (from PAAgent)
     *
     */
    void endLiveAnnouncement();

    /**
     * beginLiveAnnouncement
     *
     * Makes a request to the PA Agent to begin the live announcement
     *
     * @exception CORBA::Exception upon comms error
     *
     * @exception BadParameterException from PAAgent
     *
     * @exception BroadcastIdNotFound on internal error (from PAAgent)
     *
     */
    void beginLiveAnnouncement();

    /**
     * existsTrainsWaitingForBroadcast
     *
     * @return true if there are one or more trains currently waiting for a broadcast request
     *          (meaning they're connected and ready)
     *
     */
    bool existsTrainsWaitingForBroadcast() {
        return m_existTrainsWaitingToBeginBroadcast;
    }

    /**
     * resetContinueRequiredStates
     *
     * Resets the internal states relating to the continue required queue
     *
     */
    void resetContinueRequiredStates();

protected:

    /**
     * processProgressUpdateData
     *
     * @see GenericStatusModel::processProgressUpdateData
     *
     */
    virtual void processProgressUpdateData(const TA_Base_Bus::IPAAgentCorbaDef::BroadcastProgressUpdateData& lockedData);

    /**
     * getHasOverrideOption
     *
     * @see LiveStatusModel::getHasOverrideOption base class for details
     *
     */
    virtual bool getHasOverrideOption();

private:

    /**
     * processContinueRequiredStateChange
     *
     * Helper function, updates state of continue required queue based on input states
     *
     * @param trainId the ID of the train being processed
     *
     * @param state the new state of the train
     *
     */
    void processContinueRequiredStateChange(int trainId, TA_Base_Bus::IPAAgentCorbaDef::ELocalState state);

    // True if model is currnetly in override mode
    bool    m_hasOverrideOption;

    // Tracks the continue required state of the trains
    std::deque<TrainContinueState> m_continueStateChangeQueue;
    // Maintains current state data for the trains
    std::map<int, TA_Base_Bus::IPAAgentCorbaDef::ELocalState> m_continueRequiredMap;

    bool    m_existTrainsWaitingToBeginBroadcast;
};

// Bohong++ for CL-20875, to set a special corba timeout value when doing PA train live boradcast, cos
// the hardware may takes more than 20 seconds to make-dynamic-group when more than 3 trains.
// Also modified PAAgent & TrainAgent to consistent this timeout value, refer to PAAgent & TrainAgent
#define DGNA_TIMEOUT 80

#endif // !defined(TrainLiveStatusModel_4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_)
