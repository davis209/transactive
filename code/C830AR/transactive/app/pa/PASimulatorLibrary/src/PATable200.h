#ifndef PA_TABLE_200_H
#define PA_TABLE_200_H
/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable200.h $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * Declaration file for PATable200 class
  */
#include "app/pa/PASimulatorLibrary/src/PASimTable.h"
#include "app/pa/PASimulatorLibrary/src/PASimTypes.h"

namespace TA_App
{
    /**
      * @class PATable200
      *
      * PATable200 is a derived table class.
      */
    class PATable200 : public PASimTable
    {

    public:
        /**
          * Preferred constructor
          * @param parent the owner of this object
          */
        PATable200(PATableRequestProcessor * parent);

        /**
          * Destructor
          */
        ~PATable200();



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
        typedef struct ZoneDescriptor
        {
            unsigned char sourceCode;
            bool          status  :1;
            unsigned char priority:7;
        }
#ifdef __GNUG__
        __attribute__((packed))
#endif // #ifdef __GNUG__
        ;
#ifdef WIN32
#pragma pack(pop)
#endif // #ifdef WIN32

        /**
          * Array of ZoneDescriptors
          */
        typedef ZoneDescriptor ZoneDescriptorArray[MAXZONEID];

        /**
          * Array of 2-Bytes
          */
        typedef Byte TwoByteZoneArray[MAXZONEID][2];

        /**
          *
          */
        typedef TwoByteZoneArray Table200DataArray;

        /**
          * The table data
          */
        Table200DataArray m_tableArray;


        /**
          * overridden to make this class concrete. To be implemented later.
          */
        UserReply * processUserRead(UserRequest * ur);

        /**
          * overridden to make this class concrete. To be implemented later.
          */
        UserReply * processUserWrite(UserRequest * ur);

    }; // class PATable200
} // namespace TA_App
#endif // #ifndef PA_TABLE_200_H
