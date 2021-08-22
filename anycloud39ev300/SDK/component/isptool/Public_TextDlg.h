#if !defined(AFX_PUBLIC_TEXTDLG_H__BED1005C_FD98_4DA1_93FB_BAE8873EE1CC__INCLUDED_)
#define AFX_PUBLIC_TEXTDLG_H__BED1005C_FD98_4DA1_93FB_BAE8873EE1CC__INCLUDED_

#include "basepage.h"
#include "anyka_types.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Public_TextDlg.h : header file
//

#define  MAX_BUF_LEN   (32*1024)
#define  MAX_BUF_SHARP_LEN   (80*1024)

typedef enum {
    MODE_10 = 0,	//10进制
	MODE_16,		//16进制

    DISPLAY_MODE_NUM
} T_DISPLAY_MODE;

/////////////////////////////////////////////////////////////////////////////
// Public_TextDlg dialog

class Public_TextDlg : public CDialog, public CBasePage
{
// Construction
public:
	Public_TextDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(Public_TextDlg)
	enum { IDD = IDD_DIALOG_TEXT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Public_TextDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:

	char buf_lsc[MAX_BUF_LEN];
	char text_buf[MAX_BUF_LEN];

	UINT m_sensor_num;
	CToolTipCtrl    m_Mytip;

	 BOOL decode_packet_BB(char *buf, UINT buf_len);
	 BOOL Get_packetData_BB(char *buf, UINT buf_len);
	 UINT Get_text_info(char *buf);
	 UINT hex2int(const char *str);
	 void OnButton_Enable(T_DIALOG_ID pageID);
	 void Set_buf_info(char *buf, UINT *buf_idex, UINT addr, int value, T_DISPLAY_MODE valuemode);
	 int Get_value_by_buf(char *srcbuf, UINT *buf_idex, UINT buf_len);

	BOOL decode_packet_LSC(char *buf, UINT buf_len) ;
	BOOL Get_packetData_LSC(char *buf, UINT buf_len) ;
	BOOL decode_packet_RAW_LUT(char *buf, UINT buf_len) ;
	BOOL Get_packetData_RAW_LUT(char *buf, UINT buf_len) ;
	BOOL decode_packet_NR(char *buf, UINT buf_len) ;
	BOOL Get_packetData_NR(char *buf, UINT buf_len) ;
	BOOL decode_packet_3DNR(char *buf, UINT buf_len);
	BOOL Get_packetData_3DNR(char *buf, UINT buf_len);
	BOOL decode_packet_GB(char *buf, UINT buf_len) ;
	BOOL Get_packetData_GB(char *buf, UINT buf_len) ;
	BOOL decode_packet_DEMO(char *buf, UINT buf_len) ;
	BOOL Get_packetData_DEMO(char *buf, UINT buf_len) ;
	BOOL decode_packet_GAMMA(char *buf, UINT buf_len) ;
	BOOL Get_packetData_GAMMA(char *buf, UINT buf_len) ;
	BOOL decode_packet_Y_GAMMA(char *buf, UINT buf_len) ;
	BOOL Get_packetData_Y_GAMMA(char *buf, UINT buf_len) ;
	BOOL decode_packet_CCM(char *buf, UINT buf_len) ;
	BOOL Get_packetData_CCM(char *buf, UINT buf_len) ;
	BOOL decode_packet_FCS(char *buf, UINT buf_len) ;
	BOOL Get_packetData_FCS(char *buf, UINT buf_len) ;
	BOOL decode_packet_WDR(char *buf, UINT buf_len) ;
	BOOL Get_packetData_WDR(char *buf, UINT buf_len) ;
	BOOL decode_packet_EDGE(char *buf, UINT buf_len) ;
	BOOL Get_packetData_EDGE(char *buf, UINT buf_len) ;
	BOOL decode_packet_SHARP(char *buf, UINT buf_len) ;
	BOOL Get_packetData_SHARP(char *buf, UINT buf_len) ;
	BOOL decode_packet_SATURATION(char *buf, UINT buf_len) ;
	BOOL Get_packetData_SATURATION(char *buf, UINT buf_len) ;
	BOOL decode_packet_CONSTRAST(char *buf, UINT buf_len) ;
	BOOL Get_packetData_CONSTRAST(char *buf, UINT buf_len) ;
	BOOL decode_packet_YUVEFFECT(char *buf, UINT buf_len);
	BOOL Get_packetData_YUVEFFECT(char *buf, UINT buf_len) ;
	BOOL decode_packet_DPC(char *buf, UINT buf_len);
	BOOL Get_packetData_DPC(char *buf, UINT buf_len);
	BOOL decode_packet_WEIGHT(char *buf, UINT buf_len); 
	BOOL Get_packetData_WEIGHT(char *buf, UINT buf_len) ;
	BOOL decode_packet_AF(char *buf, UINT buf_len) ;
	BOOL Get_packetData_AF(char *buf, UINT buf_len);
	BOOL decode_packet_WB(char *buf, UINT buf_len);
	BOOL Get_packetData_WB(char *buf, UINT buf_len);
	BOOL decode_packet_EXP(char *buf, UINT buf_len);
	BOOL Get_packetData_EXP(char *buf, UINT buf_len);
	BOOL decode_packet_MISC(char *buf, UINT buf_len);
	BOOL Get_packetData_MISC(char *buf, UINT buf_len);
	BOOL decode_packet_RGBTOYUV(char *buf, UINT buf_len); 
	BOOL Get_packetData_RGBTOYUV(char *buf, UINT buf_len); 
	BOOL decode_packet_sensor(char *buf, UINT buf_len) ;
	BOOL Get_packetData_sensor(char *buf, UINT buf_len) ;
	UINT Get_text_sensor_info(void);
	int Get_num_by_buf_for_sensor(char *srcbuf, UINT *buf_idex , UINT buf_len);
	int Get_value_by_buf_for_sensor(char *srcbuf, UINT *buf_idex , UINT buf_len, T_U16 *sensor_addr, BOOL value_flag);//, char *dstbuf)
	BOOL decode_packet_hue(char *buf, UINT buf_len);	
	BOOL Get_packetData_hue(char *buf, UINT buf_len);

	void OnButtonGet_Enable(BOOL enable);
	void OnButtonSet_Enable(BOOL enable); 

	protected:

	// Generated message map functions
	//{{AFX_MSG(Public_TextDlg)
	afx_msg void OnButtonGet();
	afx_msg void OnButtonSet();
	afx_msg void OnButtonRead();
	afx_msg void OnButtonWrite();
	afx_msg void OnButtonCopy1();
	afx_msg void OnButtonCopy2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PUBLIC_TEXTDLG_H__BED1005C_FD98_4DA1_93FB_BAE8873EE1CC__INCLUDED_)
