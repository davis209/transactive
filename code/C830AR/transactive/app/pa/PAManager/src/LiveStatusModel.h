/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/LiveStatusModel.h $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * LiveStatusModel controls the shared business logic for live broadcast status dialog screens
 */

#if !defined(LiveStatusModel_4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_)
#define LiveStatusModel_4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_

#include "app/pa/PAManager/src/GenericStatusModel.h"

class LiveStatusModel : public GenericStatusModel
{
public:

    /**
     * LiveStatusModel(constructor)
     *
     */
    LiveStatusModel(GenericStatusPage& page,
                    const std::string& broadcastId,
                    const EModelSubType& subType);

protected:

private:
};

#endif // !defined(LiveStatusModel_4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_)
