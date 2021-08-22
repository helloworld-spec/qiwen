// CCM_RGBvalDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "CCM_RGBvalDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCCM_RGBvalDlg dialog

T_U8 g_R_tar_standard[4][6] = {115, 194, 98, 87, 133, 103, 214, 80, 193, 94, 157, 224, 56, 70, 175, 231, 187, 8, 243, 200, 160, 122, 85, 52};
T_U8 g_G_tar_standard[4][6] = {82, 150, 122, 108, 128, 189, 126, 91, 90, 60, 188, 163, 61, 148, 54, 199, 86, 133, 243, 200, 160, 122, 85, 52};
T_U8 g_B_tar_standard[4][6] = {68, 130, 157, 67, 177, 170, 44, 166, 99, 108, 64, 46, 150, 73, 60, 31, 149, 161, 242, 200, 160, 121, 85, 52};


CCCM_RGBvalDlg::CCCM_RGBvalDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCCM_RGBvalDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCCM_RGBvalDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	ZeroMemory(R_in, sizeof(R_in));
	ZeroMemory(R_tar, sizeof(R_tar));
	ZeroMemory(R_out, sizeof(R_out));

	ZeroMemory(G_in, sizeof(G_in));
	ZeroMemory(G_tar, sizeof(G_tar));
	ZeroMemory(G_out, sizeof(G_out));

	ZeroMemory(B_in, sizeof(B_in));
	ZeroMemory(B_tar, sizeof(B_tar));
	ZeroMemory(B_out, sizeof(B_out));

	memcpy(R_tar, g_R_tar_standard, 4*6);
	memcpy(G_tar, g_G_tar_standard, 4*6);
	memcpy(B_tar, g_B_tar_standard, 4*6);
}


void CCCM_RGBvalDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCCM_RGBvalDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP


	/********************R input value************************/
	DDX_Text(pDX, IDC_EDIT_R_Input_0, R_in[0][0]);
	DDV_MinMaxInt(pDX, R_in[0][0], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Input_1, R_in[0][1]);
	DDV_MinMaxInt(pDX, R_in[0][1], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Input_2, R_in[0][2]);
	DDV_MinMaxInt(pDX, R_in[0][2], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Input_3, R_in[0][3]);
	DDV_MinMaxInt(pDX, R_in[0][3], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Input_4, R_in[0][4]);
	DDV_MinMaxInt(pDX, R_in[0][4], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Input_5, R_in[0][5]);
	DDV_MinMaxInt(pDX, R_in[0][5], 0, 255);

	DDX_Text(pDX, IDC_EDIT_R_Input_6, R_in[1][0]);
	DDV_MinMaxInt(pDX, R_in[1][0], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Input_7, R_in[1][1]);
	DDV_MinMaxInt(pDX, R_in[1][1], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Input_8, R_in[1][2]);
	DDV_MinMaxInt(pDX, R_in[1][2], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Input_9, R_in[1][3]);
	DDV_MinMaxInt(pDX, R_in[1][3], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Input_10, R_in[1][4]);
	DDV_MinMaxInt(pDX, R_in[1][4], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Input_11, R_in[1][5]);
	DDV_MinMaxInt(pDX, R_in[1][5], 0, 255);

	DDX_Text(pDX, IDC_EDIT_R_Input_12, R_in[2][0]);
	DDV_MinMaxInt(pDX, R_in[2][0], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Input_13, R_in[2][1]);
	DDV_MinMaxInt(pDX, R_in[2][1], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Input_14, R_in[2][2]);
	DDV_MinMaxInt(pDX, R_in[2][2], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Input_15, R_in[2][3]);
	DDV_MinMaxInt(pDX, R_in[2][3], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Input_16, R_in[2][4]);
	DDV_MinMaxInt(pDX, R_in[2][4], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Input_17, R_in[2][5]);
	DDV_MinMaxInt(pDX, R_in[2][5], 0, 255);

	DDX_Text(pDX, IDC_EDIT_R_Input_18, R_in[3][0]);
	DDV_MinMaxInt(pDX, R_in[3][0], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Input_19, R_in[3][1]);
	DDV_MinMaxInt(pDX, R_in[3][1], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Input_20, R_in[3][2]);
	DDV_MinMaxInt(pDX, R_in[3][2], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Input_21, R_in[3][3]);
	DDV_MinMaxInt(pDX, R_in[3][3], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Input_22, R_in[3][4]);
	DDV_MinMaxInt(pDX, R_in[3][4], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Input_23, R_in[3][5]);
	DDV_MinMaxInt(pDX, R_in[3][5], 0, 255);



	/********************G input value************************/
	DDX_Text(pDX, IDC_EDIT_G_Input_0, G_in[0][0]);
	DDV_MinMaxInt(pDX, G_in[0][0], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Input_1, G_in[0][1]);
	DDV_MinMaxInt(pDX, G_in[0][1], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Input_2, G_in[0][2]);
	DDV_MinMaxInt(pDX, G_in[0][2], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Input_3, G_in[0][3]);
	DDV_MinMaxInt(pDX, G_in[0][3], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Input_4, G_in[0][4]);
	DDV_MinMaxInt(pDX, G_in[0][4], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Input_5, G_in[0][5]);
	DDV_MinMaxInt(pDX, G_in[0][5], 0, 255);

	DDX_Text(pDX, IDC_EDIT_G_Input_6, G_in[1][0]);
	DDV_MinMaxInt(pDX, G_in[1][0], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Input_7, G_in[1][1]);
	DDV_MinMaxInt(pDX, G_in[1][1], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Input_8, G_in[1][2]);
	DDV_MinMaxInt(pDX, G_in[1][2], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Input_9, G_in[1][3]);
	DDV_MinMaxInt(pDX, G_in[1][3], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Input_10, G_in[1][4]);
	DDV_MinMaxInt(pDX, G_in[1][4], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Input_11, G_in[1][5]);
	DDV_MinMaxInt(pDX, G_in[1][5], 0, 255);

	DDX_Text(pDX, IDC_EDIT_G_Input_12, G_in[2][0]);
	DDV_MinMaxInt(pDX, G_in[2][0], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Input_13, G_in[2][1]);
	DDV_MinMaxInt(pDX, G_in[2][1], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Input_14, G_in[2][2]);
	DDV_MinMaxInt(pDX, G_in[2][2], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Input_15, G_in[2][3]);
	DDV_MinMaxInt(pDX, G_in[2][3], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Input_16, G_in[2][4]);
	DDV_MinMaxInt(pDX, G_in[2][4], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Input_17, G_in[2][5]);
	DDV_MinMaxInt(pDX, G_in[2][5], 0, 255);

	DDX_Text(pDX, IDC_EDIT_G_Input_18, G_in[3][0]);
	DDV_MinMaxInt(pDX, G_in[3][0], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Input_19, G_in[3][1]);
	DDV_MinMaxInt(pDX, G_in[3][1], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Input_20, G_in[3][2]);
	DDV_MinMaxInt(pDX, G_in[3][2], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Input_21, G_in[3][3]);
	DDV_MinMaxInt(pDX, G_in[3][3], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Input_22, G_in[3][4]);
	DDV_MinMaxInt(pDX, G_in[3][4], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Input_23, G_in[3][5]);
	DDV_MinMaxInt(pDX, G_in[3][5], 0, 255);


	/********************B input value************************/
	DDX_Text(pDX, IDC_EDIT_B_Input_0, B_in[0][0]);
	DDV_MinMaxInt(pDX, B_in[0][0], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Input_1, B_in[0][1]);
	DDV_MinMaxInt(pDX, B_in[0][1], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Input_2, B_in[0][2]);
	DDV_MinMaxInt(pDX, B_in[0][2], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Input_3, B_in[0][3]);
	DDV_MinMaxInt(pDX, B_in[0][3], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Input_4, B_in[0][4]);
	DDV_MinMaxInt(pDX, B_in[0][4], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Input_5, B_in[0][5]);
	DDV_MinMaxInt(pDX, B_in[0][5], 0, 255);

	DDX_Text(pDX, IDC_EDIT_B_Input_6, B_in[1][0]);
	DDV_MinMaxInt(pDX, B_in[1][0], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Input_7, B_in[1][1]);
	DDV_MinMaxInt(pDX, B_in[1][1], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Input_8, B_in[1][2]);
	DDV_MinMaxInt(pDX, B_in[1][2], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Input_9, B_in[1][3]);
	DDV_MinMaxInt(pDX, B_in[1][3], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Input_10, B_in[1][4]);
	DDV_MinMaxInt(pDX, B_in[1][4], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Input_11, B_in[1][5]);
	DDV_MinMaxInt(pDX, B_in[1][5], 0, 255);

	DDX_Text(pDX, IDC_EDIT_B_Input_12, B_in[2][0]);
	DDV_MinMaxInt(pDX, B_in[2][0], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Input_13, B_in[2][1]);
	DDV_MinMaxInt(pDX, B_in[2][1], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Input_14, B_in[2][2]);
	DDV_MinMaxInt(pDX, B_in[2][2], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Input_15, B_in[2][3]);
	DDV_MinMaxInt(pDX, B_in[2][3], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Input_16, B_in[2][4]);
	DDV_MinMaxInt(pDX, B_in[2][4], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Input_17, B_in[2][5]);
	DDV_MinMaxInt(pDX, B_in[2][5], 0, 255);

	DDX_Text(pDX, IDC_EDIT_B_Input_18, B_in[3][0]);
	DDV_MinMaxInt(pDX, B_in[3][0], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Input_19, B_in[3][1]);
	DDV_MinMaxInt(pDX, B_in[3][1], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Input_20, B_in[3][2]);
	DDV_MinMaxInt(pDX, B_in[3][2], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Input_21, B_in[3][3]);
	DDV_MinMaxInt(pDX, B_in[3][3], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Input_22, B_in[3][4]);
	DDV_MinMaxInt(pDX, B_in[3][4], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Input_23, B_in[3][5]);
	DDV_MinMaxInt(pDX, B_in[3][5], 0, 255);

	/********************R target value************************/
	DDX_Text(pDX, IDC_EDIT_R_Target_0, R_tar[0][0]);
	DDV_MinMaxInt(pDX, R_tar[0][0], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Target_1, R_tar[0][1]);
	DDV_MinMaxInt(pDX, R_tar[0][1], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Target_2, R_tar[0][2]);
	DDV_MinMaxInt(pDX, R_tar[0][2], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Target_3, R_tar[0][3]);
	DDV_MinMaxInt(pDX, R_tar[0][3], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Target_4, R_tar[0][4]);
	DDV_MinMaxInt(pDX, R_tar[0][4], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Target_5, R_tar[0][5]);
	DDV_MinMaxInt(pDX, R_tar[0][5], 0, 255);

	DDX_Text(pDX, IDC_EDIT_R_Target_6, R_tar[1][0]);
	DDV_MinMaxInt(pDX, R_tar[1][0], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Target_7, R_tar[1][1]);
	DDV_MinMaxInt(pDX, R_tar[1][1], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Target_8, R_tar[1][2]);
	DDV_MinMaxInt(pDX, R_tar[1][2], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Target_9, R_tar[1][3]);
	DDV_MinMaxInt(pDX, R_tar[1][3], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Target_10, R_tar[1][4]);
	DDV_MinMaxInt(pDX, R_tar[1][4], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Target_11, R_tar[1][5]);
	DDV_MinMaxInt(pDX, R_tar[1][5], 0, 255);

	DDX_Text(pDX, IDC_EDIT_R_Target_12, R_tar[2][0]);
	DDV_MinMaxInt(pDX, R_tar[2][0], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Target_13, R_tar[2][1]);
	DDV_MinMaxInt(pDX, R_tar[2][1], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Target_14, R_tar[2][2]);
	DDV_MinMaxInt(pDX, R_tar[2][2], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Target_15, R_tar[2][3]);
	DDV_MinMaxInt(pDX, R_tar[2][3], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Target_16, R_tar[2][4]);
	DDV_MinMaxInt(pDX, R_tar[2][4], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Target_17, R_tar[2][5]);
	DDV_MinMaxInt(pDX, R_tar[2][5], 0, 255);

	DDX_Text(pDX, IDC_EDIT_R_Target_18, R_tar[3][0]);
	DDV_MinMaxInt(pDX, R_tar[3][0], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Target_19, R_tar[3][1]);
	DDV_MinMaxInt(pDX, R_tar[3][1], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Target_20, R_tar[3][2]);
	DDV_MinMaxInt(pDX, R_tar[3][2], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Target_21, R_tar[3][3]);
	DDV_MinMaxInt(pDX, R_tar[3][3], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Target_22, R_tar[3][4]);
	DDV_MinMaxInt(pDX, R_tar[3][4], 0, 255);
	DDX_Text(pDX, IDC_EDIT_R_Target_23, R_tar[3][5]);
	DDV_MinMaxInt(pDX, R_tar[3][5], 0, 255);



	/********************G target value************************/
	DDX_Text(pDX, IDC_EDIT_G_Target_0, G_tar[0][0]);
	DDV_MinMaxInt(pDX, G_tar[0][0], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Target_1, G_tar[0][1]);
	DDV_MinMaxInt(pDX, G_tar[0][1], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Target_2, G_tar[0][2]);
	DDV_MinMaxInt(pDX, G_tar[0][2], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Target_3, G_tar[0][3]);
	DDV_MinMaxInt(pDX, G_tar[0][3], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Target_4, G_tar[0][4]);
	DDV_MinMaxInt(pDX, G_tar[0][4], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Target_5, G_tar[0][5]);
	DDV_MinMaxInt(pDX, G_tar[0][5], 0, 255);

	DDX_Text(pDX, IDC_EDIT_G_Target_6, G_tar[1][0]);
	DDV_MinMaxInt(pDX, G_tar[1][0], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Target_7, G_tar[1][1]);
	DDV_MinMaxInt(pDX, G_tar[1][1], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Target_8, G_tar[1][2]);
	DDV_MinMaxInt(pDX, G_tar[1][2], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Target_9, G_tar[1][3]);
	DDV_MinMaxInt(pDX, G_tar[1][3], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Target_10, G_tar[1][4]);
	DDV_MinMaxInt(pDX, G_tar[1][4], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Target_11, G_tar[1][5]);
	DDV_MinMaxInt(pDX, G_tar[1][5], 0, 255);

	DDX_Text(pDX, IDC_EDIT_G_Target_12, G_tar[2][0]);
	DDV_MinMaxInt(pDX, G_tar[2][0], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Target_13, G_tar[2][1]);
	DDV_MinMaxInt(pDX, G_tar[2][1], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Target_14, G_tar[2][2]);
	DDV_MinMaxInt(pDX, G_tar[2][2], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Target_15, G_tar[2][3]);
	DDV_MinMaxInt(pDX, G_tar[2][3], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Target_16, G_tar[2][4]);
	DDV_MinMaxInt(pDX, G_tar[2][4], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Target_17, G_tar[2][5]);
	DDV_MinMaxInt(pDX, G_tar[2][5], 0, 255);

	DDX_Text(pDX, IDC_EDIT_G_Target_18, G_tar[3][0]);
	DDV_MinMaxInt(pDX, G_tar[3][0], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Target_19, G_tar[3][1]);
	DDV_MinMaxInt(pDX, G_tar[3][1], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Target_20, G_tar[3][2]);
	DDV_MinMaxInt(pDX, G_tar[3][2], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Target_21, G_tar[3][3]);
	DDV_MinMaxInt(pDX, G_tar[3][3], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Target_22, G_tar[3][4]);
	DDV_MinMaxInt(pDX, G_tar[3][4], 0, 255);
	DDX_Text(pDX, IDC_EDIT_G_Target_23, G_tar[3][5]);
	DDV_MinMaxInt(pDX, G_tar[3][5], 0, 255);


	/********************B target value************************/
	DDX_Text(pDX, IDC_EDIT_B_Target_0, B_tar[0][0]);
	DDV_MinMaxInt(pDX, B_tar[0][0], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Target_1, B_tar[0][1]);
	DDV_MinMaxInt(pDX, B_tar[0][1], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Target_2, B_tar[0][2]);
	DDV_MinMaxInt(pDX, B_tar[0][2], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Target_3, B_tar[0][3]);
	DDV_MinMaxInt(pDX, B_tar[0][3], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Target_4, B_tar[0][4]);
	DDV_MinMaxInt(pDX, B_tar[0][4], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Target_5, B_tar[0][5]);
	DDV_MinMaxInt(pDX, B_tar[0][5], 0, 255);

	DDX_Text(pDX, IDC_EDIT_B_Target_6, B_tar[1][0]);
	DDV_MinMaxInt(pDX, B_tar[1][0], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Target_7, B_tar[1][1]);
	DDV_MinMaxInt(pDX, B_tar[1][1], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Target_8, B_tar[1][2]);
	DDV_MinMaxInt(pDX, B_tar[1][2], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Target_9, B_tar[1][3]);
	DDV_MinMaxInt(pDX, B_tar[1][3], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Target_10, B_tar[1][4]);
	DDV_MinMaxInt(pDX, B_tar[1][4], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Target_11, B_tar[1][5]);
	DDV_MinMaxInt(pDX, B_tar[1][5], 0, 255);

	DDX_Text(pDX, IDC_EDIT_B_Target_12, B_tar[2][0]);
	DDV_MinMaxInt(pDX, B_tar[2][0], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Target_13, B_tar[2][1]);
	DDV_MinMaxInt(pDX, B_tar[2][1], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Target_14, B_tar[2][2]);
	DDV_MinMaxInt(pDX, B_tar[2][2], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Target_15, B_tar[2][3]);
	DDV_MinMaxInt(pDX, B_tar[2][3], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Target_16, B_tar[2][4]);
	DDV_MinMaxInt(pDX, B_tar[2][4], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Target_17, B_tar[2][5]);
	DDV_MinMaxInt(pDX, B_tar[2][5], 0, 255);

	DDX_Text(pDX, IDC_EDIT_B_Target_18, B_tar[3][0]);
	DDV_MinMaxInt(pDX, B_tar[3][0], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Target_19, B_tar[3][1]);
	DDV_MinMaxInt(pDX, B_tar[3][1], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Target_20, B_tar[3][2]);
	DDV_MinMaxInt(pDX, B_tar[3][2], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Target_21, B_tar[3][3]);
	DDV_MinMaxInt(pDX, B_tar[3][3], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Target_22, B_tar[3][4]);
	DDV_MinMaxInt(pDX, B_tar[3][4], 0, 255);
	DDX_Text(pDX, IDC_EDIT_B_Target_23, B_tar[3][5]);
	DDV_MinMaxInt(pDX, B_tar[3][5], 0, 255);
}


BEGIN_MESSAGE_MAP(CCCM_RGBvalDlg, CDialog)
	//{{AFX_MSG_MAP(CCCM_RGBvalDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCCM_RGBvalDlg message handlers
BOOL CCCM_RGBvalDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetDataValue();

	return AK_TRUE;
}

BOOL CCCM_RGBvalDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}



void CCCM_RGBvalDlg::SetDataValue(void)
{
	CWnd *pWnd = NULL;
	CString str;

	UpdateData(FALSE);

	/*****************input R********************/
#if 0
	pWnd = GetDlgItem(IDC_STATIC_R_Input_0);
	str.Format("%d", R_in[0][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Input_1);
	str.Format("%d", R_in[0][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Input_2);
	str.Format("%d", R_in[0][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Input_3);
	str.Format("%d", R_in[0][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Input_4);
	str.Format("%d", R_in[0][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Input_5);
	str.Format("%d", R_in[0][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_R_Input_6);
	str.Format("%d", R_in[1][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Input_7);
	str.Format("%d", R_in[1][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Input_8);
	str.Format("%d", R_in[1][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Input_9);
	str.Format("%d", R_in[1][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Input_10);
	str.Format("%d", R_in[1][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Input_11);
	str.Format("%d", R_in[1][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_R_Input_12);
	str.Format("%d", R_in[2][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Input_13);
	str.Format("%d", R_in[2][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Input_14);
	str.Format("%d", R_in[2][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Input_15);
	str.Format("%d", R_in[2][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Input_16);
	str.Format("%d", R_in[2][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Input_17);
	str.Format("%d", R_in[2][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_R_Input_18);
	str.Format("%d", R_in[3][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Input_19);
	str.Format("%d", R_in[3][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Input_20);
	str.Format("%d", R_in[3][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Input_21);
	str.Format("%d", R_in[3][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Input_22);
	str.Format("%d", R_in[3][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Input_23);
	str.Format("%d", R_in[3][5]);
	pWnd->SetWindowText(str);

	/*****************input G********************/

	pWnd = GetDlgItem(IDC_STATIC_G_Input_0);
	str.Format("%d", G_in[0][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Input_1);
	str.Format("%d", G_in[0][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Input_2);
	str.Format("%d", G_in[0][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Input_3);
	str.Format("%d", G_in[0][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Input_4);
	str.Format("%d", G_in[0][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Input_5);
	str.Format("%d", G_in[0][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_G_Input_6);
	str.Format("%d", G_in[1][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Input_7);
	str.Format("%d", G_in[1][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Input_8);
	str.Format("%d", G_in[1][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Input_9);
	str.Format("%d", G_in[1][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Input_10);
	str.Format("%d", G_in[1][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Input_11);
	str.Format("%d", G_in[1][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_G_Input_12);
	str.Format("%d", G_in[2][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Input_13);
	str.Format("%d", G_in[2][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Input_14);
	str.Format("%d", G_in[2][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Input_15);
	str.Format("%d", G_in[2][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Input_16);
	str.Format("%d", G_in[2][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Input_17);
	str.Format("%d", G_in[2][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_G_Input_18);
	str.Format("%d", G_in[3][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Input_19);
	str.Format("%d", G_in[3][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Input_20);
	str.Format("%d", G_in[3][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Input_21);
	str.Format("%d", G_in[3][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Input_22);
	str.Format("%d", G_in[3][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Input_23);
	str.Format("%d", G_in[3][5]);
	pWnd->SetWindowText(str);

	/*****************input B********************/

	pWnd = GetDlgItem(IDC_STATIC_B_Input_0);
	str.Format("%d", B_in[0][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Input_1);
	str.Format("%d", B_in[0][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Input_2);
	str.Format("%d", B_in[0][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Input_3);
	str.Format("%d", B_in[0][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Input_4);
	str.Format("%d", B_in[0][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Input_5);
	str.Format("%d", B_in[0][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_B_Input_6);
	str.Format("%d", B_in[1][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Input_7);
	str.Format("%d", B_in[1][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Input_8);
	str.Format("%d", B_in[1][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Input_9);
	str.Format("%d", B_in[1][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Input_10);
	str.Format("%d", B_in[1][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Input_11);
	str.Format("%d", B_in[1][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_B_Input_12);
	str.Format("%d", B_in[2][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Input_13);
	str.Format("%d", B_in[2][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Input_14);
	str.Format("%d", B_in[2][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Input_15);
	str.Format("%d", B_in[2][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Input_16);
	str.Format("%d", B_in[2][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Input_17);
	str.Format("%d", B_in[2][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_B_Input_18);
	str.Format("%d", B_in[3][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Input_19);
	str.Format("%d", B_in[3][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Input_20);
	str.Format("%d", B_in[3][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Input_21);
	str.Format("%d", B_in[3][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Input_22);
	str.Format("%d", B_in[3][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Input_23);
	str.Format("%d", B_in[3][5]);
	pWnd->SetWindowText(str);
#endif

	/*****************output R********************/


	pWnd = GetDlgItem(IDC_STATIC_R_Output_0);
	str.Format("%d", R_out[0][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Output_1);
	str.Format("%d", R_out[0][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Output_2);
	str.Format("%d", R_out[0][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Output_3);
	str.Format("%d", R_out[0][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Output_4);
	str.Format("%d", R_out[0][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Output_5);
	str.Format("%d", R_out[0][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_R_Output_6);
	str.Format("%d", R_out[1][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Output_7);
	str.Format("%d", R_out[1][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Output_8);
	str.Format("%d", R_out[1][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Output_9);
	str.Format("%d", R_out[1][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Output_10);
	str.Format("%d", R_out[1][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Output_11);
	str.Format("%d", R_out[1][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_R_Output_12);
	str.Format("%d", R_out[2][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Output_13);
	str.Format("%d", R_out[2][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Output_14);
	str.Format("%d", R_out[2][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Output_15);
	str.Format("%d", R_out[2][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Output_16);
	str.Format("%d", R_out[2][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Output_17);
	str.Format("%d", R_out[2][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_R_Output_18);
	str.Format("%d", R_out[3][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Output_19);
	str.Format("%d", R_out[3][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Output_20);
	str.Format("%d", R_out[3][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Output_21);
	str.Format("%d", R_out[3][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Output_22);
	str.Format("%d", R_out[3][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_R_Output_23);
	str.Format("%d", R_out[3][5]);
	pWnd->SetWindowText(str);

	/*****************output G********************/

	pWnd = GetDlgItem(IDC_STATIC_G_Output_0);
	str.Format("%d", G_out[0][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Output_1);
	str.Format("%d", G_out[0][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Output_2);
	str.Format("%d", G_out[0][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Output_3);
	str.Format("%d", G_out[0][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Output_4);
	str.Format("%d", G_out[0][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Output_5);
	str.Format("%d", G_out[0][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_G_Output_6);
	str.Format("%d", G_out[1][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Output_7);
	str.Format("%d", G_out[1][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Output_8);
	str.Format("%d", G_out[1][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Output_9);
	str.Format("%d", G_out[1][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Output_10);
	str.Format("%d", G_out[1][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Output_11);
	str.Format("%d", G_out[1][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_G_Output_12);
	str.Format("%d", G_out[2][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Output_13);
	str.Format("%d", G_out[2][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Output_14);
	str.Format("%d", G_out[2][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Output_15);
	str.Format("%d", G_out[2][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Output_16);
	str.Format("%d", G_out[2][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Output_17);
	str.Format("%d", G_out[2][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_G_Output_18);
	str.Format("%d", G_out[3][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Output_19);
	str.Format("%d", G_out[3][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Output_20);
	str.Format("%d", G_out[3][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Output_21);
	str.Format("%d", G_out[3][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Output_22);
	str.Format("%d", G_out[3][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_G_Output_23);
	str.Format("%d", G_out[3][5]);
	pWnd->SetWindowText(str);

	/*****************output B********************/

	pWnd = GetDlgItem(IDC_STATIC_B_Output_0);
	str.Format("%d", B_out[0][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Output_1);
	str.Format("%d", B_out[0][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Output_2);
	str.Format("%d", B_out[0][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Output_3);
	str.Format("%d", B_out[0][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Output_4);
	str.Format("%d", B_out[0][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Output_5);
	str.Format("%d", B_out[0][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_B_Output_6);
	str.Format("%d", B_out[1][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Output_7);
	str.Format("%d", B_out[1][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Output_8);
	str.Format("%d", B_out[1][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Output_9);
	str.Format("%d", B_out[1][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Output_10);
	str.Format("%d", B_out[1][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Output_11);
	str.Format("%d", B_out[1][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_B_Output_12);
	str.Format("%d", B_out[2][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Output_13);
	str.Format("%d", B_out[2][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Output_14);
	str.Format("%d", B_out[2][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Output_15);
	str.Format("%d", B_out[2][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Output_16);
	str.Format("%d", B_out[2][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Output_17);
	str.Format("%d", B_out[2][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_B_Output_18);
	str.Format("%d", B_out[3][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Output_19);
	str.Format("%d", B_out[3][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Output_20);
	str.Format("%d", B_out[3][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Output_21);
	str.Format("%d", B_out[3][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Output_22);
	str.Format("%d", B_out[3][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_STATIC_B_Output_23);
	str.Format("%d", B_out[3][5]);
	pWnd->SetWindowText(str);

}

void CCCM_RGBvalDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CCCM_RGBvalDlg::OnOK() 
{
	// TODO: Add extra validation here
	UpdateData(TRUE);
	
	CDialog::OnOK();
}
<<<<<<< HEAD
=======

void CCCM_RGBvalDlg::SetDataValue_tar(void)
{
	CWnd *pWnd = NULL;
	CString str;
	
	UpdateData(FALSE);

	pWnd = GetDlgItem(IDC_EDIT_R_Target_0);
	str.Format("%d", R_tar[0][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_R_Target_1);
	str.Format("%d", R_tar[0][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_R_Target_2);
	str.Format("%d", R_tar[0][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_R_Target_3);
	str.Format("%d", R_tar[0][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_R_Target_4);
	str.Format("%d", R_tar[0][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_R_Target_5);
	str.Format("%d", R_tar[0][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_EDIT_R_Target_6);
	str.Format("%d", R_tar[1][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_R_Target_7);
	str.Format("%d", R_tar[1][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_R_Target_8);
	str.Format("%d", R_tar[1][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_R_Target_9);
	str.Format("%d", R_tar[1][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_R_Target_10);
	str.Format("%d", R_tar[1][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_R_Target_11);
	str.Format("%d", R_tar[1][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_EDIT_R_Target_12);
	str.Format("%d", R_tar[2][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_R_Target_13);
	str.Format("%d", R_tar[2][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_R_Target_14);
	str.Format("%d", R_tar[2][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_R_Target_15);
	str.Format("%d", R_tar[2][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_R_Target_16);
	str.Format("%d", R_tar[2][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_R_Target_17);
	str.Format("%d", R_tar[2][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_EDIT_R_Target_18);
	str.Format("%d", R_tar[3][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_R_Target_19);
	str.Format("%d", R_tar[3][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_R_Target_20);
	str.Format("%d", R_tar[3][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_R_Target_21);
	str.Format("%d", R_tar[3][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_R_Target_22);
	str.Format("%d", R_tar[3][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_R_Target_23);
	str.Format("%d", R_tar[3][5]);
	pWnd->SetWindowText(str);

	/*****************output G********************/

	pWnd = GetDlgItem(IDC_EDIT_G_Target_0);
	str.Format("%d", G_tar[0][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_G_Target_1);
	str.Format("%d", G_tar[0][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_G_Target_2);
	str.Format("%d", G_tar[0][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_G_Target_3);
	str.Format("%d", G_tar[0][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_G_Target_4);
	str.Format("%d", G_tar[0][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_G_Target_5);
	str.Format("%d", G_tar[0][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_EDIT_G_Target_6);
	str.Format("%d", G_tar[1][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_G_Target_7);
	str.Format("%d", G_tar[1][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_G_Target_8);
	str.Format("%d", G_tar[1][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_G_Target_9);
	str.Format("%d", G_tar[1][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_G_Target_10);
	str.Format("%d", G_tar[1][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_G_Target_11);
	str.Format("%d", G_tar[1][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_EDIT_G_Target_12);
	str.Format("%d", G_tar[2][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_G_Target_13);
	str.Format("%d", G_tar[2][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_G_Target_14);
	str.Format("%d", G_tar[2][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_G_Target_15);
	str.Format("%d", G_tar[2][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_G_Target_16);
	str.Format("%d", G_tar[2][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_G_Target_17);
	str.Format("%d", G_tar[2][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_EDIT_G_Target_18);
	str.Format("%d", G_tar[3][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_G_Target_19);
	str.Format("%d", G_tar[3][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_G_Target_20);
	str.Format("%d", G_tar[3][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_G_Target_21);
	str.Format("%d", G_tar[3][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_G_Target_22);
	str.Format("%d", G_tar[3][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_G_Target_23);
	str.Format("%d", G_tar[3][5]);
	pWnd->SetWindowText(str);

	/*****************output B********************/

	pWnd = GetDlgItem(IDC_EDIT_B_Target_0);
	str.Format("%d", B_tar[0][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_B_Target_1);
	str.Format("%d", B_tar[0][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_B_Target_2);
	str.Format("%d", B_tar[0][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_B_Target_3);
	str.Format("%d", B_tar[0][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_B_Target_4);
	str.Format("%d", B_tar[0][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_B_Target_5);
	str.Format("%d", B_tar[0][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_EDIT_B_Target_6);
	str.Format("%d", B_tar[1][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_B_Target_7);
	str.Format("%d", B_tar[1][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_B_Target_8);
	str.Format("%d", B_tar[1][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_B_Target_9);
	str.Format("%d", B_tar[1][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_B_Target_10);
	str.Format("%d", B_tar[1][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_B_Target_11);
	str.Format("%d", B_tar[1][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_EDIT_B_Target_12);
	str.Format("%d", B_tar[2][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_B_Target_13);
	str.Format("%d", B_tar[2][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_B_Target_14);
	str.Format("%d", B_tar[2][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_B_Target_15);
	str.Format("%d", B_tar[2][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_B_Target_16);
	str.Format("%d", B_tar[2][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_B_Target_17);
	str.Format("%d", B_tar[2][5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_EDIT_B_Target_18);
	str.Format("%d", B_tar[3][0]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_B_Target_19);
	str.Format("%d", B_tar[3][1]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_B_Target_20);
	str.Format("%d", B_tar[3][2]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_B_Target_21);
	str.Format("%d", B_tar[3][3]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_B_Target_22);
	str.Format("%d", B_tar[3][4]);
	pWnd->SetWindowText(str);
	pWnd = GetDlgItem(IDC_EDIT_B_Target_23);
	str.Format("%d", B_tar[3][5]);
	pWnd->SetWindowText(str);

}

void CCCM_RGBvalDlg::lab_to_rgb(char *buf, UINT size, UINT m, UINT n)
{
	UINT i = 0, idex = 0, times = 0, idex_offset = 0;
	float L,  a,  b;
	float X, float Y, float Z;
	char buf_in[256] = {0};

	while (1)
	{
		if (size < i)
		{
			break;
		}

		if (buf[i] == 0x09 || (times == 2 && buf[i] == 0x0))
		{
			memset(buf_in, 0x0, 256);
			memcpy(buf_in, &buf[idex_offset], idex);
			if(times == 0)
			{
				L = atof(buf_in);
				times = 1;
			}
			else if (times == 1)
			{
				a = atof(buf_in);
				times = 2;
			}
			else if (times == 2)
			{
				b = atof(buf_in);
				times = 3;
			}
			idex = 0;
			idex_offset  = i + 1;
		} 
		else
		{
			idex++;
		}
		i++;

	}

	Lab2XYZ(L, a, b, &X, &Y, &Z);  
	XYZ2RGB(X, Y, Z, &R_tar[m][n], &G_tar[m][n], &B_tar[m][n]); 

}

void CCCM_RGBvalDlg::get_one_line_date(char *buf, UINT size)
{
	UINT i = 0, idex = 0, idex_offset = 0;
	char buf_in[256] = {0};
	UINT m = 0, n = 0;

	while (1)
	{
		if (size <= i)
		{
			break;
		}

		if(buf[i] == 0x0D && buf[i+1] == 0x0A)
		{
			if (idex < 2)
			{
				idex++;
				i += 2;
				continue;
			}
			memset(buf_in, 0x0, 256);
			memcpy(buf_in, &buf[idex_offset], idex);
			lab_to_rgb(buf_in, idex, m, n);
			if (size == i)
			{
				break;
			}
			i += 2;
			idex = 0;
			idex_offset  = i;
			if(n == 5)
			{
				n=0;
				m++;
			}
			else
			{
				n++;
			}
		}
		else
		{
			idex++;
			i++;
		}
	}
}

void CCCM_RGBvalDlg::OnButtonImport() 
{
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here
	CFile file;
	CFileException e;
	CFileDialog dlg(TRUE, "*.txt", NULL, OFN_HIDEREADONLY,
		"Config File(*.txt)|*.txt|All Files (*.*)|*.*||");
	if (dlg.DoModal()!=IDOK)
		return;
	if (!file.Open(dlg.GetPathName(), CFile::modeRead, &e))
	{
		CString str;
		str.Format("���ļ�ʧ�ܣ������룺%u", e.m_cause);
		MessageBox(str, ISPTOOL_NAME, MB_OK|MB_ICONWARNING);
		return;
	}
	
	char *buf = NULL;
	UINT size = 0;
	
	size = file.GetLength();
	
	buf = (char*)malloc(size);
	
	if (NULL == buf)
	{
		AfxMessageBox("OnButtonRead buf malloc failed!", MB_OK);
		return;
	}
	
	file.Read(buf, size);
	file.Close();
	
	get_one_line_date(buf, size);
	//DecodeTotalFile(buf, size);

	SetDataValue_tar();
	
	free(buf);
}

void CCCM_RGBvalDlg::OnButtonReset() 
{
	// TODO: Add your control notification handler code here
	memcpy(R_tar, g_R_tar_standard, sizeof(g_R_tar_standard));
	memcpy(G_tar, g_G_tar_standard, sizeof(g_G_tar_standard));
	memcpy(B_tar, g_B_tar_standard, sizeof(g_B_tar_standard));

	SetDataValue_tar();
}
>>>>>>> 73eea11... [整理规范]1. 修改工具版本号为V4.0.44；2.规范工具里面的一些书写内容以及版权信息内容
