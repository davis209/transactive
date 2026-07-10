#ifndef PA_TABLE_INVALID_H
#define PA_TABLE_INVALID_H
/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATableInvalid.h $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * Declaration file for PATableInvalid class
  */
#include "app/pa/PASimulatorLibrary/src/PASimTable.h"
#include "app/pa/PASimulatorLibrary/src/PASimTypes.h"

namespace TA_App
{
    /**
      * @class PATableInvalid
      *
      * PATableInvalid is a derived table class. It is a special table. Its
      * purpose is to have stub methods so that a table will always be
      * available for the user to specify, without crashing the simulator.
      */
    class PATableInvalid : public PASimTable
    {

    public:
        /**
          * Preferred constructor
          * @param parent the owner of this object
          */
        PATableInvalid(PATableRequestProcessor * parent);

        /**
          * Destructor
          */
        ~PATableInvalid();


    private:
        /**
          * Placeholder.  This table shouldn't do anything, but this function
          * is pure virtual in the base class so needs to be overridden.
          */
        UserReply * processUserRead(UserRequest * ur);

        /**
          * Placeholder.  This table shouldn't do anything, but this function
          * is pure virtual in the base class so needs to be overridden.
          */
        UserReply * processUserWrite(UserRequest * ur);
    }; // class PATableInvalid
} // namespace TA_App
#endif // #ifndef PA_TABLE_INVALID_H
