#ifndef PA_SIM_TYPES_H
#define PA_SIM_TYPES_H
/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PASimTypes.h $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * Declaration file for typedefs common to PA Simulation classes.
  */
#include "core/types/src/ta_types.h"
#include <string>
#include <vector>

namespace TA_App
{
    /**
      * Byte
      *
      * The same as unsigned char.  Would have liked to go via the ta_types
      * ta_uint8 to ensure cross-platformability, however on Windows it maps
	  * to "unsigned __int8" which apparently isn't the same as unsigned char,
      * or at least not the same enough to stop vc++ disallowing conversion
      * between types templated on it.
      */
    typedef unsigned char Byte;

    /**
      * ByteArray
      *
      * Essentially the same as unsigned char *, but via the ta_types
      * unsigned 8-bit type.
      */
    typedef Byte * ByteArray;

    /**
      * ByteString
      *
      * Essentially the same as std::string, but templated on the ta_types
      * unsigned 8-bit type.
      */
    typedef std::basic_string<Byte, std::char_traits<Byte>, std::allocator<Byte> > ByteString;

    /**
      * ByteVector
      *
      * Template for an STL vector templated on the unsigned 8-bit type.
      */
    typedef std::vector<Byte> ByteVector;


    /**
      * BinaryData
      *
      * This is the type that the PASimulatorLibrary code uses to represent the
      * stream of binary data that is sent to and received from the PASAgent.
      *
      * It won't really work with anything other than the ByteVector, as that
      * is what TcpObservedSocket<> uses for its read and write buffer.
      */
    typedef ByteVector BinaryData;



    /**
      * Constants representing Write Request Reply Packet return values
      */
    const unsigned char PAS_ERR_OK                  = 0;
    const unsigned char PAS_ERR_GENERIC             = 1;
    const unsigned char PAS_ERR_BUSY_MESSAGE_SEQ    = 2;
    const unsigned char PAS_ERR_BUSY_SOURCE         = 3;
    const unsigned char PAS_ERR_BUSY_RECORDING      = 4;
    const unsigned char PAS_ERR_BUSY_BROADCASTING   = 5;
    const unsigned char PAS_ERR_INVALID_ANNOUNCE    = 6;
    const unsigned char PAS_ERR_NO_FLAG             = 7;
    const unsigned char PAS_ERR_NO_FIRE             = 8;


    /**
      * Constants representing various maximum constants return values
      */
    const int MAXZONEID = 32 ;
    const int MAXFLAGID = 32 ;
    const int MAXMODEID = 32 ;
    const int MAXZMODEID= 32 ;
    const int MAXSRCID  = 32 ;
    const int MAXSTNID  = 50 ;
    const int MAXMSGSEQ = 4  ;
    const int MAXAMPID  = 70 ;
    const int MAXDVAMSG = 500;
    const int NBPREREC  = 494;
    const int NBADHOC   = 6  ;

    /**
      * The LIST 50 defines the value of each physical source of all PAS
      * devices. Each physical input on a particular PAS corresponds to a
      * unique value of this list : it is the same on each PAS and ISCS
      * computers of the whole line.
      *
      * These values are fixed and cannot be changed. The typical list is shown
      * below. Note that the exact list per station can be found in the SIDD.
      *
      * It is not a table (these values have no address). A same list is defined
      * to be used for OCR, Station and DCR to simplify the implementation of
      * the system.
      */
    enum
    {
        LIST50_STATION_MFT_SO_Paging_Console = 1,
        LIST50_DCR_MFT_DCO_Paging_Console = 2,
        LIST50_DCR_MFT_DPSCO_Paging_Console = 3,
        LIST50_OCR_MFT_CS_Paging_Console = 4,
        LIST50_OCR_MFT_TCO_1_Paging_Console = 5,
        LIST50_OCR_MFT_TCO_2_Paging_Console = 6,
        LIST50_OCR_MFT_TCO_3_Paging_Console = 7,
        LIST50_OCR_MFT_SCO_1_Paging_Console = 8,
        LIST50_OCR_MFT_SCO_2_Paging_Console = 9,
        LIST50_OCR_MFT_MCO_1_Paging_Console = 10,
        LIST50_OCR_MFT_MCO_2_Paging_Console = 11,
        LIST50_OCR_MFT_PSCO_Paging_Console = 12,
        // LIST50_SPARE = 13,
        // LIST50_SPARE = 14,
        LIST50_STATION_PCP_Paging_Console_1_Platform_Communication_Point_1 = 15,
        LIST50_STATION_PCP_Paging_Console_2_Platform_Communication_Point_2 = 16,
        LIST50_Radio_EPAX = 17,
        LIST50_OCR_BGM = 18,
        LIST50_PSC_BGM = 19,
        LIST50_DCR_BGM = 20,
        LIST50_DVA_Set_A = 21,
        LIST50_DVA_Set_B = 22,
        LIST50_DVA_Set_D = 23,
        LIST50_CD_Civil_Defence_input = 24,
        LIST50_NEL = 25,
        LIST50_FCC_Paging_Console = 26
    };

    /**
      * LIST 51 defines the unique index for each station across the Circle
      * Line.  This list is used to interpret the table data for the global
      * tables used only at the OCC.  These values are fixed and cannot be
      * changed. The typical list is shown below
      */
    enum
    {
        LIST51_DBG = 1,
        LIST51_MSM = 2,
        LIST51_CVC = 3,
        LIST51_MLN = 4,
        LIST51_NCH = 5,
        LIST51_BLV = 6,
        LIST51_OAR = 7,
        LIST51_TKT = 8,
        LIST51_PYL = 9,
        LIST51_MPS = 10,
        LIST51_UPL = 11,
        LIST51_KCD = 12,
        LIST51_BLY = 13,
        LIST51_LRC = 14,
        LIST51_SER = 15,
        LIST51_BSH = 16,
        LIST51_MRM = 17,
        LIST51_TMS = 18,
        LIST51_BKB = 19,
        LIST51_ADA = 20,
        LIST51_FRR = 21,
        LIST51_HLD = 22,
        LIST51_BNV = 23,
        LIST51_ONO = 24,
        LIST51_NUH = 25,
        LIST51_PPT = 26,
        LIST51_PPD = 27,
        LIST51_ALA = 28,
        LIST51_HBV = 29,
        LIST51_HBF = 30
    };
} // namespace TA_App
#endif // #ifndef PA_SIM_TYPES_H
