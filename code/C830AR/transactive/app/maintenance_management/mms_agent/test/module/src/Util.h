/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/mms_agent/test/module/src/Util.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
// Util.h: interface for the Util class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UTIL_H__464EC01F_814B_4353_AC59_52912280108B__INCLUDED_)
#define AFX_UTIL_H__464EC01F_814B_4353_AC59_52912280108B__INCLUDED_

#include <string>

class Util  
{
public:

	const static std::string DATA_NODE_ENTITY_NAME;

	const static std::string AUTO_ALARM_UID;
	const static std::string AUTO_ALARM_LOC;

	const static std::string SEMI_ALARM_UID;
	const static std::string SEMI_ALARM_LOC;

	Util();
	virtual ~Util();

};

#endif // !defined(AFX_UTIL_H__464EC01F_814B_4353_AC59_52912280108B__INCLUDED_)
