/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/RunParamListener.h $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * RunParamListener listens out for general RunParam changes,
 *     and queue changes (to ensure non are missed)
 *
 */

#if !defined(RunParamListener_BB23A710_0466_469f_952A_E7A090EFB3CD__INCLUDED_)
#define RunParamListener_BB23A710_0466_469f_952A_E7A090EFB3CD__INCLUDED_

#include <string>

#include "app/pa/PAManager/src/IRunParamChangedListener.h"
#include "core/synchronisation/src/NonReEntrantThreadLockable.h"
#include "core/utilities/src/RunParams.h"
#include "core/data_access_interface/src/PrimitiveWrapper.h"
#include "ace/Synch.h"
#include "ace/Guard_T.h"

interface IRestartApplicationCallback;

class RunParamListener
    :
    public TA_Base_Core::RunParamUser
{
    friend class RunParamListenerTestCases;

public:

    // Only store up to 50 changes (to stop any queue runaway)
    enum { MAX_VALUE_CHANGES = 50 };

    /**
     * getInstance
     *
     * @return the singleton instance of the run param listener
     */
    static RunParamListener& getInstance() {
        return s_instance;
    }

    virtual ~RunParamListener();

    /**
     * onRunParamChange (interface implementation)
     *
     * @see RunParamUser::onRunParamChange for more details
     *
     */
    virtual void onRunParamChange(const std::string& name, const std::string& value);

    /**
     * registerListener
     *
     * Registers a listener for run param changes - if any run param changes are
     *      queued, the listener will be immediately sent the queue updates
     *
     * @param name the name of the parameter value
     *
     * @param listener the object to receive update notifications
     *
     * @pre name corresponds to a known (registered) tag
     *
     * @pre there isn't already a listener associated with the input tag
     *
     */
    void registerListener(const std::string& name, IRunParamChangedListener& listener);

    /**
     * deRegisterListener
     *
     * deRegisterListener a listener for run param changes -
     *      to ensure no more updates sent to it
     *
     * @param listener the object we want to stop receiving notifications
     *
     * @pre the input listener is associated with at least one tag
     *
     */
    void deRegisterListener(IRunParamChangedListener& listener);

    /**
     * retrieveInitialRunParamIdentity
     *
     * Examines any available runparam to determine what the identity of this
     *  session is (required for subsequent filtering of input messages)
     *
     * @return true if the application was identified successfully
     *
     */
    bool retrieveInitialRunParamIdentity();

    /**
     * setIsCurrentlyVisible (simple setter)
     *
     * @param visible true if the PAManager is visible
     *
     */
    void setIsCurrentlyVisible(bool visible) {
        m_visible = visible;
    }

    /**
     * isCurrentlyVisible (simple accessor)
     *
     */
    bool isCurrentlyVisible() const {
        return m_visible;
    }

    /**
     * findVirtualLocation
     *
     * @return the location information from the result of findIdentityString
     *          - this represents the virtual (operating) location for the application
     *
     */
    std::string findVirtualLocation();

    /**
     * getSchematicIdentifier
     *
     * Extracts the schematic name from the current identity information of runparams
     *
     * @return the schematic name as defined by the runparams that this class has processed
     *
     * @exception ValueNotSetException if the schematic name hasn't been set
     *
     */
    std::string getSchematicIdentifier();

    /**
     * setRestartApplicationCallback
     *
     * Sets a callback function to be used when the run param listener gets into
     *  a state where it wants the application to be restarted (incoming runparams
     *  require an application restart)
     *
     * @param pCallback the callback object that will receive a request to restart
     *
     */
    void setRestartApplicationCallback(IRestartApplicationCallback* pCallback);

    /**
     * getCommandLine
     *
     * @return the command line required for forward all the runparams to the new application
     *
     */
    std::string getCommandLine();

    // TD 15349
    /**
     * getSelectedStationMessage
     *
     * This will parse the command-line and search for runparam "SelectedStationPAMessage".
     * Return the value, which is the station DVA message label/description to be pre-selected,
     * if any in the lists.
     *
     * @return std::string: The selected station DVA message label/description.
     * Otherwise, return empty string.
     *
     */
    const std::string getSelectedStationMessage();

    /**
     * getSelectedTrainMessage
     *
     * This will parse the command-line and search for runparam "SelectedTrainPAMessage".
     * Return the value, which is the train DVA message label/description to be pre-selected,
     * if any in the lists.
     *
     * @return std::string: The selected train DVA message label/description.
     * Otherwise, return empty string.
     *
     */
    const std::string getSelectedTrainMessage();
    // TD 15349

    // Various run param strings that we listen out for
    static const std::string SELECT_TAG;
    static const std::string DESELECT_TAG;
    static const std::string DISPLAY_TAG;
    static const std::string UPDATE_SELECTION_TAG;
    // TD 15349
    static const std::string SELECTED_STATION_MSG_TAG;
    static const std::string SELECTED_TRAIN_MSG_TAG;
    // TD 15349
    static volatile int DONTGOFRONT_TAG;
    static ACE_Thread_Mutex Tag_mutex;

private:

    /// Maintains details on registered RunParam strings
    struct RunParamChangeStruct
    {
        RunParamChangeStruct()
            :
            m_listener(0)
        {}

        std::string             m_paramName;
        IRunParamChangedListener* m_listener;
        std::list<std::string>  m_valueChanges;
    };

    /**
     * RunParamListener (private - singleton interface)
     */
    RunParamListener();

    /**
     * processInternalRunParamChange
     *
     * Performs internal processing of run parameter changes
     *
     * @param name the name of the parameter value
     *
     * @param value the string identifying the cahnged value
     *
     * @exception InvalidPasConfigurationException if expected input parameters
     *      (contained in 'value' were not found - meaning bad format)
     *
     * @see onRunParamChange
     *
     */
    void    processInternalRunParamChange(const std::string& name, const std::string& value);

    /**
     * setNewIdentityFromIdentifier
     *
     * Sets new identity for this RunParams instance (subsequent messages identifiers
     *      must match the identity passed in here)
     *
     * @param identifier the string identity (in form STATION.<unique screen identifier>)
     *
     * @return true if the identity was successfully set, false if the input
     *          identity wasn't accepted (perhaps incorrect station etc)
     *
     */
    bool    setNewIdentityFromIdentifier(const std::string& identifier);

    /**
     * extractActualComand
     *
     * Extracts actual command from the input strings
     *
     * @return the actual command (stripping any leading whitespace)
     *
     * @param identifier the leading part of the substring in fullValue that identifies
     *          the string source (and is not part of actual command)
     *
     * @param fullValue the original, raw param (with identifier information)
     *
     */
    std::string extractActualComand(const std::string& identifier,
                                    const std::string& fullValue);

    /**
     * findIdentityString
     *
     * Searches through all known runparams searching for a valid command
     *  and extracts the string identifier part from command
     *
     * @return a string identifier of the current application, if present (empty string if not)
     *
     */
    std::string findIdentityString();

    /**
     * registerTag
     *
     * Registers a new name value to listen out for, this should be called
     *  from within the constructor to set up listening set
     *
     * @param tagName the name of the value to listen out for
     *
     */
    void registerTag(const std::string& tagName);

    /**
     * findChangeStruct (helper function)
     *
     * Finds the change struct defined by specified name
     *  Not threadlocked, so ensure lock acquired before calling
     *
     * @param name the name value to search for in change struct array
     *
     * @return the change struct whose parameter name corresponds to name, or
     *          m_registeredTags.end() if not found
     *
     */
    std::vector<RunParamChangeStruct>::iterator findChangeStruct(const std::string& name);

    /**
     * dispatchChangeToListener
     *
     * Dispatches a run parameter change notification to the input listener
     *      provided the data 'belongs' to this application instance (since
     *      multiple applications listen out to same RunParam data)
     *
     * @param listener the listener who is to receive the update information
     *
     * @param name the parameter name of the update
     *
     * @param value the parameter value to be passed onto listener - only
     *          a substring will be passed along (the identification information
     *          which determines whether it's passed on in the first place will
     *          be stripped out before dispatching to listener)
     *
     * @return true if the name/value pair was passed onto the listener,
     *          false if the data wasn't for this session (and was not
     *          passed on..)
     *
     * @pre m_identityString has been set internally (this should be set
     *          on startup by analysing the command line,
     *          @see setRunParamIdentityFromCommandLine)
     *
     * @exception InvalidPasConfigurationException if expected input parameters
     *      (contained in 'value' were not found - meaning bad format)
     *
     */
    bool dispatchChangeToListener(IRunParamChangedListener& listener,
                                  const std::string& name,
                                  const std::string& value);

    /**
     * findEndOfIdentityString
     *
     * @param input the 'value' data from which we should determine the end offset
     *          of the identifier information
     *
     * @return the offset at which the identity part of value ends, or npos
     *          if the format is unrecognized
     *
     */
    std::string::size_type findEndOfIdentityString(const std::string& input);

    /**
     * extractStringIdentifierString
     *
     * @return the identification portion of the input string, or an empty
     *          string if no identifier was found
     *
     * @param value the parameter value we want to extract an identifier from
     *
     * @see findEndOfIdentityString
     *
     */
    std::string extractStringIdentifierString(const std::string& value);

    /**
     * getSchematicName
     *
     * Extracts the schematic name from the input identity string
     *
     * @param identifierString the identity string we're extracting the schematic name from
     *
     * @return the schematic name
     *
     * @exception ValueNotSetException if the schematic name couldn't be found
     *
     */
    std::string RunParamListener::getSchematicName(const std::string& identifierString);

    /**
     * getStationName
     *
     * Extracts the station name from the input identity string
     *
     * @param identifierString the identity string we're extracting the station name from
     *
     * @return the station name
     *
     */
    std::string getStationName(const std::string& identifierString);

    /**
     * getSelectedMessage
     *
     * Extracts the selected message from runparams
     *
     * @param the tag name which is looking for in runparams
     *
     * @return the selected message
     *
     */
    std::string getSelectedMessage(const std::string tagName);

    /**
     * checkIfNeedToRestartPaManager
     *
     * @return true if we should restart PA Manager, based on the input parameters
     *      (check for a Display=TRUE combination)
     *
     * @param identifier the identifier component of the 'value' string
     *
     * @param name, value the name value pair of run param change
     *
     */
    bool checkIfNeedToRestartPaManager(const std::string identifier,
                                       const std::string& name,
                                       const std::string& value);

    /**
     * restartApplication
     *
     * Called whenever there are conditions requiring the PA Manager to restart
     *  such requests can not always be satified (depending on the current state of
     *  PA Manager)
     *
     * @param targetIdentifier the target identity of schematic we want to launch this on
     *
     */
    void restartApplication(const std::string& targetIdentifier);

    /// String storing the identity of this instance (as passed through in RunParams)
    TA_Base_Core::PrimitiveWrapper<std::string>  m_identityString;

    TA_Base_Core::NonReEntrantThreadLockable  m_tagLock;
    std::vector<RunParamChangeStruct>    m_registeredTags;

    // Singleton instance (must be immediately instanced to ensure
    // we get all run param changes)
    static RunParamListener s_instance;

    /// Maintains visible state of application
    bool    m_visible;

    IRestartApplicationCallback*    m_restartWindowCallback;

    // Copy constructor not used so made private
    RunParamListener(const RunParamListener&);
    // Assignment not used so it is made private
    RunParamListener& operator=(const RunParamListener&);

    ///////////////////////////////////////////////
    // This is only supposed to be in place till post-FAT tests, when
    // proper PA Manager switch can be implemented
    bool switchIdentityBasedOnMessage_For_FAT(
        const std::string identifier,
        const std::string& name,
        const std::string& value);
    bool isPhysicallyLocatedAt(const std::string& stationName);
    ////////////////////////////////////////////////
};

#endif // !defined(RunParamListener_BB23A710_0466_469f_952A_E7A090EFB3CD__INCLUDED_)
