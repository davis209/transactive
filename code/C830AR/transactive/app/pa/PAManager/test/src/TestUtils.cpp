/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/TestUtils.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#pragma warning (disable : 4786)

#include "StdAfx.h"
#include "TestUtils.h"
#include "WindowStub.h"
#include "app/pa/PAManager/src/PaErrorHandler.h"

namespace TestUtils
{

    bool confirmErrorReceivedAndClearWindowStub(DWORD errorId, WindowStub& windowStub)
    {
        bool updateReceived = false;
        WindowStub::processMsgQueue();
        WindowStub::MessageQueue data = windowStub.getReceived();
        
        if (data.size() > 0)
        {
            if (WM_PA_ERROR_MESSAGE == data[0].message)
            {
                if (errorId == data[0].wParam)
                {
                    PaErrorHandler::confirmReceiptOfRequest(data[0].wParam);
                    updateReceived = true;
                }
            }
        }
        
        windowStub.reset();
        return updateReceived;
    }

} // end namespace TestUtils