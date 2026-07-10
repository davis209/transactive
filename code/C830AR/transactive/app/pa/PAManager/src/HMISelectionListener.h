/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/HMISelectionListener.h $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/03/05 13:16:38 $
 * Last modified by: $Author: limin.zhu $
 *
 * HmiSelectionListener manages asynchronous Zone List updates received from the HMI,
 * posting out notifications as required (to the PA GUI)
 *
 */

#pragma once

#include "core/synchronisation/src/NonReEntrantThreadLockable.h"
#include "bus/pa/cachedmaps/src/ICachedMapFilter.h"
#include "bus/pa/cachedmaps/src/CachedMapDefinitions.h"
#include "app/pa/PAManager/src/IRunParamChangedListener.h"

class HmiSelectionListener : public IRunParamChangedListener
{
    friend class HmiSelectionListenerTestCases;

public:

    enum EDataPointType
    {
        UNKNOWN,    // Could be station or OCC
        STATION,
        OCC
    };

    BOOST_DESCRIBE_NESTED_ENUM(EDataPointType, UNKNOWN, STATION, OCC);

    /**
     * HmiSelectionListener (constructor)
     *
     * @param hWnd the window we should send update notifications to upon change
     *
     */
    HmiSelectionListener(HWND hWnd);

    virtual ~HmiSelectionListener();

    /**
     * getZoneList
     *
     * @return the current zone list selection from the HMI
     *
     */
    const std::list<unsigned long> getZoneList();

    /**
     * processRunParamChange (interface implementation)
     *
     * @see IRunParamChangedListener::processRunParamChange for more details
     *
     * @pre the input identifier (name) is a recognized command
     *
     */
    virtual void processRunParamChange(const std::string& name, const std::string& value);

private:

    enum EOperation
    {
        SELECT,
        DESELECT
    };

    BOOST_DESCRIBE_NESTED_ENUM(EOperation, SELECT, DESELECT);

    /**
     * processZoneRefresh
     *
     * Processes a zone refresh message received from the schematics
     *
     * @param value the string received from the schematics defining the zones
     *
     * @param operation defines type of operation this is (select or deselect)
     *
     * @return true if there was a change in selection
     *
     */
    bool processZoneRefresh(const std::string& value, EOperation operation);

    /**
     * refreshSelectionUsingZones
     *
     * Using the zones contained in the input set, refresh the current selection
     *  entirely (that is, if operation is SELECT, then deselect everything
     *      and replace selection with 'zoneNames')
     *
     * @param zoneNames the set of names defining the selection [MUST BE LOWERCASE]
     *
     * @param operation SELECT to deselect everything and replace with selection defined by zoneNames
     *          or DESELECT to select everything and remove selection defined by zoneNames
     *
     */
    void refreshSelectionUsingZones(const std::set<std::string>& zoneNames, EOperation operation);

    /**
     * clearAllZonesFromSelection
     *
     * Clears all zones from the current selection (provided for efficiency)
     *
     */
    void    clearAllZonesFromSelection();

    /**
     * addAllZonesToSelection
     *
     * Adds all zones from the current selection (provided for efficiency)
     *
     */
    void    addAllZonesToSelection();

    /**
     * getTokenValue
     *
     * Returns token at point defined in input array
     *  assuming the following format <StationName> PA <AreaName> <ZoneShortName>
     * May use wildcards in some or all of StationName, AreaName or ZoneShortName
     *      AreaName is of form Public/Non-Public
     * Uses direct value of PaZoneRecord::ETokenIdentifier enum as index into tokens array
     *
     * @return the token value at position identified by id
     *
     * @param tokens an array of string values corresponding to assumed format
     *
     * @param id tokens which element to retrieve
     *
     * @exception InvalidPasConfigurationException if the input set of tokens doesn't
     *          contain the element defined by id
     *
     */
    std::string getTokenValue(const std::vector<std::string>& tokens, TA_IRS_Bus::PaZoneRecord::ETokenIdentifier id);

    /**
     * onZoneSelectionChange
     *
     * Called when there is a request to update the zone selection from the HMI
     *  (via the onRunParamChange function)
     *
     * @param value the string identifying the zones that need to be changed
     *
     * @param operation the value defining what to do with the identified zones
     *          (for example, remove them from current selection)
     *
     * @exception InvalidPasConfigurationException if expected input parameters
     *      (contained in 'value' were not found - meaning bad format)
     *
     * @return true if there was a change in selection
     *
     */
    bool    onZoneSelectionChange(const std::string& value, EOperation operation);

    /**
     * modifyZoneSelection
     *
     * Modifies the internal zone selection
     *
     * @param zones identifies zones to be operated on
     *
     * @param operation defines the operation to perform (ie select or deselect)
     *
     */
    void    modifyZoneSelection(const std::vector<unsigned long>& zones,
                                EOperation operation);

    /**
     * getSpecifiedZones
     *
     * @param tokens an array of strings in format that corresponds to
     *      format stated in function getTokenValue - wildcards accepted for any element
     *      A wildcard can be ??? or an empty string (or non existent element)
     *      In the event of all being wildcards, or tokens is empty, all zones returned
     *
     * @return an array containing all zones matching input criteria
     *
     */
    std::vector<unsigned long> getSpecifiedZones(const std::vector<std::string>& tokens);

    TA_Base_Core::NonReEntrantThreadLockable         m_zoneListLock;
    // The current zone list as selected from HMI
    std::set<unsigned long>                    m_zoneList;

    // Window to post notifications of HMI / zone updates
    HWND    m_notificationHwnd;

    bool    m_tagRegistrationComplete;

    class IChainedFilter
        :
        public TA_IRS_Bus::ICachedMapFilter<TA_IRS_Bus::PaZoneRecord>
    {
    public:

        virtual void addFilter(const TA_IRS_Bus::ICachedMapFilter<TA_IRS_Bus::PaZoneRecord>* filter) = 0;
    };

    /**
     * getPaManagerSpecificZones
     *
     * @return set of zone IDs for zones matching the input filter, that are
     *      usable by PA Manager.  There exist certain zones that should remain
     *      hidden to PA Manager, this function performs the filtering to ensure this
     *      filtering occurs
     *
     */
    const std::vector<unsigned long> getPaManagerSpecificZones(IChainedFilter* filter);

    /**
     * Custom filter for getting specific PA Zones, allows logical, recursive
     * chaining of filters to make more advanced ones
     *
     */
    template <class CompareValue>
    class FilterPaZoneChain
        :
        public IChainedFilter
    {
    public:

        /**
         * FilterPaZoneChain (constructor)
         *
         * @param filterType defines what the record parameter the internal
         *      (template) value should be compared to, in the isMatch function
         *
         * @param value the compare value for the isMatch function
         *
         * @param datapointSpecification
         */
        FilterPaZoneChain(TA_IRS_Bus::PaZoneRecord::ETokenIdentifier filterType,
                          const CompareValue& value,
                          EDataPointType datapointSpecification);

        /**
         * isMatch
         *
         * @return false if we don't match, or any of sub filters don't match
         *
         * @see ICachedMapFilter::isMatch
         *
         */
        virtual bool isMatch(const TA_IRS_Bus::PaZoneRecord& record) const;

        /**
         * addFilter
         *
         * Adds a new filter to this object, which will be tested as part
         *  of the isMatch test (all filters added in must be a match, for
         *  this filter to be a match)
         *
         * @param filter the filter to add in for use in isMatch test
         *
         * @pre filter is not null
         *
         */
        virtual void addFilter(const TA_IRS_Bus::ICachedMapFilter<TA_IRS_Bus::PaZoneRecord>* filter);

    private:

        // Parameter true if the input filter specification corresponds to an OCC datapoint
        // name specification (something along the lines of OCC_DBG.*...)
        EDataPointType      m_datapointType;
        TA_IRS_Bus::PaZoneRecord::ETokenIdentifier  m_filterType;
        CompareValue        m_filterValue;

        std::list<const TA_IRS_Bus::ICachedMapFilter<TA_IRS_Bus::PaZoneRecord>*> m_subFilters;
    };

    /**
     * createNewFilter
     *
     * Allocates a new filter using the input parameters, where id is the filter type
     *      and tokens[id] will be filter value (if possible), otherwise null
     *
     * @return a filter that filters by details specified in input parameters, null if
     *          the input parameters represent a wildcard, or id is out of range of tokens.size()
     *
     * @param tokens set of input token strings, for which id is used to index into
     *
     * @param id the index into 'tokens', which is used as parameter value to filter
     *
     */
    IChainedFilter* createNewFilter(const std::vector<std::string>& tokens,
                                    TA_IRS_Bus::PaZoneRecord::ETokenIdentifier id);

    /**
     * linkFilters
     *
     * Links together two filters together
     *
     * @return the primary filter for link - current (if not null), otherwise link
     *
     * @param current the current filter representing the current filter - may be null
     *          if there is no current filter present
     *
     * @param link the link to join to 'current', null acceptable (no change)
     *
     */
    IChainedFilter* linkFilters(IChainedFilter* current,
                                IChainedFilter* link);

    // Buffer of entity names for a synchronised selection operation
    std::set<std::string>   m_entityNameBuffer;
    // The number of duplicate items that couldn't be added to 'entityNameBuffer'
    unsigned int m_duplicateNames;
};
