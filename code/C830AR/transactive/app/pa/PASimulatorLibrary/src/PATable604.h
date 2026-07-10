#ifndef PA_TABLE_604_H
#define PA_TABLE_604_H
/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable604.h $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * Declaration file for PATable604 class
  */
#include "app/pa/PASimulatorLibrary/src/PASimTable.h"
#include "app/pa/PASimulatorLibrary/src/PASimTypes.h"

namespace TA_App
{
    /**
      * @class PATable604
      *
      * PATable604 is a derived table class. Description from the SICD:
      *
      * A.26.1 Function
      *     This table gives the type of each adhoc DVA message on a PAS (setA
      *     at OCR, setB in station, setD in DCR).
      *     The adhoc DVA message types may be modified by maintenance staff
      *     directly on the PAS or by ISCS operators on the MFT.
      *
      * A.26.2 Addressing
      *     This table is indexed by DVA message.
      *
      * A.26.3 Flow
      *     PA -> ISCS and ISCS -> PA
      *
      *
      */
    class PATable604 : public PASimTable
    {

    public:
        /**
          * Preferred constructor
          * @param parent the owner of this object
          */
        PATable604(PATableRequestProcessor * parent);

        /**
          * Destructor
          */
        ~PATable604();


    private:
        static const unsigned char no_message;
        static const unsigned char emergency_message;
        static const unsigned char normal_message;


        /**
          * typedef for the array as described in the SICD.
          */
        typedef unsigned char Table604DataArray[NBPREREC];

        /**
          * 
          */
        Table604DataArray m_tableArray;


        /**
          * overridden to make this class concrete. To be implemented later.
          */
        virtual UserReply * processUserRead(UserRequest * ur);

        /**
          * overridden to make this class concrete. To be implemented later.
          */
        virtual UserReply * processUserWrite(UserRequest * ur);

    }; // class PATable604
} // namespace TA_App
#endif // #ifndef PA_TABLE_604_H
