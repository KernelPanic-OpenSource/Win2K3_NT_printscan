/******************************************************************************

  Header File:  Add Code Points.H

  This defines the CAddCodePoints class, which uses passed lists to present a 
  list of code points and their associated code pages for selection.  This
  class should be usable for both the glyph map and font structure editors.

  Copyright (c) 1997 by Microsoft Corporation.  All Rights Reserved.

  A Pretty Penny Enterprises Production

  Change History:
  03-01-1997    Bob_Kjelgaard@Prodigy.Net   Created it

******************************************************************************/

#if defined(LONG_NAMES)
#include    <Utility Classes.H>
#else
#include    <Utility.H>
#endif

class CAddCodePoints : public CDialog {

    CMapWordToDWord&    m_cmw2dPoints;
    CDWordArray&        m_cdaPages;
    CString             m_csItem;   //  Name of the item being edited
// Construction
public:
	CAddCodePoints(CWnd* pParent, CMapWordToDWord& cmw2d, CDWordArray& cda,
                   CString csItemName);

// Dialog Data
	//{{AFX_DATA(CAddCodePoints)
	enum { IDD = IDD_AddCodePoints };
	CListBox	m_clbList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddCodePoints)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAddCodePoints)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
