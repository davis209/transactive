#ifndef PA_TABLE_301_H
#define PA_TABLE_301_H
/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable301.h $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * Declaration file for PATable301 class
  */
#include "app/pa/PASimulatorLibrary/src/PASimTable.h"
#include "app/pa/PASimulatorLibrary/src/PASimTypes.h"

namespace TA_App
{
    /**
      * @class PATable301
      *
      * PATable301 is a derived table class.
      */
    class PATable301 : public PASimTable
    {

    public:
        /**
          * Preferred constructor
          * @param parent the owner of this object
          */
        PATable301(PATableRequestProcessor * parent);

        /**
          * Destructor
          */
        ~PATable301();



    private:
        /**
          * Typedef for the 2-byte struct for each zone
          *
          * Unfortunately, we can't really use this as the basis for data,
          * since VC++ struct member alignment defaults to 8 bytes.  That
          * can, of course, be changed in various ways, but none of them
          * actually work.
          * Using the /Zp1 compiler option on this library causes the other
          * (that out of our control) code to go berserk and cause exceptions.
          *
          * Using "#pragma pack(1)" only squeezes the size of ZoneDescriptor to 6 !
          *
          * Update: unless the last field is an unsigned char, not an unsigned
          *         int (doh!)
          * Update: despite getting the ZoneDescriptor into two bytes, it is still
          * a little useless since on Intel the bytes are the darned wrong way
          * around, and its just too much bother to keep calling ntohs and htons
          * to overcome the problem.  We may as well use
          *
          *     Byte twoBytes[2] = m_tableArray[someZoneId];
          *     unsigned char theSourceCode = twoBytes[0];
          *     bool theStatus = (0x80 & twoBytes[1])
          *     unsigned char thePriority = (0x7F & twoBytes[1])
          */
#ifdef WIN32
#pragma pack(push,1)
#endif // #ifdef WIN32
        typedef struct Table301Record
        {
            Byte            messageSequenceId;
            Byte            announceId;
            unsigned short  period;
            unsigned long   start;
            unsigned long   stop;
            Byte            eventTriggered;
            Byte            localCoverage[4];
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
        typedef Table301Record Table301DataArray;

        /**
          * The table data
          */
        Table301DataArray m_tableArray;


        /**
          * overridden to make this class concrete. To be implemented later.
          */
        UserReply * processUserRead(UserRequest * ur);

        /**
          * overridden to make this class concrete. To be implemented later.
          */
        UserReply * processUserWrite(UserRequest * ur);

    }; // class PATable301
} // namespace TA_App
#endif // #ifndef PA_TABLE_301_H
