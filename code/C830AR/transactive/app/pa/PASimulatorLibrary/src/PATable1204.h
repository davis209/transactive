#ifndef PA_TABLE_1204_H
#define PA_TABLE_1204_H
/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable1204.h $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * Declaration file for PATable1204 class
  */
#include "app/pa/PASimulatorLibrary/src/PASimTable.h"
#include "app/pa/PASimulatorLibrary/src/PASimTypes.h"

namespace TA_App
{
    /**
      * @class PATable1204
      *
      * PATable1204 is a derived table class.
      */
    class PATable1204 : public PASimTable
    {

    public:
        /**
          * Preferred constructor
          * @param parent the owner of this object
          */
        PATable1204(PATableRequestProcessor * parent);

        /**
          * Destructor
          */
        ~PATable1204();

        /**
          * overridden to set up dependency on table 201
          */
        void setupDependents();

        /**
          * overridden to flag changes
          */
        void providerChanged(PASimTable * t, ChangeReasonCode reasonCode);



    private:
        /**
          *
          */
        typedef Byte Table1204DataArray[MAXSRCID][MAXSTNID][4];

        /**
          * The table data
          */
        Table1204DataArray m_tableArray;


        /**
          * overridden to make this class concrete. To be implemented later.
          */
        UserReply * processUserRead(UserRequest * ur);

        /**
          * overridden to make this class concrete. To be implemented later.
          */
        UserReply * processUserWrite(UserRequest * ur);

    }; // class PATable1204
} // namespace TA_App
#endif // #ifndef PA_TABLE_1204_H
