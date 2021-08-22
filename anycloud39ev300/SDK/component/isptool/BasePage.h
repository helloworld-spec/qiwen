#pragma once

#define WM_ENABLE_CHANGE	        WM_USER + 200
#define WM_SUBMISSION		        WM_USER + 201
#define WM_GET_ISP_INFO		        WM_USER + 202
#define WM_SUBMISSION_FOR_TEXT		WM_USER + 203
#define WM_GET_ISP_INFO_FOR_TEXT    WM_USER + 204
#define WM_READ_FOR_TEXT			WM_USER + 205
#define WM_SAVE_FOR_TEXT			WM_USER + 206
#define WM_CLEAR_SENSOR_PARAM		WM_USER + 207
#define WM_COPY_UI_TO_TEXT			WM_USER + 208
#define WM_COPY_TEXT_TO_UI			WM_USER + 209
#define WM_GET_YUV_IMG				WM_USER + 210
#define WM_GET_WB_INFO				WM_USER + 211
#define WM_SET_WB_INFO				WM_USER + 212
#define WM_GET_EXP_INFO				WM_USER + 213
#define WM_GET_LSC_INFO				WM_USER + 214
#define WM_SET_LSC_INFO				WM_USER + 215
#define WM_GET_RGB_GAMMA			WM_USER + 216

#define MAX_PAGE_STRUCT_LEN	300

#define MAKEMESSAGELP(id, msg)		((id << 16) | (msg))
#define GETMESSAGEID(x)				((x) >> 16)
#define GETMESSAGEINFO(x)			((x) & 0xFFFF)

typedef enum {
    DIALOG_SYS = 0,
    DIALOG_BB,
    DIALOG_DPC,
    DIALOG_GB,
    DIALOG_LSC,

    DIALOG_RAWLUT,
	DIALOG_LUT_RGB,
    DIALOG_DEMOSAIC,
    DIALOG_CCM,
    DIALOG_GAMMA,
	DIALOG_GAMMA_RGB,
    DIALOG_DENOISE,

    DIALOG_SHARP,
    DIALOG_EDGE,
    DIALOG_3DNR,
    DIALOG_WDR,
    DIALOG_FCS,

    DIALOG_RGB2YUV,
	DIALOG_YUVEFFECT,
    DIALOG_SATURATION,
	DIALOG_CONTRAST,
    DIALOG_WB,
	DIALOG_EXP,
	DIALOG_AF,
	DIALOG_STAT,
	DIALOG_ZONE_WEIGHT,
	DIALOG_MISC,
	DIALOG_Y_GAMMA,
	DIALOG_HUE,

	DIALOG_3DSTAT,
	DIALOG_AESTAT,
	DIALOG_AFSTAT,
	DIALOG_AWBSTAT,

	DIALOG_COMPARE,

    DIALOG_NUM
} T_DIALOG_ID;

class CBasePage
{
public:
	CBasePage(void);
	virtual ~CBasePage(void);
	
	virtual int SetPageID(int nPageId);
	virtual int GetPageID(int & nPageId);
	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int GetPageInfoStAll(int & nPageID, void * pPageInfoSt, int & nStlen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	virtual int SetPageInfoStAll(void * pPageInfoSt, int nStLen);
	virtual BOOL GetPageEnable();
	virtual int SendPageMessage(CWnd * pWnd, int nPageMsg, int nFlag, unsigned short nMessage);
	virtual int SetMessageWindow(CWnd * pWnd);
	virtual int Clear();
	virtual int SavePageInfo(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int ReadPageInfo(void * pPageInfoSt, int nStLen);

protected:
	int m_nID;
	CWnd * m_pMessageWnd;
};
