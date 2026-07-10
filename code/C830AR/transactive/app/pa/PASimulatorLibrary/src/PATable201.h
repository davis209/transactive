#ifndef PA_TABLE_201_H
#define PA_TABLE_201_H
/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable201.h $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * Declaration file for PATable201 class
  */
#include "app/pa/PASimulatorLibrary/src/PASimTable.h"
#include "app/pa/PASimulatorLibrary/src/PASimTypes.h"

namespace TA_App
{
    /**
      * @class PATable201
      *
      * PATable201 is a derived table class.
      *
      */
    class PATable201 : public PASimTable
    {

    public:
        /**
          * Preferred constructor
          * @param parent the owner of this object
          */
        PATable201(PATableRequestProcessor * parent);

        /**
          * Destructor
          */
        ~PATable201();



    private:
        /**
          * There are 32 zone flags, each one BIT plus 2 bytes for
          * the Source Code and Announce Id
          */
        typedef Byte Table201DataArray[MAXZONEID/8 +2];

        /**
          * The table data
          */
        Table201DataArray m_tableArray;


        /**
          * overridden to make this class concrete. To be implemented later.
          */
        UserReply * processUserRead(UserRequest * ur);

        /**
          * overridden to make this class concrete. To be implemented later.
          */
        UserReply * processUserWrite(UserRequest * ur);

    }; // class PATable201
} // namespace TA_App
#endif // #ifndef PA_TABLE_201_H
