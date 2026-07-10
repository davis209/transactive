#ifndef PA_TABLE_350_H
#define PA_TABLE_350_H
/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable350.h $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * Declaration file for PATable350 class
  */
#include "app/pa/PASimulatorLibrary/src/PASimTable.h"
#include "app/pa/PASimulatorLibrary/src/PASimTypes.h"

namespace TA_App
{
    /**
      * @class PATable350
      *
      * PATable350 is a derived table class.
      */
    class PATable350 : public PASimTable
    {

    public:
        /**
          * Preferred constructor
          * @param parent the owner of this object
          */
        PATable350(PATableRequestProcessor * parent);

        /**
          * Destructor
          */
        ~PATable350();



    private:
        /**
          * Typedef for the -byte struct for each zone
          */
#ifdef WIN32
#pragma pack(push,1)
#endif // #ifdef WIN32
        typedef struct Table350Record
        {
            Byte            messageSequenceId;
            Byte            chime;
            unsigned short  dvaMessage1;
            unsigned short  dvaMessage2;
            unsigned short  dvaMessage3;
            unsigned short  dvaMessage4;
            unsigned short  dwellTime;
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
        typedef Table350Record Table350DataArray;

        /**
          * The table data
          */
        Table350DataArray m_tableArray;


        /**
          * overridden to make this class concrete. To be implemented later.
          */
        UserReply * processUserRead(UserRequest * ur);

        /**
          * overridden to make this class concrete. To be implemented later.
          */
        UserReply * processUserWrite(UserRequest * ur);

    }; // class PATable350
} // namespace TA_App
#endif // #ifndef PA_TABLE_350_H
