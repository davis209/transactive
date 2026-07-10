#ifndef PA_TABLE_550_H
#define PA_TABLE_550_H
/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable550.h $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * Declaration file for PATable550 class
  */
#include "app/pa/PASimulatorLibrary/src/PASimTable.h"
#include "app/pa/PASimulatorLibrary/src/PASimTypes.h"

namespace TA_App
{
    /**
      * @class PATable550
      *
      * PATable550 is a derived table class.
      */
    class PATable550 : public PASimTable
    {

    public:
        /**
          * Preferred constructor
          * @param parent the owner of this object
          */
        PATable550(PATableRequestProcessor * parent);

        /**
          * Destructor
          */
        ~PATable550();



    private:
        /**
          * Array of 2-Bytes
          */
        typedef Byte NineWordArray[9][2];

        /**
          *
          */
        typedef NineWordArray Table550DataArray;

        /**
          * The table data
          */
        Table550DataArray m_tableArray;


        /**
          * overridden to make this class concrete. To be implemented later.
          */
        UserReply * processUserRead(UserRequest * ur);

        /**
          * overridden to make this class concrete. To be implemented later.
          */
        UserReply * processUserWrite(UserRequest * ur);

    }; // class PATable550
} // namespace TA_App
#endif // #ifndef PA_TABLE_550_H
