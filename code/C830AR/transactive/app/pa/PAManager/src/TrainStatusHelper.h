/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/TrainStatusHelper.h $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * TrainStatusHelper contains helper functions for all functionality
 * common between train status dialogs (because the hierarchy isn't
 * split up to allow sharing of train specific functionality)
 *
 */

#if !defined(AFX_TrainStatusHelper_H__4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_)
#define AFX_TrainStatusHelper_H__4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_

#include <string>

#include "app/pa/PAManager/src/GenericStatusModel.h"

class TrainStatusHelper
{
public:

    /**
     * addToTrainReportLists
     *
     * @param failuresList the list control to receive the failure broadcast items
     *
     * @param progressList the list control to receive the successful broadcast items
     *
     * @param zoneProgresses the sequence of elements to be displayed in the controls
     *
     */
    static bool addToTrainReportLists(CListCtrl& failuresList,
                                      CListCtrl& progressList,
                                      const TA_Base_Bus::IPAAgentCorbaDef::LocalProgressSeq& trainProgresses,
                                      // TD16865
                                      std::map<unsigned int, std::string>& trainIdAndServiceNum);
    // TD16865

    /**
     * retryTrainBroadcast
     *
     * Retries a train specific broadcast, according to supplied parameters
     *
     * @param agent the agent used to send out the request
     *
     * @param broadcastId the identifier of broadcast to be retried
     *
     * @param lockedProgressReport the progress report containing trains the request is
     *          to be done for
     *
     * @param mode the mode of the broadcast (ie retry all / retry failed)
     *
     * @param hasOverrideOption state of retry mode (override enabled or disabled)
     *
     * @exception BroadcastIdNotFound local copy of broadcast ID must be out of date
     *
     * @exception InvalidRequest there were unexpected (malformed) input parameters
     *
     */
    static void retryTrainBroadcast(PAAgentNamedObject& agent,
                                    const std::string& broadcastId,
                                    const LockedPacketProgressReport& lockedProgressReport,
                                    const GenericStatusModel::ERetryMode& mode,
                                    bool hasOverrideOption);

    /**
     * setupReportWindowList
     *
     * Updates the specified list with some default properties to define a report
     * style window (that is, the Broadcast Progress and Broadcast Failures lists)
     *
     * @param targetList the list to update the properties of
     *
     */
    static void setupReportWindowList(CListCtrl& targetList);

private:

    /**
     *
     * collectTrains
     *
     * Returns all train IDs within the current broadcast group, that do not have a status matching any
     * of the input states
     *
     * @param lockedProgressReport the progress report containing trains the request is
     *          to be done for
     *
     * @param includeFailedOnly set to true to include include trains that have a failed state
     *
     * @return a list of train keys, where for each train, it's last known state does not match any of the states
     *          present in the input argument array
     *
     */
    static TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage
    collectTrains(const LockedPacketProgressReport& lockedProgressReport,
                  bool includeFailedOnly);

    /**
     * addTrainReportEntry
     *
     * Adds a new entry to the 'report' windows (items in list controls provided
     * as arguments)
     *
     * @param trainProgress current status information for this entry
     *
     * @param failuresList reference to the failures list control on the dialog
     *
     * @param progressList reference to the progress list control on the dialog
     *
     * @return false if there were any errors while adding items to lists
     *
     */
    static bool addTrainReportEntry(const TA_Base_Bus::IPAAgentCorbaDef::LocalProgress& trainProgress,
                                    CListCtrl& failuresList,
                                    CListCtrl& progressList,
                                    // TD16865
                                    std::map<unsigned int, std::string>& trainIdAndServiceNum);
    // TD16865

    /**
     * getTrainStatusString
     *
     * @param status the current train radio status
     *
     * @return string representation of this status
     *
     */
    static const std::string getTrainStatusString(
        const TA_Base_Bus::IPAAgentCorbaDef::ELocalState& status);
};

#endif /* AFX_TrainStatusHelper_H__4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_ */
