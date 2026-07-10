#ifndef PA_SIM_UTILITY_H
#define PA_SIM_UTILITY_H
/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PASimUtility.h $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * Declaration file for PASimUtility class
  */
#include "app/pa/PASimulatorLibrary/src/PASimTypes.h"
#include <string>

namespace TA_App
{
    /**
      * @class PASimUtility
      *
      * PASimUtility is a utility class for PA Tables.  It contains only static
      * methods.
      *
      */
    class PASimUtility
    {
    public:

        /**
          * data2string
          *
          * Converts the binary data in ByteVector to its hexadecimal representation
          *
          * @return the number of bytes converted to hex
          */
        static int data2string(const ByteVector & data, std::string & str);

        /**
          * string2data
          *
          * Converts the binary data in ByteVector to its hexadecimal representation
          *
          * @return the number of binary bytes converted from hex
          */
        static int string2data(const std::string & str, ByteVector & data);

        /**
          * bool2string
          *
          * Converts the boolean to a string
          *
          * @param b the boolean value
          * @param str the string representation. One of "true " or "false"
          *             (note the trailing space, for formatting purposes)
          * @param yesNo flag: if true, yields "yes" and "no " instead of T/F.
          *             Defaults to false.
          */
        static void bool2string(bool b, std::string & str, bool yesNo=false);

        /**
          * sequenceNumberToString
          * 
          * This method converts an integer to a 5-digit leading-zero-padded
          * string eg. 00023
          *
          * @param sequenceNumber The number to convert to string
          * @param str The converted string
          */
        static void sequenceNumberToString( int sequenceNumber, std::string & str );


        /**
          * string full of readable ascii for filling up tables with.  It
          * happens to be the hex bytes from 0x30 to 0x7E
          */
        static const std::string DataFiller;

    private:
        /**
          * Hidden constructor, as this class is not to be instantiated
          */
        PASimUtility();

        /**
          * Hidden destructor
          */
        ~PASimUtility();

    }; // class PASimUtility
} // namespace TA_App
#endif // #ifndef PA_SIM_UTILITY_H
