/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/IAsyncCancelCondition.h $
 * @author Jade Lee
 * @version $Revision: #2 $
 *
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * Provides basic interface to test if a cancel (termination) condition has been met
 *
 */

#pragma once

interface IAsyncCancelCondition
{
public:

    virtual bool shouldCancel() = 0;
};
