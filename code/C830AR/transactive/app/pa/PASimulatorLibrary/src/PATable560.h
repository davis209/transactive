#ifndef PA_TABLE_560_H
#define PA_TABLE_560_H
/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable560.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#include "core/types/src/ta_types.h"
#include "app/pa/PASimulatorLibrary/src/PASimTable.h"
#include "app/pa/PASimulatorLibrary/src/PASimTypes.h"

#include <bitset>
#include <iostream>

namespace TA_App
{
    /**
      * @class PATable560
      *
      * PATable560 is a derived table class. It is a very special table. Its
      * purpose is to be flagged anytime that any of its set of tables its
      * poll tables are modified.
      *
      * In the private section, we have defined a number of ways of accessing
      * the data in the table in a manner more befitting of its being a table
      * of flags.  The specs for the table are in the SICD ISCS to COM Vol 3
      * (ISCS-PA), Appendix A, pp A-80.
      *
      * The idea of having such more usable data accessors is to enable the
      * table to be displayed to the simulation user more meaningfully.
      */
    class PATable560 : public PASimTable
    {

    public:
        /**
          * Preferred constructor
          * @param parent the owner of this object
          */
        PATable560(PATableRequestProcessor * parent);

        /**
          * Destructor
          */
        ~PATable560();

        /**
          * tableChanged
          *
          * @param tableNumber the number of the table that has changed.
          */
        void flagWrite(int tableNumber);


    private:
        /**
          * Sets the flag as described by the pattern, a la the enumeration
          * below Table560Flags.
          *
          * For those tables that may eventually be in the upper word, just
          * additionally use the optional highLongWord, eg.
          *
          *     setFlag(0,FLAG_XXXX_A);
          *     setFlag(0,FLAG_XXXX_B);
          */
        void setFlag(unsigned long int flagPattern, unsigned long int highLongWord=0);

        /**
          * 
          */
        void clearFlag(unsigned long int flagPattern, unsigned long int highLongWord=0);

        /**
          * Overwritten to update flags when the PAS has acknowledged that
          * it has read Table 560 and plans to get the previously flagged
          * table[s].
          */
        void processWriteRequest(ISCSRequest * request);

        /**
          * Typedef for a 64-bit set
          */
        typedef std::bitset<64> DataBitset;

        /**
          * Interesting representation of table data
          */
        DataBitset m_dataBitset;


        /**
          * Union of more interesting representations of table data
          */
        union Table560DataUnion
        {
            unsigned char ucArray[8];
            ta_uint16 shortArray[4];
            ta_uint32 longArray[2];
        };

        /**
          * More interesting representation of table data
          */
        Table560DataUnion m_dataUnion;


    public:

        /**
          * Enumeration representing positions of the bits of the flags in Table 560
          * Index Flag A Flag B
          * 1 Flag_100_A Flag_100_B
          * 2 Flag_202_A Flag_202_B
          * 3 Flag_204_A Flag_204_B
          * 4 Flag_1204_A Flag_1204_B
          * 5 Flag_303_A Flag_303_B
          * 6 Flag_1303_A Flag_1303_B
          * 7 Flag_550_A Flag_550_B
          * 8 Flag_570_A Flag_570_B
          * 9 Flag_590_A Flag_590_B
          * 10 Flag_603_A Flag_603_B
          * 11 Flag_604_A Flag_604_B
          * 12 Flag_610_A Flag_610_B
          * 13 Flag_1610_A Flag_1610_B
          */
 
        enum Table560FlagPositions {
            FLAG_100_A_POS  = 0,
            FLAG_100_B_POS  = 1,
            FLAG_202_A_POS  = 2,
            FLAG_202_B_POS  = 3,
            FLAG_204_A_POS  = 4,
            FLAG_204_B_POS  = 5,
            FLAG_1204_A_POS = 6,
            FLAG_1204_B_POS = 7,
            FLAG_303_A_POS  = 8,
            FLAG_303_B_POS  = 9,
            FLAG_1303_A_POS = 10,
            FLAG_1303_B_POS = 11,
            FLAG_550_A_POS  = 12,
            FLAG_550_B_POS  = 13,
            FLAG_570_A_POS  = 14,
            FLAG_570_B_POS  = 15,
            FLAG_590_A_POS  = 16,
            FLAG_590_B_POS  = 17,
            FLAG_603_A_POS  = 18,
            FLAG_603_B_POS  = 19,
            FLAG_604_A_POS  = 20,
            FLAG_604_B_POS  = 21,
            FLAG_610_A_POS  = 22,
            FLAG_610_B_POS  = 23,
            FLAG_1610_A_POS = 24,
            FLAG_1610_B_POS = 25
        };

        /**
          * Enumeration representing the bits of the flags in Table 560
          */
        enum Table560Flags {
            FLAG_100_A      =  0x80000000 >> FLAG_100_A_POS,
            FLAG_100_B      =  0x80000000 >> FLAG_100_B_POS,
            FLAG_202_A      =  0x80000000 >> FLAG_202_A_POS,
            FLAG_202_B      =  0x80000000 >> FLAG_202_B_POS,
            FLAG_204_A      =  0x80000000 >> FLAG_204_A_POS,
            FLAG_204_B      =  0x80000000 >> FLAG_204_B_POS,
            FLAG_1204_A     =  0x80000000 >> FLAG_1204_A_POS,
            FLAG_1204_B     =  0x80000000 >> FLAG_1204_B_POS,
            FLAG_303_A      =  0x80000000 >> FLAG_303_A_POS,
            FLAG_303_B      =  0x80000000 >> FLAG_303_B_POS,
            FLAG_1303_A     =  0x80000000 >> FLAG_1303_A_POS,
            FLAG_1303_B     =  0x80000000 >> FLAG_1303_B_POS,
            FLAG_550_A      =  0x80000000 >> FLAG_550_A_POS,
            FLAG_550_B      =  0x80000000 >> FLAG_550_B_POS,
            FLAG_570_A      =  0x80000000 >> FLAG_570_A_POS,
            FLAG_570_B      =  0x80000000 >> FLAG_570_B_POS,
            FLAG_590_A      =  0x80000000 >> FLAG_590_A_POS,
            FLAG_590_B      =  0x80000000 >> FLAG_590_B_POS,
            FLAG_603_A      =  0x80000000 >> FLAG_603_A_POS,
            FLAG_603_B      =  0x80000000 >> FLAG_603_B_POS,
            FLAG_604_A      =  0x80000000 >> FLAG_604_A_POS,
            FLAG_604_B      =  0x80000000 >> FLAG_604_B_POS,
            FLAG_610_A      =  0x80000000 >> FLAG_610_A_POS,
            FLAG_610_B      =  0x80000000 >> FLAG_610_B_POS,
            FLAG_1610_A     =  0x80000000 >> FLAG_1610_A_POS,
            FLAG_1610_B     =  0x80000000 >> FLAG_1610_B_POS
        };

        /**
          * Enumeration representing the bitwise NOT of the flags in Table 560
          */
        enum Table560Masks {
            MASK_100_A  = ~FLAG_100_A  ,
            MASK_100_B  = ~FLAG_100_B  ,
            MASK_202_A  = ~FLAG_202_A  ,
            MASK_202_B  = ~FLAG_202_B  ,
            MASK_204_A  = ~FLAG_204_A  ,
            MASK_204_B  = ~FLAG_204_B  ,
            MASK_1204_A  = ~FLAG_1204_A  ,
            MASK_1204_B  = ~FLAG_1204_B  ,
            MASK_303_A  = ~FLAG_303_A  ,
            MASK_303_B  = ~FLAG_303_B  ,
            MASK_1303_A = ~FLAG_1303_A ,
            MASK_1303_B = ~FLAG_1303_B ,
            MASK_550_A  = ~FLAG_550_A  ,
            MASK_550_B  = ~FLAG_550_B  ,
            MASK_570_A  = ~FLAG_570_A  ,
            MASK_570_B  = ~FLAG_570_B  ,
            MASK_590_A  = ~FLAG_590_A  ,
            MASK_590_B  = ~FLAG_590_B  ,
            MASK_603_A  = ~FLAG_603_A  ,
            MASK_603_B  = ~FLAG_603_B  ,
            MASK_604_A  = ~FLAG_604_A  ,
            MASK_604_B  = ~FLAG_604_B  ,
            MASK_610_A  = ~FLAG_610_A  ,
            MASK_610_B  = ~FLAG_610_B  ,
            MASK_1610_A = ~FLAG_1610_A ,
            MASK_1610_B = ~FLAG_1610_B
        };


        /**
          * 
          */
        virtual UserReply * processUserRead(UserRequest * ur);

        /**
          * 
          */
        virtual UserReply * processUserWrite(UserRequest * ur);

        /**
          * overridden to set up dependencies on all the tables with flags
          * on this table
          */
        void setupDependents();

        /**
          * overridden to flag changes
          */
        void providerChanged(PASimTable * t, ChangeReasonCode reasonCode);

    }; // class PATable560
} // namespace TA_App
#endif // #ifndef PA_TABLE_560_H
