// FcsLinkageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "FcsLinkageDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFcsLinkageDlg dialog


CFcsLinkageDlg::CFcsLinkageDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFcsLinkageDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFcsLinkageDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	ZeroMemory(m_Enable, sizeof(m_Enable));
	ZeroMemory(m_uv_Enable, sizeof(m_uv_Enable));
}


void CFcsLinkageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFcsLinkageDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

	DDX_Radio(pDX, IDC_RADIO1, m_Enable[0]);
	DDX_Radio(pDX, IDC_RADIO33, m_Enable[1]);
	DDX_Radio(pDX, IDC_RADIO35, m_Enable[2]);
	DDX_Radio(pDX, IDC_RADIO37, m_Enable[3]);
	DDX_Radio(pDX, IDC_RADIO39, m_Enable[4]);
	DDX_Radio(pDX, IDC_RADIO73, m_Enable[5]);
	DDX_Radio(pDX, IDC_RADIO75, m_Enable[6]);
	DDX_Radio(pDX, IDC_RADIO79, m_Enable[7]);
	DDX_Radio(pDX, IDC_RADIO83, m_Enable[8]);
	
	DDX_Radio(pDX, IDC_RADIO3, m_uv_Enable[0]);
	DDX_Radio(pDX, IDC_RADIO5, m_uv_Enable[1]);
	DDX_Radio(pDX, IDC_RADIO7, m_uv_Enable[2]);
	DDX_Radio(pDX, IDC_RADIO10, m_uv_Enable[3]);
	DDX_Radio(pDX, IDC_RADIO19, m_uv_Enable[4]);
	DDX_Radio(pDX, IDC_RADIO29, m_uv_Enable[5]);
	DDX_Radio(pDX, IDC_RADIO77, m_uv_Enable[6]);
	DDX_Radio(pDX, IDC_RADIO81, m_uv_Enable[7]);
	DDX_Radio(pDX, IDC_RADIO85, m_uv_Enable[8]);

	DDX_Control(pDX, IDC_SPIN1, m_th[0]);
	DDX_Control(pDX, IDC_SPIN4, m_th[1]);
	DDX_Control(pDX, IDC_SPIN7, m_th[2]);
	DDX_Control(pDX, IDC_SPIN10, m_th[3]);
	DDX_Control(pDX, IDC_SPIN19, m_th[4]);
	DDX_Control(pDX, IDC_SPIN24, m_th[5]);
	DDX_Control(pDX, IDC_SPIN146, m_th[6]);
	DDX_Control(pDX, IDC_SPIN34, m_th[7]);
	DDX_Control(pDX, IDC_SPIN39, m_th[8]);

	DDX_Control(pDX, IDC_SPIN2, m_gain_slop[0]);
	DDX_Control(pDX, IDC_SPIN140, m_gain_slop[1]);
	DDX_Control(pDX, IDC_SPIN141, m_gain_slop[2]);
	DDX_Control(pDX, IDC_SPIN99, m_gain_slop[3]);
	DDX_Control(pDX, IDC_SPIN144, m_gain_slop[4]);
	DDX_Control(pDX, IDC_SPIN145, m_gain_slop[5]);
	DDX_Control(pDX, IDC_SPIN30, m_gain_slop[6]);
	DDX_Control(pDX, IDC_SPIN148, m_gain_slop[7]);
	DDX_Control(pDX, IDC_SPIN149, m_gain_slop[8]);

	DDX_Control(pDX, IDC_SPIN3, m_uv_nr_th[0]);
	DDX_Control(pDX, IDC_SPIN6, m_uv_nr_th[1]);
	DDX_Control(pDX, IDC_SPIN9, m_uv_nr_th[2]);
	DDX_Control(pDX, IDC_SPIN143, m_uv_nr_th[3]);
	DDX_Control(pDX, IDC_SPIN21, m_uv_nr_th[4]);
	DDX_Control(pDX, IDC_SPIN26, m_uv_nr_th[5]);
	DDX_Control(pDX, IDC_SPIN147, m_uv_nr_th[6]);
	DDX_Control(pDX, IDC_SPIN36, m_uv_nr_th[7]);
	DDX_Control(pDX, IDC_SPIN49, m_uv_nr_th[8]);
}


BEGIN_MESSAGE_MAP(CFcsLinkageDlg, CDialog)
	//{{AFX_MSG_MAP(CFcsLinkageDlg)
	ON_BN_CLICKED(IDC_BUTTON_OK, OnButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnButtonCancel)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_BN_CLICKED(IDC_RADIO3, OnRadio3)
	ON_BN_CLICKED(IDC_RADIO4, OnRadio4)
	ON_BN_CLICKED(IDC_RADIO33, OnRadio33)
	ON_BN_CLICKED(IDC_RADIO34, OnRadio34)
	ON_BN_CLICKED(IDC_RADIO5, OnRadio5)
	ON_BN_CLICKED(IDC_RADIO6, OnRadio6)
	ON_BN_CLICKED(IDC_RADIO35, OnRadio35)
	ON_BN_CLICKED(IDC_RADIO36, OnRadio36)
	ON_BN_CLICKED(IDC_RADIO7, OnRadio7)
	ON_BN_CLICKED(IDC_RADIO8, OnRadio8)
	ON_BN_CLICKED(IDC_RADIO37, OnRadio37)
	ON_BN_CLICKED(IDC_RADIO38, OnRadio38)
	ON_BN_CLICKED(IDC_RADIO10, OnRadio10)
	ON_BN_CLICKED(IDC_RADIO11, OnRadio11)
	ON_BN_CLICKED(IDC_RADIO39, OnRadio39)
	ON_BN_CLICKED(IDC_RADIO72, OnRadio72)
	ON_BN_CLICKED(IDC_RADIO19, OnRadio19)
	ON_BN_CLICKED(IDC_RADIO20, OnRadio20)
	ON_BN_CLICKED(IDC_RADIO73, OnRadio73)
	ON_BN_CLICKED(IDC_RADIO74, OnRadio74)
	ON_BN_CLICKED(IDC_RADIO29, OnRadio29)
	ON_BN_CLICKED(IDC_RADIO30, OnRadio30)
	ON_BN_CLICKED(IDC_RADIO75, OnRadio75)
	ON_BN_CLICKED(IDC_RADIO76, OnRadio76)
	ON_BN_CLICKED(IDC_RADIO77, OnRadio77)
	ON_BN_CLICKED(IDC_RADIO78, OnRadio78)
	ON_BN_CLICKED(IDC_RADIO79, OnRadio79)
	ON_BN_CLICKED(IDC_RADIO80, OnRadio80)
	ON_BN_CLICKED(IDC_RADIO81, OnRadio81)
	ON_BN_CLICKED(IDC_RADIO82, OnRadio82)
	ON_BN_CLICKED(IDC_RADIO83, OnRadio83)
	ON_BN_CLICKED(IDC_RADIO84, OnRadio84)
	ON_BN_CLICKED(IDC_RADIO85, OnRadio85)
	ON_BN_CLICKED(IDC_RADIO86, OnRadio86)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFcsLinkageDlg message handlers
BOOL CFcsLinkageDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	int i = 0;

	for (i=0; i<9; i++)
	{
		m_th[i].SetRange(0,255);
		m_gain_slop[i].SetRange(0,63);
		m_uv_nr_th[i].SetRange(0,1023);
	}

	SetDataValue();

	return TRUE;
}

BOOL CFcsLinkageDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}


void CFcsLinkageDlg::OnButtonOk() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();

	GetDataValue();
}

void CFcsLinkageDlg::OnButtonCancel() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}

void CFcsLinkageDlg::GetDataValue(void)
{
	UpdateData(TRUE);

	int i = 0;

	for (i=0; i<9; i++)
	{
		m_linkage[i].fcs_th = (T_U16)m_th[i].GetPos();
		m_linkage[i].fcs_gain_slop = (T_U16)m_gain_slop[i].GetPos();
		m_linkage[i].fcs_uv_nr_th = (T_U16)m_uv_nr_th[i].GetPos();
		m_linkage[i].fcs_enable = (T_U16)m_Enable[i];
		m_linkage[i].fcs_uv_nr_enable = (T_U16)m_uv_Enable[i];
	}

}

void CFcsLinkageDlg::SetDataValue(void)
{
	int i = 0;

	for (i=0; i<9; i++)
	{
		m_th[i].SetPos(m_linkage[i].fcs_th);
		m_gain_slop[i].SetPos(m_linkage[i].fcs_gain_slop);
		m_uv_nr_th[i].SetPos(m_linkage[i].fcs_uv_nr_th);

		m_Enable[i] = m_linkage[i].fcs_enable;
		m_uv_Enable[i] = m_linkage[i].fcs_uv_nr_enable;
	}

	UpdateData(FALSE);
}

void CFcsLinkageDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio3() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio4() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio33() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio34() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio5() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio6() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio35() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio36() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio7() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio8() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio37() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio38() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio10() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio11() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio39() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio72() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio19() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio20() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio73() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio74() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio29() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio30() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio75() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio76() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio77() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio78() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio79() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio80() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio81() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio82() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio83() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio84() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio85() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsLinkageDlg::OnRadio86() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}
