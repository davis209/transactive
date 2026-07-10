/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/job_request_gui/src/BadRunParamException.h $
  * @author:  
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * This class provides an exception that should be thrown if you are provided with
  * invalid runparams or runparam values, either during init or during a runparam change
  *
  */
#if !defined(AFX_BADRUNPARAMEXCEPTION_H__27C5C1BD_FDD1_4626_B72F_A5FEEB9E1D62__INCLUDED_)
#define AFX_BADRUNPARAMEXCEPTION_H__27C5C1BD_FDD1_4626_B72F_A5FEEB9E1D62__INCLUDED_

#include "core/exceptions/src/TransactiveException.h"

namespace TA_IRS_App
{
	class BadRunParamException : public TA_Base_Core::TransactiveException  
	{
	public:
		BadRunParamException() {}

		BadRunParamException(const std::string& msg)		
			: TA_Base_Core::TransactiveException(msg) {}

		virtual ~BadRunParamException() {};

	};
} //namespace TA_IRS_App

#endif // !defined(AFX_BADRUNPARAMEXCEPTION_H__27C5C1BD_FDD1_4626_B72F_A5FEEB9E1D62__INCLUDED_)
