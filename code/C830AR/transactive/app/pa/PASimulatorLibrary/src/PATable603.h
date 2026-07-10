#ifndef PA_TABLE_603_H
#define PA_TABLE_603_H
/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable603.h $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * Declaration file for PATable603 class
  */
#include "app/pa/PASimulatorLibrary/src/PASimTable.h"
#include "app/pa/PASimulatorLibrary/src/PASimTypes.h"

namespace TA_App
{
    /**
      * @class PATable603
      *
      * PATable603 is a derived table class. Description from the SICD:
      *
      * A.25.1	Function
      *     This table gives the label and type of each prerecorded DVA message
      *     on a PAS (setA at OCR, setB only in station, setD in DCR).
      *     The prerecorded DVA message labels and types may be modified only
      *     by maintenance staff directly on the PAS.
      *
      * A.25.2	Addressing
      *     This table is indexed by DVA message starting at DVA Message 7
      *     (1st prerecorded message).
      *
      * A.25.3	Flow
      *     PA -> ISCS
      *
      *
      */
    class PATable603 : public PASimTable
    {

    public:
        /**
          * Preferred constructor
          * @param parent the owner of this object
          */
        PATable603(PATableRequestProcessor * parent);

        /**
          * Destructor
          */
        ~PATable603();


    private:
        static const unsigned char no_message;
        static const unsigned char emergency_message;
        static const unsigned char normal_message;

#ifdef WIN32
#pragma pack(push,1)
#endif // #ifdef WIN32
        typedef struct Table603Record
        {
            unsigned char label[40];
            unsigned char type;
        }
#ifdef __GNUG__
        __attribute__((packed))
#endif // #ifdef __GNUG__
        ;
#ifdef WIN32
#pragma pack(pop)
#endif // #ifdef WIN32

        /**
          * typedef for the 3D array as described in the SICD.
          */
        typedef Table603Record Table603DataArray[NBPREREC];

        /**
          * 
          */
        Table603DataArray m_tableArray;


        /**
          * overridden to make this class concrete. To be implemented later.
          */
        virtual UserReply * processUserRead(UserRequest * ur);

        /**
          * overridden to make this class concrete. To be implemented later.
          */
        virtual UserReply * processUserWrite(UserRequest * ur);

    }; // class PATable603
} // namespace TA_App
#endif // #ifndef PA_TABLE_603_H
