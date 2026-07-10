/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/mms_agent/src/MmsMessageUtil.h $
  * @author: christiandw
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * This class provides access to remote dependencies. As example, you can get data from the 
  * database from here...
  *
  */

#if !defined(AFX_MmsMessageUtil_H__38ABA0F3_A9BE_449C_9AF8_397F6617127D__INCLUDED_)
#define AFX_MmsMessageUtil_H__38ABA0F3_A9BE_449C_9AF8_397F6617127D__INCLUDED_

#pragma warning(disable:4786)

#include <string>
#include "core/alarm/IDL/src/AlarmMessageCorbaDef.h"


#include "app/maintenance_management/mms_agent/src/DatabaseCache.h"
#include "core/synchronisation/src/ReEntrantThreadLockable.h"

namespace TA_Base_Core
{
	class MmsAgentEntityData;
}

namespace TA_IRS_App
{

	class MmsMessageUtil  
	{
	public:

		/**
		* getInstance
		* This class is a singleton
		*/
		static MmsMessageUtil* getInstance();

		/**
		* DTOR
		*
		*/
		virtual ~MmsMessageUtil();

		/**
		* getAgentData
		*
		* @return the updated data for the agent
		*/
		TA_Base_Core::MmsAgentEntityData* getAgentData();

		/**
		* getNodeKey
		* 
		* @return the pkey of the parent datanode
		* returns 0 if request fails for any reason
		*/
		unsigned long getNodeKey(unsigned long dataPointKey);

        /**
         * getOperatorName
         *
         * Retrieves the operator name for the specified key
         *
         * @param unsigned long - The pkey of the operator
         *
         * @return string - The operator name. An empty string if the operator name cannot
         *                  be found
         */
        std::string getOperatorName(unsigned long key) {return m_dbCache.getOperatorName(key);}

        /**
         * getSubsystemName
         *
         * Retrieves the subsystem name for the specified key
         *
         * @param unsigned long - The pkey of the subsystem
         *
         * @return string - The subsystem name. An empty string if the operator name cannot
         *                  be found
         */
        std::string getSubsystemName(unsigned long key) {return m_dbCache.getSubsystemName(key);}

		/**
		* getAlarmSeverityName
		*
		* @return the text desc of the alarm's severity
		*
		* If the required call to the database fails this method does not throw.
		* It just returns an empty string and logs an error to Debug util
		*/
		std::string getAlarmSeverityName(const TA_Base_Core::AlarmDetailCorbaDef& p_alarm);

	private: //data members:	
		
		static MmsMessageUtil* m_instance;
		DatabaseCache m_dbCache;
		TA_Base_Core::MmsAgentEntityData* m_agentData;

		//provides a thread lock
		static TA_Base_Core::ReEntrantThreadLockable m_lock;

	private: //methods:	

		//this class is a singleton
		MmsMessageUtil();

	}; //end of class

} //end namespace TA_IRS_App

#endif // !defined(AFX_MmsMessageUtil_H__38ABA0F3_A9BE_449C_9AF8_397F6617127D__INCLUDED_)
