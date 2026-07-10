#ifndef PA_TABLE_100_H
#define PA_TABLE_100_H
/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable100.h $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * Declaration file for PATable100 class
  */
#include "app/pa/PASimulatorLibrary/src/PASimTable.h"
#include "app/pa/PASimulatorLibrary/src/PASimTypes.h"

namespace TA_App
{
    /**
      * @class PATable100
      *
      * PATable100 is a derived table class.
      *
      * TABLE 100 defines the priority for each PA broadcast. This priority is
      * used when several sources require a same zone for a broadcast.  The
      * priority of the sources may be modified only by maintenance staff
      * directly on the PAS.  ISCS uses this table in order to estimate the
      * priority.  This calculation is used only to display to the operator
      * the likely success of the proposed broadcast. 
      *
      * The PAS manages this priority to select the zones for the broadcast,
      * according to other broadcasts in progress.
      * 
      * The table data is conceptually arranged as a seriralized 3D array of
      * elements indexed by station, zone and mode.
      *
      */
    class PATable100 : public PASimTable
    {

    public:
        /**
          * Preferred constructor
          * @param parent the owner of this object
          */
        PATable100(PATableRequestProcessor * parent);

        /**
          * Destructor
          */
        ~PATable100();


    private:
        /**
          * typedef for the 3D array as described in the SICD.
          */
        typedef Byte Table100_3DDataArray[MAXSTNID][MAXZONEID][MAXMODEID];

        /**
          * 51200 bytes of holographic data
          */
        Table100_3DDataArray m_tableArray;


        /**
          * overridden to make this class concrete. To be implemented later.
          */
        virtual UserReply * processUserRead(UserRequest * ur);

        /**
          * overridden to make this class concrete. To be implemented later.
          */
        virtual UserReply * processUserWrite(UserRequest * ur);

    }; // class PATable100
} // namespace TA_App
#endif // #ifndef PA_TABLE_100_H
