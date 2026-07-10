#ifndef PA_SIM_TABLE_H
#define PA_SIM_TABLE_H
/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PASimTable.h $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * Declaration file for PASimTable class
  */
#include "app/pa/PASimulatorLibrary/src/PASimTypes.h"
#include "app/pa/PASimulatorLibrary/src/PASimPackets.h"
#include <list>

namespace TA_App
{
    class PATableRequestProcessor;

    /**
      * @class PASimTable
      *
      * PASimTable is an abstract base class for PA Tables.  Whether a table is
      * only read from or written to by the ISCS is not distinguished, as all
      * tables (except 560) can be written or read by the Simulation User.
      *
      */
    class PASimTable
    {

    public:
        /**
          * Preferred constructor
          * @param tableNumber the number that this table is
          * @param parent this table's owner
          */
        PASimTable(int tableNumber, PATableRequestProcessor * parent);

        /**
          * Destructor
          */
        virtual ~PASimTable();

        /**
          * Accessor for the number of this table
          * @return the number that this table is
          */
        short getTableNumber() const;

        /**
          * Accessor for the size of this table
          * @return the size of the data in this table as determined by the
          *         value of m_tableData.size()
          */
        int getTableSize() const;

        /**
          * Means of "upward" object navigation
          * @return this instance's parent
          */
        PATableRequestProcessor * getParent() const;

        /**
          * 
          */
        bool isDirty() const;

        /**
          * Processes the Read Request Packet that has arrived from the ISCS
          *
          * @param request the ReadRequestPacket from the ISCS
          */
        virtual void processReadRequest(ISCSRequest * request);

        /**
          * Processes the Write Request Packet that has arrived from the ISCS
          *
          * Subclasses that override this function MUST call this base class
          * function at the end of their processing.
          *
          * @param request the WriteRequestPacket from the ISCS
          */
        virtual void processWriteRequest(ISCSRequest * request);

        /**
          * 
          */
        virtual UserReply * processUserRead(UserRequest * ur) = 0;

        /**
          * Processes the Write Request Packet that has arrived from the ISCS
          *
          * Subclasses that override this function MUST call this base class
          * function at the end of their processing.
          *
          * @param ur the UserRequest packet from the CommandProcessor
          * @return a UserReply
          */
        virtual UserReply * processUserWrite(UserRequest * ur) = 0;

        /**
          * Outputs the table data to the stream as a string of bytes using
          * PASimUtility::data2string, which at the moment means hexadecimal.
          */
        virtual void toStream(std::ostream& os) const;
        
        /**
          * Adds the given table to this table's set of dependents
          *
          * Part of a small inter-table observer/subscriber/dependence
          * mechanism built in to this class. &c 
          *
          * @param t the table that has declared a dependency upon this table
          */
        virtual void addDependent(PASimTable * t);

        /**
          * Default action is to do nothing. Subclasses that need to setup
          * dependencies with other tables must override this and call:
          *
          *     m_parent->dependUpon(tableNumber, this);
          *
          * for each required depended upon table
          */
        virtual void setupDependents();

    protected:

        //
        // Accessor Methods
        // 
        // Used to get values from the table
        //

        /**
          * Retrieves a byte from the table data.
          *
          * @param index the position within the table to retrieve
          * @return the byte at the specified position
          */
        Byte getByte(unsigned int index) const;

        /**
          * Retrieves an unsigned short value from the table data, converting
          * from network order first
          *
          * @param index the position within the table to retrieve
          * @return the unsigned short value at the specified position
          */
        unsigned short getUShort(unsigned int index) const;

        /**
          * Retrieves an unsigned long value from the table data, converting
          * from network order first
          *
          * @param index the position within the table to retrieve
          * @return the unsigned long value at the specified position
          */
        unsigned long getULong(unsigned int index) const;

        /**
          * Accessor for the underlying ByteVector, ie. the std::vector of
          * unsigned chars that represents the table data.
          * 
          * @return a const reference to the m_tableData member
          */
        const ByteVector & getByteVector() const;

        //
        // Mutator Methods
        // 
        // Used to set values in the table
        //

        /**
          * Sets a byte in the data
          *
          * @param value the byte value
          * @param index the [optional] position within the table to
          *              set the value
          */
        void setByte(Byte value, unsigned int index=0);

        /**
          * Sets an unsigned short value. Converts to network order first.
          *
          * @param value the unsigned short value
          * @param index the [optional] position within the table to
          *              set the value
          */
        void setUShort(unsigned short value, unsigned int index=0);

        /**
          * Sets an unsigned long value. Converts to network order first.
          *
          * @param value the unsigned long value
          * @param index the [optional] position within the table to
          *              set the value
          */
        void setULong(unsigned long value, unsigned int index=0);

        /**
          * setTableData
          *
          * This overloaded version copies byte by byte to the internal
          * representation from the given pointer.
          *
          * @param address pointer to the new table data\
          * @param bytes the number of data bytes to set. The default of -1
          *             causes m_tableData.size() bytes to be transferred.
          *             (due to the way default parameters work, it is not
          *             possible to specify the index without specifying
          *             the bytes parameter)
          * @param index the index at which to start transferring to.  If
          *             bytes+index > m_tableData.size(), an out_of_range
          *             exception is thrown.
          */
        void setTableData(void * address, int bytes=-1, unsigned int index=0);

    protected:
        /**
          * Allows subclasses that have overwritten processWriteRequest()
          * but that use PASimTable::processWriteRequest to sent the Write
          * Reply, to specify what return value to stick in that field.
          */
        void setWriteReturnValue(Byte value);

        /**
          * 
          */
        typedef enum ChangeReasonCode
        {
            no_change           = 0L,
            user_modification   = 1L,
            iscs_write_request  = 2L
        };

        /**
          * Notifies all dependents when this table is changed
          */
        void notifyDependents(ChangeReasonCode reasonCode);

        /**
          * Called but subclasses to register for notification
          */
        void dependUpon(int tableNumber);

        /**
          * This is the notification that a provider has changed, ie. a table
          * that we have setup a dependency on has called notifyDependents.
          *
          * Subclasses can override this to take special actions depending
          * on why the provider has changed.
          */
        virtual void providerChanged(PASimTable * t, ChangeReasonCode reasonCode);

    private:
        /**
          * The number of this beast
          */
        short m_tableNumber;

        /**
          * This instance's owner
          */
        PATableRequestProcessor * m_parent;

        /**
          * The data contained in the table.
          */
        ByteVector m_tableData;

        /**
          * Flag to track if this table has been changed since the last time
          * it was read
          */
        bool m_dirty;

        /**
          * 
          */
        Byte m_writeReturnValue;

        /**
          * 
          */
        typedef std::list< PASimTable * > DependentTableContainer;

        /**
          * 
          */
        DependentTableContainer m_dependents;
    }; // class PASimTable

    /**
      * Streaming operator.  Since operators are not virtual, it calls
      * the virtual toStream().  Also, since the first parameter must be the
      * OStream ref, this is a nonmember function, .
      *
      * @param os the OStream to stream to
      * @param &t
      */
    std::ostream &operator<< (std::ostream &os, const PASimTable &table);
} // namespace TA_App
#endif // #ifndef PA_SIM_TABLE_H
