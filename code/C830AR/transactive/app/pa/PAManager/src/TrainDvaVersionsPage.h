/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/TrainDvaVersionsPage.h $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * Controls the Train DVA version display page
 *
 */

#if !defined(AFX_TRAINDVAVERSIONSPAGE_H__DBEA8DBB_6608_434F_8933_65E20F120110__INCLUDED_)
#define AFX_TRAINDVAVERSIONSPAGE_H__DBEA8DBB_6608_434F_8933_65E20F120110__INCLUDED_

#include "bus/mfc_extensions/src/coloured_controls/HighlightItemListCtrl.h"
#include "bus/pa/cachedmaps/src/PaTrainDvaMessageCachedMap.h"
#include "bus/trains/TrainCorbaProxy/src/TrainPaCorbaProxy.h"
#include "bus/trains/TrainSelectionComponent/src/TrainListCtrl.h"

#include <map>

class PAManagerDlg;

// AZ++ TES 10924
class TrainDvaVersionsPage : public CPropertyPage,
    public TA_IRS_Bus::PaTrainDvaMessageCachedMap::IGlobalTrainVersionObserver,
    public TA_IRS_Bus::ICustomTrainDataProvider,
    public TA_IRS_Bus::ITrainPaUpdateObserver
// ++AZ
{
    DECLARE_DYNCREATE( TrainDvaVersionsPage )

    // Construction

public:

    TrainDvaVersionsPage(PAManagerDlg* paManagerDlg = NULL);
    ~TrainDvaVersionsPage();

    /**
     * postDialogCreate
     *
     * This function should be called once this property page has been initialized
     * and added to the tab control.  Some of the GetClientRect functions *do not* work
     * properly when put in OnInitDialog, driving the need for this function.  It should
     * be called before the page is to become visible (as view related setup is performed)
     *
     */
    void postDialogCreate();

    /**
     * onGlobalTrainVersionChanged (interface implementation)
     * @see IGlobalTrainVersionObserver::onGlobalTrainVersionChanged
     */
    virtual void onGlobalTrainVersionChanged();

    typedef TA_Base_Bus::HighlightItemListCtrl HighlightItemListCtrl;

    std::string getCustomData( unsigned int trainId,
                               const std::string& columnName );

    void updateTrainList( const TA_IRS_Bus::ITrainSelectorGUI::EAction& action,
                          const TA_IRS_Bus::TrainInformationTypes::TrainDetails& trainDetails );

    /**
     * This will receive a PaAnnouncementUpdate message from the local train agent.
     *
     * @param event    The event to process
     */
    virtual void processPaAnnouncementUpdate( const TA_IRS_Bus::PaTypes::PaAnnouncementUpdate& event ) {};

    /**
     * This will receive a PaUpdateEvent message from the local train agent.
     *
     * @param event    The event to process
     */
    virtual void processPaUpdateEvent( const TA_IRS_Bus::PaTypes::PaUpdateEvent& event ) {};

    /**
     * This will receive a TrainLibraryInfo message from the local train agent.
     *
     * @param event    The event to process
     */
    virtual void processTrainLibraryInfo( const TA_IRS_Bus::PaTypes::TrainLibraryInfo& event );

    // Overrides
    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(TrainDvaVersionsPage)

protected:

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

    // Implementation

protected:

    // Generated message map functions
    //{{AFX_MSG(TrainDvaVersionsPage)
    afx_msg void OnItemchangingDvaHeaderLst(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDestroy();
    afx_msg LRESULT OnGlobalTrainVersionChange(UINT wParam, LONG lParam);
    afx_msg LRESULT OnSingleTrainVersionChange(UINT wParam, LONG lParam);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:

    /**
     * setupMainVersionList
     *
     * Helper function to set up the main Train DVA version list
     *
     */
    void setupMainVersionList();

    /**
     * setupFirstHeaderList
     *
     * Helper function to set up the topmost header list control
     *
     */
    void setupFirstHeaderList();

    /**
     * setupListView (helper function)
     *
     * Sets up basic elements of input list (adds columns)
     *
     * @param list the list to apply changes to
     *
     * @param columnCount the number of columns to add to the list
     *
     * @param accountForScroll whether or not to account for width of scroll
     *          bar when setting the default column width
     *
     * @param format the formatting (left / centre / right aligned) to apply
     *
     */
    static void setupListView( CListCtrl& list,
                               DWORD columnCount,
                               bool accountForScroll,
                               DWORD format );

    /**
     * refreshISCSVersion
     *
     * Updates the ISCS version row with most up to date information from the database
     * @pre the ISCS version row must exist in the version list control
     */
    void refreshISCSVersion();

    /**
     * populateInitialTrainVersions
     *
     * Get the initial train PA versions from the train agent.
     */
    void populateInitialTrainVersions();

    /**
     * startSubscriptions
     *
     * Get Train PA Version updates
     * Set up train selection lists
     */
    void startSubscriptions();

    // Dialog Data
    //{{AFX_DATA(TrainDvaVersionsPage)
    enum { IDD = IDD_TRAIN_DVA_VERSIONS };
    CListCtrl    m_firstVersionHeaderList;
    // AZ++ TES 10924
    //    HighlightItemListCtrl    m_versionList;
    TA_IRS_Bus::TrainListCtrl m_versionList;
    // ++AZ
    //}}AFX_DATA

    TA_IRS_Bus::TrainPaCorbaProxyPtr m_trainPaCorbaProxy;

    int m_trainIdColumnIndex;
    int m_versionColumnIndex;

    std::map<unsigned char, int> m_trainLibraryVersions;

    static const std::string TRAIN_ID_COLUMN;
    static const std::string VERSION_COLUMN;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRAINDVAVERSIONSPAGE_H__DBEA8DBB_6608_434F_8933_65E20F120110__INCLUDED_)
