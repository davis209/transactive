#ifndef PA_TABLE_1200_H
#define PA_TABLE_1200_H
/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable1200.h $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * Declaration file for PATable1200 class
  */
#include "app/pa/PASimulatorLibrary/src/PASimTable.h"
#include "app/pa/PASimulatorLibrary/src/PASimTypes.h"

namespace TA_App
{
    /**
      * @class PATable1200
      *
      * PATable1200 is a derived table class.
      */
    class PATable1200 : public PASimTable
    {

    public:
        /**
          * Preferred constructor
          * @param parent the owner of this object
          */
        PATable1200(PATableRequestProcessor * parent);

        /**
          * Destructor
          */
        ~PATable1200();



    private:
        /**
          *
          */
        typedef Byte Table1200DataArray[MAXSTNID][MAXZONEID][2];

        /**
          * The table data
          */
        Table1200DataArray m_tableArray;


        /**
          * overridden to make this class concrete. To be implemented later.
          */
        UserReply * processUserRead(UserRequest * ur);

        /**
          * overridden to make this class concrete. To be implemented later.
          */
        UserReply * processUserWrite(UserRequest * ur);

    }; // class PATable1200
} // namespace TA_App
#endif // #ifndef PA_TABLE_1200_H
