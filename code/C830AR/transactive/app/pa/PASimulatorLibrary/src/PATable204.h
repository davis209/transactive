#ifndef PA_TABLE_204_H
#define PA_TABLE_204_H
/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable204.h $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * Declaration file for PATable204 class
  */
#include "app/pa/PASimulatorLibrary/src/PASimTable.h"
#include "app/pa/PASimulatorLibrary/src/PASimTypes.h"

namespace TA_App
{
    /**
      * @class PATable204
      *
      * PATable204 is a derived table class.
      */
    class PATable204 : public PASimTable
    {

    public:
        /**
          * Preferred constructor
          * @param parent the owner of this object
          */
        PATable204(PATableRequestProcessor * parent);

        /**
          * Destructor
          */
        ~PATable204();

        /**
          * overridden to set up dependencies on all the tables with flags
          * on this table
          */
        void setupDependents();

        /**
          * overridden to flag changes
          */
        void providerChanged(PASimTable * t, ChangeReasonCode reasonCode);



    private:
        /**
          * Typedef for the 4x8-bit data for each "Local Coverage"
          */
        typedef union LocalCoverage
        {
            unsigned char four_bytes[4];
            unsigned long one_long;
        };


        /**
          *
          */
        typedef LocalCoverage Table204DataArray[MAXSRCID];

        /**
          * The table data
          */
        Table204DataArray m_tableArray;


        /**
          * overridden to make this class concrete. To be implemented later.
          */
        UserReply * processUserRead(UserRequest * ur);

        /**
          * overridden to make this class concrete. To be implemented later.
          */
        UserReply * processUserWrite(UserRequest * ur);

    }; // class PATable204
} // namespace TA_App
#endif // #ifndef PA_TABLE_204_H
