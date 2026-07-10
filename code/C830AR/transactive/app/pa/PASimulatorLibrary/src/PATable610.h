#ifndef PA_TABLE_610_H
#define PA_TABLE_610_H
/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable610.h $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * Declaration file for PATable610 class
  */
#include "app/pa/PASimulatorLibrary/src/PASimTable.h"
#include "app/pa/PASimulatorLibrary/src/PASimTypes.h"

namespace TA_App
{
    /**
      * @class PATable610
      *
      * PATable610 is a derived table class. Description from the SICD:
      *
      * A.27.1	Function
      * This table contains the identification of the audio file associated to
      * each DVA messages stored in a station, Depot or OCC PAS. It allows ISCS
      * to check the version of private set messages (set A, B or D). 
      *
      * A.27.2	Addressing
      * This table has a specific index.
      *
      * A.27.3	Flow
      * PA -> ISCS 
      *
      *
      */
    class PATable610 : public PASimTable
    {

    public:
        /**
          * Constructor
          *
          * @param parent the owner of this object
          */
        PATable610(PATableRequestProcessor * parent);

        /**
          * Destructor
          */
        ~PATable610();


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
        
#ifdef WIN32
#pragma pack(push,1)
#endif // #ifdef WIN32
        typedef Byte VersionID[6];

        typedef struct VersionIDArray
        {
            VersionID   adhoc[NBADHOC];
            VersionID   prerecorded;
        }
#ifdef __GNUG__
        __attribute__((packed))
#endif // #ifdef __GNUG__
        ;

        /**
          * typedef for the array as described in the SICD.
          */
        typedef struct Table610DataArray
        {
            VersionIDArray  privateSet;
            VersionIDArray  commonSet;
        }
#ifdef __GNUG__
        __attribute__((packed))
#endif // #ifdef __GNUG__
        ;

        /**
          * alternative typedef that is more certain of compiling to the
          * correct size regardless of cpu architecture
          */
        // typedef Byte Table610DataArray[2][NBADHOC+1][6];

#ifdef WIN32
#pragma pack(pop)
#endif // #ifdef WIN32
        /**
          * 
          */
        Table610DataArray m_tableArray;


        /**
          * overridden to make this class concrete. To be implemented later.
          */
        virtual UserReply * processUserRead(UserRequest * ur);

        /**
          * overridden to make this class concrete. To be implemented later.
          */
        virtual UserReply * processUserWrite(UserRequest * ur);

    }; // class PATable610
} // namespace TA_App
#endif // #ifndef PA_TABLE_610_H
