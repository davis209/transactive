/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution in any form.
  * 
  * Source:    $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/mms_agent/src/MmsAgent.cpp $
  * @author:   Chris DeWolfe
  * @version:  $Revision: #1 $
  * @see:      9999-Q01-02-0016-GenericAgent_Usage_Guidelines.doc
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last mofified by:  $Author: builder $
  *
  * MmsAgent class implimentation. See header file for class descrption
  */

#if defined(_MSC_VER)
#pragma warning(disable:4786)
#endif // defined (_MSC_VER)

#include <iostream>
#include <vector>

#include "core/data_access_interface/entity_access/src/MmsPeriodicEntityData.h"
#include "core/data_access_interface/entity_access/src/MmsConnectiontEntityData.h"
#include "core/data_access_interface/entity_access/src/MmsJobRequestSubmitterEntityData.h"
#include "core/data_access_interface/entity_access/src/MmsAlarmSubmitterEntityData.h"
#include "core/data_access_interface/entity_access/src/DataPointEntityData.h"
#include "core/data_access_interface/entity_access/src/DataNodeEntityData.h"
#include "core/data_access_interface/entity_access/src/DataNodeAccessFactory.h"
#include "core/exceptions/IDL/src/CommonExceptionsCorbaDef.h"

#include "bus/generic_agent/src/IEntity.h"
#include "bus/generic_agent/src/GenericAgent.h"

#include "bus/scada/common_library/src/ScadaUtilities.h"
#include "bus/scada/datapoint_library/src/BooleanDataPoint.h"
#include "bus/scada/datapoint_library/src/DataPoint.h"
#include "bus/scada/datanode_library/src/DataNode.h"
#include "bus/scada/datapoint_library/src/DataPointFactory.h"
#include "bus/scada/proxy_library/src/ScadaProxyFactory.h"
#include "bus/scada/datanode_library/src/SummaryProcessor.h"
#include "bus/scada/DataNodeAgentCorbaDef_Impl/src/DataNodeAgentCorbaDef_Impl.h"
#include "bus/scada/DataNodeAgentCorbaDef_Impl/src/DataNodeAccessCorbaDef_Impl.h"


#include "app/maintenance_management/mms_agent/src/MmsAlarmSubmitter.h"
#include "app/maintenance_management/mms_agent/src/MmsJobRequestSubmitter.h"
#include "app/maintenance_management/mms_agent/src/MmsPeriodicSubmitter.h"
#include "app/maintenance_management/mms_agent/src/MmsConnection.h"
#include "app/maintenance_management/mms_agent/src/MmsServerState.h"
#include "app/maintenance_management/mms_agent/src/MmsAgent.h"

namespace TA_IRS_App
{
	//made this static so that I could return it from the static method getAgentData()
	TA_Base_Core::MmsAgentEntityDataPtr MmsAgent::m_agentEntityData;

	MmsAgent::MmsAgent(int argc, char* argv[])
		:   m_genericAgent(0), 
			m_operationMode(TA_Base_Core::NotApplicable), 
			m_agentKey(0), 
			m_dpBulkPolledInterface (NULL),
			m_dpAccessInterface (NULL),
			m_dnBulkPolledInterface(NULL),
			m_dnAccessInterface(NULL),
			m_scadaProxyFactory(NULL),
			m_summaryProcessor(NULL),
            m_dataPointFactory (NULL)
	{
		FUNCTION_ENTRY("MmsAgent");

		// get a handle to the ScadaProxyFactory
		m_scadaProxyFactory = &(TA_Base_Bus::ScadaProxyFactory::getInstance());

		//The MMS connection will only be created if
		// The GenericAgent CTOR calls createEntity for each of your entities
		// createEntity call will use DataPoint Factory to register a datapoint
		//object. If 
        m_dataPointFactory = new TA_Base_Bus::DataPointFactory(this);
        
		m_genericAgent = new TA_Base_Bus::GenericAgent(argc, argv, this);

		// get configuration data for this Agent
		TA_Base_Core::IEntityDataPtr entityData = m_genericAgent->getAgentEntityConfiguration();
		TA_ASSERT(0 != entityData.get(), "entityData pointer is null");
		m_agentKey = entityData->getKey();
		// get the agent name
		m_agentName = TA_Base_Core::RunParams::getInstance().get(RPARAM_ENTITYNAME).c_str();

		// dynamic cast the entity data to MmsAgentEntityData
		m_agentEntityData = boost::dynamic_pointer_cast< TA_Base_Core::MmsAgentEntityData>(entityData);

		TA_ASSERT(0 != m_agentEntityData.get(), "dynamic_cast < TA_Base_Core::MmsAgentEntityData* > ( entityData ) failed");

		// register the local entities into ScadaProxyFactory
		m_scadaProxyFactory->registerLocalEntities(m_agentEntityData->getAgentName() , m_genericAgent->getEntityMap());
		
		// set the proxies to monitor mode
		m_scadaProxyFactory->setProxiesToMonitorMode();
		
		// create SummaryProcessor
        m_summaryProcessor = TA_Base_Bus::SummaryProcessor::getInstance();
		m_summaryProcessor->setEntityMap(m_genericAgent->getEntityMap());

        //the actual wrapper will need some agent configuration
        //like the server address, however the stub does not need.
#ifndef MMSAPI_STUB
		try
		{
			MmsApiWrapper::getInstance()->initConfiguration(m_agentEntityData);
		}
		catch(...)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError, "failed to set the api wrapper configuration!");
		}
#endif
		FUNCTION_EXIT;
	}


	MmsAgent::~MmsAgent()
	{
		FUNCTION_ENTRY("~MmsAgent");


        // if bulk interface is created
		if ( 0 != m_dpBulkPolledInterface )
		{
			delete m_dpBulkPolledInterface;
			m_dpBulkPolledInterface = 0;
		}
		if ( 0 != m_dpAccessInterface )
		{
			delete m_dpAccessInterface;
			m_dpAccessInterface = 0;
		}
		if ( 0 != m_dnBulkPolledInterface )
		{
			delete m_dnBulkPolledInterface;
			m_dnBulkPolledInterface = 0;
		}
		if ( 0 != m_dnAccessInterface )
		{
			delete m_dnAccessInterface;
			m_dnAccessInterface = 0;
		}


		if( m_genericAgent != 0 )
		{
            TA_Base_Bus::ScadaUtilities::removeInstance();
			m_genericAgent->deactivateAndDeleteServant();
			m_genericAgent = 0;
		}

		// set the proxies to monitor mode
		m_scadaProxyFactory->setProxiesToMonitorMode();
		
		// clean up handle to SummaryProcessor
		if ( NULL != m_summaryProcessor )
		{
			TA_Base_Bus::SummaryProcessor::removeInstance(m_summaryProcessor);
			m_summaryProcessor = NULL;
		}
		
		// remove handle to ScadaProxyFactory
		TA_Base_Bus::ScadaProxyFactory::removeInstance();
		m_scadaProxyFactory = NULL;

		FUNCTION_EXIT;
	}

    void MmsAgent::agentTerminate()
	{
		FUNCTION_ENTRY("agentTerminate");

        TA_Base_Bus::ScadaUtilities::removeInstance();
        // if bulk interface is created
		if ( 0 != m_dpBulkPolledInterface )
		{
			delete m_dpBulkPolledInterface;
			m_dpBulkPolledInterface = 0;
		}
		if ( 0 != m_dpAccessInterface )
		{
			delete m_dpAccessInterface;
			m_dpAccessInterface = 0;
		}
		if ( 0 != m_dnBulkPolledInterface )
		{
			delete m_dnBulkPolledInterface;
			m_dnBulkPolledInterface = 0;
		}
		if ( 0 != m_dnAccessInterface )
		{
			delete m_dnAccessInterface;
			m_dnAccessInterface = 0;
		}

		FUNCTION_EXIT;
	}

    bool MmsAgent::createAllEntities( TA_Base_Core::IEntityDataPtr agentEntityData,
        const TA_Base_Core::SharedIEntityDataList& entityDataList,
        TA_Base_Bus::IEntityList& createdEntities)
    {
        bool createdDnSpecificUtils = false;
        
        for ( TA_Base_Core::SharedIEntityDataList::const_iterator iter = entityDataList.begin();
        iter != entityDataList.end(); ++iter )
        {
            // only datanodes need these things, so if there are no datanodes, why create them?
            if ( (false == createdDnSpecificUtils) &&
                ((*iter)->getType() == TA_Base_Core::DataNodeEntityData::getStaticType()) )
            {
                // create an instance of DataNodeAccessFactory if not done already
                LOG_GENERIC( SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
                    "Start of create DataNode Hierarchy Map" );
                
                TA_Base_Core::DataNodeAccessFactory::getInstance()->createDataNodeHierarchyMap(agentEntityData->getKey(), entityDataList);
                
                // initialise ScadaPersistence
                m_persistence = TA_Base_Bus::ScadaPersistencePtr( TA_Base_Bus::ScadaPersistence::createScadaPersistenceForAgent(TA_Base_Bus::ScadaPersistence::DATANODE, agentEntityData->getKey()) );
                
                createdDnSpecificUtils = true;
            }
            
            createdEntities.push_back( 
                TA_Base_Bus::IEntityList::value_type( (*iter)->getKey(), createEntity(*iter)) );
        }
        
        // return true to indicate not to call createEntity
        return true;
    }
    
    
    void MmsAgent::entityCreationComplete()
    {
        // loading complete, free some memory
        TA_Base_Core::DataNodeAccessFactory::getInstance()->destroyDataNodeHierarchyMap();
    }
    
    
    TA_Base_Bus::IEntity* MmsAgent::createEntity(TA_Base_Core::IEntityDataPtr entityData)
	{
		FUNCTION_ENTRY("createEntity");

		TA_Base_Bus::IEntity * theEntity = 0;
		
		std::string entityDataType = entityData->getType();

		if ( TA_Base_Core::MmsAlarmSubmitterEntityData::getStaticType() == entityDataType  )
		{
			// cast the EntityData to specific type
			TA_Base_Core::MmsAlarmSubmitterEntityDataPtr data = 
                boost::dynamic_pointer_cast< TA_Base_Core::MmsAlarmSubmitterEntityData > ( entityData );

			TA_ASSERT(0 != data.get(), "dynamic_cast failed");

			MmsAlarmSubmitter* pAlarmSubmitter = new MmsAlarmSubmitter();
			pAlarmSubmitter->initConfiguration(data);

			std::cout << entityDataType << std::endl;
			theEntity = pAlarmSubmitter;				
		}
		else if ( TA_Base_Core::MmsJobRequestSubmitterEntityData::getStaticType() == entityDataType  )
		{
			// cast the EntityData to specific type
			TA_Base_Core::MmsJobRequestSubmitterEntityDataPtr data = 
                boost::dynamic_pointer_cast< TA_Base_Core::MmsJobRequestSubmitterEntityData > ( entityData );

			TA_ASSERT(0 != data.get(), "dynamic_cast failed");

			MmsJobRequestSubmitter* pJobRequestSubmitter = new MmsJobRequestSubmitter();
			pJobRequestSubmitter->initConfiguration(data);

			std::cout << entityDataType << std::endl;
			theEntity =  pJobRequestSubmitter;				
		}
		else if ( TA_Base_Core::MmsPeriodicEntityData::getStaticType() == entityDataType  )
		{
			// cast the EntityData to specific type
			TA_Base_Core::MmsPeriodicEntityDataPtr data = 
                boost::dynamic_pointer_cast< TA_Base_Core::MmsPeriodicEntityData > ( entityData );

			TA_ASSERT(0 != data.get(), "dynamic_cast failed");

			MmsPeriodicSubmitter* pPeriodicSubmitter = new MmsPeriodicSubmitter();
			pPeriodicSubmitter->initConfiguration(data);

			std::cout << entityDataType << std::endl;

			theEntity = pPeriodicSubmitter;			
		}
		else if ( TA_Base_Core::MmsConnectiontEntityData::getStaticType() == entityDataType  )
		{
			// cast the EntityData to specific type
			TA_Base_Core::MmsConnectiontEntityDataPtr data = 
                boost::dynamic_pointer_cast< TA_Base_Core::MmsConnectiontEntityData > ( entityData );

			TA_ASSERT(0 != data.get(), "dynamic_cast failed");

			MmsConnection* pConnection = MmsConnection::getInstance();
			pConnection->initConfiguration(data);

			std::cout << entityDataType << std::endl;

			theEntity = pConnection;			
		}
		else if ( TA_Base_Core::DataPointEntityData::getStaticType() == entityDataType  )
		{
			// cast the EntityData to specific type
			TA_Base_Core::DataPointEntityDataPtr data = 
                boost::dynamic_pointer_cast< TA_Base_Core::DataPointEntityData > ( entityData );

			TA_ASSERT(0 != data.get(), "dynamic_cast failed");

			//MmsServerState::createDataPoint() will throw TA_Base_Core::BadParameterException if it don't like the config
			TA_Base_Bus::IEntity* pDataPoint = MmsServerState::getInstance()->createDataPoint(data, m_dataPointFactory); 

			std::cout << entityDataType << std::endl;
            {
	            TA_Base_Core::ThreadGuard guard ( m_dataPointListtLock );
                TA_Base_Bus::DataPoint* thedatapoint = dynamic_cast<TA_Base_Bus::DataPoint*>(pDataPoint);
                m_datapoints.insert(std::pair<unsigned long, TA_Base_Bus::DataPoint * >(thedatapoint->getEntityKey(),thedatapoint ));
            }
			
			theEntity = pDataPoint;			
		}
		else if(TA_Base_Core::DataNodeEntityData::getStaticType() == entityDataType)
		{
			TA_Base_Bus::DataNode* dn = NULL;
			try
			{
                dn = new TA_Base_Bus::DataNode (boost::dynamic_pointer_cast<TA_Base_Core::DataNodeEntityData>(entityData), m_persistence);
                
                m_dataNodes.push_back( dn );
				theEntity = dn;
			}
			catch (...)
			{
				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError, "Error creating DataNode %s (%ul)",
                    entityData->getName().c_str(), entityData->getKey() );
				
				// To ensure the generic agent is constructed even if one DataNode creation fails
			}
		}

		//if we got to here the configuration was unknown, do not care just log some warning
		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugWarn,
			"Unknow IEntityData type [%s] pkey[%d] and name[%s]" , entityDataType.c_str(), entityData->getKey(), entityData->getName().c_str());
		//throw new TA_Base_Core::BadParameterException("createEntity() called with unknown IEntityData param");

		FUNCTION_EXIT;
		
		return theEntity;
	}


    void MmsAgent::agentSetMonitor()
	{
		FUNCTION_ENTRY("agentSetMonitor");	
		
		switch(m_operationMode)
		{
			case TA_Base_Core::Monitor :
			{
				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugWarn,
					"agent is already in monitor mode");
				break;
			}
			case TA_Base_Core::NotApplicable :
			{
				setChildrenToMonitor();
				break;
			}
			case TA_Base_Core::Control :
			{
				//the framework will set the state of our children to monitor
				break;
			}
			default:
			{
				TA_ASSERT(false, "current operation mode is bad");
			}
		} //end switch

		//currently does nothing
		registerForStateUpdates();
		// if bulk interface is set up
		if ( 0 != m_dpBulkPolledInterface )
		{
			// stop receiving remote requests
			m_dpBulkPolledInterface->setToMonitor();
		}
		if ( 0 != m_dpAccessInterface )
		{
			// stop receiving remote requests
			m_dpAccessInterface->setToMonitor();
		}
		if ( 0 != m_dnBulkPolledInterface )
		{
			// stop receiving remote requests
			m_dnBulkPolledInterface->setToMonitor();
		}
		if ( 0 != m_dnAccessInterface )
		{
			// stop receiving remote requests
			m_dnAccessInterface->setToMonitor();
		}

		// set all datanodes to monitor mode
		for ( DataNodeList::iterator itr = m_dataNodes.begin(); itr != m_dataNodes.end(); ++itr )
		{
			(*itr)->setOperationMode( false );
		}
		
		// set the proxies to monitor mode
		m_scadaProxyFactory->setProxiesToMonitorMode();

		m_operationMode = TA_Base_Core::Monitor;

		FUNCTION_EXIT;
	}

    void MmsAgent::agentSetControl()
	{
		FUNCTION_ENTRY("agentSetControl");	

		switch(m_operationMode)
		{
			case TA_Base_Core::Control : //do nothing 
			{
				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugWarn,
					"agent is already in control mode");

				break;
			}
			case TA_Base_Core::NotApplicable :
			{
				//when the agent is started the children do not get set to the apropriate mode
				//hence, we do it here
				setChildrenToControl();

				break;
			}
			case TA_Base_Core::Monitor :
			{
				//the framework will set the state of our children to control
				break;
			}
			default:
			{
				TA_ASSERT(false, "current operation mode is bad");
			}
		} //end switch

		// set all datanodes to control mode
		for ( DataNodeList::reverse_iterator itr = m_dataNodes.rbegin(); itr != m_dataNodes.rend(); ++itr )
		{
			(*itr)->setOperationMode( true );
		}
		// set the proxies to control mode
		m_scadaProxyFactory->setProxiesToControlMode();	

		//currently does nothing
		registerForStateUpdates();
		// if bulk interface is set up
		if ( 0 != m_dpBulkPolledInterface )
		{
			// stop receiving remote requests
			m_dpBulkPolledInterface->setToControl();
		}
		if ( 0 != m_dpAccessInterface )
		{
			// stop receiving remote requests
			m_dpAccessInterface->setToControl();
		}
		if ( 0 != m_dnBulkPolledInterface )
		{
			// stop receiving remote requests
			m_dnBulkPolledInterface->setToControl();
		}
		if ( 0 != m_dnAccessInterface )
		{
			// stop receiving remote requests
			m_dnAccessInterface->setToControl();
		}


		m_operationMode = TA_Base_Core::Control;

		FUNCTION_EXIT;
	}

	void MmsAgent::setChildrenToControl()
	{
        const std::map<unsigned long, TA_Base_Bus::IEntity*>* entities = m_genericAgent->getEntityMap();

        std::map<unsigned long, TA_Base_Bus::IEntity*>::const_iterator entityPtr = entities->begin();
		
		while( entityPtr != entities->end() )
        {
            if( entityPtr->second != NULL )
            {
               entityPtr->second->setToControlMode();
            }
            entityPtr++;
        }
	}

	void MmsAgent::setChildrenToMonitor()
	{
        const std::map<unsigned long, TA_Base_Bus::IEntity*>* entities = m_genericAgent->getEntityMap();

        std::map<unsigned long, TA_Base_Bus::IEntity*>::const_iterator entityPtr = entities->begin();

        while( entityPtr != entities->end() )
        {
            if( entityPtr->second != NULL )
            {
               entityPtr->second->setToMonitorMode();
            }
            entityPtr++;
        }
	}
    void MmsAgent::processOnlineUpdate(const TA_Base_Core::ConfigUpdateDetails& updateEvent)
	{
		FUNCTION_ENTRY("processOnlineUpdate");

		unsigned long keyToUpdate = updateEvent.getKey();

		//if it is the agent...
		if(m_agentKey == keyToUpdate)
		{
			//then just return because the agent has no data
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugWarn,
				"This agent has no data so why is it getting ConfigUpdateDetails?");

			FUNCTION_EXIT;
			return;

		}

//		//must not be the agent so try and find a child entity with the specified key:
//        const std::map<unsigned long, TA_Base_Bus::IEntity*>* entities = m_genericAgent->getEntityMap();
//		std::map<unsigned long, TA_Base_Bus::IEntity*>::const_iterator it = entities->find(keyToUpdate);
//
//		//if you found the key
//		if (it != entities->end())
//		{
//			//see if the entity is a datapoint:
//			TA_Base_Bus::DataPoint* theDataPoint = dynamic_cast<TA_Base_Bus::DataPoint*>((*it).second);
//			if( 0 != theDataPoint)
//			{
//				//if you found the datapoint log the fact that you don't care and return
//				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugWarn,
//				"Online update of datapoints are not supported. You will need to restart the agent for these changes to take effect.");
//				FUNCTION_EXIT;
//				return;
//			}
//
//			// must be a child entity other than a datapoint, so update it
//			it->second->update(updateEvent);
//		}
//		else //you did not find the key
//		{
//			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
//				"processOnlineUpdate() called for update on entity I do not own. Key = %u", updateEvent.getKey());
//		}

		FUNCTION_EXIT;
	}

	void MmsAgent::run()
	{
		FUNCTION_ENTRY("run");

		TA_ASSERT(0 != m_genericAgent, "m_genericAgent pointer is null");

		
		//Do not set the m_operationMode member here! It will be set in following call
		//to SetMonitor or SetControl
		//Note it is initialized to TA_Base_Core::NotApplicable in CTOR to drive init
		TA_Base_Core::EOperationMode operationMode = m_genericAgent->getOperationMode();

		// if bulk interface is setup
		// create an instance of the class DataPointAgentCorbaDef_Impl
		m_dpBulkPolledInterface = new TA_Base_Bus::DataPointAgentCorbaDef_Impl ( m_agentName, this );
		TA_ASSERT(m_dpBulkPolledInterface, "can't create dp bulkpoll interface");

		if ( TA_Base_Core::Control == operationMode )
		{
			// start receiving remote requests
			m_dpBulkPolledInterface->setToControl();
		}
		else
		{
			// stop receiving remote requests
			m_dpBulkPolledInterface->setToMonitor();
		}
		// else do nothing
		m_dpAccessInterface = new TA_Base_Bus::DataPointAccessCorbaDef_Impl ( m_agentName, this );
		TA_ASSERT(m_dpAccessInterface, "can't create datapoint access interface");

		if ( TA_Base_Core::Control == operationMode )
		{
			// start receiving remote requests
			m_dpAccessInterface->setToControl();
		}
		else
		{
			// stop receiving remote requests
			m_dpAccessInterface->setToMonitor();
		}

		m_dnBulkPolledInterface = new TA_Base_Bus::DataNodeAgentCorbaDef_Impl ( m_agentName, this );
		TA_ASSERT(m_dnBulkPolledInterface, "can't create dn bulkpoll interface");
		
		if ( TA_Base_Core::Control == operationMode )
		{
			// start receiving remote requests
			m_dnBulkPolledInterface->setToControl();
		}
		else
		{
			// stop receiving remote requests
			m_dnBulkPolledInterface->setToMonitor();
		}
		// else do nothing
		m_dnAccessInterface = new TA_Base_Bus::DataNodeAccessCorbaDef_Impl ( m_agentName, this );
		TA_ASSERT(m_dnAccessInterface, "can't create dataNode access interface");
		
		if ( TA_Base_Core::Control == operationMode )
		{
			// start receiving remote requests
			m_dnAccessInterface->setToControl();
		}
		else
		{
			// stop receiving remote requests
			m_dnAccessInterface->setToMonitor();
		}

		switch(operationMode)
		{
			case TA_Base_Core::Control :
			{
				agentSetControl();
				break;
			}
			case TA_Base_Core::Monitor :
			{
				agentSetMonitor();
				break;
			}
			default:
			{
				TA_ASSERT(false, 
					"MmsAgent operation mode was not set when MmsAgent::run() was called");

				agentSetMonitor();
			}
		} //end switch

		//At this point in the code your are saying that you have applied the mode
		//and are good to go

		m_genericAgent->run();

		FUNCTION_EXIT;
	}

    void MmsAgent::notifyGroupOffline( const std::string& group )
	{
		FUNCTION_ENTRY("notifyGroupOffline");
		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
			"This agent takes no action on based on state of the System Controller");
		FUNCTION_EXIT;
	}


    void MmsAgent::notifyGroupOnline( const std::string& group )
	{
		FUNCTION_ENTRY("notifyGroupOnline");
		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
			"This agent takes no action on based on state of the System Controller.");
		FUNCTION_EXIT;
	}

    void MmsAgent::registerForStateUpdates()
	{
		FUNCTION_ENTRY("registerForStateUpdates");		

 		FUNCTION_EXIT;
	}

    void MmsAgent::receiveSpecialisedMessage(const TA_Base_Core::StateUpdateMessageCorbaDef& message)
	{
		FUNCTION_ENTRY("receiveSpecialisedMessage");    

		FUNCTION_EXIT;
	}

	TA_Base_Bus::DataPoint * MmsAgent::getDataPoint ( unsigned long entityKey )
	{
		// stop multi threads accessing the list of DataPoint objects at the same time
		TA_Base_Core::ThreadGuard guard ( m_dataPointListtLock );

		TA_Base_Bus::DataPoint * theDataPoint = 0;

		// find if there is a data point whose pkey matches with the specified entity key
		std::map< unsigned long, TA_Base_Bus::DataPoint * >::iterator itr = m_datapoints.find ( entityKey );

		// if found one
		if ( itr != m_datapoints.end() )
		{
			// get the reference to the data point
			theDataPoint = itr->second;
		}
		// else do nothing

		return theDataPoint;
	}

    void MmsAgent::getAllDataPoints (std::map < unsigned long, TA_Base_Bus::DataPoint *>& dataPointList )
	{
		// stop multi threads accessing the list of DataPoint objects at the same time
		TA_Base_Core::ThreadGuard guard ( m_dataPointListtLock );

        dataPointList.clear();

        dataPointList = m_datapoints;
	}

    TA_Base_Bus::DataPointFactory* MmsAgent::getDataPointFactory()
    {
        TA_ASSERT(m_dataPointFactory != NULL, "m_dataPointFactory is not created successfully.");
        
        return m_dataPointFactory;
    }
    
	TA_Base_Bus::DataNode * MmsAgent::getDataNode( unsigned long entityKey )
	{
		// stop multi threads accessing the list of DataNode objects at the same time
		TA_Base_Core::ThreadGuard guard ( m_dataNodeListLock );
		
		TA_Base_Bus::DataNode * dn = NULL;
        
        const std::map<unsigned long, TA_Base_Bus::IEntity*>* dataNodes = m_genericAgent->getEntityMap();
		std::map<unsigned long, TA_Base_Bus::IEntity*>::const_iterator itr = dataNodes->find( entityKey );
		
		if ( itr != dataNodes->end() )
		{
			dn = dynamic_cast<TA_Base_Bus::DataNode *>(itr->second);
			
            if ( NULL == dn )
			{
				LOG_GENERIC( SourceInfo, TA_Base_Core::DebugUtil::DebugWarn, "Could not convert entity %ul to a DataNode", entityKey );
			}
        }
		
		return dn;
	}
	
	void MmsAgent::getAllDataNodes (std::map < unsigned long, TA_Base_Bus::DataNode *>& dataNodeList )
    {
        TA_Base_Core::ThreadGuard guard ( m_dataNodeListLock );
		
        dataNodeList.clear();
        for ( DataNodeList::iterator itr = m_dataNodes.begin(); itr != m_dataNodes.end(); ++itr )
        {
            dataNodeList[ (*itr)->getEntityKey() ] = *itr;
        }        
    }
}



