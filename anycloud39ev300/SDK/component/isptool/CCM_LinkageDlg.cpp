// CCM_LinkageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "CCM_LinkageDlg.h"
#include "anyka_types.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCCM_LinkageDlg dialog


CCCM_LinkageDlg::CCCM_LinkageDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCCM_LinkageDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCCM_LinkageDlg)
	//}}AFX_DATA_INIT
}


void CCCM_LinkageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCCM_LinkageDlg)
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_A_C11, m_slider_ccm_linkage_A_C11);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_A_C12, m_slider_ccm_linkage_A_C12);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_A_C13, m_slider_ccm_linkage_A_C13);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_A_C21, m_slider_ccm_linkage_A_C21);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_A_C22, m_slider_ccm_linkage_A_C22);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_A_C23, m_slider_ccm_linkage_A_C23);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_A_C31, m_slider_ccm_linkage_A_C31);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_A_C32, m_slider_ccm_linkage_A_C32);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_A_C33, m_slider_ccm_linkage_A_C33);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_D50_C11, m_slider_ccm_linkage_D50_C11);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_D50_C12, m_slider_ccm_linkage_D50_C12);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_D50_C13, m_slider_ccm_linkage_D50_C13);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_D50_C21, m_slider_ccm_linkage_D50_C21);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_D50_C22, m_slider_ccm_linkage_D50_C22);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_D50_C23, m_slider_ccm_linkage_D50_C23);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_D50_C31, m_slider_ccm_linkage_D50_C31);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_D50_C32, m_slider_ccm_linkage_D50_C32);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_D50_C33, m_slider_ccm_linkage_D50_C33);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_D65_C11, m_slider_ccm_linkage_D65_C11);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_D65_C12, m_slider_ccm_linkage_D65_C12);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_D65_C13, m_slider_ccm_linkage_D65_C13);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_D65_C21, m_slider_ccm_linkage_D65_C21);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_D65_C22, m_slider_ccm_linkage_D65_C22);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_D65_C23, m_slider_ccm_linkage_D65_C23);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_D65_C31, m_slider_ccm_linkage_D65_C31);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_D65_C32, m_slider_ccm_linkage_D65_C32);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_D65_C33, m_slider_ccm_linkage_D65_C33);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_TL84_C11, m_slider_ccm_linkage_TL84_C11);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_TL84_C12, m_slider_ccm_linkage_TL84_C12);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_TL84_C13, m_slider_ccm_linkage_TL84_C13);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_TL84_C21, m_slider_ccm_linkage_TL84_C21);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_TL84_C22, m_slider_ccm_linkage_TL84_C22);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_TL84_C23, m_slider_ccm_linkage_TL84_C23);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_TL84_C31, m_slider_ccm_linkage_TL84_C31);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_TL84_C32, m_slider_ccm_linkage_TL84_C32);
	DDX_Control(pDX, IDC_SLIDER_CCM_LINKAGE_TL84_C33, m_slider_ccm_linkage_TL84_C33);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_A_C11, m_spin_ccm_linkage_A_C11);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_A_C12, m_spin_ccm_linkage_A_C12);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_A_C13, m_spin_ccm_linkage_A_C13);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_A_C21, m_spin_ccm_linkage_A_C21);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_A_C22, m_spin_ccm_linkage_A_C22);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_A_C23, m_spin_ccm_linkage_A_C23);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_A_C31, m_spin_ccm_linkage_A_C31);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_A_C32, m_spin_ccm_linkage_A_C32);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_A_C33, m_spin_ccm_linkage_A_C33);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_D50_C11, m_spin_ccm_linkage_D50_C11);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_D50_C12, m_spin_ccm_linkage_D50_C12);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_D50_C13, m_spin_ccm_linkage_D50_C13);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_D50_C21, m_spin_ccm_linkage_D50_C21);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_D50_C22, m_spin_ccm_linkage_D50_C22);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_D50_C23, m_spin_ccm_linkage_D50_C23);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_D50_C31, m_spin_ccm_linkage_D50_C31);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_D50_C32, m_spin_ccm_linkage_D50_C32);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_D50_C33, m_spin_ccm_linkage_D50_C33);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_D65_C11, m_spin_ccm_linkage_D65_C11);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_D65_C12, m_spin_ccm_linkage_D65_C12);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_D65_C13, m_spin_ccm_linkage_D65_C13);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_D65_C21, m_spin_ccm_linkage_D65_C21);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_D65_C22, m_spin_ccm_linkage_D65_C22);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_D65_C23, m_spin_ccm_linkage_D65_C23);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_D65_C31, m_spin_ccm_linkage_D65_C31);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_D65_C32, m_spin_ccm_linkage_D65_C32);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_D65_C33, m_spin_ccm_linkage_D65_C33);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_TL84_C11, m_spin_ccm_linkage_TL84_C11);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_TL84_C12, m_spin_ccm_linkage_TL84_C12);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_TL84_C13, m_spin_ccm_linkage_TL84_C13);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_TL84_C21, m_spin_ccm_linkage_TL84_C21);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_TL84_C22, m_spin_ccm_linkage_TL84_C22);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_TL84_C23, m_spin_ccm_linkage_TL84_C23);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_TL84_C31, m_spin_ccm_linkage_TL84_C31);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_TL84_C32, m_spin_ccm_linkage_TL84_C32);
	DDX_Control(pDX, IDC_SPIN_CCM_LINKAGE_TL84_C33, m_spin_ccm_linkage_TL84_C33);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCCM_LinkageDlg, CDialog)
	//{{AFX_MSG_MAP(CCCM_LinkageDlg)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_A_C11, OnKillfocusEditCcmLinkageAC11)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_A_C12, OnKillfocusEditCcmLinkageAC12)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_A_C13, OnKillfocusEditCcmLinkageAC13)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_A_C21, OnKillfocusEditCcmLinkageAC21)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_A_C22, OnKillfocusEditCcmLinkageAC22)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_A_C23, OnKillfocusEditCcmLinkageAC23)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_A_C31, OnKillfocusEditCcmLinkageAC31)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_A_C32, OnKillfocusEditCcmLinkageAC32)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_A_C33, OnKillfocusEditCcmLinkageAC33)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_D50_C11, OnKillfocusEditCcmLinkageD50C11)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_D50_C12, OnKillfocusEditCcmLinkageD50C12)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_D50_C13, OnKillfocusEditCcmLinkageD50C13)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_D50_C21, OnKillfocusEditCcmLinkageD50C21)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_D50_C22, OnKillfocusEditCcmLinkageD50C22)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_D50_C23, OnKillfocusEditCcmLinkageD50C23)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_D50_C31, OnKillfocusEditCcmLinkageD50C31)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_D50_C32, OnKillfocusEditCcmLinkageD50C32)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_D50_C33, OnKillfocusEditCcmLinkageD50C33)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_D65_C11, OnKillfocusEditCcmLinkageD65C11)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_D65_C12, OnKillfocusEditCcmLinkageD65C12)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_D65_C13, OnKillfocusEditCcmLinkageD65C13)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_D65_C21, OnKillfocusEditCcmLinkageD65C21)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_D65_C22, OnKillfocusEditCcmLinkageD65C22)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_D65_C23, OnKillfocusEditCcmLinkageD65C23)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_D65_C31, OnKillfocusEditCcmLinkageD65C31)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_D65_C32, OnKillfocusEditCcmLinkageD65C32)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_D65_C33, OnKillfocusEditCcmLinkageD65C33)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_TL84_C11, OnKillfocusEditCcmLinkageTl84C11)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_TL84_C12, OnKillfocusEditCcmLinkageTl84C12)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_TL84_C13, OnKillfocusEditCcmLinkageTl84C13)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_TL84_C21, OnKillfocusEditCcmLinkageTl84C21)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_TL84_C22, OnKillfocusEditCcmLinkageTl84C22)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_TL84_C23, OnKillfocusEditCcmLinkageTl84C23)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_TL84_C31, OnKillfocusEditCcmLinkageTl84C31)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_TL84_C32, OnKillfocusEditCcmLinkageTl84C32)
	ON_EN_KILLFOCUS(IDC_EDIT_CCM_LINKAGE_TL84_C33, OnKillfocusEditCcmLinkageTl84C33)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_A_C11, OnCustomdrawSliderCcmLinkageAC11)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_A_C12, OnCustomdrawSliderCcmLinkageAC12)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_A_C13, OnCustomdrawSliderCcmLinkageAC13)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_A_C21, OnCustomdrawSliderCcmLinkageAC21)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_A_C22, OnCustomdrawSliderCcmLinkageAC22)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_A_C23, OnCustomdrawSliderCcmLinkageAC23)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_A_C31, OnCustomdrawSliderCcmLinkageAC31)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_A_C32, OnCustomdrawSliderCcmLinkageAC32)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_A_C33, OnCustomdrawSliderCcmLinkageAC33)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_D50_C11, OnCustomdrawSliderCcmLinkageD50C11)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_D50_C12, OnCustomdrawSliderCcmLinkageD50C12)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_D50_C13, OnCustomdrawSliderCcmLinkageD50C13)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_D50_C21, OnCustomdrawSliderCcmLinkageD50C21)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_D50_C22, OnCustomdrawSliderCcmLinkageD50C22)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_D50_C23, OnCustomdrawSliderCcmLinkageD50C23)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_D50_C31, OnCustomdrawSliderCcmLinkageD50C31)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_D50_C32, OnCustomdrawSliderCcmLinkageD50C32)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_D50_C33, OnCustomdrawSliderCcmLinkageD50C33)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_D65_C11, OnCustomdrawSliderCcmLinkageD65C11)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_D65_C12, OnCustomdrawSliderCcmLinkageD65C12)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_D65_C13, OnCustomdrawSliderCcmLinkageD65C13)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_D65_C21, OnCustomdrawSliderCcmLinkageD65C21)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_D65_C22, OnCustomdrawSliderCcmLinkageD65C22)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_D65_C23, OnCustomdrawSliderCcmLinkageD65C23)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_D65_C31, OnCustomdrawSliderCcmLinkageD65C31)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_D65_C32, OnCustomdrawSliderCcmLinkageD65C32)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_D65_C33, OnCustomdrawSliderCcmLinkageD65C33)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_TL84_C11, OnCustomdrawSliderCcmLinkageTl84C11)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_TL84_C12, OnCustomdrawSliderCcmLinkageTl84C12)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_TL84_C13, OnCustomdrawSliderCcmLinkageTl84C13)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_TL84_C21, OnCustomdrawSliderCcmLinkageTl84C21)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_TL84_C22, OnCustomdrawSliderCcmLinkageTl84C22)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_TL84_C23, OnCustomdrawSliderCcmLinkageTl84C23)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_TL84_C31, OnCustomdrawSliderCcmLinkageTl84C31)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_TL84_C32, OnCustomdrawSliderCcmLinkageTl84C32)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CCM_LINKAGE_TL84_C33, OnCustomdrawSliderCcmLinkageTl84C33)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_A_C11, OnDeltaposSpinCcmLinkageAC11)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_A_C12, OnDeltaposSpinCcmLinkageAC12)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_A_C13, OnDeltaposSpinCcmLinkageAC13)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_A_C21, OnDeltaposSpinCcmLinkageAC21)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_A_C22, OnDeltaposSpinCcmLinkageAC22)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_A_C23, OnDeltaposSpinCcmLinkageAC23)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_A_C31, OnDeltaposSpinCcmLinkageAC31)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_A_C32, OnDeltaposSpinCcmLinkageAC32)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_A_C33, OnDeltaposSpinCcmLinkageAC33)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_D50_C11, OnDeltaposSpinCcmLinkageD50C11)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_D50_C12, OnDeltaposSpinCcmLinkageD50C12)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_D50_C13, OnDeltaposSpinCcmLinkageD50C13)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_D50_C21, OnDeltaposSpinCcmLinkageD50C21)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_D50_C22, OnDeltaposSpinCcmLinkageD50C22)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_D50_C23, OnDeltaposSpinCcmLinkageD50C23)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_D50_C31, OnDeltaposSpinCcmLinkageD50C31)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_D50_C32, OnDeltaposSpinCcmLinkageD50C32)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_D50_C33, OnDeltaposSpinCcmLinkageD50C33)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_D65_C11, OnDeltaposSpinCcmLinkageD65C11)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_D65_C12, OnDeltaposSpinCcmLinkageD65C12)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_D65_C13, OnDeltaposSpinCcmLinkageD65C13)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_D65_C21, OnDeltaposSpinCcmLinkageD65C21)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_D65_C22, OnDeltaposSpinCcmLinkageD65C22)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_D65_C23, OnDeltaposSpinCcmLinkageD65C23)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_D65_C31, OnDeltaposSpinCcmLinkageD65C31)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_D65_C32, OnDeltaposSpinCcmLinkageD65C32)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_D65_C33, OnDeltaposSpinCcmLinkageD65C33)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_TL84_C11, OnDeltaposSpinCcmLinkageTl84C11)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_TL84_C12, OnDeltaposSpinCcmLinkageTl84C12)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_TL84_C13, OnDeltaposSpinCcmLinkageTl84C13)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_TL84_C21, OnDeltaposSpinCcmLinkageTl84C21)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_TL84_C22, OnDeltaposSpinCcmLinkageTl84C22)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_TL84_C23, OnDeltaposSpinCcmLinkageTl84C23)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_TL84_C31, OnDeltaposSpinCcmLinkageTl84C31)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_TL84_C32, OnDeltaposSpinCcmLinkageTl84C32)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CCM_LINKAGE_TL84_C33, OnDeltaposSpinCcmLinkageTl84C33)
	ON_BN_CLICKED(IDC_RADIO_CCM_LINKAGE_A_DISABLE, OnRadioCcmLinkageADisable)
	ON_BN_CLICKED(IDC_RADIO_CCM_LINKAGE_A_ENABLE, OnRadioCcmLinkageAEnable)
	ON_BN_CLICKED(IDC_RADIO_CCM_LINKAGE_D50_DISABLE3, OnRadioCcmLinkageD50Disable3)
	ON_BN_CLICKED(IDC_RADIO_CCM_LINKAGE_D50_ENABLE3, OnRadioCcmLinkageD50Enable3)
	ON_BN_CLICKED(IDC_RADIO_CCM_LINKAGE_D65_DISABLE4, OnRadioCcmLinkageD65Disable4)
	ON_BN_CLICKED(IDC_RADIO_CCM_LINKAGE_D65_ENABLE4, OnRadioCcmLinkageD65Enable4)
	ON_BN_CLICKED(IDC_RADIO_CCM_LINKAGE_TL84_DISABLE2, OnRadioCcmLinkageTl84Disable2)
	ON_BN_CLICKED(IDC_RADIO_CCM_LINKAGE_TL84_ENABLE2, OnRadioCcmLinkageTl84Enable2)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCCM_LinkageDlg message handlers

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageAC11() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_A_C11.SetPos((T_S16)m_spin_ccm_linkage_A_C11.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageAC12() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_A_C12.SetPos((T_S16)m_spin_ccm_linkage_A_C12.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageAC13() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_A_C13.SetPos((T_S16)m_spin_ccm_linkage_A_C13.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageAC21() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_A_C21.SetPos((T_S16)m_spin_ccm_linkage_A_C21.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageAC22() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_A_C22.SetPos((T_S16)m_spin_ccm_linkage_A_C22.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageAC23() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_A_C23.SetPos((T_S16)m_spin_ccm_linkage_A_C23.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageAC31() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_A_C31.SetPos((T_S16)m_spin_ccm_linkage_A_C31.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageAC32() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_A_C32.SetPos((T_S16)m_spin_ccm_linkage_A_C32.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageAC33() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_A_C33.SetPos((T_S16)m_spin_ccm_linkage_A_C33.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageD50C11() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D50_C11.SetPos((T_S16)m_spin_ccm_linkage_D50_C11.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageD50C12() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D50_C12.SetPos((T_S16)m_spin_ccm_linkage_D50_C12.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageD50C13() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D50_C13.SetPos((T_S16)m_spin_ccm_linkage_D50_C13.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageD50C21() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D50_C21.SetPos((T_S16)m_spin_ccm_linkage_D50_C21.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageD50C22() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D50_C22.SetPos((T_S16)m_spin_ccm_linkage_D50_C22.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageD50C23() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D50_C23.SetPos((T_S16)m_spin_ccm_linkage_D50_C23.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageD50C31() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D50_C31.SetPos((T_S16)m_spin_ccm_linkage_D50_C31.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageD50C32() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D50_C32.SetPos((T_S16)m_spin_ccm_linkage_D50_C32.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageD50C33() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D50_C33.SetPos((T_S16)m_spin_ccm_linkage_D50_C33.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageD65C11() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D65_C11.SetPos((T_S16)m_spin_ccm_linkage_D65_C11.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageD65C12() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D65_C12.SetPos((T_S16)m_spin_ccm_linkage_D65_C12.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageD65C13() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D65_C13.SetPos((T_S16)m_spin_ccm_linkage_D65_C13.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageD65C21() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D65_C21.SetPos((T_S16)m_spin_ccm_linkage_D65_C21.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageD65C22() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D65_C22.SetPos((T_S16)m_spin_ccm_linkage_D65_C22.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageD65C23() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D65_C23.SetPos((T_S16)m_spin_ccm_linkage_D65_C23.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageD65C31() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D65_C31.SetPos((T_S16)m_spin_ccm_linkage_D65_C31.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageD65C32() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D65_C32.SetPos((T_S16)m_spin_ccm_linkage_D65_C32.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageD65C33() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D65_C33.SetPos((T_S16)m_spin_ccm_linkage_D65_C33.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageTl84C11() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_TL84_C11.SetPos((T_S16)m_spin_ccm_linkage_TL84_C11.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageTl84C12() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_TL84_C12.SetPos((T_S16)m_spin_ccm_linkage_TL84_C12.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageTl84C13() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_TL84_C13.SetPos((T_S16)m_spin_ccm_linkage_TL84_C13.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageTl84C21() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_TL84_C21.SetPos((T_S16)m_spin_ccm_linkage_TL84_C21.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageTl84C22() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_TL84_C22.SetPos((T_S16)m_spin_ccm_linkage_TL84_C22.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageTl84C23() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_TL84_C23.SetPos((T_S16)m_spin_ccm_linkage_TL84_C23.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageTl84C31() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_TL84_C31.SetPos((T_S16)m_spin_ccm_linkage_TL84_C31.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageTl84C32() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_TL84_C32.SetPos((T_S16)m_spin_ccm_linkage_TL84_C32.GetPos());
}

void CCCM_LinkageDlg::OnKillfocusEditCcmLinkageTl84C33() 
{
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_TL84_C33.SetPos((T_S16)m_spin_ccm_linkage_TL84_C33.GetPos());
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageAC11(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_A_C11.SetPos((T_S16)m_slider_ccm_linkage_A_C11.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageAC12(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_A_C12.SetPos((T_S16)m_slider_ccm_linkage_A_C12.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageAC13(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_A_C13.SetPos((T_S16)m_slider_ccm_linkage_A_C13.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageAC21(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_A_C21.SetPos((T_S16)m_slider_ccm_linkage_A_C21.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageAC22(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_A_C22.SetPos((T_S16)m_slider_ccm_linkage_A_C22.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageAC23(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_A_C23.SetPos((T_S16)m_slider_ccm_linkage_A_C23.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageAC31(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_A_C31.SetPos((T_S16)m_slider_ccm_linkage_A_C31.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageAC32(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_A_C32.SetPos((T_S16)m_slider_ccm_linkage_A_C32.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageAC33(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_A_C33.SetPos((T_S16)m_slider_ccm_linkage_A_C33.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageD50C11(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_D50_C11.SetPos((T_S16)m_slider_ccm_linkage_D50_C11.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageD50C12(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_D50_C12.SetPos((T_S16)m_slider_ccm_linkage_D50_C12.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageD50C13(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_D50_C13.SetPos((T_S16)m_slider_ccm_linkage_D50_C13.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageD50C21(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_D50_C21.SetPos((T_S16)m_slider_ccm_linkage_D50_C21.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageD50C22(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_D50_C22.SetPos((T_S16)m_slider_ccm_linkage_D50_C22.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageD50C23(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_D50_C23.SetPos((T_S16)m_slider_ccm_linkage_D50_C23.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageD50C31(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_D50_C31.SetPos((T_S16)m_slider_ccm_linkage_D50_C31.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageD50C32(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_D50_C32.SetPos((T_S16)m_slider_ccm_linkage_D50_C32.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageD50C33(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_D50_C33.SetPos((T_S16)m_slider_ccm_linkage_D50_C33.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageD65C11(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_D65_C11.SetPos((T_S16)m_slider_ccm_linkage_D65_C11.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageD65C12(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_D65_C12.SetPos((T_S16)m_slider_ccm_linkage_D65_C12.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageD65C13(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_D65_C13.SetPos((T_S16)m_slider_ccm_linkage_D65_C13.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageD65C21(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_D65_C21.SetPos((T_S16)m_slider_ccm_linkage_D65_C21.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageD65C22(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_D65_C22.SetPos((T_S16)m_slider_ccm_linkage_D65_C22.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageD65C23(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_D65_C23.SetPos((T_S16)m_slider_ccm_linkage_D65_C23.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageD65C31(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_D65_C31.SetPos((T_S16)m_slider_ccm_linkage_D65_C31.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageD65C32(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_D65_C32.SetPos((T_S16)m_slider_ccm_linkage_D65_C32.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageD65C33(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_D65_C33.SetPos((T_S16)m_slider_ccm_linkage_D65_C33.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageTl84C11(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_TL84_C11.SetPos((T_S16)m_slider_ccm_linkage_TL84_C11.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageTl84C12(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_TL84_C12.SetPos((T_S16)m_slider_ccm_linkage_TL84_C12.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageTl84C13(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_TL84_C13.SetPos((T_S16)m_slider_ccm_linkage_TL84_C13.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageTl84C21(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_TL84_C21.SetPos((T_S16)m_slider_ccm_linkage_TL84_C21.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageTl84C22(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_TL84_C22.SetPos((T_S16)m_slider_ccm_linkage_TL84_C22.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageTl84C23(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_TL84_C23.SetPos((T_S16)m_slider_ccm_linkage_TL84_C23.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageTl84C31(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_TL84_C31.SetPos((T_S16)m_slider_ccm_linkage_TL84_C31.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageTl84C32(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_TL84_C32.SetPos((T_S16)m_slider_ccm_linkage_TL84_C32.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCustomdrawSliderCcmLinkageTl84C33(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_ccm_linkage_TL84_C33.SetPos((T_S16)m_slider_ccm_linkage_TL84_C33.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageAC11(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_A_C11.SetPos((T_S16)m_spin_ccm_linkage_A_C11.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageAC12(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_A_C12.SetPos((T_S16)m_spin_ccm_linkage_A_C12.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageAC13(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_A_C13.SetPos((T_S16)m_spin_ccm_linkage_A_C13.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageAC21(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_A_C21.SetPos((T_S16)m_spin_ccm_linkage_A_C21.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageAC22(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_A_C22.SetPos((T_S16)m_spin_ccm_linkage_A_C22.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageAC23(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_A_C23.SetPos((T_S16)m_spin_ccm_linkage_A_C23.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageAC31(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_A_C31.SetPos((T_S16)m_spin_ccm_linkage_A_C31.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageAC32(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_A_C32.SetPos((T_S16)m_spin_ccm_linkage_A_C32.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageAC33(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_A_C33.SetPos((T_S16)m_spin_ccm_linkage_A_C33.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageD50C11(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D50_C11.SetPos((T_S16)m_spin_ccm_linkage_D50_C11.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageD50C12(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D50_C12.SetPos((T_S16)m_spin_ccm_linkage_D50_C12.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageD50C13(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D50_C13.SetPos((T_S16)m_spin_ccm_linkage_D50_C13.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageD50C21(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D50_C21.SetPos((T_S16)m_spin_ccm_linkage_D50_C21.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageD50C22(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D50_C22.SetPos((T_S16)m_spin_ccm_linkage_D50_C22.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageD50C23(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D50_C23.SetPos((T_S16)m_spin_ccm_linkage_D50_C23.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageD50C31(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D50_C31.SetPos((T_S16)m_spin_ccm_linkage_D50_C31.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageD50C32(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D50_C32.SetPos((T_S16)m_spin_ccm_linkage_D50_C32.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageD50C33(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D50_C33.SetPos((T_S16)m_spin_ccm_linkage_D50_C33.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageD65C11(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D65_C11.SetPos((T_S16)m_spin_ccm_linkage_D65_C11.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageD65C12(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D65_C12.SetPos((T_S16)m_spin_ccm_linkage_D65_C12.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageD65C13(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D65_C13.SetPos((T_S16)m_spin_ccm_linkage_D65_C13.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageD65C21(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D65_C21.SetPos((T_S16)m_spin_ccm_linkage_D65_C21.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageD65C22(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D65_C22.SetPos((T_S16)m_spin_ccm_linkage_D65_C22.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageD65C23(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D65_C23.SetPos((T_S16)m_spin_ccm_linkage_D65_C23.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageD65C31(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D65_C31.SetPos((T_S16)m_spin_ccm_linkage_D65_C31.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageD65C32(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D65_C32.SetPos((T_S16)m_spin_ccm_linkage_D65_C32.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageD65C33(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_D65_C33.SetPos((T_S16)m_spin_ccm_linkage_D65_C33.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageTl84C11(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_TL84_C11.SetPos((T_S16)m_spin_ccm_linkage_TL84_C11.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageTl84C12(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_TL84_C12.SetPos((T_S16)m_spin_ccm_linkage_TL84_C12.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageTl84C13(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_TL84_C13.SetPos((T_S16)m_spin_ccm_linkage_TL84_C13.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageTl84C21(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_TL84_C21.SetPos((T_S16)m_spin_ccm_linkage_TL84_C21.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageTl84C22(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_TL84_C22.SetPos((T_S16)m_spin_ccm_linkage_TL84_C22.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageTl84C23(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_TL84_C23.SetPos((T_S16)m_spin_ccm_linkage_TL84_C23.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageTl84C31(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_TL84_C31.SetPos((T_S16)m_spin_ccm_linkage_TL84_C31.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageTl84C32(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_TL84_C32.SetPos((T_S16)m_spin_ccm_linkage_TL84_C32.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnDeltaposSpinCcmLinkageTl84C33(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_ccm_linkage_TL84_C33.SetPos((T_S16)m_spin_ccm_linkage_TL84_C33.GetPos());
	*pResult = 0;
}

void CCCM_LinkageDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CCCM_LinkageDlg::OnOK() 
{
	// TODO: Add extra validation here
	GetDataValue();

	CDialog::OnOK();
}

void CCCM_LinkageDlg::OnRadioCcmLinkageADisable() 
{
	// TODO: Add your control notification handler code here
	
}

void CCCM_LinkageDlg::OnRadioCcmLinkageAEnable() 
{
	// TODO: Add your control notification handler code here
	
}

void CCCM_LinkageDlg::OnRadioCcmLinkageD50Disable3() 
{
	// TODO: Add your control notification handler code here
	
}

void CCCM_LinkageDlg::OnRadioCcmLinkageD50Enable3() 
{
	// TODO: Add your control notification handler code here
	
}

void CCCM_LinkageDlg::OnRadioCcmLinkageD65Disable4() 
{
	// TODO: Add your control notification handler code here
	
}

void CCCM_LinkageDlg::OnRadioCcmLinkageD65Enable4() 
{
	// TODO: Add your control notification handler code here
	
}

void CCCM_LinkageDlg::OnRadioCcmLinkageTl84Disable2() 
{
	// TODO: Add your control notification handler code here
	
}

void CCCM_LinkageDlg::OnRadioCcmLinkageTl84Enable2() 
{
	// TODO: Add your control notification handler code here
	
}

BOOL CCCM_LinkageDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
    //A
	m_spin_ccm_linkage_A_C11.SetRange(-2048,2047);
	m_spin_ccm_linkage_A_C11.SetPos(-2048);
	m_slider_ccm_linkage_A_C11.SetRange(-2048,2047);
	m_slider_ccm_linkage_A_C11.SetPos(-2048);
	
	m_spin_ccm_linkage_A_C12.SetRange(-2048,2047);
	m_spin_ccm_linkage_A_C12.SetPos(-2048);
	m_slider_ccm_linkage_A_C12.SetRange(-2048,2047);
	m_slider_ccm_linkage_A_C12.SetPos(-2048);
	
	m_spin_ccm_linkage_A_C13.SetRange(-2048,2047);
	m_spin_ccm_linkage_A_C13.SetPos(-2048);
	m_slider_ccm_linkage_A_C13.SetRange(-2048,2047);
	m_slider_ccm_linkage_A_C13.SetPos(-2048);
	
	m_spin_ccm_linkage_A_C21.SetRange(-2048,2047);
	m_spin_ccm_linkage_A_C21.SetPos(-2048);
	m_slider_ccm_linkage_A_C21.SetRange(-2048,2047);
	m_slider_ccm_linkage_A_C21.SetPos(-2048);
	
	m_spin_ccm_linkage_A_C22.SetRange(-2048,2047);
	m_spin_ccm_linkage_A_C22.SetPos(-2048);
	m_slider_ccm_linkage_A_C22.SetRange(-2048,2047);
	m_slider_ccm_linkage_A_C22.SetPos(-2048);
	
	m_spin_ccm_linkage_A_C23.SetRange(-2048,2047);
	m_spin_ccm_linkage_A_C23.SetPos(-2048);
	m_slider_ccm_linkage_A_C23.SetRange(-2048,2047);
	m_slider_ccm_linkage_A_C23.SetPos(-2048);
	
	m_spin_ccm_linkage_A_C31.SetRange(-2048,2047);
	m_spin_ccm_linkage_A_C31.SetPos(-2048);
	m_slider_ccm_linkage_A_C31.SetRange(-2048,2047);
	m_slider_ccm_linkage_A_C31.SetPos(-2048);
	
	m_spin_ccm_linkage_A_C32.SetRange(-2048,2047);
	m_spin_ccm_linkage_A_C32.SetPos(-2048);
	m_slider_ccm_linkage_A_C32.SetRange(-2048,2047);
	m_slider_ccm_linkage_A_C32.SetPos(-2048);
	
	m_spin_ccm_linkage_A_C33.SetRange(-2048,2047);
	m_spin_ccm_linkage_A_C33.SetPos(-2048);
	m_slider_ccm_linkage_A_C33.SetRange(-2048,2047);
	m_slider_ccm_linkage_A_C33.SetPos(-2048);

    //tl84
	m_spin_ccm_linkage_TL84_C11.SetRange(-2048,2047);
	m_spin_ccm_linkage_TL84_C11.SetPos(-2048);
	m_slider_ccm_linkage_TL84_C11.SetRange(-2048,2047);
	m_slider_ccm_linkage_TL84_C11.SetPos(-2048);
	
	m_spin_ccm_linkage_TL84_C12.SetRange(-2048,2047);
	m_spin_ccm_linkage_TL84_C12.SetPos(-2048);
	m_slider_ccm_linkage_TL84_C12.SetRange(-2048,2047);
	m_slider_ccm_linkage_TL84_C12.SetPos(-2048);
	
	m_spin_ccm_linkage_TL84_C13.SetRange(-2048,2047);
	m_spin_ccm_linkage_TL84_C13.SetPos(-2048);
	m_slider_ccm_linkage_TL84_C13.SetRange(-2048,2047);
	m_slider_ccm_linkage_TL84_C13.SetPos(-2048);
	
	m_spin_ccm_linkage_TL84_C21.SetRange(-2048,2047);
	m_spin_ccm_linkage_TL84_C21.SetPos(-2048);
	m_slider_ccm_linkage_TL84_C21.SetRange(-2048,2047);
	m_slider_ccm_linkage_TL84_C21.SetPos(-2048);
	
	m_spin_ccm_linkage_TL84_C22.SetRange(-2048,2047);
	m_spin_ccm_linkage_TL84_C22.SetPos(-2048);
	m_slider_ccm_linkage_TL84_C22.SetRange(-2048,2047);
	m_slider_ccm_linkage_TL84_C22.SetPos(-2048);
	
	m_spin_ccm_linkage_TL84_C23.SetRange(-2048,2047);
	m_spin_ccm_linkage_TL84_C23.SetPos(-2048);
	m_slider_ccm_linkage_TL84_C23.SetRange(-2048,2047);
	m_slider_ccm_linkage_TL84_C23.SetPos(-2048);
	
	m_spin_ccm_linkage_TL84_C31.SetRange(-2048,2047);
	m_spin_ccm_linkage_TL84_C31.SetPos(-2048);
	m_slider_ccm_linkage_TL84_C31.SetRange(-2048,2047);
	m_slider_ccm_linkage_TL84_C31.SetPos(-2048);
	
	m_spin_ccm_linkage_TL84_C32.SetRange(-2048,2047);
	m_spin_ccm_linkage_TL84_C32.SetPos(-2048);
	m_slider_ccm_linkage_TL84_C32.SetRange(-2048,2047);
	m_slider_ccm_linkage_TL84_C32.SetPos(-2048);
	
	m_spin_ccm_linkage_TL84_C33.SetRange(-2048,2047);
	m_spin_ccm_linkage_TL84_C33.SetPos(-2048);
	m_slider_ccm_linkage_TL84_C33.SetRange(-2048,2047);
	m_slider_ccm_linkage_TL84_C33.SetPos(-2048);

	//d50
	m_spin_ccm_linkage_D50_C11.SetRange(-2048,2047);
	m_spin_ccm_linkage_D50_C11.SetPos(-2048);
	m_slider_ccm_linkage_D50_C11.SetRange(-2048,2047);
	m_slider_ccm_linkage_D50_C11.SetPos(-2048);
	
	m_spin_ccm_linkage_D50_C12.SetRange(-2048,2047);
	m_spin_ccm_linkage_D50_C12.SetPos(-2048);
	m_slider_ccm_linkage_D50_C12.SetRange(-2048,2047);
	m_slider_ccm_linkage_D50_C12.SetPos(-2048);
	
	m_spin_ccm_linkage_D50_C13.SetRange(-2048,2047);
	m_spin_ccm_linkage_D50_C13.SetPos(-2048);
	m_slider_ccm_linkage_D50_C13.SetRange(-2048,2047);
	m_slider_ccm_linkage_D50_C13.SetPos(-2048);
	
	m_spin_ccm_linkage_D50_C21.SetRange(-2048,2047);
	m_spin_ccm_linkage_D50_C21.SetPos(-2048);
	m_slider_ccm_linkage_D50_C21.SetRange(-2048,2047);
	m_slider_ccm_linkage_D50_C21.SetPos(-2048);
	
	m_spin_ccm_linkage_D50_C22.SetRange(-2048,2047);
	m_spin_ccm_linkage_D50_C22.SetPos(-2048);
	m_slider_ccm_linkage_D50_C22.SetRange(-2048,2047);
	m_slider_ccm_linkage_D50_C22.SetPos(-2048);
	
	m_spin_ccm_linkage_D50_C23.SetRange(-2048,2047);
	m_spin_ccm_linkage_D50_C23.SetPos(-2048);
	m_slider_ccm_linkage_D50_C23.SetRange(-2048,2047);
	m_slider_ccm_linkage_D50_C23.SetPos(-2048);
	
	m_spin_ccm_linkage_D50_C31.SetRange(-2048,2047);
	m_spin_ccm_linkage_D50_C31.SetPos(-2048);
	m_slider_ccm_linkage_D50_C31.SetRange(-2048,2047);
	m_slider_ccm_linkage_D50_C31.SetPos(-2048);
	
	m_spin_ccm_linkage_D50_C32.SetRange(-2048,2047);
	m_spin_ccm_linkage_D50_C32.SetPos(-2048);
	m_slider_ccm_linkage_D50_C32.SetRange(-2048,2047);
	m_slider_ccm_linkage_D50_C32.SetPos(-2048);
	
	m_spin_ccm_linkage_D50_C33.SetRange(-2048,2047);
	m_spin_ccm_linkage_D50_C33.SetPos(-2048);
	m_slider_ccm_linkage_D50_C33.SetRange(-2048,2047);
	m_slider_ccm_linkage_D50_C33.SetPos(-2048);

	//D65
	m_spin_ccm_linkage_D65_C11.SetRange(-2048,2047);
	m_spin_ccm_linkage_D65_C11.SetPos(-2048);
	m_slider_ccm_linkage_D65_C11.SetRange(-2048,2047);
	m_slider_ccm_linkage_D65_C11.SetPos(-2048);
	
	m_spin_ccm_linkage_D65_C12.SetRange(-2048,2047);
	m_spin_ccm_linkage_D65_C12.SetPos(-2048);
	m_slider_ccm_linkage_D65_C12.SetRange(-2048,2047);
	m_slider_ccm_linkage_D65_C12.SetPos(-2048);
	
	m_spin_ccm_linkage_D65_C13.SetRange(-2048,2047);
	m_spin_ccm_linkage_D65_C13.SetPos(-2048);
	m_slider_ccm_linkage_D65_C13.SetRange(-2048,2047);
	m_slider_ccm_linkage_D65_C13.SetPos(-2048);
	
	m_spin_ccm_linkage_D65_C21.SetRange(-2048,2047);
	m_spin_ccm_linkage_D65_C21.SetPos(-2048);
	m_slider_ccm_linkage_D65_C21.SetRange(-2048,2047);
	m_slider_ccm_linkage_D65_C21.SetPos(-2048);
	
	m_spin_ccm_linkage_D65_C22.SetRange(-2048,2047);
	m_spin_ccm_linkage_D65_C22.SetPos(-2048);
	m_slider_ccm_linkage_D65_C22.SetRange(-2048,2047);
	m_slider_ccm_linkage_D65_C22.SetPos(-2048);
	
	m_spin_ccm_linkage_D65_C23.SetRange(-2048,2047);
	m_spin_ccm_linkage_D65_C23.SetPos(-2048);
	m_slider_ccm_linkage_D65_C23.SetRange(-2048,2047);
	m_slider_ccm_linkage_D65_C23.SetPos(-2048);
	
	m_spin_ccm_linkage_D65_C31.SetRange(-2048,2047);
	m_spin_ccm_linkage_D65_C31.SetPos(-2048);
	m_slider_ccm_linkage_D65_C31.SetRange(-2048,2047);
	m_slider_ccm_linkage_D65_C31.SetPos(-2048);
	
	m_spin_ccm_linkage_D65_C32.SetRange(-2048,2047);
	m_spin_ccm_linkage_D65_C32.SetPos(-2048);
	m_slider_ccm_linkage_D65_C32.SetRange(-2048,2047);
	m_slider_ccm_linkage_D65_C32.SetPos(-2048);
	
	m_spin_ccm_linkage_D65_C33.SetRange(-2048,2047);
	m_spin_ccm_linkage_D65_C33.SetPos(-2048);
	m_slider_ccm_linkage_D65_C33.SetRange(-2048,2047);
	m_slider_ccm_linkage_D65_C33.SetPos(-2048);


	SetDataValue();

	m_timer = SetTimer(1, 500, NULL);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL CCCM_LinkageDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}


void CCCM_LinkageDlg::GetDataValue(void)
{
	BOOL check = FALSE;
	
	
	//A
	check = ((CButton *)GetDlgItem(IDC_RADIO_CCM_LINKAGE_A_ENABLE))->GetCheck();
	m_Isp_ccm_linkage[0].cc_enable = check;
	m_Isp_ccm_linkage[0].ccm[0][0]  = (T_S16)m_spin_ccm_linkage_A_C11.GetPos();
	m_Isp_ccm_linkage[0].ccm[0][1]  = (T_S16)m_spin_ccm_linkage_A_C12.GetPos();
	m_Isp_ccm_linkage[0].ccm[0][2]  = (T_S16)m_spin_ccm_linkage_A_C13.GetPos();
	
	m_Isp_ccm_linkage[0].ccm[1][0]  = (T_S16)m_spin_ccm_linkage_A_C21.GetPos();
	m_Isp_ccm_linkage[0].ccm[1][1]  = (T_S16)m_spin_ccm_linkage_A_C22.GetPos();
	m_Isp_ccm_linkage[0].ccm[1][2]  = (T_S16)m_spin_ccm_linkage_A_C23.GetPos();
	
	m_Isp_ccm_linkage[0].ccm[2][0]  = (T_S16)m_spin_ccm_linkage_A_C31.GetPos();
	m_Isp_ccm_linkage[0].ccm[2][1]  = (T_S16)m_spin_ccm_linkage_A_C32.GetPos();
	m_Isp_ccm_linkage[0].ccm[2][2]  = (T_S16)m_spin_ccm_linkage_A_C33.GetPos();

	//TL84
	check = ((CButton *)GetDlgItem(IDC_RADIO_CCM_LINKAGE_TL84_ENABLE2))->GetCheck();
	m_Isp_ccm_linkage[1].cc_enable = check;
	m_Isp_ccm_linkage[1].ccm[0][0]  = (T_S16)m_spin_ccm_linkage_TL84_C11.GetPos();
	m_Isp_ccm_linkage[1].ccm[0][1]  = (T_S16)m_spin_ccm_linkage_TL84_C12.GetPos();
	m_Isp_ccm_linkage[1].ccm[0][2]  = (T_S16)m_spin_ccm_linkage_TL84_C13.GetPos();
	
	m_Isp_ccm_linkage[1].ccm[1][0]  = (T_S16)m_spin_ccm_linkage_TL84_C21.GetPos();
	m_Isp_ccm_linkage[1].ccm[1][1]  = (T_S16)m_spin_ccm_linkage_TL84_C22.GetPos();
	m_Isp_ccm_linkage[1].ccm[1][2]  = (T_S16)m_spin_ccm_linkage_TL84_C23.GetPos();
	
	m_Isp_ccm_linkage[1].ccm[2][0]  = (T_S16)m_spin_ccm_linkage_TL84_C31.GetPos();
	m_Isp_ccm_linkage[1].ccm[2][1]  = (T_S16)m_spin_ccm_linkage_TL84_C32.GetPos();
	m_Isp_ccm_linkage[1].ccm[2][2]  = (T_S16)m_spin_ccm_linkage_TL84_C33.GetPos();


	//D50
	check = ((CButton *)GetDlgItem(IDC_RADIO_CCM_LINKAGE_D50_ENABLE3))->GetCheck();
	m_Isp_ccm_linkage[2].cc_enable = check;
	m_Isp_ccm_linkage[2].ccm[0][0]  = (T_S16)m_spin_ccm_linkage_D50_C11.GetPos();
	m_Isp_ccm_linkage[2].ccm[0][1]  = (T_S16)m_spin_ccm_linkage_D50_C12.GetPos();
	m_Isp_ccm_linkage[2].ccm[0][2]  = (T_S16)m_spin_ccm_linkage_D50_C13.GetPos();
	
	m_Isp_ccm_linkage[2].ccm[1][0]  = (T_S16)m_spin_ccm_linkage_D50_C21.GetPos();
	m_Isp_ccm_linkage[2].ccm[1][1]  = (T_S16)m_spin_ccm_linkage_D50_C22.GetPos();
	m_Isp_ccm_linkage[2].ccm[1][2]  = (T_S16)m_spin_ccm_linkage_D50_C23.GetPos();
	
	m_Isp_ccm_linkage[2].ccm[2][0]  = (T_S16)m_spin_ccm_linkage_D50_C31.GetPos();
	m_Isp_ccm_linkage[2].ccm[2][1]  = (T_S16)m_spin_ccm_linkage_D50_C32.GetPos();
	m_Isp_ccm_linkage[2].ccm[2][2]  = (T_S16)m_spin_ccm_linkage_D50_C33.GetPos();


	//65
	check = ((CButton *)GetDlgItem(IDC_RADIO_CCM_LINKAGE_D65_ENABLE4))->GetCheck();
	m_Isp_ccm_linkage[3].cc_enable = check;
	m_Isp_ccm_linkage[3].ccm[0][0]  = (T_S16)m_spin_ccm_linkage_D65_C11.GetPos();
	m_Isp_ccm_linkage[3].ccm[0][1]  = (T_S16)m_spin_ccm_linkage_D65_C12.GetPos();
	m_Isp_ccm_linkage[3].ccm[0][2]  = (T_S16)m_spin_ccm_linkage_D65_C13.GetPos();
	
	m_Isp_ccm_linkage[3].ccm[1][0]  = (T_S16)m_spin_ccm_linkage_D65_C21.GetPos();
	m_Isp_ccm_linkage[3].ccm[1][1]  = (T_S16)m_spin_ccm_linkage_D65_C22.GetPos();
	m_Isp_ccm_linkage[3].ccm[1][2]  = (T_S16)m_spin_ccm_linkage_D65_C23.GetPos();
	
	m_Isp_ccm_linkage[3].ccm[2][0]  = (T_S16)m_spin_ccm_linkage_D65_C31.GetPos();
	m_Isp_ccm_linkage[3].ccm[2][1]  = (T_S16)m_spin_ccm_linkage_D65_C32.GetPos();
	m_Isp_ccm_linkage[3].ccm[2][2]  = (T_S16)m_spin_ccm_linkage_D65_C33.GetPos();

}

void CCCM_LinkageDlg::SetDataValue(void)
{
	//A
	if (m_Isp_ccm_linkage[0].cc_enable)
	{
		((CButton *)GetDlgItem(IDC_RADIO_CCM_LINKAGE_A_ENABLE))->SetCheck(1);
		((CButton *)GetDlgItem(IDC_RADIO_CCM_LINKAGE_A_DISABLE))->SetCheck(0);
	}
	else
	{
		((CButton *)GetDlgItem(IDC_RADIO_CCM_LINKAGE_A_ENABLE))->SetCheck(0);
		((CButton *)GetDlgItem(IDC_RADIO_CCM_LINKAGE_A_DISABLE))->SetCheck(1);
	}
	
	m_spin_ccm_linkage_A_C11.SetPos(m_Isp_ccm_linkage[0].ccm[0][0]);
	m_slider_ccm_linkage_A_C11.SetPos(m_Isp_ccm_linkage[0].ccm[0][0]);
	m_spin_ccm_linkage_A_C12.SetPos(m_Isp_ccm_linkage[0].ccm[0][1]);
	m_slider_ccm_linkage_A_C12.SetPos(m_Isp_ccm_linkage[0].ccm[0][1]);
	m_spin_ccm_linkage_A_C13.SetPos(m_Isp_ccm_linkage[0].ccm[0][2]);
	m_slider_ccm_linkage_A_C13.SetPos(m_Isp_ccm_linkage[0].ccm[0][2]);
	
	m_spin_ccm_linkage_A_C21.SetPos(m_Isp_ccm_linkage[0].ccm[1][0]);
	m_slider_ccm_linkage_A_C21.SetPos(m_Isp_ccm_linkage[0].ccm[1][0]);
	m_spin_ccm_linkage_A_C22.SetPos(m_Isp_ccm_linkage[0].ccm[1][1]);
	m_slider_ccm_linkage_A_C22.SetPos(m_Isp_ccm_linkage[0].ccm[1][1]);
	m_spin_ccm_linkage_A_C23.SetPos(m_Isp_ccm_linkage[0].ccm[1][2]);
	m_slider_ccm_linkage_A_C23.SetPos(m_Isp_ccm_linkage[0].ccm[1][2]);
	
	m_spin_ccm_linkage_A_C31.SetPos(m_Isp_ccm_linkage[0].ccm[2][0]);
	m_slider_ccm_linkage_A_C31.SetPos(m_Isp_ccm_linkage[0].ccm[2][0]);
	m_spin_ccm_linkage_A_C32.SetPos(m_Isp_ccm_linkage[0].ccm[2][1]);
	m_slider_ccm_linkage_A_C32.SetPos(m_Isp_ccm_linkage[0].ccm[2][1]);
	m_spin_ccm_linkage_A_C33.SetPos(m_Isp_ccm_linkage[0].ccm[2][2]);
	m_slider_ccm_linkage_A_C33.SetPos(m_Isp_ccm_linkage[0].ccm[2][2]);

	//TL84
	if (m_Isp_ccm_linkage[1].cc_enable)
	{
		((CButton *)GetDlgItem(IDC_RADIO_CCM_LINKAGE_TL84_ENABLE2))->SetCheck(1);
		((CButton *)GetDlgItem(IDC_RADIO_CCM_LINKAGE_TL84_DISABLE2))->SetCheck(0);
	}
	else
	{
		((CButton *)GetDlgItem(IDC_RADIO_CCM_LINKAGE_TL84_ENABLE2))->SetCheck(0);
		((CButton *)GetDlgItem(IDC_RADIO_CCM_LINKAGE_TL84_DISABLE2))->SetCheck(1);
	}
	
	m_spin_ccm_linkage_TL84_C11.SetPos(m_Isp_ccm_linkage[1].ccm[0][0]);
	m_slider_ccm_linkage_TL84_C11.SetPos(m_Isp_ccm_linkage[1].ccm[0][0]);
	m_spin_ccm_linkage_TL84_C12.SetPos(m_Isp_ccm_linkage[1].ccm[0][1]);
	m_slider_ccm_linkage_TL84_C12.SetPos(m_Isp_ccm_linkage[1].ccm[0][1]);
	m_spin_ccm_linkage_TL84_C13.SetPos(m_Isp_ccm_linkage[1].ccm[0][2]);
	m_slider_ccm_linkage_TL84_C13.SetPos(m_Isp_ccm_linkage[1].ccm[0][2]);
	
	m_spin_ccm_linkage_TL84_C21.SetPos(m_Isp_ccm_linkage[1].ccm[1][0]);
	m_slider_ccm_linkage_TL84_C21.SetPos(m_Isp_ccm_linkage[1].ccm[1][0]);
	m_spin_ccm_linkage_TL84_C22.SetPos(m_Isp_ccm_linkage[1].ccm[1][1]);
	m_slider_ccm_linkage_TL84_C22.SetPos(m_Isp_ccm_linkage[1].ccm[1][1]);
	m_spin_ccm_linkage_TL84_C23.SetPos(m_Isp_ccm_linkage[1].ccm[1][2]);
	m_slider_ccm_linkage_TL84_C23.SetPos(m_Isp_ccm_linkage[1].ccm[1][2]);
	
	m_spin_ccm_linkage_TL84_C31.SetPos(m_Isp_ccm_linkage[1].ccm[2][0]);
	m_slider_ccm_linkage_TL84_C31.SetPos(m_Isp_ccm_linkage[1].ccm[2][0]);
	m_spin_ccm_linkage_TL84_C32.SetPos(m_Isp_ccm_linkage[1].ccm[2][1]);
	m_slider_ccm_linkage_TL84_C32.SetPos(m_Isp_ccm_linkage[1].ccm[2][1]);
	m_spin_ccm_linkage_TL84_C33.SetPos(m_Isp_ccm_linkage[1].ccm[2][2]);
	m_slider_ccm_linkage_TL84_C33.SetPos(m_Isp_ccm_linkage[1].ccm[2][2]);


	//D50
	if (m_Isp_ccm_linkage[2].cc_enable)
	{
		((CButton *)GetDlgItem(IDC_RADIO_CCM_LINKAGE_D50_ENABLE3))->SetCheck(1);
		((CButton *)GetDlgItem(IDC_RADIO_CCM_LINKAGE_D50_DISABLE3))->SetCheck(0);
	}
	else
	{
		((CButton *)GetDlgItem(IDC_RADIO_CCM_LINKAGE_D50_ENABLE3))->SetCheck(0);
		((CButton *)GetDlgItem(IDC_RADIO_CCM_LINKAGE_D50_DISABLE3))->SetCheck(1);
	}
	
	m_spin_ccm_linkage_D50_C11.SetPos(m_Isp_ccm_linkage[2].ccm[0][0]);
	m_slider_ccm_linkage_D50_C11.SetPos(m_Isp_ccm_linkage[2].ccm[0][0]);
	m_spin_ccm_linkage_D50_C12.SetPos(m_Isp_ccm_linkage[2].ccm[0][1]);
	m_slider_ccm_linkage_D50_C12.SetPos(m_Isp_ccm_linkage[2].ccm[0][1]);
	m_spin_ccm_linkage_D50_C13.SetPos(m_Isp_ccm_linkage[2].ccm[0][2]);
	m_slider_ccm_linkage_D50_C13.SetPos(m_Isp_ccm_linkage[2].ccm[0][2]);
	
	m_spin_ccm_linkage_D50_C21.SetPos(m_Isp_ccm_linkage[2].ccm[1][0]);
	m_slider_ccm_linkage_D50_C21.SetPos(m_Isp_ccm_linkage[2].ccm[1][0]);
	m_spin_ccm_linkage_D50_C22.SetPos(m_Isp_ccm_linkage[2].ccm[1][1]);
	m_slider_ccm_linkage_D50_C22.SetPos(m_Isp_ccm_linkage[2].ccm[1][1]);
	m_spin_ccm_linkage_D50_C23.SetPos(m_Isp_ccm_linkage[2].ccm[1][2]);
	m_slider_ccm_linkage_D50_C23.SetPos(m_Isp_ccm_linkage[2].ccm[1][2]);
	
	m_spin_ccm_linkage_D50_C31.SetPos(m_Isp_ccm_linkage[2].ccm[2][0]);
	m_slider_ccm_linkage_D50_C31.SetPos(m_Isp_ccm_linkage[2].ccm[2][0]);
	m_spin_ccm_linkage_D50_C32.SetPos(m_Isp_ccm_linkage[2].ccm[2][1]);
	m_slider_ccm_linkage_D50_C32.SetPos(m_Isp_ccm_linkage[2].ccm[2][1]);
	m_spin_ccm_linkage_D50_C33.SetPos(m_Isp_ccm_linkage[2].ccm[2][2]);
	m_slider_ccm_linkage_D50_C33.SetPos(m_Isp_ccm_linkage[2].ccm[2][2]);

	//D65
	if (m_Isp_ccm_linkage[3].cc_enable)
	{
		((CButton *)GetDlgItem(IDC_RADIO_CCM_LINKAGE_D65_ENABLE4))->SetCheck(1);
		((CButton *)GetDlgItem(IDC_RADIO_CCM_LINKAGE_D65_DISABLE4))->SetCheck(0);
	}
	else
	{
		((CButton *)GetDlgItem(IDC_RADIO_CCM_LINKAGE_D65_ENABLE4))->SetCheck(0);
		((CButton *)GetDlgItem(IDC_RADIO_CCM_LINKAGE_D65_DISABLE4))->SetCheck(1);
	}
	
	m_spin_ccm_linkage_D65_C11.SetPos(m_Isp_ccm_linkage[3].ccm[0][0]);
	m_slider_ccm_linkage_D65_C11.SetPos(m_Isp_ccm_linkage[3].ccm[0][0]);
	m_spin_ccm_linkage_D65_C12.SetPos(m_Isp_ccm_linkage[3].ccm[0][1]);
	m_slider_ccm_linkage_D65_C12.SetPos(m_Isp_ccm_linkage[3].ccm[0][1]);
	m_spin_ccm_linkage_D65_C13.SetPos(m_Isp_ccm_linkage[3].ccm[0][2]);
	m_slider_ccm_linkage_D65_C13.SetPos(m_Isp_ccm_linkage[3].ccm[0][2]);
	
	m_spin_ccm_linkage_D65_C21.SetPos(m_Isp_ccm_linkage[3].ccm[1][0]);
	m_slider_ccm_linkage_D65_C21.SetPos(m_Isp_ccm_linkage[3].ccm[1][0]);
	m_spin_ccm_linkage_D65_C22.SetPos(m_Isp_ccm_linkage[3].ccm[1][1]);
	m_slider_ccm_linkage_D65_C22.SetPos(m_Isp_ccm_linkage[3].ccm[1][1]);
	m_spin_ccm_linkage_D65_C23.SetPos(m_Isp_ccm_linkage[3].ccm[1][2]);
	m_slider_ccm_linkage_D65_C23.SetPos(m_Isp_ccm_linkage[3].ccm[1][2]);
	
	m_spin_ccm_linkage_D65_C31.SetPos(m_Isp_ccm_linkage[3].ccm[2][0]);
	m_slider_ccm_linkage_D65_C31.SetPos(m_Isp_ccm_linkage[3].ccm[2][0]);
	m_spin_ccm_linkage_D65_C32.SetPos(m_Isp_ccm_linkage[3].ccm[2][1]);
	m_slider_ccm_linkage_D65_C32.SetPos(m_Isp_ccm_linkage[3].ccm[2][1]);
	m_spin_ccm_linkage_D65_C33.SetPos(m_Isp_ccm_linkage[3].ccm[2][2]);
	m_slider_ccm_linkage_D65_C33.SetPos(m_Isp_ccm_linkage[3].ccm[2][2]);
}

int CCCM_LinkageDlg::GetPageInfoSt_CCMlinkage( void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < (sizeof(AK_ISP_CCM) *4 ))) return -1;
	
	//GetDataValue();
	
	memcpy(pPageInfoSt, &m_Isp_ccm_linkage, sizeof(AK_ISP_CCM)*4);
	
	//nPageID = m_nID;
	nStLen = sizeof(AK_ISP_CCM)*4;
	
	return 0;
}

int CCCM_LinkageDlg::SetPageInfoSt_CCMlinkage(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < (sizeof(AK_ISP_CCM) *4))) return -1;
	
	memcpy(&m_Isp_ccm_linkage, pPageInfoSt, sizeof(AK_ISP_CCM)*4);
	
	//SetDataValue();
	
	return 0;
}

void CCCM_LinkageDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	CString str;
	UINT total_sum[4][3] = {0};

	if (1 == nIDEvent)
	{
		total_sum[0][0] = (T_S16)m_spin_ccm_linkage_A_C11.GetPos() + (T_S16)m_spin_ccm_linkage_A_C12.GetPos() + (T_S16)m_spin_ccm_linkage_A_C13.GetPos();
		total_sum[0][1] = (T_S16)m_spin_ccm_linkage_A_C21.GetPos() + (T_S16)m_spin_ccm_linkage_A_C22.GetPos() + (T_S16)m_spin_ccm_linkage_A_C23.GetPos();
		total_sum[0][2] = (T_S16)m_spin_ccm_linkage_A_C31.GetPos() + (T_S16)m_spin_ccm_linkage_A_C32.GetPos() + (T_S16)m_spin_ccm_linkage_A_C33.GetPos();

		total_sum[1][0] = (T_S16)m_spin_ccm_linkage_TL84_C11.GetPos() + (T_S16)m_spin_ccm_linkage_TL84_C12.GetPos() + (T_S16)m_spin_ccm_linkage_TL84_C13.GetPos();
		total_sum[1][1] = (T_S16)m_spin_ccm_linkage_TL84_C21.GetPos() + (T_S16)m_spin_ccm_linkage_TL84_C22.GetPos() + (T_S16)m_spin_ccm_linkage_TL84_C23.GetPos();
		total_sum[1][2] = (T_S16)m_spin_ccm_linkage_TL84_C31.GetPos() + (T_S16)m_spin_ccm_linkage_TL84_C32.GetPos() + (T_S16)m_spin_ccm_linkage_TL84_C33.GetPos();

		total_sum[2][0] = (T_S16)m_spin_ccm_linkage_D50_C11.GetPos() + (T_S16)m_spin_ccm_linkage_D50_C12.GetPos() + (T_S16)m_spin_ccm_linkage_D50_C13.GetPos();
		total_sum[2][1] = (T_S16)m_spin_ccm_linkage_D50_C21.GetPos() + (T_S16)m_spin_ccm_linkage_D50_C22.GetPos() + (T_S16)m_spin_ccm_linkage_D50_C23.GetPos();
		total_sum[2][2] = (T_S16)m_spin_ccm_linkage_D50_C31.GetPos() + (T_S16)m_spin_ccm_linkage_D50_C32.GetPos() + (T_S16)m_spin_ccm_linkage_D50_C33.GetPos();

		total_sum[3][0] = (T_S16)m_spin_ccm_linkage_D65_C11.GetPos() + (T_S16)m_spin_ccm_linkage_D65_C12.GetPos() + (T_S16)m_spin_ccm_linkage_D65_C13.GetPos();
		total_sum[3][1] = (T_S16)m_spin_ccm_linkage_D65_C21.GetPos() + (T_S16)m_spin_ccm_linkage_D65_C22.GetPos() + (T_S16)m_spin_ccm_linkage_D65_C23.GetPos();
		total_sum[3][2] = (T_S16)m_spin_ccm_linkage_D65_C31.GetPos() + (T_S16)m_spin_ccm_linkage_D65_C32.GetPos() + (T_S16)m_spin_ccm_linkage_D65_C33.GetPos();

		str.Format(_T("%d"), total_sum[0][0]);
		SetDlgItemText(IDC_STATIC_A_C1_SUM, str);

		str.Format(_T("%d"), total_sum[0][1]);
		SetDlgItemText(IDC_STATIC_A_C2_SUM, str);

		str.Format(_T("%d"), total_sum[0][2]);
		SetDlgItemText(IDC_STATIC_A_C3_SUM, str);

		str.Format(_T("%d"), total_sum[1][0]);
		SetDlgItemText(IDC_STATIC_TL84_C1_SUM, str);

		str.Format(_T("%d"), total_sum[1][1]);
		SetDlgItemText(IDC_STATIC_TL84_C2_SUM, str);

		str.Format(_T("%d"), total_sum[1][2]);
		SetDlgItemText(IDC_STATIC_TL84_C3_SUM, str);

		str.Format(_T("%d"), total_sum[2][0]);
		SetDlgItemText(IDC_STATIC_D50_C1_SUM, str);

		str.Format(_T("%d"), total_sum[2][1]);
		SetDlgItemText(IDC_STATIC_D50_C2_SUM, str);

		str.Format(_T("%d"), total_sum[2][2]);
		SetDlgItemText(IDC_STATIC_D50_C3_SUM, str);

		str.Format(_T("%d"), total_sum[3][0]);
		SetDlgItemText(IDC_STATIC_D65_C1_SUM, str);

		str.Format(_T("%d"), total_sum[3][1]);
		SetDlgItemText(IDC_STATIC_D65_C2_SUM, str);

		str.Format(_T("%d"), total_sum[3][2]);
		SetDlgItemText(IDC_STATIC_D65_C3_SUM, str);
	}

	CDialog::OnTimer(nIDEvent);
}
