// NrLinkageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "NrLinkageDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNrLinkageDlg dialog


CNrLinkageDlg::CNrLinkageDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNrLinkageDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNrLinkageDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	ZeroMemory(m_Nr1_Enable, sizeof(m_Nr1_Enable));
	ZeroMemory(m_Nr2_Enable, sizeof(m_Nr2_Enable));
	ZeroMemory(m_Y_Dpc_Enable, sizeof(m_Y_Dpc_Enable));
	ZeroMemory(m_Y_Black_Dpc_Enable, sizeof(m_Y_Black_Dpc_Enable));
	ZeroMemory(m_Y_White_Dpc_Enable, sizeof(m_Y_White_Dpc_Enable));
}


void CNrLinkageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNrLinkageDlg)
	
	//}}AFX_DATA_MAP
	DDX_Radio(pDX, IDC_RADIO1, m_Nr1_Enable[0]);
	DDX_Radio(pDX, IDC_RADIO40, m_Nr1_Enable[1]);
	DDX_Radio(pDX, IDC_RADIO44, m_Nr1_Enable[2]);
	DDX_Radio(pDX, IDC_RADIO48, m_Nr1_Enable[3]);
	DDX_Radio(pDX, IDC_RADIO52, m_Nr1_Enable[4]);
	DDX_Radio(pDX, IDC_RADIO56, m_Nr1_Enable[5]);
	DDX_Radio(pDX, IDC_RADIO60, m_Nr1_Enable[6]);
	DDX_Radio(pDX, IDC_RADIO64, m_Nr1_Enable[7]);
	DDX_Radio(pDX, IDC_RADIO68, m_Nr1_Enable[8]);
	
	DDX_Radio(pDX, IDC_RADIO33, m_Nr2_Enable[0]);
	DDX_Radio(pDX, IDC_RADIO42, m_Nr2_Enable[1]);
	DDX_Radio(pDX, IDC_RADIO46, m_Nr2_Enable[2]);
	DDX_Radio(pDX, IDC_RADIO50, m_Nr2_Enable[3]);
	DDX_Radio(pDX, IDC_RADIO54, m_Nr2_Enable[4]);
	DDX_Radio(pDX, IDC_RADIO58, m_Nr2_Enable[5]);
	DDX_Radio(pDX, IDC_RADIO62, m_Nr2_Enable[6]);
	DDX_Radio(pDX, IDC_RADIO66, m_Nr2_Enable[7]);
	DDX_Radio(pDX, IDC_RADIO70, m_Nr2_Enable[8]);

	DDX_Radio(pDX, IDC_RADIO35, m_Y_Dpc_Enable[0]);
	DDX_Radio(pDX, IDC_RADIO73, m_Y_Dpc_Enable[1]);
	DDX_Radio(pDX, IDC_RADIO98, m_Y_Dpc_Enable[2]);
	DDX_Radio(pDX, IDC_RADIO104, m_Y_Dpc_Enable[3]);
	DDX_Radio(pDX, IDC_RADIO110, m_Y_Dpc_Enable[4]);
	DDX_Radio(pDX, IDC_RADIO116, m_Y_Dpc_Enable[5]);
	DDX_Radio(pDX, IDC_RADIO120, m_Y_Dpc_Enable[6]);
	DDX_Radio(pDX, IDC_RADIO126, m_Y_Dpc_Enable[7]);
	DDX_Radio(pDX, IDC_RADIO132, m_Y_Dpc_Enable[8]);

	DDX_Radio(pDX, IDC_RADIO37, m_Y_Black_Dpc_Enable[0]);
	DDX_Radio(pDX, IDC_RADIO94, m_Y_Black_Dpc_Enable[1]);
	DDX_Radio(pDX, IDC_RADIO100, m_Y_Black_Dpc_Enable[2]);
	DDX_Radio(pDX, IDC_RADIO106, m_Y_Black_Dpc_Enable[3]);
	DDX_Radio(pDX, IDC_RADIO112, m_Y_Black_Dpc_Enable[4]);
	DDX_Radio(pDX, IDC_RADIO118, m_Y_Black_Dpc_Enable[5]);
	DDX_Radio(pDX, IDC_RADIO122, m_Y_Black_Dpc_Enable[6]);
	DDX_Radio(pDX, IDC_RADIO128, m_Y_Black_Dpc_Enable[7]);
	DDX_Radio(pDX, IDC_RADIO134, m_Y_Black_Dpc_Enable[8]);

	DDX_Radio(pDX, IDC_RADIO39, m_Y_White_Dpc_Enable[0]);
	DDX_Radio(pDX, IDC_RADIO96, m_Y_White_Dpc_Enable[1]);
	DDX_Radio(pDX, IDC_RADIO102, m_Y_White_Dpc_Enable[2]);
	DDX_Radio(pDX, IDC_RADIO108, m_Y_White_Dpc_Enable[3]);
	DDX_Radio(pDX, IDC_RADIO114, m_Y_White_Dpc_Enable[4]);
	DDX_Radio(pDX, IDC_RADIO86, m_Y_White_Dpc_Enable[5]);
	DDX_Radio(pDX, IDC_RADIO124, m_Y_White_Dpc_Enable[6]);
	DDX_Radio(pDX, IDC_RADIO130, m_Y_White_Dpc_Enable[7]);
	DDX_Radio(pDX, IDC_RADIO136, m_Y_White_Dpc_Enable[8]);

	DDX_Text(pDX, IDC_EDIT1, m_Rstrength[0]);
	DDV_MinMaxInt(pDX, m_Rstrength[0], 0, 65535);
	DDX_Text(pDX, IDC_EDIT91, m_Rstrength[1]);
	DDV_MinMaxInt(pDX, m_Rstrength[1], 0, 65535);
	DDX_Text(pDX, IDC_EDIT101, m_Rstrength[2]);
	DDV_MinMaxInt(pDX, m_Rstrength[2], 0, 65535);
	DDX_Text(pDX, IDC_EDIT107, m_Rstrength[3]);
	DDV_MinMaxInt(pDX, m_Rstrength[3], 0, 65535);
	DDX_Text(pDX, IDC_EDIT113, m_Rstrength[4]);
	DDV_MinMaxInt(pDX, m_Rstrength[4], 0, 65535);
	DDX_Text(pDX, IDC_EDIT119, m_Rstrength[5]);
	DDV_MinMaxInt(pDX, m_Rstrength[5], 0, 65535);
	DDX_Text(pDX, IDC_EDIT125, m_Rstrength[6]);
	DDV_MinMaxInt(pDX, m_Rstrength[6], 0, 65535);
	DDX_Text(pDX, IDC_EDIT131, m_Rstrength[7]);
	DDV_MinMaxInt(pDX, m_Rstrength[7], 0, 65535);
	DDX_Text(pDX, IDC_EDIT137, m_Rstrength[8]);
	DDV_MinMaxInt(pDX, m_Rstrength[8], 0, 65535);
	

	DDX_Text(pDX, IDC_EDIT2, m_Gstrength[0]);
	DDV_MinMaxInt(pDX, m_Gstrength[0], 0, 65535);
	DDX_Text(pDX, IDC_EDIT96, m_Gstrength[1]);
	DDV_MinMaxInt(pDX, m_Gstrength[1], 0, 65535);
	DDX_Text(pDX, IDC_EDIT102, m_Gstrength[2]);
	DDV_MinMaxInt(pDX, m_Gstrength[2], 0, 65535);
	DDX_Text(pDX, IDC_EDIT108, m_Gstrength[3]);
	DDV_MinMaxInt(pDX, m_Gstrength[3], 0, 65535);
	DDX_Text(pDX, IDC_EDIT114, m_Gstrength[4]);
	DDV_MinMaxInt(pDX, m_Gstrength[4], 0, 65535);
	DDX_Text(pDX, IDC_EDIT120, m_Gstrength[5]);
	DDV_MinMaxInt(pDX, m_Gstrength[5], 0, 65535);
	DDX_Text(pDX, IDC_EDIT126, m_Gstrength[6]);
	DDV_MinMaxInt(pDX, m_Gstrength[6], 0, 65535);
	DDX_Text(pDX, IDC_EDIT132, m_Gstrength[7]);
	DDV_MinMaxInt(pDX, m_Gstrength[7], 0, 65535);
	DDX_Text(pDX, IDC_EDIT138, m_Gstrength[8]);
	DDV_MinMaxInt(pDX, m_Gstrength[8], 0, 65535);

	DDX_Text(pDX, IDC_EDIT3, m_Bstrength[0]);
	DDV_MinMaxInt(pDX, m_Bstrength[0], 0, 65535);
	DDX_Text(pDX, IDC_EDIT97, m_Bstrength[1]);
	DDV_MinMaxInt(pDX, m_Bstrength[1], 0, 65535);
	DDX_Text(pDX, IDC_EDIT103, m_Bstrength[2]);
	DDV_MinMaxInt(pDX, m_Bstrength[2], 0, 65535);
	DDX_Text(pDX, IDC_EDIT109, m_Bstrength[3]);
	DDV_MinMaxInt(pDX, m_Bstrength[3], 0, 65535);
	DDX_Text(pDX, IDC_EDIT115, m_Bstrength[4]);
	DDV_MinMaxInt(pDX, m_Bstrength[4], 0, 65535);
	DDX_Text(pDX, IDC_EDIT121, m_Bstrength[5]);
	DDV_MinMaxInt(pDX, m_Bstrength[5], 0, 65535);
	DDX_Text(pDX, IDC_EDIT127, m_Bstrength[6]);
	DDV_MinMaxInt(pDX, m_Bstrength[6], 0, 65535);
	DDX_Text(pDX, IDC_EDIT133, m_Bstrength[7]);
	DDV_MinMaxInt(pDX, m_Bstrength[7], 0, 65535);
	DDX_Text(pDX, IDC_EDIT139, m_Bstrength[8]);
	DDV_MinMaxInt(pDX, m_Bstrength[8], 0, 65535);

	DDX_Text(pDX, IDC_EDIT4, m_Nr1_k[0]);
	DDV_MinMaxInt(pDX, m_Nr1_k[0], 0, 15);
	DDX_Text(pDX, IDC_EDIT98, m_Nr1_k[1]);
	DDV_MinMaxInt(pDX, m_Nr1_k[1], 0, 15);
	DDX_Text(pDX, IDC_EDIT104, m_Nr1_k[2]);
	DDV_MinMaxInt(pDX, m_Nr1_k[2], 0, 15);
	DDX_Text(pDX, IDC_EDIT110, m_Nr1_k[3]);
	DDV_MinMaxInt(pDX, m_Nr1_k[3], 0, 15);
	DDX_Text(pDX, IDC_EDIT116, m_Nr1_k[4]);
	DDV_MinMaxInt(pDX, m_Nr1_k[4], 0, 15);
	DDX_Text(pDX, IDC_EDIT122, m_Nr1_k[5]);
	DDV_MinMaxInt(pDX, m_Nr1_k[5], 0, 15);
	DDX_Text(pDX, IDC_EDIT128, m_Nr1_k[6]);
	DDV_MinMaxInt(pDX, m_Nr1_k[6], 0, 15);
	DDX_Text(pDX, IDC_EDIT134, m_Nr1_k[7]);
	DDV_MinMaxInt(pDX, m_Nr1_k[7], 0, 15);
	DDX_Text(pDX, IDC_EDIT140, m_Nr1_k[8]);
	DDV_MinMaxInt(pDX, m_Nr1_k[8], 0, 15);

	DDX_Text(pDX, IDC_EDIT6, m_Nr2strength[0]);
	DDV_MinMaxInt(pDX, m_Nr2strength[0], 0, 65535);
	DDX_Text(pDX, IDC_EDIT99, m_Nr2strength[1]);
	DDV_MinMaxInt(pDX, m_Nr2strength[1], 0, 65535);
	DDX_Text(pDX, IDC_EDIT105, m_Nr2strength[2]);
	DDV_MinMaxInt(pDX, m_Nr2strength[2], 0, 65535);
	DDX_Text(pDX, IDC_EDIT111, m_Nr2strength[3]);
	DDV_MinMaxInt(pDX, m_Nr2strength[3], 0, 65535);
	DDX_Text(pDX, IDC_EDIT117, m_Nr2strength[4]);
	DDV_MinMaxInt(pDX, m_Nr2strength[4], 0, 65535);
	DDX_Text(pDX, IDC_EDIT123, m_Nr2strength[5]);
	DDV_MinMaxInt(pDX, m_Nr2strength[5], 0, 65535);
	DDX_Text(pDX, IDC_EDIT129, m_Nr2strength[6]);
	DDV_MinMaxInt(pDX, m_Nr2strength[6], 0, 65535);
	DDX_Text(pDX, IDC_EDIT135, m_Nr2strength[7]);
	DDV_MinMaxInt(pDX, m_Nr2strength[7], 0, 65535);
	DDX_Text(pDX, IDC_EDIT141, m_Nr2strength[8]);
	DDV_MinMaxInt(pDX, m_Nr2strength[8], 0, 65535);
	
	

	DDX_Text(pDX, IDC_EDIT7, m_Nr2_k[0]);
	DDV_MinMaxInt(pDX, m_Nr2_k[0], 0, 15);
	DDX_Text(pDX, IDC_EDIT100, m_Nr2_k[1]);
	DDV_MinMaxInt(pDX, m_Nr2_k[1], 0, 15);
	DDX_Text(pDX, IDC_EDIT106, m_Nr2_k[2]);
	DDV_MinMaxInt(pDX, m_Nr2_k[2], 0, 15);
	DDX_Text(pDX, IDC_EDIT112, m_Nr2_k[3]);
	DDV_MinMaxInt(pDX, m_Nr2_k[3], 0, 15);
	DDX_Text(pDX, IDC_EDIT118, m_Nr2_k[4]);
	DDV_MinMaxInt(pDX, m_Nr2_k[4], 0, 15);
	DDX_Text(pDX, IDC_EDIT124, m_Nr2_k[5]);
	DDV_MinMaxInt(pDX, m_Nr2_k[5], 0, 15);
	DDX_Text(pDX, IDC_EDIT130, m_Nr2_k[6]);
	DDV_MinMaxInt(pDX, m_Nr2_k[6], 0, 15);
	DDX_Text(pDX, IDC_EDIT136, m_Nr2_k[7]);
	DDV_MinMaxInt(pDX, m_Nr2_k[7], 0, 15);
	DDX_Text(pDX, IDC_EDIT142, m_Nr2_k[8]);
	DDV_MinMaxInt(pDX, m_Nr2_k[8], 0, 15);

	DDX_Text(pDX, IDC_EDIT10, m_Y_Dpc_th[0]);
	DDV_MinMaxInt(pDX, m_Y_Dpc_th[0], 0, 1023);
	DDX_Text(pDX, IDC_EDIT29, m_Y_Dpc_th[1]);
	DDV_MinMaxInt(pDX, m_Y_Dpc_th[1], 0, 1023);
	DDX_Text(pDX, IDC_EDIT36, m_Y_Dpc_th[2]);
	DDV_MinMaxInt(pDX, m_Y_Dpc_th[2], 0, 1023);
	DDX_Text(pDX, IDC_EDIT160, m_Y_Dpc_th[3]);
	DDV_MinMaxInt(pDX, m_Y_Dpc_th[3], 0, 1023);
	DDX_Text(pDX, IDC_EDIT161, m_Y_Dpc_th[4]);
	DDV_MinMaxInt(pDX, m_Y_Dpc_th[4], 0, 1023);
	DDX_Text(pDX, IDC_EDIT162, m_Y_Dpc_th[5]);
	DDV_MinMaxInt(pDX, m_Y_Dpc_th[5], 0, 1023);
	DDX_Text(pDX, IDC_EDIT163, m_Y_Dpc_th[6]);
	DDV_MinMaxInt(pDX, m_Y_Dpc_th[6], 0, 1023);
	DDX_Text(pDX, IDC_EDIT164, m_Y_Dpc_th[7]);
	DDV_MinMaxInt(pDX, m_Y_Dpc_th[7], 0, 1023);
	DDX_Text(pDX, IDC_EDIT165, m_Y_Dpc_th[8]);
	DDV_MinMaxInt(pDX, m_Y_Dpc_th[8], 0, 1023);

}


BEGIN_MESSAGE_MAP(CNrLinkageDlg, CDialog)
	//{{AFX_MSG_MAP(CNrLinkageDlg)
	ON_BN_CLICKED(IDC_BUTTON_OK, OnButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnButtonCancel)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_BN_CLICKED(IDC_RADIO40, OnRadio40)
	ON_BN_CLICKED(IDC_RADIO41, OnRadio41)
	ON_BN_CLICKED(IDC_RADIO44, OnRadio44)
	ON_BN_CLICKED(IDC_RADIO45, OnRadio45)
	ON_BN_CLICKED(IDC_RADIO33, OnRadio33)
	ON_BN_CLICKED(IDC_RADIO34, OnRadio34)
	ON_BN_CLICKED(IDC_RADIO42, OnRadio42)
	ON_BN_CLICKED(IDC_RADIO43, OnRadio43)
	ON_BN_CLICKED(IDC_RADIO46, OnRadio46)
	ON_BN_CLICKED(IDC_RADIO47, OnRadio47)
	ON_BN_CLICKED(IDC_RADIO48, OnRadio48)
	ON_BN_CLICKED(IDC_RADIO49, OnRadio49)
	ON_BN_CLICKED(IDC_RADIO50, OnRadio50)
	ON_BN_CLICKED(IDC_RADIO51, OnRadio51)
	ON_BN_CLICKED(IDC_RADIO52, OnRadio52)
	ON_BN_CLICKED(IDC_RADIO53, OnRadio53)
	ON_BN_CLICKED(IDC_RADIO54, OnRadio54)
	ON_BN_CLICKED(IDC_RADIO55, OnRadio55)
	ON_BN_CLICKED(IDC_RADIO56, OnRadio56)
	ON_BN_CLICKED(IDC_RADIO57, OnRadio57)
	ON_BN_CLICKED(IDC_RADIO58, OnRadio58)
	ON_BN_CLICKED(IDC_RADIO59, OnRadio59)
	ON_BN_CLICKED(IDC_RADIO60, OnRadio60)
	ON_BN_CLICKED(IDC_RADIO61, OnRadio61)
	ON_BN_CLICKED(IDC_RADIO62, OnRadio62)
	ON_BN_CLICKED(IDC_RADIO63, OnRadio63)
	ON_BN_CLICKED(IDC_RADIO64, OnRadio64)
	ON_BN_CLICKED(IDC_RADIO65, OnRadio65)
	ON_BN_CLICKED(IDC_RADIO66, OnRadio66)
	ON_BN_CLICKED(IDC_RADIO67, OnRadio67)
	ON_BN_CLICKED(IDC_RADIO68, OnRadio68)
	ON_BN_CLICKED(IDC_RADIO69, OnRadio69)
	ON_BN_CLICKED(IDC_RADIO70, OnRadio70)
	ON_BN_CLICKED(IDC_RADIO71, OnRadio71)

	ON_BN_CLICKED(IDC_RADIO35, OnRadio35)
	ON_BN_CLICKED(IDC_RADIO36, OnRadio36)
	ON_BN_CLICKED(IDC_RADIO37, OnRadio37)
	ON_BN_CLICKED(IDC_RADIO38, OnRadio38)
	ON_BN_CLICKED(IDC_RADIO39, OnRadio39)
	ON_BN_CLICKED(IDC_RADIO93, OnRadio93)
	ON_BN_CLICKED(IDC_RADIO73, OnRadio73)
	ON_BN_CLICKED(IDC_RADIO74, OnRadio74)
	ON_BN_CLICKED(IDC_RADIO94, OnRadio94)
	ON_BN_CLICKED(IDC_RADIO95, OnRadio95)
	ON_BN_CLICKED(IDC_RADIO96, OnRadio96)
	ON_BN_CLICKED(IDC_RADIO97, OnRadio97)
	ON_BN_CLICKED(IDC_RADIO98, OnRadio98)
	ON_BN_CLICKED(IDC_RADIO99, OnRadio99)
	ON_BN_CLICKED(IDC_RADIO100, OnRadio100)
	ON_BN_CLICKED(IDC_RADIO101, OnRadio101)
	ON_BN_CLICKED(IDC_RADIO102, OnRadio102)
	ON_BN_CLICKED(IDC_RADIO103, OnRadio103)
	ON_BN_CLICKED(IDC_RADIO104, OnRadio104)
	ON_BN_CLICKED(IDC_RADIO105, OnRadio105)
	ON_BN_CLICKED(IDC_RADIO106, OnRadio106)
	ON_BN_CLICKED(IDC_RADIO107, OnRadio107)
	ON_BN_CLICKED(IDC_RADIO108, OnRadio108)
	ON_BN_CLICKED(IDC_RADIO109, OnRadio109)
	ON_BN_CLICKED(IDC_RADIO110, OnRadio110)
	ON_BN_CLICKED(IDC_RADIO111, OnRadio111)
	ON_BN_CLICKED(IDC_RADIO112, OnRadio112)
	ON_BN_CLICKED(IDC_RADIO113, OnRadio113)
	ON_BN_CLICKED(IDC_RADIO114, OnRadio114)
	ON_BN_CLICKED(IDC_RADIO115, OnRadio115)
	ON_BN_CLICKED(IDC_RADIO116, OnRadio116)
	ON_BN_CLICKED(IDC_RADIO117, OnRadio117)
	ON_BN_CLICKED(IDC_RADIO118, OnRadio118)
	ON_BN_CLICKED(IDC_RADIO85, OnRadio85)
	ON_BN_CLICKED(IDC_RADIO86, OnRadio86)
	ON_BN_CLICKED(IDC_RADIO119, OnRadio119)
	ON_BN_CLICKED(IDC_RADIO120, OnRadio120)
	ON_BN_CLICKED(IDC_RADIO121, OnRadio121)
	ON_BN_CLICKED(IDC_RADIO122, OnRadio122)
	ON_BN_CLICKED(IDC_RADIO123, OnRadio123)
	ON_BN_CLICKED(IDC_RADIO124, OnRadio124)
	ON_BN_CLICKED(IDC_RADIO125, OnRadio125)
	ON_BN_CLICKED(IDC_RADIO126, OnRadio126)
	ON_BN_CLICKED(IDC_RADIO127, OnRadio127)
	ON_BN_CLICKED(IDC_RADIO128, OnRadio128)
	ON_BN_CLICKED(IDC_RADIO129, OnRadio129)
	ON_BN_CLICKED(IDC_RADIO130, OnRadio130)
	ON_BN_CLICKED(IDC_RADIO131, OnRadio131)
	ON_BN_CLICKED(IDC_RADIO132, OnRadio132)
	ON_BN_CLICKED(IDC_RADIO133, OnRadio133)
	ON_BN_CLICKED(IDC_RADIO134, OnRadio134)
	ON_BN_CLICKED(IDC_RADIO135, OnRadio135)
	ON_BN_CLICKED(IDC_RADIO136, OnRadio136)
	ON_BN_CLICKED(IDC_RADIO137, OnRadio137)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNrLinkageDlg message handlers
BOOL CNrLinkageDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	int i = 0;

	for (i=0; i<9; i++)
	{
		m_Rstrength[i] = m_linkage[i].nr1_calc_r_k;
		m_Gstrength[i] = m_linkage[i].nr1_calc_g_k;
		m_Bstrength[i] = m_linkage[i].nr1_calc_b_k;

		m_Nr2strength[i] = m_linkage[i].nr2_calc_y_k;
		m_Nr1_k[i] = m_linkage[i].nr1_k;
		m_Nr2_k[i] = m_linkage[i].nr2_k;
		m_Y_Dpc_th[i] = m_linkage[i].y_dpc_th;


		
		m_Nr1_Enable[i] = m_linkage[i].nr1_enable;
		m_Nr2_Enable[i] = m_linkage[i].nr2_enable;
		m_Y_Dpc_Enable[i] = m_linkage[i].y_dpc_enable;
		m_Y_Black_Dpc_Enable[i] = m_linkage[i].y_black_dpc_enable;
		m_Y_White_Dpc_Enable[i] = m_linkage[i].y_white_dpc_enable;
	}

	UpdateData(FALSE);

	return TRUE;
}

BOOL CNrLinkageDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}


void CNrLinkageDlg::OnButtonOk() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();

	int i = 0;

	UpdateData(TRUE);

	for (i=0; i<9; i++)
	{
		m_linkage[i].nr1_calc_r_k = (T_U16)m_Rstrength[i];
		m_linkage[i].nr1_calc_g_k = (T_U16)m_Gstrength[i];
		m_linkage[i].nr1_calc_b_k = (T_U16)m_Bstrength[i];
		m_linkage[i].nr1_k = (T_U16)m_Nr1_k[i];
		m_linkage[i].nr1_enable = (T_U16)m_Nr1_Enable[i];

		m_linkage[i].nr2_calc_y_k = (T_U16)m_Nr2strength[i];
		m_linkage[i].nr2_k = (T_U16)m_Nr2_k[i];
		m_linkage[i].nr2_enable = (T_U16)m_Nr2_Enable[i];
		m_linkage[i].y_dpc_enable = (T_U16)m_Y_Dpc_Enable[i];
		m_linkage[i].y_dpc_th = (T_U16)m_Y_Dpc_th[i];
		m_linkage[i].y_black_dpc_enable = (T_U16)m_Y_Black_Dpc_Enable[i];
		m_linkage[i].y_white_dpc_enable = (T_U16)m_Y_White_Dpc_Enable[i];
	}
}

void CNrLinkageDlg::OnButtonCancel() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}

void CNrLinkageDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio40() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio41() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio44() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio45() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio33() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio34() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio42() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio43() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio46() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio47() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio48() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio49() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio50() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio51() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio52() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio53() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio54() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio55() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio56() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio57() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio58() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio59() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio60() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio61() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio62() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio63() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio64() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio65() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio66() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio67() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio68() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio69() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio70() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio71() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio35() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}


void CNrLinkageDlg::OnRadio36() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio37() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio38() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio39() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}


void CNrLinkageDlg::OnRadio73() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio74() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}


void CNrLinkageDlg::OnRadio93() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}


void CNrLinkageDlg::OnRadio94() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio95() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio96() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio97() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio98() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio99() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio100() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}


void CNrLinkageDlg::OnRadio85() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}
void CNrLinkageDlg::OnRadio86() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}


void CNrLinkageDlg::OnRadio101() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio102() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio103() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio104() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio105() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio106() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio107() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio108() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio109() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio110() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio111() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio112() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio113() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}
void CNrLinkageDlg::OnRadio114() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio115() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio116() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio117() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio118() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio119() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio120() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}


void CNrLinkageDlg::OnRadio121() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio122() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio123() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio124() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio125() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio126() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio127() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio128() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio129() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio130() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}


void CNrLinkageDlg::OnRadio131() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio132() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio133() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio134() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio135() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio136() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CNrLinkageDlg::OnRadio137() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}





void CNrLinkageDlg::GetDataValue(char* pbuf, int* size)
{
	if (NULL == pbuf || *size < 9 * sizeof(NR_LINKAGE))
	{
		return;
	}

	memcpy(pbuf, m_linkage, 9 * sizeof(NR_LINKAGE));
	*size = 9 * sizeof(NR_LINKAGE);
}


void CNrLinkageDlg::SetDataValue(char* pbuf, int size)
{
	if (NULL == pbuf || size < 9 * sizeof(NR_LINKAGE))
	{
		return;
	}

	memcpy(m_linkage, pbuf, 9 * sizeof(NR_LINKAGE));
}
