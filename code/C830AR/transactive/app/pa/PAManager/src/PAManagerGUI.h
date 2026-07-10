/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/PAManagerGUI.h $
 * @author:  Ripple
 * @version: $Revision: #2 $
 *
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by:  $Author: limin.zhu $
 *
 */
#if !defined(AFX_PAMANAGERGUI_H__4F99FEB2_FD1A_4478_B3C4_9942A9CAFFD1__INCLUDED_)
#define AFX_PAMANAGERGUI_H__4F99FEB2_FD1A_4478_B3C4_9942A9CAFFD1__INCLUDED_

#if _MSC_VER > 1000
    #pragma once
#endif // _MSC_VER > 1000
// PAManagerGUI.h : header file
//

#include "bus/generic_gui/src/AbstractDialogGUI.h"

class PAManagerDlg;
/////////////////////////////////////////////////////////////////////////////
// PAManagerGUI window

class PAManagerGUI : public TA_Base_Bus::AbstractDialogGUI
{
    // Construction

public:

    /**
     * Constructor
     *
     * This must construct an instance of the dialog and store it in m_application. When
     * constructing the dialog this class passes in an instance of itself.
     */
    PAManagerGUI();

    /**
     * Destructor
     *
     * This should delete the object stored at m_application
     */
    ~PAManagerGUI();
    //TD18095, jianghp
    virtual void onInitGenericGUICompleted();

    /**
     * getApplicationType
     *
     * This returns the application type for this GUI. GenericGUI will then save
     * this in the RunParams and will use it to pass to the Control Station. In the
     * existing system the application type would need to be hard-coded into here.
     *
     * @return unsigned long representing the application type
     *
     */
    unsigned long getApplicationType();

    /**
     * checkEntity
     *
     * This will check that the entity is valid and then store a local pointer to the
     * entity.
     *
     * @param IEntityData* guiEntity - A pointer to the gui entity to check and store.
     *
     * @exceptions GenericGUIException with the type set to ENTITY_CONFIGURATION_INVALID
     *             This should be thrown if the application checks the entity and it is
     *             not configured correctly for the GUI to run.
     *
     */
    void checkEntity(TA_Base_Core::IEntityData* guiEntity) throw(TA_Base_Core::TransactiveException);

    /**
     * checkCommandLine
     *
     * This checks that all the appropriate RunParams have been set from the command line.
     * If anything is invalid or missing then this will throw an exception.
     * All settings SHOULD be in the GUI entity and the only additional command line
     * parameter should be --control-entity.
     *
     * @exceptions GenericGUIException with the type set to COMMAND_LINE_INVALID
     *             It must throw this if the command line does not contain the correct
     *             information for the GUI to run.
     */
    void checkCommandLine() throw(TA_Base_Core::TransactiveException);

    /**
     * entityChanged
     *
     * This will be called by GenericGUI when it receives a callback indicating that the
     * GUI entity has been modified. GenericGUI will have invalidated the entity database
     * object and will tell the GUI which items were modified. The GUI just has to retrieve
     * the details out of it and update accordingly.
     *
     * @param vector<string>& changes - This is a list of all attributes changed in the
     *                                  object.
     */
    void entityChanged(const std::vector<std::string>& changes);

    /**
     * serverIsDown
     *
     * This is called when the server the workstation is using goes down. The GUI should
     * indicate to the user that the server is down. It should also then limit operations
     * so that the user can only view existing data.
     */
    void serverIsDown();

    /**
     * serverIsUp
     *
     * This is called when the server the workstation is using starts functioning again.
     * The GUI should indicate to the user that the server is functioning. It should
     * then allow the user to perform all valid operations again.
     */
    void serverIsUp();

    /**
     * loadAllUserSettings
     *
     * This will load all the application's user preferences if configured to do so.
     * This should be called by the application itself in OnInit() or equivalent.
     *
     * @exceptions TODO:
     */
    void loadAllUserSettings();

    /**
     * saveAllUserSettings
     *
     * This will save all the application's user preferences if configured to do so.
     * This is provided in case the GUI needs to save user preferences at a predetermined
     * time. The settings are already saved when the application exits.
     *
     * @exceptions TODO:
     */
    void saveAllUserSettings();

    /**
     * changeFocus
     *
     * Code taken from STISManager.
     *
     * Implementing for IGUIApplication. The Control Station needs to alert the GUI
     * applications when they should regain the focus on the screen. This is the most
     * common focus request by the Control Station but it may also need to tell the
     * applications to minimise or maximise. This is done through this method. This takes
     * an enumeration indicating how the GUI should change it's focus. Changing the focus
     * of a window is the same for all window types so this can be implemented here.
     *
     * @param    EFocusType focus - This indicates how the window should change it's focus.
     */
    // TES #720
    //void changeFocus(const TA_Base_Core::EFocusType focus);
    // jeffrey++ TES640
    // changeFocus() is removed by TES720. It caused PA Manager always get focus (show up) when
    // selecting zone at schematic. Resuming it back and can't reproduce TES720 any more.
    //void changeFocus(const TA_Base_Core::EFocusType focus); //TES230 added implementation in setWindowPosition
    // ++jeffrey TES640

protected:

    /**
     * prepareForClose
     *
     * This method is used to prepare the GUI for closing. It should save the user settings
     * and perform any other tidying up. eg Stop any processing threads, ask the user if they
     * wish to save anything etc.
     */
    void prepareForClose();

    /**
     * createApplicationObject
     *
     * This method must set the m_application member variable. It should create a new
     * instance of the dialog/document/property sheet and assign it to m_application
     */
    virtual void createApplicationObject();

protected:

    //PAManagerDlg* m_PAManagerDlg;

public:

    CDialog* m_PAManagerDlg;

private:

    //
    // Hide the copy constructor and assignment operator as they
    // are not needed.
    //
    PAManagerGUI( const PAManagerGUI& thePAManagerGUI);
    PAManagerGUI& operator=(const PAManagerGUI&);

    bool m_commandLineInvalid;

    class NullDialog : public CDialog
    {
    public:

        // Do nothing when modal request made
        virtual int DoModal() {
            return IDOK;
        }

        virtual BOOL Create( UINT nIDTemplate, CWnd* pParentWnd = NULL )
        {
            return FALSE;
        };
    };
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAMANAGERGUI_H__4F99FEB2_FD1A_4478_B3C4_9942A9CAFFD1__INCLUDED_)
