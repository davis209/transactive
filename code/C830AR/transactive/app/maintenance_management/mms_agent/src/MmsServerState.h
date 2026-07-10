/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/mms_agent/src/MmsServerState.h $
  * @author: christiandw
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * This class provides methods to report server status and owns a datapoint that 
  * represents this status.
  * It also provides a read and a write datapoint that represents the global 
  * inhibition state
  */

#if !defined(AFX_MmsServerState_H__38ABA0F3_A9BE_449C_9AF8_397F6617127D__INCLUDED_)
#define AFX_MmsServerState_H__38ABA0F3_A9BE_449C_9AF8_397F6617127D__INCLUDED_

#include "bus/scada/datapoint_library/src/DataPointFactory.h"
#include "core/synchronisation/src/ReEntrantThreadLockable.h"
#include "core/data_access_interface/entity_access/src/MmsConnectiontEntityData.h"

namespace TA_Base_Bus
{
	class MmsConnectionFailure;
	class LocalUpdateEventProcessor;
	class BooleanDataPoint;
}

namespace TA_IRS_App
{

	class MmsServerState  
	{
	public:

		/**
		* getInstance
		* This class is a singleton
		*/
		static MmsServerState* getInstance();

		/**
		* DTOR
		*
		*/
		virtual ~MmsServerState();

		/**
		* initMmsServerState
		*
		* This method will:
		* set MMS Server failure datapoint to QUALITY_BAD_NOT_CONNECTED
		* and get the Global Inhibit state from the agent data 
		* and set the related datapoint to the value retrieved
		*/
		void initMmsServerState();

		/**
		* createDataPoint
		*
		* This method will create and register the datapoints representing the MMS Server state
		* and the global inhibit state.
		* @return the newly created datapoints
		* @throws TA_Base_Core::BadParameterException if param data is not appropriate to create the
		* above datapoints
		*/
		TA_Base_Bus::DataPoint* createDataPoint(TA_Base_Core::DataPointEntityDataPtr data, TA_Base_Bus::DataPointFactory* dataPointFactory);

		/**
		* reportMmsServerFailure
		*
		* This method will raise alarm and update MMS Server datapoint 
		* as required.
		*
		*/
		void reportMmsServerFailure(TA_Base_Bus::MmsConnectionFailure& ex);


		/**
		* reportMmsServerSucess
		*
		* This method will close alarm and update MMS Server datapoint 
		* as required.
		*
		* A client should call this if they know that the MMS connection is working
		*/
		void reportMmsServerSucess();

		/**
		* setGlobalInhibitDataPoint
		*
		* set the global MMS inhibit datapoint to the specified state
		*/
		void setGlobalInhibitDataPoint(bool applied);

		/**
		* reportMmsServerSucess
		*
		* get the global MMS inhibit state
		*/
		bool getGlobalInhibit();

		/**
		* getGlobalInhibitKey
		*
		* get the key to the datapoint representing the global MMS inhibit state
		*/
		unsigned long getGlobalInhibitKey();

		/**
		* setData
		*
		* just stores the pointer for later use
		*/
		void setData(TA_Base_Core::MmsConnectiontEntityDataPtr pData);


	private: //data members:	
		
		static MmsServerState* m_instance;
		static TA_Base_Core::ReEntrantThreadLockable m_lock;

		bool m_globalInhibitApplied;
		// Deliberately created as a member because it is needed before generic agent is created.
		TA_Base_Bus::BooleanDataPoint* m_globalInhibitPoint;

		//high is fault, low is OK
		TA_Base_Bus::BooleanDataPoint* m_serverStatusPoint;
		TA_Base_Core::MmsConnectiontEntityDataPtr m_connectionData;

	private: //methods:	

		//this class is a singleton
		MmsServerState();

		//fails silently, other than logging to debug util
		void setMmsServerStatePoint(bool value);



	}; //end of class

} //end namespace TA_IRS_App

#endif // !defined(AFX_MmsServerState_H__38ABA0F3_A9BE_449C_9AF8_397F6617127D__INCLUDED_)
