#ifndef PA_TABLE_1303_H
#define PA_TABLE_1303_H
/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable1303.h $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * Declaration file for PATable1303 class
  */
#include "app/pa/PASimulatorLibrary/src/PASimTable.h"
#include "app/pa/PASimulatorLibrary/src/PASimTypes.h"

namespace TA_App
{
    /**
      * @class PATable1303
      *
      * PATable1303 is a derived table class.
      */
    class PATable1303 : public PASimTable
    {

    public:
        /**
          * Preferred constructor
          * @param parent the owner of this object
          */
        PATable1303(PATableRequestProcessor * parent);

        /**
          * Destructor
          */
        ~PATable1303();


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
        /**
          * Typedef for the 
          */
        typedef struct Table1303DataStruct
        {
            Byte            chime;
            unsigned short  dva_msg_1;
            unsigned short  dva_msg_2;
            unsigned short  dva_msg_3;
            unsigned short  dva_msg_4;
            unsigned short  dwell_time;
            unsigned short  period;
            unsigned long   start_time;
            unsigned long   stop_time;
            Byte            event_triggered;
            Byte            global_coverage[MAXSTNID][4];
        }
#ifdef __GNUG__
        __attribute__((packed))
#endif // #ifdef __GNUG__
        ;
#ifdef WIN32
#pragma pack(pop)
#endif // #ifdef WIN32


        /**
          *
          */
        typedef Table1303DataStruct Table1303DataArray[MAXMSGSEQ];

        /**
          * The table data
          */
        Table1303DataArray m_tableArray;


        /**
          * overridden to make this class concrete. To be implemented later.
          */
        UserReply * processUserRead(UserRequest * ur);

        /**
          * overridden to make this class concrete. To be implemented later.
          */
        UserReply * processUserWrite(UserRequest * ur);

    }; // class PATable1303
} // namespace TA_App
#endif // #ifndef PA_TABLE_1303_H
