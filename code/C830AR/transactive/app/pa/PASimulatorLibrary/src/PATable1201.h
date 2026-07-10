#ifndef PA_TABLE_1201_H
#define PA_TABLE_1201_H
/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable1201.h $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * Declaration file for PATable1201 class
  */
#include "app/pa/PASimulatorLibrary/src/PASimTable.h"
#include "app/pa/PASimulatorLibrary/src/PASimTypes.h"

namespace TA_App
{
    /**
      * @class PATable1201
      *
      * PATable1201 is a derived table class.
      *
      */
    class PATable1201 : public PASimTable
    {

    public:
        /**
          * Preferred constructor
          * @param parent the owner of this object
          */
        PATable1201(PATableRequestProcessor * parent);

        /**
          * Destructor
          */
        ~PATable1201();



    private:
        /**
          * The data in this table is just like Table201, with an additional
          * dimension by Station
          */
        typedef Byte Table1201DataArray[MAXSTNID*(MAXZONEID/8) +2];

        /**
          * The table data
          */
        Table1201DataArray m_tableArray;


        /**
          * overridden to make this class concrete. To be implemented later.
          */
        UserReply * processUserRead(UserRequest * ur);

        /**
          * overridden to make this class concrete. To be implemented later.
          */
        UserReply * processUserWrite(UserRequest * ur);

    }; // class PATable1201
} // namespace TA_App
#endif // #ifndef PA_TABLE_1201_H
