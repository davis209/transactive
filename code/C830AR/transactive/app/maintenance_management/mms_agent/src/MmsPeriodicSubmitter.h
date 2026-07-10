/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution in any form.
  * 
  * Source:    $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/mms_agent/src/MmsPeriodicSubmitter.h $
  * @author:   Chris DeWolfe
  * @version:  $Revision: #1 $
  * @see:      9999-Q01-02-0016-GenericAgent_Usage_Guidelines.doc
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last mofified by:  $Author: builder $
  *
  * This class checks to see if any periodic maintenance messages are due to be
  * sent and if so sends them
  *
  *This class is not threadsafe 
  */
#ifndef MMS_PERIODIC_SUBMITTER_H
#define MMS_PERIODIC_SUBMITTER_H

#if defined(_MSC_VER)
#pragma warning(disable:4786)
#endif // defined (_MSC_VER)

#include "bus/generic_agent/src/IEntity.h"
#include "core/data_access_interface/entity_access/src/MmsPeriodicEntityData.h"
#include "app/maintenance_management/mms_agent/src/PeriodicThread.h"
#include "core/data_access_interface/entity_access/src/MmsPeriodicEntityData.h"

namespace TA_IRS_App
{
    class MmsPeriodicSubmitter
		: public virtual TA_Base_Bus::IEntity
    {
    public:


        /**
          * The only implimnented CTOR
          */
        MmsPeriodicSubmitter();


        /**
          * DTOR
		  * Calls setMmsConnection(0) on all the registered users
		  * Diconnects from the MMS Server.
		  * Deletes the instance of this class
		  * 
          */
        virtual ~MmsPeriodicSubmitter();

        /**
          * initConfiguration
		  *
		  * Precondition: pData != 0
		  * Will apply the data in the config to the object
		  * The ping rate and incomming message check rate are updatable
		  *
		 */
		void initConfiguration(TA_Base_Core::MmsPeriodicEntityDataPtr pData);


	//IEntity methods:

        /**
          * isValid
          *
          * allways returns true
          */
        virtual bool isValid();

        /**
          * start
          *
          * does nothing
          */
        virtual void start();

        /**
          * stop
          *
          * does nothing
          */
        virtual void stop();

        /**
          * update
          *
          * This method is called by the agent controlling the entity to ask 
          * the entity to update its configuration. Called after an update 
          * configuration event is received.
		  * Will update tester and wrapper as required
          */
        virtual void update(const TA_Base_Core::ConfigUpdateDetails& updateEvent);

        /**
          * remove
          *
          * does nothing
          */
        virtual void remove();

        /**
          * setToControlMode
          *
          * clear alarms that connection may have raised
		  * start the MmsConnectionTester thread
		  * 
          */
        virtual void setToControlMode();

        /**
          * setToMonitorMode
          *
          * stop the MmsConnectionTester thread
          */
        virtual void setToMonitorMode();

	//end IEntity methods

	private: //data members:


		TA_Base_Core::MmsPeriodicEntityDataPtr m_data;
		unsigned long m_entityKey;
		std::string m_entityName;

		PeriodicThread* m_thread;

	private: //methods:

        // Disable copy constructor and assignment operator
        MmsPeriodicSubmitter( const MmsPeriodicSubmitter& con);
        MmsPeriodicSubmitter& operator=(const MmsPeriodicSubmitter& con);

		//stops and deletes the thread if not already in this state
		void terminateAndDeleteThread();

		//create and start the thread if not already in this state
		void createAndStartThread();

		
    };
} // end namespace TA_IRS_App

#endif // !defined(MMS_PERIODIC_SUBMITTER_H)