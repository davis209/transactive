/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/mms_agent/src/MmsConnectionException.h $
  * @author:  
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * This class provides an exception that should be thrown if a call to
  * the MMS API returns 0
  */
#if !defined(MmsConnectionException_H__27C5C1BD_FDD1_4626_B72F_A5FEEB9E1D62__INCLUDED_)
#define MmsConnectionException_H__27C5C1BD_FDD1_4626_B72F_A5FEEB9E1D62__INCLUDED_

#include "core/exceptions/src/TransactiveException.h"
#include <string>

namespace TA_IRS_App
{
	class MmsConnectionException : public TA_Base_Core::TransactiveException
	{
	public:

		MmsConnectionException(const std::string& errorMsg, long errorCode)
			: TA_Base_Core::TransactiveException(errorMsg), m_errorCode(errorCode) {}

		virtual ~MmsConnectionException() throw() {};

		long getErrorCode() const {return m_errorCode;} 

		std::string getErrorMessage() const {return what();} 

	private:
		const long m_errorCode;

	};
} //namespace TA_IRS_App

#endif // !defined(MmsConnectionException_H__27C5C1BD_FDD1_4626_B72F_A5FEEB9E1D62__INCLUDED_)
