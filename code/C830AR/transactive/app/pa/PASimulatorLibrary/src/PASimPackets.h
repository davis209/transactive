#ifndef PA_SIM_PACKETS_H
#define PA_SIM_PACKETS_H
/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PASimPackets.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  * Declaration file for classes that represent basic data types. They could
  * just as easily be structs, but that confuses most C++ programmers.
  */

#include "app/pa/PASimulatorLibrary/src/PASimTypes.h"

namespace TA_App
{
    /**
      * constant representing the size of a read request packet from the
      * ISCS to the PAS (client->server)
      */
    const size_t SIZEOF_ReadRequestPacket=3;


    /**
      * constant representing the size of a write request reply packet from the
      * PAS to the ISCS (server->client)
      */
    const size_t SIZEOF_WriteRequestReplyPacket=4;


    /**
      *                              +-- ISCSReadRequest
      *                              |
      *              +- ISCSRequest -+
      *              |               |
      *              |               +-- ISCSWriteRequest
      * ISCSPacket --+
      *              |               +-- ISCSReadReply
      *              |               |
      *              +-- ISCSReply --+
      *                              |
      *                              +-- ISCSWriteReply
      *                              
      * Packet  : contains data members
      * Request : implements fromNetwork()
      * Reply   : implements toNetwork()
      *
      *
      */
    class ISCSPacket
    {
        public:
            ISCSPacket(unsigned char type);
            virtual ~ISCSPacket();

            unsigned char   m_requestType;
            unsigned short  m_tableNumber;
            unsigned short  m_tableSize;
            unsigned char   m_returnValue;
            ByteVector      m_tableData;
    };

    class ISCSRequest : public ISCSPacket
    {
        public:
            ISCSRequest(unsigned char type);

            virtual void fromNetwork(std::vector<unsigned char>::iterator & iter) = 0;
    };

    class ISCSReadRequest : public ISCSRequest
    {
        public:
            ISCSReadRequest();

            /**
              * ISCS->PAS Read request packet format
              * (see ISCS-COM SICD Vol3 (PA) Appendix A A.1.1)
              *
              * <PRE>
              *     BYTE    |   SIZE    |      DESCRIPTION      |   DATA
              * ------------+-----------+-----------------------+------------------------
              *      1      |     1     |  Table Read Request   |   "R"
              * ------------+-----------+-----------------------+------------------------
              *     2-3     |    2      |  Table Number         |   eg. 200
              * ------------+-----------+-----------------------+------------------------
              * </PRE>
              *
              */
            void fromNetwork(std::vector<unsigned char>::iterator & iter);
    };
        
    class ISCSWriteRequest : public ISCSRequest
    {
        public:
            ISCSWriteRequest();

            /**
              *
              * ISCS->PAS Write request packet format
              *
              * <PRE>
              *     BYTE    |   SIZE    |      DESCRIPTION      |   DATA
              * ------------+-----------+-----------------------+------------------------
              *      1      |     1     |  Table Write Request  |   "W"
              * ------------+-----------+-----------------------+------------------------
              *     2-3     |     2     |  Table Number         |   eg. 201
              * ------------+-----------+-----------------------+------------------------
              *     4-5     |     2     |  Table Size           |   eg. 6
              * ------------+-----------+-----------------------+------------------------
              *     6 to    |           |                       |
              * (Table size |    ...    |  Table Data           |   ...
              *  - 1 + 6)   |           |                       |
              * ------------+-----------+-----------------------+------------------------
              * </PRE>
              *
              */
            void fromNetwork(std::vector<unsigned char>::iterator & iter);
    };
        
    class ISCSReply : public ISCSPacket
    {
        public:
            ISCSReply(unsigned char type);

            virtual void toNetwork(std::vector<unsigned char> & packet) = 0;
    };
        
    class ISCSReadReply : public ISCSReply
    {
        public:
            ISCSReadReply();

            void toNetwork(std::vector<unsigned char> & packet);
    };
        
    class ISCSWriteReply : public ISCSReply
    {
        public:
            ISCSWriteReply();

            void toNetwork(std::vector<unsigned char> & packet);
    };
        
    /**
      * A class formed by the PASimulator::execXXXX methods from user
      * requests given by PASimulatorCommandProcessor and sent to
      * PATableRequestProcessor.
      */
    class UserRequest
    {
        public:
            unsigned char   m_requestType;
            unsigned short  m_tableNumber;
            std::string     m_requestInfo;
    };

    /**
      * A struct used to reply to execXXX requests received from the
      * PASimulatorCommandProcessor
      */
    class UserReply
    {
        public:
            unsigned char   m_replyType;
            unsigned short  m_tableNumber;
            std::string     m_replyData;
    };

} // namespace TA_App
#endif // #ifndef PA_SIM_PACKETS_H
