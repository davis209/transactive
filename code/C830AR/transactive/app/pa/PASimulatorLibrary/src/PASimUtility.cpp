/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PASimUtility.cpp $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * Definition/Implementation file for PASimUtility class
  */
#include "app/pa/PASimulatorLibrary/src/PASimUtility.h"

#include <stdexcept>

using namespace TA_App;

/**
  * data2string
  *
  * Converts the binary data in ByteVector to its hexadecimal representation
  *
  * @return the number of binary bytes converted to hex
  */
int PASimUtility::data2string(const ByteVector & data, std::string & str)
{
    int converted = 0;
    ByteVector::const_iterator it;

    for ( it = data.begin() ; it != data.end() ; it++ )
    {
        // going from *(vector<Byte>::iterator) to int in two steps overcomes
        // some cross-platform compiler type-coercion idiosyncrasies 
        Byte b = *it;
        int i = b;
        char c[3];

        sprintf(c,"%.2X",i);

        str.append(c);

        ++converted;
    }

    return converted;
}

/**
  *
  */
void PASimUtility::sequenceNumberToString( int sequenceNumber, std::string & str )
{
    char buffStr[16];
    sprintf(buffStr, "%.5d",sequenceNumber);
    str=buffStr;
}


/**
  * string2data
  *
  * Converts a string of hexadecimal characters into a vector of binary bytes.
  * An assumption is made that if there are an odd number of characters in the
  * string to be converted, a leading zero is missing (and hence added).
  *
  * @return the number of binary bytes converted from hex.
  *
  * @todo implement this
  */
int PASimUtility::string2data(const std::string & str, ByteVector & data)
{
    throw std::logic_error("PASimUtility::string2data not implemented");

    return 0;
}

/**
  */
void PASimUtility::bool2string(bool b, std::string & str, bool yesNo)
{
    if (b)
    {
        if (yesNo)
        {
            str = "yes";
        }
        else
        {
            str = "true ";
        }
    }
    else
    {
        if (yesNo)
        {
            str = "no ";
        }
        else
        {
            str = "false";
        }
    }
}

const std::string PASimUtility::DataFiller = "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

