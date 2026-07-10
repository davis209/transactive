/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/TestUtils.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
// Helper functions for unit tests

#pragma once

class WindowStub;

namespace TestUtils
{
    /// Confirms the input stub has a single message waiting, and that
    /// it corresponds to the input PaErrorHandler notification message
    /// (defined by errorId); then the window queue is cleared
    bool confirmErrorReceivedAndClearWindowStub(DWORD errorId, WindowStub& windowStub);
}