// NrLutDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "NrLutDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CNrLutDlg dialog


CNrLutDlg::CNrLutDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNrLutDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNrLutDlg)
	m_lutmode = 0;
	//}}AFX_DATA_INIT
}


void CNrLutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNrLutDlg)
	DDX_Radio(pDX, IDC_RADIO0, m_lutmode);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNrLutDlg, CDialog)
	//{{AFX_MSG_MAP(CNrLutDlg)
		ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_OK, OnButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnButtonCancel)
	ON_BN_CLICKED(IDC_RADIO0, OnRadio0)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_BN_CLICKED(IDC_RADIO3, OnRadio3)
	ON_BN_CLICKED(IDC_RADIO4, OnRadio4)
	ON_BN_CLICKED(IDC_RADIO5, OnRadio5)
	ON_BN_CLICKED(IDC_RADIO6, OnRadio6)
	ON_BN_CLICKED(IDC_RADIO7, OnRadio7)
	ON_BN_CLICKED(IDC_RADIO8, OnRadio8)
	ON_BN_CLICKED(IDC_RADIO9, OnRadio9)
	ON_BN_CLICKED(IDC_RADIO10, OnRadio10)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNrLutDlg message handlers
BOOL CNrLutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect clientRect, rect;
	
	this->GetClientRect(&clientRect); 

	m_CurveDlg.Create(IDD_DIALOG_CURVE, this);
	m_CurveDlg.GetClientRect(&rect); 
	m_CurveDlg.SetEnable(TRUE);
	//m_CurveDlg.SetLevelNum(NR_LUT_LEVEL_NUM);

	rect.left = clientRect.left + 30;
	rect.top = clientRect.top + 60;
	rect.right += 30;
	rect.bottom += 60;

	m_CurveDlg.MoveWindow(&rect);
	m_CurveDlg.ShowWindow(SW_SHOW);

	CWnd * pCurveName = m_CurveDlg.GetDlgItem(IDC_CURVE_NAME);
	
	pCurveName->SetWindowText("Nr_lut");

	SetKeyPts();

	return TRUE;
}


void CNrLutDlg::SetKeyPts(void) 
{
	vector<CPoint> *pts_tmp = NULL;
	int i = 0;
	int size = m_lut_keyPts[m_lutmode].size();

	pts_tmp = m_CurveDlg.GetKeyPts();
	(*pts_tmp).resize(size);
	for (i=0; i<size; i++)
	{
		(*pts_tmp)[i].x = m_lut_keyPts[m_lutmode][i].x;
		(*pts_tmp)[i].y = m_lut_keyPts[m_lutmode][i].y;
	}

	m_CurveDlg.Refresh();
}

void CNrLutDlg::SaveKeyPts(void) 
{
	vector<CPoint> *pts_tmp = NULL;
	int i = 0;
	int size = 0;

	pts_tmp = m_CurveDlg.GetKeyPts();
	size = (*pts_tmp).size();

	m_lut_keyPts[m_lutmode].resize(size);

	for (i=0; i<size; i++)
	{
		m_lut_keyPts[m_lutmode][i].x = (*pts_tmp)[i].x;
		m_lut_keyPts[m_lutmode][i].y = (*pts_tmp)[i].y;
	}

}

void CNrLutDlg::ChangeCurve(void) 
{
	int size = NR_LUT_LEVEL_NUM * sizeof(unsigned short);
	m_CurveDlg.GetLevel((char*)m_lc_lut[m_lutmode], &size);


	SaveKeyPts();

	UpdateData(TRUE);
	
	SetKeyPts();
}


void CNrLutDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	m_CurveDlg.OnClose();

	CDialog::OnClose();
}

void CNrLutDlg::OnButtonOk() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
	int size = NR_LUT_LEVEL_NUM * sizeof(unsigned short);
	m_CurveDlg.GetLevel((char*)m_lc_lut[m_lutmode], &size);
	SaveKeyPts();
	
	OnClose();
}

void CNrLutDlg::OnButtonCancel() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
	OnClose();
}

void CNrLutDlg::OnRadio0() 
{
	// TODO: Add your control notification handler code here
	ChangeCurve();
}

void CNrLutDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	ChangeCurve();
}

void CNrLutDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	ChangeCurve();
}

void CNrLutDlg::OnRadio3() 
{
	// TODO: Add your control notification handler code here
	ChangeCurve();
}

void CNrLutDlg::OnRadio4() 
{
	// TODO: Add your control notification handler code here
	ChangeCurve();
}

void CNrLutDlg::OnRadio5() 
{
	// TODO: Add your control notification handler code here
	ChangeCurve();
}

void CNrLutDlg::OnRadio6() 
{
	// TODO: Add your control notification handler code here
	ChangeCurve();
}

void CNrLutDlg::OnRadio7() 
{
	// TODO: Add your control notification handler code here
	ChangeCurve();
}

void CNrLutDlg::OnRadio8() 
{
	// TODO: Add your control notification handler code here
	ChangeCurve();
}

void CNrLutDlg::OnRadio9() 
{
	// TODO: Add your control notification handler code here
	ChangeCurve();
}

void CNrLutDlg::OnRadio10() 
{
	// TODO: Add your control notification handler code here
	ChangeCurve();
}
