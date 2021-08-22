// CompareDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "CompareDlg.h"
#include "ISPCTRL_TOOLDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define  COMPARE_FILE_LOG     "compare_log.txt"


extern HINSTANCE _hInstance;

/////////////////////////////////////////////////////////////////////////////
// CCompareDlg dialog


CCompareDlg::CCompareDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCompareDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCompareDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCompareDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCompareDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCompareDlg, CDialog)
	//{{AFX_MSG_MAP(CCompareDlg)
	ON_BN_CLICKED(IDC_BUTTON_COMPARE_FILE1, OnButtonCompareFile1)
	ON_BN_CLICKED(IDC_BUTTON_COMPARE_FILE2, OnButtonCompareFile2)
	ON_BN_CLICKED(IDC_BUTTON_COMPARE, OnButtonCompare)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCompareDlg message handlers

void CCompareDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	//CDialog::OnOK();
}

void CCompareDlg::OnButtonCompareFile1() 
{
	// TODO: Add your control notification handler code here
	OPENFILENAME ofn;
	TCHAR pstrFileName[260] = {0}, pstrTitleName[260] = {0};
	
	TCHAR szFilter[] =	TEXT ("CONFG Files (*.conf;*.conf0)\0*.conf;*.conf0\0")  \
		TEXT ("All Files (*.*)\0*.*\0\0") ;
	
	memset ( &ofn, 0, sizeof ( ofn ) );
	
	ofn.lStructSize       = sizeof (OPENFILENAME) ;
	ofn.hInstance         = _hInstance ;
	ofn.hwndOwner         = GetSafeHwnd();
	ofn.lpstrFilter       = szFilter;
	ofn.nMaxFile          = MAX_PATH ;
	ofn.lpstrDefExt       = TEXT ("bin") ;
	ofn.lpstrFile         = pstrFileName ;
	ofn.Flags             = OFN_FILEMUSTEXIST; 
	
	
	if(GetOpenFileName (&ofn))
	{
		TCHAR * relative_path = pstrFileName;
		
		if((relative_path = _tcsstr(pstrFileName, path_module)) != NULL)
		{
			relative_path = pstrFileName + _tcslen(path_module);
			SetDlgItemText(IDC_EDIT_COMPARE_FILE1, relative_path);
		}
		else
		{
			SetDlgItemText(IDC_EDIT_COMPARE_FILE1, pstrFileName);
		}
	}
}

void CCompareDlg::OnButtonCompareFile2() 
{
	// TODO: Add your control notification handler code here
	OPENFILENAME ofn;
	TCHAR pstrFileName[260] = {0}, pstrTitleName[260] = {0};
	
	TCHAR szFilter[] =	TEXT ("CONFG Files (*.conf;*.conf0)\0*.conf;*.conf0\0")  \
		TEXT ("All Files (*.*)\0*.*\0\0") ;
	
	memset ( &ofn, 0, sizeof ( ofn ) );
	
	ofn.lStructSize       = sizeof (OPENFILENAME) ;
	ofn.hInstance         = _hInstance ;
	ofn.hwndOwner         = GetSafeHwnd();
	ofn.lpstrFilter       = szFilter;
	ofn.nMaxFile          = MAX_PATH ;
	ofn.lpstrDefExt       = TEXT ("bin") ;
	ofn.lpstrFile         = pstrFileName ;
	ofn.Flags             = OFN_FILEMUSTEXIST; 
	
	
	if(GetOpenFileName (&ofn))
	{
		TCHAR * relative_path = pstrFileName;
		
		if((relative_path = _tcsstr(pstrFileName, path_module)) != NULL)
		{
			relative_path = pstrFileName + _tcslen(path_module);
			SetDlgItemText(IDC_EDIT_COMPARE_FILE2, relative_path);
		}
		else
		{
			SetDlgItemText(IDC_EDIT_COMPARE_FILE2, pstrFileName);
		}
	}
}


static T_U16 Isp_Struct_len[ISP_HUE + 1] = {sizeof(AK_ISP_INIT_BLC),
									sizeof(AK_ISP_INIT_LSC),
									sizeof(AK_ISP_INIT_RAW_LUT),
									sizeof(AK_ISP_INIT_NR),
									sizeof(AK_ISP_INIT_3DNR),
									sizeof(AK_ISP_INIT_GB),
									sizeof(AK_ISP_INIT_DEMO),
									sizeof(AK_ISP_INIT_GAMMA),
									sizeof(AK_ISP_INIT_CCM),
									sizeof(AK_ISP_INIT_FCS),
									sizeof(AK_ISP_INIT_WDR),
									//sizeof(AK_ISP_INIT_EDGE),
									sizeof(AK_ISP_INIT_SHARP),
									sizeof(AK_ISP_INIT_SATURATION),
									sizeof(AK_ISP_INIT_CONTRAST),
									sizeof(AK_ISP_INIT_RGB2YUV),
									sizeof(AK_ISP_INIT_EFFECT),
									sizeof(AK_ISP_INIT_DPC),
									sizeof(AK_ISP_INIT_WEIGHT),
									sizeof(AK_ISP_INIT_AF),
									sizeof(AK_ISP_INIT_WB),
									sizeof(AK_ISP_INIT_EXP),
									sizeof(AK_ISP_INIT_MISC),
									sizeof(AK_ISP_INIT_Y_GAMMA),
									sizeof(AK_ISP_INIT_HUE)
									};

bool CCompareDlg::CheckSubFileHeadInfo(CFGFILE_HEADINFO* headinfo)
{
	if (NULL == headinfo)
		return FALSE;

	fprintf(stderr, "CheckSubFileHeadInfo, main version : %d, file version : %s, sensor id : 0x%x, style id : %d, \
		modify time : %d-%d-%d, %02d:%02d:%02d, subFileId:%d!\n", headinfo->main_version, headinfo->file_version, 
		headinfo->sensorId, headinfo->styleId, headinfo->year, headinfo->month, headinfo->day, 
		headinfo->hour, headinfo->minute, headinfo->second, headinfo->subFileId);

	if (headinfo->year < 1900)
		return FALSE;

	if (headinfo->month > 12 || headinfo->month < 1)
		return FALSE;
	if (headinfo->day > 31 || headinfo->day < 1)
		return FALSE;

	if (headinfo->hour > 23)
		return FALSE;

	if (headinfo->minute > 59)
		return FALSE;

	if (headinfo->second > 59)
		return FALSE;

	if (headinfo->subFileId > 4)
		return FALSE;

	if (MAIN_VERSION != headinfo->main_version)
	{
		if (3 == headinfo->main_version)
		{
			fprintf(stderr, "CheckSubFileHeadInfo cfg file is old version, v3!\n");
			AfxMessageBox("配置文件版本旧，请用V3.x.xx版本的工具调试!", MB_OK);
		}

		return FALSE;
	}

	return TRUE;
}

int CCompareDlg::CheckTotalFileData(char* cfgBuf, T_U32 size)
{
	T_U8 i = 0;
	T_U32 total = 0;
	T_U16 moduleId = 0;
	T_U16 length = 0;
	T_U32 offset = 0;
	T_U8 subfileid[SUBFILE_NUM_MAX] = {0};
	T_U32 subfilelen[SUBFILE_NUM_MAX] = {0};
	T_U32 subfileoffset[SUBFILE_NUM_MAX] = {0};
	T_U8 subfilecnt = 0;
	
	if ((AK_NULL == cfgBuf) || (size <= sizeof(CFGFILE_HEADINFO)))
	{
		fprintf(stderr, "CheckTotalFileData cfgBuf null or size is too small, size:%lu!\n", size);
		return 0;
	}

	CFGFILE_HEADINFO headinfo = {0};

CHECK_ONE_SUBFILE:
	memcpy(&headinfo, cfgBuf+offset, sizeof(CFGFILE_HEADINFO));

	if (!CheckSubFileHeadInfo(&headinfo))
	{
		fprintf(stderr, "CheckTotalFileData failed!\n");
		return 0;
	}

	offset += sizeof(CFGFILE_HEADINFO);
	total += sizeof(CFGFILE_HEADINFO);
	subfilelen[subfilecnt] += sizeof(CFGFILE_HEADINFO);

	for (i=ISP_BB; i<=ISP_HUE; i++)
	{
		memcpy(&moduleId, cfgBuf + offset, ISP_MODULE_ID_SIZE);
		memcpy(&length, cfgBuf + offset + ISP_MODULE_ID_SIZE, ISP_MODULE_LEN_SIZE);

		//fprintf(stderr, "i: %d, moduleId : %d, length : %d, Structlen : %d!\n", i, moduleId, length, Isp_Struct_len[i]);

		if ((moduleId != i) || (length != Isp_Struct_len[i]))
		{
			fprintf(stderr, "CheckTotalFileData data err!\n");
			return 0;
		}
	
		offset += Isp_Struct_len[i];
		total += Isp_Struct_len[i];
		subfilelen[subfilecnt] += Isp_Struct_len[i];
	}

	//sensor
	memcpy(&moduleId, cfgBuf + offset, ISP_MODULE_ID_SIZE);

	if (moduleId != ISP_SENSOR)
	{
		fprintf(stderr, "CheckTotalFileData sensor id err!\n");
		return 0;
	}

	offset += ISP_MODULE_ID_SIZE;
	total += ISP_MODULE_ID_SIZE;
	subfilelen[subfilecnt] += ISP_MODULE_ID_SIZE;
	
	memcpy(&length, cfgBuf + offset, ISP_MODULE_LEN_SIZE);

	total += ISP_MODULE_LEN_SIZE + length;
	offset += ISP_MODULE_LEN_SIZE + length;
	subfilelen[subfilecnt] += ISP_MODULE_LEN_SIZE + length;


	subfileid[subfilecnt] = headinfo.subFileId;
	if (subfilecnt > 0)
	{
		subfileoffset[subfilecnt] = subfileoffset[subfilecnt-1] + subfilelen[subfilecnt-1];
	}

	subfilecnt++;

	if (size > total && subfilecnt < SUBFILE_NUM_MAX)
	{
		goto CHECK_ONE_SUBFILE;
	}

	return subfilecnt;
}

BOOL CCompareDlg::On_Read(CString path, AK_ISP_COMPARE_INFO *isp_compare_info, UINT *file_len) 
{
	// TODO: Add your control notification handler code here
	CFile file;
	CFileException e;
	UINT size = 0;
	T_U8 *buf = NULL;
	CString str;
	CISPCTRL_TOOLDlg Tool_dlg;
	UINT headidex = sizeof(CFGFILE_HEADINFO);
	int subfilecnt = 0;
	int i = 0;

	
	UINT buf_isp_len_temp = sizeof(AK_ISP_INIT_BLC);
	buf_isp_len_temp += sizeof(AK_ISP_INIT_LSC);
	buf_isp_len_temp += sizeof(AK_ISP_INIT_RAW_LUT);
	buf_isp_len_temp += sizeof(AK_ISP_INIT_NR);
	buf_isp_len_temp += sizeof(AK_ISP_INIT_3DNR);
	buf_isp_len_temp += sizeof(AK_ISP_INIT_GB);
	buf_isp_len_temp += sizeof(AK_ISP_INIT_DEMO);
	buf_isp_len_temp += sizeof(AK_ISP_INIT_GAMMA);
	buf_isp_len_temp += sizeof(AK_ISP_INIT_CCM);
	buf_isp_len_temp += sizeof(AK_ISP_INIT_FCS);
	buf_isp_len_temp += sizeof(AK_ISP_INIT_WDR);
	//buf_isp_len_temp += sizeof(AK_ISP_INIT_EDGE);
	buf_isp_len_temp += sizeof(AK_ISP_INIT_SHARP);
	buf_isp_len_temp += sizeof(AK_ISP_INIT_SATURATION);
	buf_isp_len_temp += sizeof(AK_ISP_INIT_CONTRAST);
	buf_isp_len_temp += sizeof(AK_ISP_INIT_RGB2YUV);
	buf_isp_len_temp += sizeof(AK_ISP_INIT_EFFECT);
	buf_isp_len_temp += sizeof(AK_ISP_INIT_DPC);
	buf_isp_len_temp += sizeof(AK_ISP_INIT_WEIGHT);
	buf_isp_len_temp += sizeof(AK_ISP_INIT_AF);
	buf_isp_len_temp += sizeof(AK_ISP_INIT_WB);
	buf_isp_len_temp += sizeof(AK_ISP_INIT_EXP);
	buf_isp_len_temp += sizeof(AK_ISP_INIT_MISC);
	buf_isp_len_temp += sizeof(AK_ISP_INIT_Y_GAMMA);
	buf_isp_len_temp += sizeof(AK_ISP_INIT_HUE);

	UINT idex = 0;

	if (!file.Open(Tool_dlg.ConvertAbsolutePath(path), CFile::modeRead, &e))
	{
		str.Format("打开文件失败！错误码：%u", e.m_cause);
		AfxMessageBox(str, MB_OK|MB_ICONWARNING);
		return FALSE;
	}

	size = file.GetLength();
	*file_len = size;

	buf = (T_U8 *)malloc(size*sizeof(T_U8));
	if (buf == NULL)
	{
		str.Format("分配内存失败,请检查");
		AfxMessageBox(str, MB_OK|MB_ICONWARNING);
		return FALSE;
	}
	memset(buf, 0, size*sizeof(T_U8));

	file.Read(buf, size);
	
	subfilecnt = CheckTotalFileData((char*)buf, size);
	
	if (0 == subfilecnt)
	{
		str.Format("文件格式错误! %s", path);
		AfxMessageBox(str, MB_OK|MB_ICONWARNING);
		return FALSE;
	}

	isp_compare_info->subfilecnt = subfilecnt;
	idex = 0;

	for (i=0; i<subfilecnt; i++)
	{
		memcpy(&isp_compare_info->headinfo[i], &buf[idex], headidex);
		idex = idex + headidex;

		memcpy(&isp_compare_info->buf_isp[i].p_Isp_blc, &buf[idex], sizeof(AK_ISP_INIT_BLC));
		idex += sizeof(AK_ISP_INIT_BLC);
		
		memcpy(&isp_compare_info->buf_isp[i].p_Isp_lsc, &buf[idex], sizeof(AK_ISP_INIT_LSC));
		idex += sizeof(AK_ISP_INIT_LSC);
		
		memcpy(&isp_compare_info->buf_isp[i].p_Isp_raw_lut, &buf[idex], sizeof(AK_ISP_INIT_RAW_LUT));
		idex += sizeof(AK_ISP_INIT_RAW_LUT);
		
		memcpy(&isp_compare_info->buf_isp[i].p_Isp_nr, &buf[idex], sizeof(AK_ISP_INIT_NR));
		idex += sizeof(AK_ISP_INIT_NR);
		
		memcpy(&isp_compare_info->buf_isp[i].p_Isp_3dnr, &buf[idex], sizeof(AK_ISP_INIT_3DNR));
		idex += sizeof(AK_ISP_INIT_3DNR);
		
		memcpy(&isp_compare_info->buf_isp[i].p_Isp_gb, &buf[idex], sizeof(AK_ISP_INIT_GB));
		idex += sizeof(AK_ISP_INIT_GB);
		
		memcpy(&isp_compare_info->buf_isp[i].p_Isp_demo, &buf[idex], sizeof(AK_ISP_INIT_DEMO));
		idex += sizeof(AK_ISP_INIT_DEMO);
		
		memcpy(&isp_compare_info->buf_isp[i].p_Isp_gamma, &buf[idex], sizeof(AK_ISP_INIT_GAMMA));
		idex += sizeof(AK_ISP_INIT_GAMMA);
		
		memcpy(&isp_compare_info->buf_isp[i].p_Isp_ccm, &buf[idex], sizeof(AK_ISP_INIT_CCM));
		idex += sizeof(AK_ISP_INIT_CCM);
		
		memcpy(&isp_compare_info->buf_isp[i].p_Isp_fcs, &buf[idex], sizeof(AK_ISP_INIT_FCS));
		idex += sizeof(AK_ISP_INIT_FCS);
		
		memcpy(&isp_compare_info->buf_isp[i].p_Isp_wdr, &buf[idex], sizeof(AK_ISP_INIT_WDR));
		idex += sizeof(AK_ISP_INIT_WDR);
		
		//memcpy(&isp_compare_info->buf_isp[i].p_Isp_edge, &buf[idex], sizeof(AK_ISP_INIT_EDGE));
		//idex += sizeof(AK_ISP_INIT_EDGE);
		
		memcpy(&isp_compare_info->buf_isp[i].p_Isp_sharp, &buf[idex], sizeof(AK_ISP_INIT_SHARP));
		idex += sizeof(AK_ISP_INIT_SHARP);
		
		memcpy(&isp_compare_info->buf_isp[i].p_Isp_saturation, &buf[idex], sizeof(AK_ISP_INIT_SATURATION));
		idex += sizeof(AK_ISP_INIT_SATURATION);
		
		memcpy(&isp_compare_info->buf_isp[i].p_Isp_contrast, &buf[idex], sizeof(AK_ISP_INIT_CONTRAST));
		idex += sizeof(AK_ISP_INIT_CONTRAST);
		
		memcpy(&isp_compare_info->buf_isp[i].p_Isp_rgb2yuv, &buf[idex], sizeof(AK_ISP_INIT_RGB2YUV));
		idex += sizeof(AK_ISP_INIT_RGB2YUV);
		
		memcpy(&isp_compare_info->buf_isp[i].p_Isp_effect, &buf[idex], sizeof(AK_ISP_INIT_EFFECT));
		idex += sizeof(AK_ISP_INIT_EFFECT);
		
		memcpy(&isp_compare_info->buf_isp[i].p_Isp_dpc, &buf[idex], sizeof(AK_ISP_INIT_DPC));
		idex += sizeof(AK_ISP_INIT_DPC);
		
		memcpy(&isp_compare_info->buf_isp[i].p_Isp_weight, &buf[idex], sizeof(AK_ISP_INIT_WEIGHT));
		idex += sizeof(AK_ISP_INIT_WEIGHT);
		
		memcpy(&isp_compare_info->buf_isp[i].p_Isp_af, &buf[idex], sizeof(AK_ISP_INIT_AF));
		idex += sizeof(AK_ISP_INIT_AF);
		
		memcpy(&isp_compare_info->buf_isp[i].p_Isp_wb, &buf[idex], sizeof(AK_ISP_INIT_WB));
		idex += sizeof(AK_ISP_INIT_WB);
		
		memcpy(&isp_compare_info->buf_isp[i].p_Isp_exp, &buf[idex], sizeof(AK_ISP_INIT_EXP));
		idex += sizeof(AK_ISP_INIT_EXP);
		
		memcpy(&isp_compare_info->buf_isp[i].p_Isp_misc, &buf[idex], sizeof(AK_ISP_INIT_MISC));
		idex += sizeof(AK_ISP_INIT_MISC);

		memcpy(&isp_compare_info->buf_isp[i].p_Isp_y_gamma, &buf[idex], sizeof(AK_ISP_INIT_Y_GAMMA));
		idex += sizeof(AK_ISP_INIT_Y_GAMMA);

		memcpy(&isp_compare_info->buf_isp[i].p_Isp_hue, &buf[idex], sizeof(AK_ISP_INIT_HUE));
		idex += sizeof(AK_ISP_INIT_HUE);

		memcpy(&isp_compare_info->buf_isp[i].p_Isp_sensor.param_id, &buf[idex], 2);
		idex = idex + 2;
		memcpy(&isp_compare_info->buf_isp[i].p_Isp_sensor.length, &buf[idex], 2);
		idex = idex + 2;
		
		isp_compare_info->sensor_num[i] = isp_compare_info->buf_isp[i].p_Isp_sensor.length/sizeof(AK_ISP_SENSOR_ATTR);

		if (isp_compare_info->buf_isp[i].p_Isp_sensor.p_sensor != NULL)
		{
			free(isp_compare_info->buf_isp[i].p_Isp_sensor.p_sensor);
			isp_compare_info->buf_isp[i].p_Isp_sensor.p_sensor = NULL;
		}
		isp_compare_info->buf_isp[i].p_Isp_sensor.p_sensor = (AK_ISP_SENSOR_ATTR  *)malloc(isp_compare_info->buf_isp[i].p_Isp_sensor.length*sizeof(char));
		if (isp_compare_info->buf_isp[i].p_Isp_sensor.p_sensor != NULL)
		{
			memset(isp_compare_info->buf_isp[i].p_Isp_sensor.p_sensor, 0,isp_compare_info->buf_isp[i].p_Isp_sensor.length*sizeof(char));
			memcpy(isp_compare_info->buf_isp[i].p_Isp_sensor.p_sensor, &buf[idex], isp_compare_info->buf_isp[i].p_Isp_sensor.length*sizeof(char));
		}
		else
		{
			isp_compare_info->buf_isp[i].p_Isp_sensor.p_sensor = NULL;
			isp_compare_info->sensor_num[i] = 0;
		}


		idex = idex + isp_compare_info->buf_isp[i].p_Isp_sensor.length;
	}
	

	free(buf);
	file.Close();
	return TRUE;
}

TCHAR *CCompareDlg::ConvertAbsolutePath(LPCTSTR path)
{
    CString sPath;
	CString filePath;
	
    if (path[0] == '\0')
    {
        return NULL;
    }
	else if ((':' == path[1]) || (('\\'==path[0]) && ('\\'==path[1])))
	{
		_tcsncpy(m_path, path, MAX_PATH);
	}
	else
	{
		GetModuleFileName(NULL,sPath.GetBufferSetLength(MAX_PATH+1),MAX_PATH);
		
		sPath.ReleaseBuffer ();
		int nPos;
		nPos=sPath.ReverseFind ('\\');
		sPath=sPath.Left (nPos+1);
		
		filePath = sPath + path;
		
		_tcsncpy(m_path, filePath, MAX_PATH);
	}
	
	return m_path;
}

void CCompareDlg::On_Write_diffrent_info(AK_ISP_COMPARE_INFO *isp_compare_1, AK_ISP_COMPARE_INFO *isp_compare_2) 
{
	//打开文件
	CString str;

	//获取属性
	DWORD faConfig = GetFileAttributes(ConvertAbsolutePath(COMPARE_FILE_LOG)); 
	if(0xFFFFFFFF != faConfig)
	{
		faConfig &= ~FILE_ATTRIBUTE_READONLY;//如果文件是只读，需要设非只读
		faConfig &= ~FILE_ATTRIBUTE_SYSTEM;  //如果文件是系统，那么设非系统
		faConfig &= ~FILE_ATTRIBUTE_TEMPORARY;//如果存在临时，那么也要设非临时
		SetFileAttributes(ConvertAbsolutePath(COMPARE_FILE_LOG), faConfig);
	}

	//open config file
	CStdioFile *pFile;
	pFile = new CStdioFile(ConvertAbsolutePath(COMPARE_FILE_LOG), 
		CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareDenyNone);
	if(NULL == pFile)
	{
		AfxMessageBox("creat compare_lig.txt fail", MB_OK);
		return;
	}

	//USES_CONVERSION;

	//写文件
	Write_info(pFile,  isp_compare_1,  isp_compare_2);

	//关闭文件
	pFile->Close();
	delete pFile;
}

void CCompareDlg::OnButtonCompare() 
{
	// TODO: Add your control notification handler code here
	AK_ISP_COMPARE_INFO isp_compare_info_1 = {0};
	AK_ISP_COMPARE_INFO isp_compare_info_2 = {0};
	UINT file1_len = 0;
	UINT file2_len = 0;
	CString compare_path_1;
	CString compare_path_2;
	CString str;
	int i = 0;

	diffrent_num = 0;
	only_day_or_night_flag = FALSE;
	SetDlgItemText(IDC_STATIC_COMPARE_INFO, "");
	SetDlgItemText(IDC_STATIC_DIFFRENT_NUM, "");
	

	GetDlgItemText(IDC_EDIT_COMPARE_FILE1, compare_path_1);
	if (compare_path_1.IsEmpty())
	{
		AfxMessageBox(_T("请选择要比较的文件"), MB_OK);
		return;
	}
	GetDlgItemText(IDC_EDIT_COMPARE_FILE2, compare_path_2);
	if (compare_path_2.IsEmpty())
	{
		AfxMessageBox(_T("请选择要比较的文件"), MB_OK);
		return;
	}

	//打开二个文件
	if (!On_Read(compare_path_1, &isp_compare_info_1, &file1_len))
	{
		for (i=0; i<SUBFILE_NUM_MAX; i++)
		{
			if (isp_compare_info_1.buf_isp[i].p_Isp_sensor.p_sensor != NULL)
			{
				free(isp_compare_info_1.buf_isp[i].p_Isp_sensor.p_sensor);
				isp_compare_info_1.buf_isp[i].p_Isp_sensor.p_sensor = NULL;
			}
		}

		str.Format("open file1 fail");
		SetDlgItemText(IDC_STATIC_COMPARE_INFO, str);
		return;
	}
	if (!On_Read(compare_path_2, &isp_compare_info_2, &file2_len))
	{
		for (i=0; i<SUBFILE_NUM_MAX; i++)
		{
			if (isp_compare_info_1.buf_isp[i].p_Isp_sensor.p_sensor != NULL)
			{
				free(isp_compare_info_1.buf_isp[i].p_Isp_sensor.p_sensor);
				isp_compare_info_1.buf_isp[i].p_Isp_sensor.p_sensor = NULL;
			}
		}

		for (i=0; i<SUBFILE_NUM_MAX; i++)
		{
			if (isp_compare_info_2.buf_isp[i].p_Isp_sensor.p_sensor != NULL)
			{
				free(isp_compare_info_2.buf_isp[i].p_Isp_sensor.p_sensor);
				isp_compare_info_2.buf_isp[i].p_Isp_sensor.p_sensor = NULL;
			}
		}

		str.Format("open file2 fail");
		SetDlgItemText(IDC_STATIC_COMPARE_INFO, str);
		return;
	}


	str.Format("the two file is comparing");
	SetDlgItemText(IDC_STATIC_COMPARE_INFO, str);

	//比较数据是否不一样
	On_Write_diffrent_info(&isp_compare_info_1, &isp_compare_info_2);
	if (diffrent_num == 0)
	{
		str.Format("the two file is same");
		SetDlgItemText(IDC_STATIC_COMPARE_INFO, str);
	}
	else
	{	
		//把不一样的数据记录到文件上
		str.Format("the two file is diffrent,\r\n pls to look compare_log.txt");
		SetDlgItemText(IDC_STATIC_COMPARE_INFO, str);

		str.Format("diffrent num is %d", diffrent_num);
		SetDlgItemText(IDC_STATIC_DIFFRENT_NUM, str);
	}

	for (i=0; i<SUBFILE_NUM_MAX; i++)
	{
		if (isp_compare_info_1.buf_isp[i].p_Isp_sensor.p_sensor != NULL)
		{
			free(isp_compare_info_1.buf_isp[i].p_Isp_sensor.p_sensor);
			isp_compare_info_1.buf_isp[i].p_Isp_sensor.p_sensor = NULL;
		}
	}

	for (i=0; i<SUBFILE_NUM_MAX; i++)
	{
		if (isp_compare_info_2.buf_isp[i].p_Isp_sensor.p_sensor != NULL)
		{
			free(isp_compare_info_2.buf_isp[i].p_Isp_sensor.p_sensor);
			isp_compare_info_2.buf_isp[i].p_Isp_sensor.p_sensor = NULL;
		}
	}
}

BOOL CCompareDlg::OnInitDialog() 
{
	CString sPath;
	int nPos;

	CDialog::OnInitDialog();

	GetModuleFileName(NULL,sPath.GetBufferSetLength(MAX_PATH+1),MAX_PATH);
	sPath.ReleaseBuffer();
	nPos=sPath.ReverseFind ('\\');
    sPath=sPath.Left(nPos+1);
	_tcsncpy(path_module, sPath, MAX_PATH); 
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCompareDlg::Write_BLC_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) 
{
	CString str;
	UINT i = 0;

	if (isp_compare_1->p_Isp_blc.param_id != isp_compare_2->p_Isp_blc.param_id)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_blc.param_id = %d, %d\r\n"), isp_compare_1->p_Isp_blc.param_id, isp_compare_2->p_Isp_blc.param_id);
		pFile->WriteString(str);
	}
	if (isp_compare_1->p_Isp_blc.length != isp_compare_2->p_Isp_blc.length)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_blc.length = %d, %d\r\n"), isp_compare_1->p_Isp_blc.length, isp_compare_2->p_Isp_blc.length);
		pFile->WriteString(str);
	}
	if (isp_compare_1->p_Isp_blc.p_blc.blc_mode != isp_compare_2->p_Isp_blc.p_blc.blc_mode)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_blc.p_blc.blc_mode = %d, %d\r\n"), isp_compare_1->p_Isp_blc.p_blc.blc_mode, isp_compare_2->p_Isp_blc.p_blc.blc_mode);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_blc.p_blc.m_blc.black_level_enable != isp_compare_2->p_Isp_blc.p_blc.m_blc.black_level_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_blc.p_blc.m_blc.black_level_enable = %d, %d\r\n"), isp_compare_1->p_Isp_blc.p_blc.m_blc.black_level_enable, isp_compare_2->p_Isp_blc.p_blc.m_blc.black_level_enable);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_blc.p_blc.m_blc.bl_r_a != isp_compare_2->p_Isp_blc.p_blc.m_blc.bl_r_a)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_blc.p_blc.m_blc.bl_r_a = %d, %d\r\n"), isp_compare_1->p_Isp_blc.p_blc.m_blc.bl_r_a, isp_compare_2->p_Isp_blc.p_blc.m_blc.bl_r_a);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_blc.p_blc.m_blc.bl_gr_a != isp_compare_2->p_Isp_blc.p_blc.m_blc.bl_gr_a)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_blc.p_blc.m_blc.bl_gr_a = %d, %d\r\n"), isp_compare_1->p_Isp_blc.p_blc.m_blc.bl_gr_a, isp_compare_2->p_Isp_blc.p_blc.m_blc.bl_gr_a);
		pFile->WriteString(str);
	}
	if (isp_compare_1->p_Isp_blc.p_blc.m_blc.bl_gb_a != isp_compare_2->p_Isp_blc.p_blc.m_blc.bl_gb_a)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_blc.p_blc.m_blc.bl_gb_a = %d, %d\r\n"), isp_compare_1->p_Isp_blc.p_blc.m_blc.bl_gb_a, isp_compare_2->p_Isp_blc.p_blc.m_blc.bl_gb_a);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_blc.p_blc.m_blc.bl_b_a != isp_compare_2->p_Isp_blc.p_blc.m_blc.bl_b_a)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_blc.p_blc.m_blc.bl_b_a = %d, %d\r\n"), isp_compare_1->p_Isp_blc.p_blc.m_blc.bl_b_a, isp_compare_2->p_Isp_blc.p_blc.m_blc.bl_b_a);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_blc.p_blc.m_blc.bl_r_offset != isp_compare_2->p_Isp_blc.p_blc.m_blc.bl_r_offset)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_blc.p_blc.m_blc.bl_r_offset = %d, %d\r\n"), isp_compare_1->p_Isp_blc.p_blc.m_blc.bl_r_offset, isp_compare_2->p_Isp_blc.p_blc.m_blc.bl_r_offset);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_blc.p_blc.m_blc.bl_gr_offset != isp_compare_2->p_Isp_blc.p_blc.m_blc.bl_gr_offset)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_blc.p_blc.m_blc.bl_gr_offset = %d, %d\r\n"), isp_compare_1->p_Isp_blc.p_blc.m_blc.bl_gr_offset, isp_compare_2->p_Isp_blc.p_blc.m_blc.bl_gr_offset);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_blc.p_blc.m_blc.bl_gb_offset != isp_compare_2->p_Isp_blc.p_blc.m_blc.bl_gb_offset)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_blc.p_blc.m_blc.bl_gb_offset = %d, %d\r\n"), isp_compare_1->p_Isp_blc.p_blc.m_blc.bl_gb_offset, isp_compare_2->p_Isp_blc.p_blc.m_blc.bl_gb_offset);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_blc.p_blc.m_blc.bl_b_offset != isp_compare_2->p_Isp_blc.p_blc.m_blc.bl_b_offset)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_blc.p_blc.m_blc.bl_b_offset = %d, %d\r\n"), isp_compare_1->p_Isp_blc.p_blc.m_blc.bl_b_offset, isp_compare_2->p_Isp_blc.p_blc.m_blc.bl_b_offset);
		pFile->WriteString(str);
	}

	for (i = 0; i < 9; i++)
	{
		if (isp_compare_1->p_Isp_blc.p_blc.linkage_blc[i].black_level_enable != isp_compare_2->p_Isp_blc.p_blc.linkage_blc[i].black_level_enable)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_blc.p_blc.linkage_blc[%d].black_level_enable = %d, %d\r\n"), i, isp_compare_1->p_Isp_blc.p_blc.linkage_blc[i].black_level_enable, isp_compare_2->p_Isp_blc.p_blc.linkage_blc[i].black_level_enable);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_blc.p_blc.linkage_blc[i].bl_r_a != isp_compare_2->p_Isp_blc.p_blc.linkage_blc[i].bl_r_a)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_blc.p_blc.linkage_blc[%d].bl_r_a = %d, %d\r\n"), i, isp_compare_1->p_Isp_blc.p_blc.linkage_blc[i].bl_r_a, isp_compare_2->p_Isp_blc.p_blc.linkage_blc[i].bl_r_a);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_blc.p_blc.linkage_blc[i].bl_gr_a != isp_compare_2->p_Isp_blc.p_blc.linkage_blc[i].bl_gr_a)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_blc.p_blc.linkage_blc[%d].bl_gr_a = %d, %d\r\n"), i, isp_compare_1->p_Isp_blc.p_blc.linkage_blc[i].bl_gr_a, isp_compare_2->p_Isp_blc.p_blc.linkage_blc[i].bl_gr_a);
			pFile->WriteString(str);
		}
		if (isp_compare_1->p_Isp_blc.p_blc.linkage_blc[i].bl_gb_a != isp_compare_2->p_Isp_blc.p_blc.linkage_blc[i].bl_gb_a)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_blc.p_blc.linkage_blc[%d].bl_gb_a = %d, %d\r\n"), i, isp_compare_1->p_Isp_blc.p_blc.linkage_blc[i].bl_gb_a, isp_compare_2->p_Isp_blc.p_blc.linkage_blc[i].bl_gb_a);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_blc.p_blc.linkage_blc[i].bl_b_a != isp_compare_2->p_Isp_blc.p_blc.linkage_blc[i].bl_b_a)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_blc.p_blc.linkage_blc[%d].bl_b_a = %d, %d\r\n"), i, isp_compare_1->p_Isp_blc.p_blc.linkage_blc[i].bl_b_a, isp_compare_2->p_Isp_blc.p_blc.linkage_blc[i].bl_b_a);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_blc.p_blc.linkage_blc[i].bl_r_offset != isp_compare_2->p_Isp_blc.p_blc.linkage_blc[i].bl_r_offset)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_blc.p_blc.linkage_blc[%d].bl_r_offset = %d, %d\r\n"), i, isp_compare_1->p_Isp_blc.p_blc.linkage_blc[i].bl_r_offset, isp_compare_2->p_Isp_blc.p_blc.linkage_blc[i].bl_r_offset);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_blc.p_blc.linkage_blc[i].bl_gr_offset != isp_compare_2->p_Isp_blc.p_blc.linkage_blc[i].bl_gr_offset)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_blc.p_blc.linkage_blc[%d].bl_gr_offset = %d, %d\r\n"), i, isp_compare_1->p_Isp_blc.p_blc.linkage_blc[i].bl_gr_offset, isp_compare_2->p_Isp_blc.p_blc.linkage_blc[i].bl_gr_offset);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_blc.p_blc.linkage_blc[i].bl_gb_offset != isp_compare_2->p_Isp_blc.p_blc.linkage_blc[i].bl_gb_offset)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_blc.p_blc.linkage_blc[%d].bl_gb_offset = %d, %d\r\n"), i, isp_compare_1->p_Isp_blc.p_blc.linkage_blc[i].bl_gb_offset, isp_compare_2->p_Isp_blc.p_blc.linkage_blc[i].bl_gb_offset);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_blc.p_blc.linkage_blc[i].bl_b_offset != isp_compare_2->p_Isp_blc.p_blc.linkage_blc[i].bl_b_offset)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_blc.p_blc.linkage_blc[%d].bl_b_offset = %d, %d\r\n"), i, isp_compare_1->p_Isp_blc.p_blc.linkage_blc[i].bl_b_offset, isp_compare_2->p_Isp_blc.p_blc.linkage_blc[i].bl_b_offset);
			pFile->WriteString(str);
		}
	}

	//pFile->WriteString(_T("\r\n"));
}


void CCompareDlg::Write_lsc_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) 
{
	CString str;
	UINT i = 0;

	if (isp_compare_1->p_Isp_lsc.param_id != isp_compare_2->p_Isp_lsc.param_id)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_lsc.param_id = %d, %d\r\n"), isp_compare_1->p_Isp_lsc.param_id, isp_compare_2->p_Isp_lsc.param_id);
		pFile->WriteString(str);
	}
	if (isp_compare_1->p_Isp_lsc.length != isp_compare_2->p_Isp_lsc.length)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_lsc.length = %d, %d\r\n"), isp_compare_1->p_Isp_lsc.length, isp_compare_2->p_Isp_lsc.length);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_lsc.lsc.enable != isp_compare_2->p_Isp_lsc.lsc.enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_lsc.lsc.enable = %d, %d\r\n"), isp_compare_1->p_Isp_lsc.lsc.enable, isp_compare_2->p_Isp_lsc.lsc.enable);
		pFile->WriteString(str);
	}
	if (isp_compare_1->p_Isp_lsc.lsc.xref != isp_compare_2->p_Isp_lsc.lsc.xref)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_lsc.lsc.xref = %d, %d\r\n"), isp_compare_1->p_Isp_lsc.lsc.xref, isp_compare_2->p_Isp_lsc.lsc.xref);
		pFile->WriteString(str);
	}
	if (isp_compare_1->p_Isp_lsc.lsc.yref != isp_compare_2->p_Isp_lsc.lsc.yref)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_lsc.lsc.yref = %d, %d\r\n"), isp_compare_1->p_Isp_lsc.lsc.yref, isp_compare_2->p_Isp_lsc.lsc.yref);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_lsc.lsc.lsc_shift != isp_compare_2->p_Isp_lsc.lsc.lsc_shift)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_lsc.lsc.lsc_shift = %d, %d\r\n"), isp_compare_1->p_Isp_lsc.lsc.lsc_shift, isp_compare_2->p_Isp_lsc.lsc.lsc_shift);
		pFile->WriteString(str);
	}

	for (i = 0; i < 10; i++)
	{
		if (isp_compare_1->p_Isp_lsc.lsc.lsc_r_coef.coef_b[i] != isp_compare_2->p_Isp_lsc.lsc.lsc_r_coef.coef_b[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_lsc.lsc.lsc_r_coef.coef_b[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_lsc.lsc.lsc_r_coef.coef_b[i], isp_compare_2->p_Isp_lsc.lsc.lsc_r_coef.coef_b[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_lsc.lsc.lsc_r_coef.coef_c[i] != isp_compare_2->p_Isp_lsc.lsc.lsc_r_coef.coef_c[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_lsc.lsc.lsc_r_coef.coef_c[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_lsc.lsc.lsc_r_coef.coef_c[i], isp_compare_2->p_Isp_lsc.lsc.lsc_r_coef.coef_c[i]);
			pFile->WriteString(str);
		}
	
		if (isp_compare_1->p_Isp_lsc.lsc.lsc_gr_coef.coef_b[i] != isp_compare_2->p_Isp_lsc.lsc.lsc_gr_coef.coef_b[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_lsc.lsc.lsc_gr_coef.coef_b[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_lsc.lsc.lsc_gr_coef.coef_b[i], isp_compare_2->p_Isp_lsc.lsc.lsc_gr_coef.coef_b[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_lsc.lsc.lsc_gr_coef.coef_c[i] != isp_compare_2->p_Isp_lsc.lsc.lsc_gr_coef.coef_c[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_lsc.lsc.lsc_gr_coef.coef_c[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_lsc.lsc.lsc_gr_coef.coef_c[i], isp_compare_2->p_Isp_lsc.lsc.lsc_gr_coef.coef_c[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_lsc.lsc.lsc_gb_coef.coef_b[i] != isp_compare_2->p_Isp_lsc.lsc.lsc_gb_coef.coef_b[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_lsc.lsc.lsc_gb_coef.coef_b[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_lsc.lsc.lsc_gb_coef.coef_b[i], isp_compare_2->p_Isp_lsc.lsc.lsc_gb_coef.coef_b[i]);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_lsc.lsc.lsc_gb_coef.coef_c[i] != isp_compare_2->p_Isp_lsc.lsc.lsc_gb_coef.coef_c[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_lsc.lsc.lsc_gb_coef.coef_c[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_lsc.lsc.lsc_gb_coef.coef_c[i], isp_compare_2->p_Isp_lsc.lsc.lsc_gb_coef.coef_c[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_lsc.lsc.lsc_b_coef.coef_b[i] != isp_compare_2->p_Isp_lsc.lsc.lsc_b_coef.coef_b[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_lsc.lsc.lsc_b_coef.coef_b[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_lsc.lsc.lsc_b_coef.coef_b[i], isp_compare_2->p_Isp_lsc.lsc.lsc_b_coef.coef_b[i]);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_lsc.lsc.lsc_b_coef.coef_c[i] != isp_compare_2->p_Isp_lsc.lsc.lsc_b_coef.coef_c[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_lsc.lsc.lsc_b_coef.coef_c[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_lsc.lsc.lsc_b_coef.coef_c[i], isp_compare_2->p_Isp_lsc.lsc.lsc_b_coef.coef_c[i]);
			pFile->WriteString(str);
		}
	}

	for (i = 0; i < 10; i++)
	{
		if (isp_compare_1->p_Isp_lsc.lsc.range[i] != isp_compare_2->p_Isp_lsc.lsc.range[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_lsc.lsc.range[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_lsc.lsc.range[i], isp_compare_2->p_Isp_lsc.lsc.range[i]);
			pFile->WriteString(str);
		}
	}

	//pFile->WriteString(_T("\r\n"));
}

void CCompareDlg::Write_raw_lut_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) 
{
	CString str;
	UINT i = 0;

	if (isp_compare_1->p_Isp_raw_lut.param_id != isp_compare_2->p_Isp_raw_lut.param_id)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_raw_lut.param_id = %d, %d\r\n"), isp_compare_1->p_Isp_raw_lut.param_id, isp_compare_2->p_Isp_raw_lut.param_id);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_raw_lut.length != isp_compare_2->p_Isp_raw_lut.length)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_raw_lut.length = %d, %d\r\n"), isp_compare_1->p_Isp_raw_lut.length, isp_compare_2->p_Isp_raw_lut.length);
		pFile->WriteString(str);
	}

	for (i = 0; i <129; i++)
	{
		if (isp_compare_1->p_Isp_raw_lut.raw_lut_p.raw_r[i] != isp_compare_2->p_Isp_raw_lut.raw_lut_p.raw_r[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_raw_lut.raw_lut_p.raw_r[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_raw_lut.raw_lut_p.raw_r[i], isp_compare_2->p_Isp_raw_lut.raw_lut_p.raw_r[i]);
			pFile->WriteString(str);
		}
		if (isp_compare_1->p_Isp_raw_lut.raw_lut_p.raw_g[i] != isp_compare_2->p_Isp_raw_lut.raw_lut_p.raw_g[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_raw_lut.raw_lut_p.raw_g[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_raw_lut.raw_lut_p.raw_g[i], isp_compare_2->p_Isp_raw_lut.raw_lut_p.raw_g[i]);
			pFile->WriteString(str);
		}
		if (isp_compare_1->p_Isp_raw_lut.raw_lut_p.raw_b[i] != isp_compare_2->p_Isp_raw_lut.raw_lut_p.raw_b[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_raw_lut.raw_lut_p.raw_b[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_raw_lut.raw_lut_p.raw_b[i], isp_compare_2->p_Isp_raw_lut.raw_lut_p.raw_b[i]);
			pFile->WriteString(str);
		}
	}
	
	for (i = 0; i <16; i++)
	{
		if (isp_compare_1->p_Isp_raw_lut.raw_lut_p.r_key[i] != isp_compare_2->p_Isp_raw_lut.raw_lut_p.r_key[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_raw_lut.raw_lut_p.r_key[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_raw_lut.raw_lut_p.r_key[i], isp_compare_2->p_Isp_raw_lut.raw_lut_p.r_key[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_raw_lut.raw_lut_p.g_key[i] != isp_compare_2->p_Isp_raw_lut.raw_lut_p.g_key[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_raw_lut.raw_lut_p.g_key[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_raw_lut.raw_lut_p.g_key[i], isp_compare_2->p_Isp_raw_lut.raw_lut_p.g_key[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_raw_lut.raw_lut_p.b_key[i] != isp_compare_2->p_Isp_raw_lut.raw_lut_p.b_key[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_raw_lut.raw_lut_p.b_key[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_raw_lut.raw_lut_p.b_key[i], isp_compare_2->p_Isp_raw_lut.raw_lut_p.b_key[i]);
			pFile->WriteString(str);
		}
	}

	if (isp_compare_1->p_Isp_raw_lut.raw_lut_p.raw_gamma_enable != isp_compare_2->p_Isp_raw_lut.raw_lut_p.raw_gamma_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_raw_lut.raw_lut_p.raw_gamma_enable = %d, %d\r\n"), isp_compare_1->p_Isp_raw_lut.raw_lut_p.raw_gamma_enable, isp_compare_2->p_Isp_raw_lut.raw_lut_p.raw_gamma_enable);
		pFile->WriteString(str);
	}
	
	//pFile->WriteString(_T("\r\n"));
}

void CCompareDlg::Write_nr_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) 
{
	CString str;
	UINT i = 0, j = 0;

	if (isp_compare_1->p_Isp_nr.param_id != isp_compare_2->p_Isp_nr.param_id)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_nr.param_id = %d, %d\r\n"), isp_compare_1->p_Isp_nr.param_id, isp_compare_2->p_Isp_nr.param_id);
		pFile->WriteString(str);
	}
	
	if (isp_compare_1->p_Isp_nr.length != isp_compare_2->p_Isp_nr.length)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_nr.length = %d, %d\r\n"), isp_compare_1->p_Isp_nr.length, isp_compare_2->p_Isp_nr.length);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_nr.p_nr1.nr1_mode != isp_compare_2->p_Isp_nr.p_nr1.nr1_mode)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_nr.p_nr1.nr1_mode = %d, %d\r\n"), isp_compare_1->p_Isp_nr.p_nr1.nr1_mode, isp_compare_2->p_Isp_nr.p_nr1.nr1_mode);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_nr.p_nr1.manual_nr1.nr1_enable != isp_compare_2->p_Isp_nr.p_nr1.manual_nr1.nr1_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_nr.p_nr1.manual_nr1.nr1_enable = %d, %d\r\n"), isp_compare_1->p_Isp_nr.p_nr1.manual_nr1.nr1_enable, isp_compare_2->p_Isp_nr.p_nr1.manual_nr1.nr1_enable);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_nr.p_nr1.manual_nr1.nr1_k != isp_compare_2->p_Isp_nr.p_nr1.manual_nr1.nr1_k)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_nr.p_nr1.manual_nr1.nr1_k = %d, %d\r\n"), isp_compare_1->p_Isp_nr.p_nr1.manual_nr1.nr1_k, isp_compare_2->p_Isp_nr.p_nr1.manual_nr1.nr1_k);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_nr.p_nr1.manual_nr1.nr1_calc_g_k != isp_compare_2->p_Isp_nr.p_nr1.manual_nr1.nr1_calc_g_k)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_nr.p_nr1.manual_nr1.nr1_calc_g_k = %d, %d\r\n"), isp_compare_1->p_Isp_nr.p_nr1.manual_nr1.nr1_calc_g_k, isp_compare_2->p_Isp_nr.p_nr1.manual_nr1.nr1_calc_g_k);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_nr.p_nr1.manual_nr1.nr1_calc_r_k != isp_compare_2->p_Isp_nr.p_nr1.manual_nr1.nr1_calc_r_k)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_nr.p_nr1.manual_nr1.nr1_calc_r_k = %d, %d\r\n"), isp_compare_1->p_Isp_nr.p_nr1.manual_nr1.nr1_calc_r_k, isp_compare_2->p_Isp_nr.p_nr1.manual_nr1.nr1_calc_r_k);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_nr.p_nr1.manual_nr1.nr1_calc_b_k != isp_compare_2->p_Isp_nr.p_nr1.manual_nr1.nr1_calc_b_k)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_nr.p_nr1.manual_nr1.nr1_calc_b_k = %d, %d\r\n"), isp_compare_1->p_Isp_nr.p_nr1.manual_nr1.nr1_calc_b_k, isp_compare_2->p_Isp_nr.p_nr1.manual_nr1.nr1_calc_b_k);
		pFile->WriteString(str);
	}

	for (i = 0; i < 17; i++)
	{
		if (isp_compare_1->p_Isp_nr.p_nr1.manual_nr1.nr1_weight_rtbl[i] != isp_compare_2->p_Isp_nr.p_nr1.manual_nr1.nr1_weight_rtbl[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_nr.p_nr1.manual_nr1.nr1_weight_rtbl[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_nr.p_nr1.manual_nr1.nr1_weight_rtbl[i], isp_compare_2->p_Isp_nr.p_nr1.manual_nr1.nr1_weight_rtbl[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_nr.p_nr1.manual_nr1.nr1_weight_gtbl[i] != isp_compare_2->p_Isp_nr.p_nr1.manual_nr1.nr1_weight_gtbl[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_nr.p_nr1.manual_nr1.nr1_weight_gtbl[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_nr.p_nr1.manual_nr1.nr1_weight_gtbl[i], isp_compare_2->p_Isp_nr.p_nr1.manual_nr1.nr1_weight_gtbl[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_nr.p_nr1.manual_nr1.nr1_weight_btbl[i] != isp_compare_2->p_Isp_nr.p_nr1.manual_nr1.nr1_weight_btbl[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_nr.p_nr1.manual_nr1.nr1_weight_btbl[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_nr.p_nr1.manual_nr1.nr1_weight_btbl[i], isp_compare_2->p_Isp_nr.p_nr1.manual_nr1.nr1_weight_btbl[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_nr.p_nr1.manual_nr1.nr1_lc_lut[i] != isp_compare_2->p_Isp_nr.p_nr1.manual_nr1.nr1_lc_lut[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_nr.p_nr1.manual_nr1.nr1_weight_btbl[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_nr.p_nr1.manual_nr1.nr1_lc_lut[i], isp_compare_2->p_Isp_nr.p_nr1.manual_nr1.nr1_lc_lut[i]);
			pFile->WriteString(str);
		}
	}

	for (i = 0; i < 16; i++)
	{
		if (isp_compare_1->p_Isp_nr.p_nr1.manual_nr1.nr1_lc_lut_key[i] != isp_compare_2->p_Isp_nr.p_nr1.manual_nr1.nr1_lc_lut_key[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_nr.p_nr1.manual_nr1.nr1_lc_lut_key[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_nr.p_nr1.manual_nr1.nr1_lc_lut_key[i], isp_compare_2->p_Isp_nr.p_nr1.manual_nr1.nr1_lc_lut_key[i]);
			pFile->WriteString(str);
		}
	}

	for (j = 0; j < 9; j++)
	{
		if (isp_compare_1->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_enable != isp_compare_2->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_enable)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_nr.p_nr1.linkage_nr1[%d].nr1_enable = %d, %d\r\n"), j, isp_compare_1->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_enable, isp_compare_2->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_enable);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_k != isp_compare_2->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_k)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_nr.p_nr1.linkage_nr1[%d].nr1_k = %d, %d\r\n"), j, isp_compare_1->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_k, isp_compare_2->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_k);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_calc_g_k != isp_compare_2->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_calc_g_k)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_nr.p_nr1.linkage_nr1[%d].nr1_calc_g_k = %d, %d\r\n"), j,isp_compare_1->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_calc_g_k, isp_compare_2->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_calc_g_k);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_calc_r_k != isp_compare_2->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_calc_r_k)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_nr.p_nr1.linkage_nr1[%d].nr1_calc_r_k = %d, %d\r\n"), j, isp_compare_1->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_calc_r_k, isp_compare_2->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_calc_r_k);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_calc_b_k != isp_compare_2->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_calc_b_k)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_nr.p_nr1.linkage_nr1[%d].nr1_calc_b_k = %d, %d\r\n"), j, isp_compare_1->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_calc_b_k, isp_compare_2->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_calc_b_k);
			pFile->WriteString(str);
		}
		
		for (i = 0; i < 17; i++)
		{
			if (isp_compare_1->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_weight_rtbl[i] != isp_compare_2->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_weight_rtbl[i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_nr.p_nr1.linkage_nr1[%d].nr1_weight_rtbl[%d] = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_weight_rtbl[i], isp_compare_2->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_weight_rtbl[i]);
				pFile->WriteString(str);
			}
			
			if (isp_compare_1->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_weight_gtbl[i] != isp_compare_2->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_weight_gtbl[i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_nr.p_nr1.linkage_nr1[%d].nr1_weight_gtbl[%d] = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_weight_gtbl[i], isp_compare_2->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_weight_gtbl[i]);
				pFile->WriteString(str);
			}
			
			if (isp_compare_1->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_weight_btbl[i] != isp_compare_2->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_weight_btbl[i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_nr.p_nr1.linkage_nr1[%d].nr1_weight_btbl[%d] = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_weight_btbl[i], isp_compare_2->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_weight_btbl[i]);
				pFile->WriteString(str);
			}
			
			if (isp_compare_1->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_lc_lut[i] != isp_compare_2->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_lc_lut[i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_nr.p_nr1.linkage_nr1[%d].nr1_weight_btbl[%d] = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_lc_lut[i], isp_compare_2->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_lc_lut[i]);
				pFile->WriteString(str);
			}
		}
		
		for (i = 0; i < 16; i++)
		{
			if (isp_compare_1->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_lc_lut_key[i] != isp_compare_2->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_lc_lut_key[i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_nr.p_nr1.linkage_nr1[%d].nr1_lc_lut_key[%d] = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_lc_lut_key[i], isp_compare_2->p_Isp_nr.p_nr1.linkage_nr1[j].nr1_lc_lut_key[i]);
				pFile->WriteString(str);
			}
		}
	}



	//NR2
	if (isp_compare_1->p_Isp_nr.p_nr2.nr2_mode != isp_compare_2->p_Isp_nr.p_nr2.nr2_mode)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_nr.p_nr2.nr2_mode = %d, %d\r\n"), isp_compare_1->p_Isp_nr.p_nr2.nr2_mode, isp_compare_2->p_Isp_nr.p_nr2.nr2_mode);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_nr.p_nr2.manual_nr2.nr2_enable != isp_compare_2->p_Isp_nr.p_nr2.manual_nr2.nr2_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_nr.p_nr2.manual_nr2.nr2_enable = %d, %d\r\n"), isp_compare_1->p_Isp_nr.p_nr2.manual_nr2.nr2_enable, isp_compare_2->p_Isp_nr.p_nr2.manual_nr2.nr2_enable);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_nr.p_nr2.manual_nr2.nr2_k != isp_compare_2->p_Isp_nr.p_nr2.manual_nr2.nr2_k)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_nr.p_nr2.manual_nr2.nr2_k = %d, %d\r\n"), isp_compare_1->p_Isp_nr.p_nr2.manual_nr2.nr2_k, isp_compare_2->p_Isp_nr.p_nr2.manual_nr2.nr2_k);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_nr.p_nr2.manual_nr2.nr2_calc_y_k != isp_compare_2->p_Isp_nr.p_nr2.manual_nr2.nr2_calc_y_k)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_nr.p_nr2.manual_nr2.nr2_calc_y_k = %d, %d\r\n"), isp_compare_1->p_Isp_nr.p_nr2.manual_nr2.nr2_calc_y_k, isp_compare_2->p_Isp_nr.p_nr2.manual_nr2.nr2_calc_y_k);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_nr.p_nr2.manual_nr2.y_dpc_th != isp_compare_2->p_Isp_nr.p_nr2.manual_nr2.y_dpc_th)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_nr.p_nr2.manual_nr2.y_dpc_th = %d, %d\r\n"), isp_compare_1->p_Isp_nr.p_nr2.manual_nr2.y_dpc_th, isp_compare_2->p_Isp_nr.p_nr2.manual_nr2.y_dpc_th);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_nr.p_nr2.manual_nr2.y_black_dpc_enable != isp_compare_2->p_Isp_nr.p_nr2.manual_nr2.y_black_dpc_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_nr.p_nr2.manual_nr2.y_black_dpc_enable = %d, %d\r\n"), isp_compare_1->p_Isp_nr.p_nr2.manual_nr2.y_black_dpc_enable, isp_compare_2->p_Isp_nr.p_nr2.manual_nr2.y_black_dpc_enable);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_nr.p_nr2.manual_nr2.y_white_dpc_enable != isp_compare_2->p_Isp_nr.p_nr2.manual_nr2.y_white_dpc_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_nr.p_nr2.manual_nr2.y_white_dpc_enable = %d, %d\r\n"), isp_compare_1->p_Isp_nr.p_nr2.manual_nr2.y_white_dpc_enable, isp_compare_2->p_Isp_nr.p_nr2.manual_nr2.y_white_dpc_enable);
		pFile->WriteString(str);
	}

	for (i = 0; i < 17; i++)
	{
		if (isp_compare_1->p_Isp_nr.p_nr2.manual_nr2.nr2_weight_tbl[i] != isp_compare_2->p_Isp_nr.p_nr2.manual_nr2.nr2_weight_tbl[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_nr.p_nr2.manual_nr2.nr2_weight_tbl[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_nr.p_nr2.manual_nr2.nr2_weight_tbl[i], isp_compare_2->p_Isp_nr.p_nr2.manual_nr2.nr2_weight_tbl[i]);
			pFile->WriteString(str);
		}

	}

	for (j = 0; j < 9; j++)
	{
		if (isp_compare_1->p_Isp_nr.p_nr2.linkage_nr2[j].nr2_enable != isp_compare_2->p_Isp_nr.p_nr2.linkage_nr2[j].nr2_enable)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_nr.p_nr2.linkage_nr2[%d].nr2_enable = %d, %d\r\n"), j, isp_compare_1->p_Isp_nr.p_nr2.linkage_nr2[j].nr2_enable, isp_compare_2->p_Isp_nr.p_nr2.linkage_nr2[j].nr2_enable);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_nr.p_nr2.linkage_nr2[j].nr2_k != isp_compare_2->p_Isp_nr.p_nr2.linkage_nr2[j].nr2_k)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_nr.p_nr2.linkage_nr2[%d].nr2_k = %d, %d\r\n"), j, isp_compare_1->p_Isp_nr.p_nr2.linkage_nr2[j].nr2_k, isp_compare_2->p_Isp_nr.p_nr2.linkage_nr2[j].nr2_k);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_nr.p_nr2.linkage_nr2[j].nr2_calc_y_k != isp_compare_2->p_Isp_nr.p_nr2.linkage_nr2[j].nr2_calc_y_k)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_nr.p_nr2.linkage_nr2[%d].nr2_calc_y_k = %d, %d\r\n"), j,isp_compare_1->p_Isp_nr.p_nr2.linkage_nr2[j].nr2_calc_y_k, isp_compare_2->p_Isp_nr.p_nr2.linkage_nr2[j].nr2_calc_y_k);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_nr.p_nr2.linkage_nr2[j].y_dpc_th != isp_compare_2->p_Isp_nr.p_nr2.linkage_nr2[j].y_dpc_th)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_nr.p_nr2.linkage_nr2[%d].y_dpc_th = %d, %d\r\n"), j, isp_compare_1->p_Isp_nr.p_nr2.linkage_nr2[j].y_dpc_th, isp_compare_2->p_Isp_nr.p_nr2.linkage_nr2[j].y_dpc_th);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_nr.p_nr2.linkage_nr2[j].y_black_dpc_enable != isp_compare_2->p_Isp_nr.p_nr2.linkage_nr2[j].y_black_dpc_enable)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_nr.p_nr2.linkage_nr2[%d].y_black_dpc_enable = %d, %d\r\n"), j, isp_compare_1->p_Isp_nr.p_nr2.linkage_nr2[j].y_black_dpc_enable, isp_compare_2->p_Isp_nr.p_nr2.linkage_nr2[j].y_black_dpc_enable);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_nr.p_nr2.linkage_nr2[j].y_white_dpc_enable != isp_compare_2->p_Isp_nr.p_nr2.linkage_nr2[j].y_white_dpc_enable)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_nr.p_nr2.linkage_nr2[%d].y_white_dpc_enable = %d, %d\r\n"), j, isp_compare_1->p_Isp_nr.p_nr2.linkage_nr2[j].y_white_dpc_enable, isp_compare_2->p_Isp_nr.p_nr2.linkage_nr2[j].y_white_dpc_enable);
			pFile->WriteString(str);
		}
		
		for (i = 0; i < 17; i++)
		{
			if (isp_compare_1->p_Isp_nr.p_nr2.linkage_nr2[j].nr2_weight_tbl[i] != isp_compare_2->p_Isp_nr.p_nr2.linkage_nr2[j].nr2_weight_tbl[i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_nr.p_nr2.linkage_nr2[%d].nr2_weight_tbl[%d] = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_nr.p_nr2.linkage_nr2[j].nr2_weight_tbl[i], isp_compare_2->p_Isp_nr.p_nr2.linkage_nr2[j].nr2_weight_tbl[i]);
				pFile->WriteString(str);
			}
		}
		
	}

	
	//pFile->WriteString(_T("\r\n"));
}

void CCompareDlg::Write_3dnr_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) 
{
	CString str;
	UINT i = 0, j = 0;

	if (isp_compare_1->p_Isp_3dnr.param_id != isp_compare_2->p_Isp_3dnr.param_id)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.param_id = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.param_id, isp_compare_2->p_Isp_3dnr.param_id);
		pFile->WriteString(str);
	}
	
	if (isp_compare_1->p_Isp_3dnr.length != isp_compare_2->p_Isp_3dnr.length)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.length = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.length, isp_compare_2->p_Isp_3dnr.length);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.isp_3d_nr_mode != isp_compare_2->p_Isp_3dnr.p_3d_nr.isp_3d_nr_mode)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.isp_3d_nr_mode = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.isp_3d_nr_mode, isp_compare_2->p_Isp_3dnr.p_3d_nr.isp_3d_nr_mode);
		pFile->WriteString(str);
	}

	//
	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.uv_min_enable != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.uv_min_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.uv_min_enable = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.uv_min_enable, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.uv_min_enable);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_y_enable != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_y_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_y_enable = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_y_enable, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_y_enable);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_uv_enable != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_uv_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_uv_enable = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_uv_enable, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_uv_enable);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.updata_ref_y != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.updata_ref_y)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.updata_ref_y = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.updata_ref_y, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.updata_ref_y);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.updata_ref_uv != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.updata_ref_uv)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.updata_ref_uv = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.updata_ref_uv, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.updata_ref_uv);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_refFrame_format  != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_refFrame_format)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_refFrame_format = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_refFrame_format, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_refFrame_format);
		pFile->WriteString(str);
	}
	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.y_2dnr_enable != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.y_2dnr_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.y_2dnr_enable = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.y_2dnr_enable, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.y_2dnr_enable);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.uv_2dnr_enable != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.uv_2dnr_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.uv_2dnr_enable = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.uv_2dnr_enable, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.uv_2dnr_enable);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.uvnr_k != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.uvnr_k)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.uvnr_k = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.uvnr_k, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.uvnr_k);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.uvlp_k != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.uvlp_k)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.uvlp_k = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.uvlp_k, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.uvlp_k);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_k != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_k)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_k = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_k, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_k);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_minstep != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_minstep)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_minstep = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_minstep, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_minstep);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_mf_th1 != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_mf_th1)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_mf_th1 = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_mf_th1, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_mf_th1);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_mf_th2 != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_mf_th2)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_mf_th2 = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_mf_th2, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_mf_th2);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_diffth_k1 != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_diffth_k1)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_diffth_k1 = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_diffth_k1, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_diffth_k1);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_diffth_k2 != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_diffth_k2)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_diffth_k2 = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_diffth_k2, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_diffth_k2);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_diffth_slop != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_diffth_slop)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_diffth_slop = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_diffth_slop, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_diffth_slop);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_mc_k != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_mc_k)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_mc_k = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_mc_k, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_mc_k);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_ac_th != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_ac_th)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_ac_th = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_ac_th, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_ac_th);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_calc_k != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_calc_k)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_calc_k = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_calc_k, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_calc_k);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_k != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_k)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_k = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_k, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_k);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_diff_shift != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_diff_shift)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_diff_shift = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_diff_shift, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_diff_shift);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.ylp_k != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.ylp_k)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.ylp_k = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.ylp_k, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.ylp_k);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_th1 != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_th1)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_th1 = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_th1, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_th1);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_k1 != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_k1)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_k1 = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_k1, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_k1);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_k2 != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_k2)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_k2 = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_k2, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_k2);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_kslop != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_kslop)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_kslop = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_kslop, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_kslop);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_minstep != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_minstep)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_minstep = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_minstep, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_minstep);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_mf_th1 != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_mf_th1)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_mf_th1 = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_mf_th1, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_mf_th1);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_mf_th2 != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_mf_th2)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_mf_th2 = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_mf_th2, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_mf_th2);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_diffth_k1 != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_diffth_k1)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_diffth_k1 = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_diffth_k1, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_diffth_k1);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_diffth_k2 != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_diffth_k2)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_diffth_k2 = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_diffth_k2, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_diffth_k2);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_diffth_slop != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_diffth_slop)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_diffth_slop = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_diffth_slop, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_diffth_slop);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_mc_k != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_mc_k)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_mc_k = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_mc_k, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_mc_k);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_ac_th != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_ac_th)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_ac_th = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_ac_th, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_ac_th);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.md_th != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.md_th)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.md_th = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.md_th, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.md_th);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_t_y_ex_k_cfg != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_t_y_ex_k_cfg)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_t_y_ex_k_cfg = %d, %d\r\n"), isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_t_y_ex_k_cfg, isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_t_y_ex_k_cfg);
		pFile->WriteString(str);
	}

	for (j=0; j<17; j++)
	{
		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_weight_tbl[j] != isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_weight_tbl[j])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_weight_tbl[%d] = %d, %d\r\n"), j, isp_compare_1->p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_weight_tbl[j], isp_compare_2->p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_weight_tbl[j]);
			pFile->WriteString(str);
		}
	}


	for (i = 0; i < 9; i++)
	{
		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].uv_min_enable != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].uv_min_enable)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].uv_min_enable = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].uv_min_enable, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].uv_min_enable);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].tnr_y_enable != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].tnr_y_enable)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].tnr_y_enable = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].tnr_y_enable, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].tnr_y_enable);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].tnr_uv_enable != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].tnr_uv_enable)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].tnr_uv_enable = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].tnr_uv_enable, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].tnr_uv_enable);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].updata_ref_y != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].updata_ref_y)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].updata_ref_y = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].updata_ref_y, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].updata_ref_y);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].updata_ref_uv != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].updata_ref_uv)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].updata_ref_uv = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].updata_ref_uv, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].updata_ref_uv);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].tnr_refFrame_format  != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].tnr_refFrame_format)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].tnr_refFrame_format = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].tnr_refFrame_format, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].tnr_refFrame_format);
			pFile->WriteString(str);
		}
		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].y_2dnr_enable != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].y_2dnr_enable)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].y_2dnr_enable = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].y_2dnr_enable, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].y_2dnr_enable);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].uv_2dnr_enable != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].uv_2dnr_enable)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].uv_2dnr_enable = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].uv_2dnr_enable, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].uv_2dnr_enable);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].uvnr_k != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].uvnr_k)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].uvnr_k = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].uvnr_k, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].uvnr_k);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].uvlp_k != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].uvlp_k)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].uvlp_k = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].uvlp_k, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].uvlp_k);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_k != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_k)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].t_uv_k = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_k, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_k);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_minstep != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_minstep)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].t_uv_minstep = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_minstep, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_minstep);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_mf_th1 != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_mf_th1)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].t_uv_mf_th1 = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_mf_th1, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_mf_th1);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_mf_th2 != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_mf_th2)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].t_uv_mf_th2 = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_mf_th2, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_mf_th2);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_diffth_k1 != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_diffth_k1)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].t_uv_diffth_k1 = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_diffth_k1, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_diffth_k1);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_diffth_k2 != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_diffth_k2)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].t_uv_diffth_k2 = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_diffth_k2, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_diffth_k2);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_diffth_slop != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_diffth_slop)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].t_uv_diffth_slop = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_diffth_slop, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_diffth_slop);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_mc_k != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_mc_k)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].t_uv_mc_k = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_mc_k, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_mc_k);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_ac_th != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_ac_th)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].t_uv_ac_th = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_ac_th, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_uv_ac_th);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].ynr_calc_k != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].ynr_calc_k)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].ynr_calc_k = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].ynr_calc_k, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].ynr_calc_k);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].ynr_k != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].ynr_k)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].ynr_k = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].ynr_k, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].ynr_k);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].ynr_diff_shift != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].ynr_diff_shift)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].ynr_diff_shift = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].ynr_diff_shift, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].ynr_diff_shift);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].ylp_k != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].ylp_k)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].ylp_k = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].ylp_k, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].ylp_k);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_th1 != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_th1)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].t_y_th1 = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_th1, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_th1);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_k1 != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_k1)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].t_y_k1 = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_k1, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_k1);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_k2 != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_k2)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].t_y_k2 = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_k2, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_k2);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_kslop != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_kslop)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].t_y_kslop = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_kslop, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_kslop);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_minstep != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_minstep)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].t_y_minstep = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_minstep, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_minstep);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_mf_th1 != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_mf_th1)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].t_y_mf_th1 = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_mf_th1, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_mf_th1);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_mf_th2 != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_mf_th2)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].t_y_mf_th2 = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_mf_th2, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_mf_th2);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_diffth_k1 != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_diffth_k1)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].t_y_diffth_k1 = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_diffth_k1, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_diffth_k1);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_diffth_k2 != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_diffth_k2)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].t_y_diffth_k2 = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_diffth_k2, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_diffth_k2);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_diffth_slop != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_diffth_slop)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].t_y_diffth_slop = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_diffth_slop, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_diffth_slop);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_mc_k != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_mc_k)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].t_y_mc_k = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_mc_k, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_mc_k);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_ac_th != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_ac_th)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].t_y_ac_th = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_ac_th, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].t_y_ac_th);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].md_th != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].md_th)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].md_th = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].md_th, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].md_th);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].tnr_t_y_ex_k_cfg != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].tnr_t_y_ex_k_cfg)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].tnr_t_y_ex_k_cfg = %d, %d\r\n"), i, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].tnr_t_y_ex_k_cfg, isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].tnr_t_y_ex_k_cfg);
			pFile->WriteString(str);
		}

		for (j=0; j<17; j++)
		{
			if (isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].ynr_weight_tbl[j] != isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].ynr_weight_tbl[j])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_3dnr.p_3d_nr.linkage_3d_nr[%d].ynr_weight_tbl[%d] = %d, %d\r\n"), i, j, isp_compare_1->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].ynr_weight_tbl[j], isp_compare_2->p_Isp_3dnr.p_3d_nr.linkage_3d_nr[i].ynr_weight_tbl[j]);
				pFile->WriteString(str);
			}
		}
	}
	
//	pFile->WriteString(_T("\r\n"));
}

void CCompareDlg::Write_gb_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) 
{
	CString str;
	UINT i = 0;

	if (isp_compare_1->p_Isp_gb.param_id != isp_compare_2->p_Isp_gb.param_id)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_gb.param_id = %d, %d\r\n"), isp_compare_1->p_Isp_gb.param_id, isp_compare_2->p_Isp_gb.param_id);
		pFile->WriteString(str);
	}
	
	if (isp_compare_1->p_Isp_gb.length != isp_compare_2->p_Isp_gb.length)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_gb.length = %d, %d\r\n"), isp_compare_1->p_Isp_gb.length, isp_compare_2->p_Isp_gb.length);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_gb.p_gb.gb_mode != isp_compare_2->p_Isp_gb.p_gb.gb_mode)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_gb.p_gb.gb_mode = %d, %d\r\n"), isp_compare_1->p_Isp_gb.p_gb.gb_mode, isp_compare_2->p_Isp_gb.p_gb.gb_mode);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_gb.p_gb.manual_gb.gb_enable != isp_compare_2->p_Isp_gb.p_gb.manual_gb.gb_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_gb.p_gb.manual_gb.gb_enable = %d, %d\r\n"), isp_compare_1->p_Isp_gb.p_gb.manual_gb.gb_enable, isp_compare_2->p_Isp_gb.p_gb.manual_gb.gb_enable);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_gb.p_gb.manual_gb.gb_en_th != isp_compare_2->p_Isp_gb.p_gb.manual_gb.gb_en_th)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_gb.p_gb.manual_gb.gb_en_th = %d, %d\r\n"), isp_compare_1->p_Isp_gb.p_gb.manual_gb.gb_en_th, isp_compare_2->p_Isp_gb.p_gb.manual_gb.gb_en_th);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_gb.p_gb.manual_gb.gb_kstep != isp_compare_2->p_Isp_gb.p_gb.manual_gb.gb_kstep)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_gb.p_gb.manual_gb.gb_kstep = %d, %d\r\n"), isp_compare_1->p_Isp_gb.p_gb.manual_gb.gb_kstep, isp_compare_2->p_Isp_gb.p_gb.manual_gb.gb_kstep);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_gb.p_gb.manual_gb.gb_threshold != isp_compare_2->p_Isp_gb.p_gb.manual_gb.gb_threshold)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_gb.p_gb.manual_gb.gb_threshold = %d, %d\r\n"), isp_compare_1->p_Isp_gb.p_gb.manual_gb.gb_threshold, isp_compare_2->p_Isp_gb.p_gb.manual_gb.gb_threshold);
		pFile->WriteString(str);
	}

	for (i = 0; i < 9; i++)
	{
		if (isp_compare_1->p_Isp_gb.p_gb.linkage_gb[i].gb_enable != isp_compare_2->p_Isp_gb.p_gb.linkage_gb[i].gb_enable)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_gb.p_gb.linkage_gb[%d].gb_enable = %d, %d\r\n"), i, isp_compare_1->p_Isp_gb.p_gb.linkage_gb[i].gb_enable, isp_compare_2->p_Isp_gb.p_gb.linkage_gb[i].gb_enable);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_gb.p_gb.linkage_gb[i].gb_en_th != isp_compare_2->p_Isp_gb.p_gb.linkage_gb[i].gb_en_th)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_gb.p_gb.linkage_gb[%d].gb_en_th = %d, %d\r\n"), i, isp_compare_1->p_Isp_gb.p_gb.linkage_gb[i].gb_en_th, isp_compare_2->p_Isp_gb.p_gb.linkage_gb[i].gb_en_th);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_gb.p_gb.linkage_gb[i].gb_kstep != isp_compare_2->p_Isp_gb.p_gb.linkage_gb[i].gb_kstep)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_gb.p_gb.linkage_gb[%d].gb_kstep = %d, %d\r\n"), i, isp_compare_1->p_Isp_gb.p_gb.linkage_gb[i].gb_kstep, isp_compare_2->p_Isp_gb.p_gb.linkage_gb[i].gb_kstep);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_gb.p_gb.linkage_gb[i].gb_threshold != isp_compare_2->p_Isp_gb.p_gb.linkage_gb[i].gb_threshold)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_gb.p_gb.linkage_gb[%d].gb_threshold = %d, %d\r\n"), i, isp_compare_1->p_Isp_gb.p_gb.linkage_gb[i].gb_threshold, isp_compare_2->p_Isp_gb.p_gb.linkage_gb[i].gb_threshold);
			pFile->WriteString(str);
		}
	}

	//pFile->WriteString(_T("\r\n"));
}

void CCompareDlg::Write_demo_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) 
{
	CString str;
	UINT i = 0;

	if (isp_compare_1->p_Isp_demo.param_id != isp_compare_2->p_Isp_demo.param_id)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_demo.param_id = %d, %d\r\n"), isp_compare_1->p_Isp_demo.param_id, isp_compare_2->p_Isp_demo.param_id);
		pFile->WriteString(str);
	}
	
	if (isp_compare_1->p_Isp_demo.length != isp_compare_2->p_Isp_demo.length)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_demo.length = %d, %d\r\n"), isp_compare_1->p_Isp_demo.length, isp_compare_2->p_Isp_demo.length);
		pFile->WriteString(str);
	}


	if (isp_compare_1->p_Isp_demo.p_demo_attr.dm_HV_th != isp_compare_2->p_Isp_demo.p_demo_attr.dm_HV_th)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_demo.p_demo_attr.dm_HV_th = %d, %d\r\n"), isp_compare_1->p_Isp_demo.p_demo_attr.dm_HV_th, isp_compare_2->p_Isp_demo.p_demo_attr.dm_HV_th);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_demo.p_demo_attr.dm_rg_thre != isp_compare_2->p_Isp_demo.p_demo_attr.dm_rg_thre)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_demo.p_demo_attr.dm_rg_thre = %d, %d\r\n"), isp_compare_1->p_Isp_demo.p_demo_attr.dm_rg_thre, isp_compare_2->p_Isp_demo.p_demo_attr.dm_rg_thre);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_demo.p_demo_attr.dm_bg_thre != isp_compare_2->p_Isp_demo.p_demo_attr.dm_bg_thre)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_demo.p_demo_attr.dm_bg_thre = %d, %d\r\n"), isp_compare_1->p_Isp_demo.p_demo_attr.dm_bg_thre, isp_compare_2->p_Isp_demo.p_demo_attr.dm_bg_thre);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_demo.p_demo_attr.dm_hf_th1 != isp_compare_2->p_Isp_demo.p_demo_attr.dm_hf_th1)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_demo.p_demo_attr.dm_hf_th1 = %d, %d\r\n"), isp_compare_1->p_Isp_demo.p_demo_attr.dm_hf_th1, isp_compare_2->p_Isp_demo.p_demo_attr.dm_hf_th1);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_demo.p_demo_attr.dm_hf_th2 != isp_compare_2->p_Isp_demo.p_demo_attr.dm_hf_th2)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_demo.p_demo_attr.dm_hf_th2 = %d, %d\r\n"), isp_compare_1->p_Isp_demo.p_demo_attr.dm_hf_th2, isp_compare_2->p_Isp_demo.p_demo_attr.dm_hf_th2);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_demo.p_demo_attr.dm_rg_gain != isp_compare_2->p_Isp_demo.p_demo_attr.dm_rg_gain)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_demo.p_demo_attr.dm_rg_gain = %d, %d\r\n"), isp_compare_1->p_Isp_demo.p_demo_attr.dm_rg_gain, isp_compare_2->p_Isp_demo.p_demo_attr.dm_rg_gain);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_demo.p_demo_attr.dm_bg_gain != isp_compare_2->p_Isp_demo.p_demo_attr.dm_bg_gain)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_demo.p_demo_attr.dm_bg_gain = %d, %d\r\n"), isp_compare_1->p_Isp_demo.p_demo_attr.dm_bg_gain, isp_compare_2->p_Isp_demo.p_demo_attr.dm_bg_gain);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_demo.p_demo_attr.dm_gr_gain != isp_compare_2->p_Isp_demo.p_demo_attr.dm_gr_gain)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_demo.p_demo_attr.dm_gr_gain = %d, %d\r\n"), isp_compare_1->p_Isp_demo.p_demo_attr.dm_gr_gain, isp_compare_2->p_Isp_demo.p_demo_attr.dm_gr_gain);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_demo.p_demo_attr.dm_gb_gain != isp_compare_2->p_Isp_demo.p_demo_attr.dm_gb_gain)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_demo.p_demo_attr.dm_gb_gain = %d, %d\r\n"), isp_compare_1->p_Isp_demo.p_demo_attr.dm_gb_gain, isp_compare_2->p_Isp_demo.p_demo_attr.dm_gb_gain);
		pFile->WriteString(str);
	}
	
	//pFile->WriteString(_T("\r\n"));
}

void CCompareDlg::Write_gamma_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) 
{
	CString str;
	UINT i = 0;

	if (isp_compare_1->p_Isp_gamma.param_id != isp_compare_2->p_Isp_gamma.param_id)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_gamma.param_id = %d, %d\r\n"), isp_compare_1->p_Isp_gamma.param_id, isp_compare_2->p_Isp_gamma.param_id);
		pFile->WriteString(str);
	}
	
	if (isp_compare_1->p_Isp_gamma.length != isp_compare_2->p_Isp_gamma.length)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_gamma.length = %d, %d\r\n"), isp_compare_1->p_Isp_gamma.length, isp_compare_2->p_Isp_gamma.length);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_gamma.p_gamma_attr.rgb_gamma_enable != isp_compare_2->p_Isp_gamma.p_gamma_attr.rgb_gamma_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_gamma.p_gamma_attr.rgb_gamma_enable = %d, %d\r\n"), isp_compare_1->p_Isp_gamma.p_gamma_attr.rgb_gamma_enable, isp_compare_2->p_Isp_gamma.p_gamma_attr.rgb_gamma_enable);
		pFile->WriteString(str);
	}

	for (i = 0; i < 129; i++)
	{
		if (isp_compare_1->p_Isp_gamma.p_gamma_attr.r_gamma[i] != isp_compare_2->p_Isp_gamma.p_gamma_attr.r_gamma[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_gamma.p_gamma_attr.r_gamma[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_gamma.p_gamma_attr.r_gamma[i], isp_compare_2->p_Isp_gamma.p_gamma_attr.r_gamma[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_gamma.p_gamma_attr.g_gamma[i] != isp_compare_2->p_Isp_gamma.p_gamma_attr.g_gamma[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_gamma.p_gamma_attr.g_gamma[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_gamma.p_gamma_attr.g_gamma[i], isp_compare_2->p_Isp_gamma.p_gamma_attr.g_gamma[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_gamma.p_gamma_attr.b_gamma[i] != isp_compare_2->p_Isp_gamma.p_gamma_attr.b_gamma[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_gamma.p_gamma_attr.b_gamma[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_gamma.p_gamma_attr.b_gamma[i], isp_compare_2->p_Isp_gamma.p_gamma_attr.b_gamma[i]);
			pFile->WriteString(str);
		}
	}

	for (i = 0; i < 16; i++)
	{
		if (isp_compare_1->p_Isp_gamma.p_gamma_attr.r_key[i] != isp_compare_2->p_Isp_gamma.p_gamma_attr.r_key[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_gamma.p_gamma_attr.r_key[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_gamma.p_gamma_attr.r_key[i], isp_compare_2->p_Isp_gamma.p_gamma_attr.r_key[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_gamma.p_gamma_attr.g_key[i] != isp_compare_2->p_Isp_gamma.p_gamma_attr.g_key[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_gamma.p_gamma_attr.g_key[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_gamma.p_gamma_attr.g_key[i], isp_compare_2->p_Isp_gamma.p_gamma_attr.g_key[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_gamma.p_gamma_attr.b_key[i] != isp_compare_2->p_Isp_gamma.p_gamma_attr.b_key[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_gamma.p_gamma_attr.b_key[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_gamma.p_gamma_attr.b_key[i], isp_compare_2->p_Isp_gamma.p_gamma_attr.b_key[i]);
			pFile->WriteString(str);
		}
	}
	
	//pFile->WriteString(_T("\r\n"));
}

void CCompareDlg::Write_y_gamma_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) 
{
	CString str;
	UINT i = 0;

	if (isp_compare_1->p_Isp_y_gamma.param_id != isp_compare_2->p_Isp_y_gamma.param_id)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_y_gamma.param_id = %d, %d\r\n"), isp_compare_1->p_Isp_y_gamma.param_id, isp_compare_2->p_Isp_y_gamma.param_id);
		pFile->WriteString(str);
	}
	
	if (isp_compare_1->p_Isp_y_gamma.length != isp_compare_2->p_Isp_y_gamma.length)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_y_gamma.length = %d, %d\r\n"), isp_compare_1->p_Isp_y_gamma.length, isp_compare_2->p_Isp_y_gamma.length);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_y_gamma.p_gamma_attr.ygamma_uv_adjust_enable != isp_compare_2->p_Isp_y_gamma.p_gamma_attr.ygamma_uv_adjust_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_y_gamma.p_gamma_attr.ygamma_uv_adjust_enable = %d, %d\r\n"), isp_compare_1->p_Isp_y_gamma.p_gamma_attr.ygamma_uv_adjust_enable, isp_compare_2->p_Isp_y_gamma.p_gamma_attr.ygamma_uv_adjust_enable);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_y_gamma.p_gamma_attr.ygamma_uv_adjust_level != isp_compare_2->p_Isp_y_gamma.p_gamma_attr.ygamma_uv_adjust_level)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_y_gamma.p_gamma_attr.ygamma_uv_adjust_level = %d, %d\r\n"), isp_compare_1->p_Isp_y_gamma.p_gamma_attr.ygamma_uv_adjust_level, isp_compare_2->p_Isp_y_gamma.p_gamma_attr.ygamma_uv_adjust_level);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_yth1 != isp_compare_2->p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_yth1)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_yth1 = %d, %d\r\n"), isp_compare_1->p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_yth1, isp_compare_2->p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_yth1);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_yth2 != isp_compare_2->p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_yth2)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_yth2 = %d, %d\r\n"), isp_compare_1->p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_yth2, isp_compare_2->p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_yth2);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_slop != isp_compare_2->p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_slop)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_slop = %d, %d\r\n"), isp_compare_1->p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_slop, isp_compare_2->p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_slop);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_gain != isp_compare_2->p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_gain)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_gain = %d, %d\r\n"), isp_compare_1->p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_gain, isp_compare_2->p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_gain);
		pFile->WriteString(str);
	}

	for (i = 0; i < 129; i++)
	{
		if (isp_compare_1->p_Isp_y_gamma.p_gamma_attr.ygamma[i] != isp_compare_2->p_Isp_y_gamma.p_gamma_attr.ygamma[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_y_gamma.p_gamma_attr.ygamma[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_y_gamma.p_gamma_attr.ygamma[i], isp_compare_2->p_Isp_y_gamma.p_gamma_attr.ygamma[i]);
			pFile->WriteString(str);
		}
	}

	for (i = 0; i < 16; i++)
	{
		if (isp_compare_1->p_Isp_y_gamma.p_gamma_attr.ygamma_key[i] != isp_compare_2->p_Isp_y_gamma.p_gamma_attr.ygamma_key[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_y_gamma.p_gamma_attr.ygamma_key[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_y_gamma.p_gamma_attr.ygamma_key[i], isp_compare_2->p_Isp_y_gamma.p_gamma_attr.ygamma_key[i]);
			pFile->WriteString(str);
		}
	}
	
	//pFile->WriteString(_T("\r\n"));
}


void CCompareDlg::Write_ccm_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) 
{
	CString str;
	UINT i = 0, j = 0, m = 0;

	if (isp_compare_1->p_Isp_ccm.param_id != isp_compare_2->p_Isp_ccm.param_id)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_ccm.param_id = %d, %d\r\n"), isp_compare_1->p_Isp_ccm.param_id, isp_compare_2->p_Isp_ccm.param_id);
		pFile->WriteString(str);
	}
	
	if (isp_compare_1->p_Isp_ccm.length != isp_compare_2->p_Isp_ccm.length)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_ccm.length = %d, %d\r\n"), isp_compare_1->p_Isp_ccm.length, isp_compare_2->p_Isp_ccm.length);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_ccm.p_ccm.cc_mode != isp_compare_2->p_Isp_ccm.p_ccm.cc_mode)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_ccm.p_ccm.cc_mode = %d, %d\r\n"), isp_compare_1->p_Isp_ccm.p_ccm.cc_mode, isp_compare_2->p_Isp_ccm.p_ccm.cc_mode);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_ccm.p_ccm.manual_ccm.cc_enable != isp_compare_2->p_Isp_ccm.p_ccm.manual_ccm.cc_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_ccm.p_ccm.manual_ccm.cc_enable = %d, %d\r\n"), isp_compare_1->p_Isp_ccm.p_ccm.manual_ccm.cc_enable, isp_compare_2->p_Isp_ccm.p_ccm.manual_ccm.cc_enable);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_ccm.p_ccm.manual_ccm.cc_cnoise_yth != isp_compare_2->p_Isp_ccm.p_ccm.manual_ccm.cc_cnoise_yth)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_ccm.p_ccm.manual_ccm.cc_cnoise_yth = %d, %d\r\n"), isp_compare_1->p_Isp_ccm.p_ccm.manual_ccm.cc_cnoise_yth, isp_compare_2->p_Isp_ccm.p_ccm.manual_ccm.cc_cnoise_yth);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_ccm.p_ccm.manual_ccm.cc_cnoise_gain != isp_compare_2->p_Isp_ccm.p_ccm.manual_ccm.cc_cnoise_gain)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_ccm.p_ccm.manual_ccm.cc_cnoise_gain = %d, %d\r\n"), isp_compare_1->p_Isp_ccm.p_ccm.manual_ccm.cc_cnoise_gain, isp_compare_2->p_Isp_ccm.p_ccm.manual_ccm.cc_cnoise_gain);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_ccm.p_ccm.manual_ccm.cc_cnoise_slop != isp_compare_2->p_Isp_ccm.p_ccm.manual_ccm.cc_cnoise_slop)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_ccm.p_ccm.manual_ccm.cc_cnoise_slop = %d, %d\r\n"), isp_compare_1->p_Isp_ccm.p_ccm.manual_ccm.cc_cnoise_slop, isp_compare_2->p_Isp_ccm.p_ccm.manual_ccm.cc_cnoise_slop);
		pFile->WriteString(str);
	}

	for (j = 0; j < 3; j++)
	{
		for (i = 0; i < 3; i++)
		{
			if (isp_compare_1->p_Isp_ccm.p_ccm.manual_ccm.ccm[j][i] != isp_compare_2->p_Isp_ccm.p_ccm.manual_ccm.ccm[j][i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_ccm.p_ccm.manual_ccm.ccm[%d][%d] = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_ccm.p_ccm.manual_ccm.ccm[j][i], isp_compare_2->p_Isp_ccm.p_ccm.manual_ccm.ccm[j][i]);
				pFile->WriteString(str);
			}
		}
	}

	for (m = 0; m < 4; m++)
	{
		if (isp_compare_1->p_Isp_ccm.p_ccm.ccm[m].cc_enable != isp_compare_2->p_Isp_ccm.p_ccm.ccm[m].cc_enable)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_ccm.p_ccm.ccm[%d].cc_enable = %d, %d\r\n"), m, isp_compare_1->p_Isp_ccm.p_ccm.ccm[m].cc_enable, isp_compare_2->p_Isp_ccm.p_ccm.ccm[m].cc_enable);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_ccm.p_ccm.ccm[m].cc_cnoise_yth != isp_compare_2->p_Isp_ccm.p_ccm.ccm[m].cc_cnoise_yth)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_ccm.p_ccm.ccm[%d].cc_cnoise_yth = %d, %d\r\n"), m, isp_compare_1->p_Isp_ccm.p_ccm.ccm[m].cc_cnoise_yth, isp_compare_2->p_Isp_ccm.p_ccm.ccm[m].cc_cnoise_yth);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_ccm.p_ccm.ccm[m].cc_cnoise_gain != isp_compare_2->p_Isp_ccm.p_ccm.ccm[m].cc_cnoise_gain)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_ccm.p_ccm.ccm[%d].cc_cnoise_gain = %d, %d\r\n"), m, isp_compare_1->p_Isp_ccm.p_ccm.ccm[m].cc_cnoise_gain, isp_compare_2->p_Isp_ccm.p_ccm.ccm[m].cc_cnoise_gain);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_ccm.p_ccm.ccm[m].cc_cnoise_slop != isp_compare_2->p_Isp_ccm.p_ccm.ccm[m].cc_cnoise_slop)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_ccm.p_ccm.ccm[%d].cc_cnoise_slop = %d, %d\r\n"), m, isp_compare_1->p_Isp_ccm.p_ccm.ccm[m].cc_cnoise_slop, isp_compare_2->p_Isp_ccm.p_ccm.ccm[m].cc_cnoise_slop);
			pFile->WriteString(str);
		}
		
		for (j = 0; j < 3; j++)
		{
			for (i = 0; i < 3; i++)
			{
				if (isp_compare_1->p_Isp_ccm.p_ccm.ccm[m].ccm[j][i] != isp_compare_2->p_Isp_ccm.p_ccm.ccm[m].ccm[j][i])
				{
					diffrent_num++;
					str.Format(_T("p_Isp_ccm.p_ccm.ccm[%d].ccm[%d][%d] = %d, %d\r\n"), m, j, i, isp_compare_1->p_Isp_ccm.p_ccm.ccm[m].ccm[j][i], isp_compare_2->p_Isp_ccm.p_ccm.ccm[m].ccm[j][i]);
					pFile->WriteString(str);
				}
			}
		}
	}

	
	//pFile->WriteString(_T("\r\n"));
}

void CCompareDlg::Write_fcs_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) 
{
	CString str;
	UINT i = 0;

	if (isp_compare_1->p_Isp_fcs.param_id != isp_compare_2->p_Isp_fcs.param_id)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_fcs.param_id = %d, %d\r\n"), isp_compare_1->p_Isp_fcs.param_id, isp_compare_2->p_Isp_fcs.param_id);
		pFile->WriteString(str);
	}
	
	if (isp_compare_1->p_Isp_fcs.length != isp_compare_2->p_Isp_fcs.length)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_fcs.length = %d, %d\r\n"), isp_compare_1->p_Isp_fcs.length, isp_compare_2->p_Isp_fcs.length);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_fcs.p_fcs.fcs_mode != isp_compare_2->p_Isp_fcs.p_fcs.fcs_mode)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_fcs.p_fcs.fcs_mode = %d, %d\r\n"), isp_compare_1->p_Isp_fcs.p_fcs.fcs_mode, isp_compare_2->p_Isp_fcs.p_fcs.fcs_mode);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_fcs.p_fcs.manual_fcs.fcs_th != isp_compare_2->p_Isp_fcs.p_fcs.manual_fcs.fcs_th)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_fcs.p_fcs.manual_fcs.fcs_th = %d, %d\r\n"), isp_compare_1->p_Isp_fcs.p_fcs.manual_fcs.fcs_th, isp_compare_2->p_Isp_fcs.p_fcs.manual_fcs.fcs_th);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_fcs.p_fcs.manual_fcs.fcs_gain_slop != isp_compare_2->p_Isp_fcs.p_fcs.manual_fcs.fcs_gain_slop)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_fcs.p_fcs.manual_fcs.fcs_gain_slop = %d, %d\r\n"), isp_compare_1->p_Isp_fcs.p_fcs.manual_fcs.fcs_gain_slop, isp_compare_2->p_Isp_fcs.p_fcs.manual_fcs.fcs_gain_slop);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_fcs.p_fcs.manual_fcs.fcs_enable != isp_compare_2->p_Isp_fcs.p_fcs.manual_fcs.fcs_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_fcs.p_fcs.manual_fcs.fcs_enable = %d, %d\r\n"), isp_compare_1->p_Isp_fcs.p_fcs.manual_fcs.fcs_enable, isp_compare_2->p_Isp_fcs.p_fcs.manual_fcs.fcs_enable);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_fcs.p_fcs.manual_fcs.fcs_uv_nr_enable != isp_compare_2->p_Isp_fcs.p_fcs.manual_fcs.fcs_uv_nr_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_fcs.p_fcs.manual_fcs.fcs_uv_nr_enable = %d, %d\r\n"), isp_compare_1->p_Isp_fcs.p_fcs.manual_fcs.fcs_uv_nr_enable, isp_compare_2->p_Isp_fcs.p_fcs.manual_fcs.fcs_uv_nr_enable);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_fcs.p_fcs.manual_fcs.fcs_uv_nr_th != isp_compare_2->p_Isp_fcs.p_fcs.manual_fcs.fcs_uv_nr_th)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_fcs.p_fcs.manual_fcs.fcs_uv_nr_th = %d, %d\r\n"), isp_compare_1->p_Isp_fcs.p_fcs.manual_fcs.fcs_uv_nr_th, isp_compare_2->p_Isp_fcs.p_fcs.manual_fcs.fcs_uv_nr_th);
		pFile->WriteString(str);
	}

	for (i= 0; i < 9; i++)
	{
		if (isp_compare_1->p_Isp_fcs.p_fcs.linkage_fcs[i].fcs_th != isp_compare_2->p_Isp_fcs.p_fcs.linkage_fcs[i].fcs_th)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_fcs.p_fcs.linkage_fcs[%d].fcs_th = %d, %d\r\n"), i, isp_compare_1->p_Isp_fcs.p_fcs.linkage_fcs[i].fcs_th, isp_compare_2->p_Isp_fcs.p_fcs.linkage_fcs[i].fcs_th);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_fcs.p_fcs.linkage_fcs[i].fcs_gain_slop != isp_compare_2->p_Isp_fcs.p_fcs.linkage_fcs[i].fcs_gain_slop)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_fcs.p_fcs.linkage_fcs[%d].fcs_gain_slop = %d, %d\r\n"), i, isp_compare_1->p_Isp_fcs.p_fcs.linkage_fcs[i].fcs_gain_slop, isp_compare_2->p_Isp_fcs.p_fcs.linkage_fcs[i].fcs_gain_slop);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_fcs.p_fcs.linkage_fcs[i].fcs_enable != isp_compare_2->p_Isp_fcs.p_fcs.linkage_fcs[i].fcs_enable)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_fcs.p_fcs.linkage_fcs[%d].fcs_enable = %d, %d\r\n"), i, isp_compare_1->p_Isp_fcs.p_fcs.linkage_fcs[i].fcs_enable, isp_compare_2->p_Isp_fcs.p_fcs.linkage_fcs[i].fcs_enable);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_fcs.p_fcs.linkage_fcs[i].fcs_uv_nr_enable != isp_compare_2->p_Isp_fcs.p_fcs.linkage_fcs[i].fcs_uv_nr_enable)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_fcs.p_fcs.linkage_fcs[%d].fcs_uv_nr_enable = %d, %d\r\n"), i, isp_compare_1->p_Isp_fcs.p_fcs.linkage_fcs[i].fcs_uv_nr_enable, isp_compare_2->p_Isp_fcs.p_fcs.linkage_fcs[i].fcs_uv_nr_enable);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_fcs.p_fcs.linkage_fcs[i].fcs_uv_nr_th != isp_compare_2->p_Isp_fcs.p_fcs.linkage_fcs[i].fcs_uv_nr_th)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_fcs.p_fcs.linkage_fcs[%d].fcs_uv_nr_th = %d, %d\r\n"), i, isp_compare_1->p_Isp_fcs.p_fcs.linkage_fcs[i].fcs_uv_nr_th, isp_compare_2->p_Isp_fcs.p_fcs.linkage_fcs[i].fcs_uv_nr_th);
			pFile->WriteString(str);
		}
	}

	//pFile->WriteString(_T("\r\n"));
}

void CCompareDlg::Write_wdr_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) 
{
	CString str;
	UINT i = 0, j = 0;

	if (isp_compare_1->p_Isp_wdr.param_id != isp_compare_2->p_Isp_wdr.param_id)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wdr.param_id = %d, %d\r\n"), isp_compare_1->p_Isp_wdr.param_id, isp_compare_2->p_Isp_wdr.param_id);
		pFile->WriteString(str);
	}
	
	if (isp_compare_1->p_Isp_wdr.length != isp_compare_2->p_Isp_wdr.length)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wdr.length = %d, %d\r\n"), isp_compare_1->p_Isp_wdr.length, isp_compare_2->p_Isp_wdr.length);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_wdr.p_wdr_attr.wdr_mode != isp_compare_2->p_Isp_wdr.p_wdr_attr.wdr_mode)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wdr.p_wdr_attr.wdr_mode = %d, %d\r\n"), isp_compare_1->p_Isp_wdr.p_wdr_attr.wdr_mode, isp_compare_2->p_Isp_wdr.p_wdr_attr.wdr_mode);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_uv_adjust_level != isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_uv_adjust_level)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_uv_adjust_level = %d, %d\r\n"), isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_uv_adjust_level, isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_uv_adjust_level);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_slop != isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_slop)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_slop = %d, %d\r\n"), isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_slop, isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_slop);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_enable != isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_enable = %d, %d\r\n"), isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_enable, isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_enable);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th1 != isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th1)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th1 = %d, %d\r\n"), isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th1, isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th1);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th2 != isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th2)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th2 = %d, %d\r\n"), isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th2, isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th2);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th3 != isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th3)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th3 = %d, %d\r\n"), isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th3, isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th3);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th4 != isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th4)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th4 = %d, %d\r\n"), isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th4, isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th4);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th5 != isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th5)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th5 = %d, %d\r\n"), isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th5, isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th5);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_uv_adjust_enable != isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_uv_adjust_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_uv_adjust_enable = %d, %d\r\n"), isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_uv_adjust_enable, isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_uv_adjust_enable);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth1 != isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth1)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth1 = %d, %d\r\n"), isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth1, isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth1);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth2 != isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth2)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth2 = %d, %d\r\n"), isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth2, isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth2);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_gain != isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_gain)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_gain = %d, %d\r\n"), isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_gain, isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_gain);
		pFile->WriteString(str);
	}


	for (i = 0; i < 65; i++)
	{
		if (isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb1[i] != isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb1[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb1[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb1[i], isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb1[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb2[i] != isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb2[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb2[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb2[i], isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb2[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb3[i] != isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb3[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb3[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb3[i], isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb3[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb4[i] != isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb4[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb4[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb4[i], isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb4[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb5[i] != isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb5[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb5[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb5[i], isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb5[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb6[i] != isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb6[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb6[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb6[i], isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb6[i]);
			pFile->WriteString(str);
		}
	}

	for (i = 0; i < 16; i++)
	{
		if (isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.area1_key[i] != isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.area1_key[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wdr.p_wdr_attr.manual_wdr.area1_key[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.area1_key[i], isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.area1_key[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.area2_key[i] != isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.area2_key[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wdr.p_wdr_attr.manual_wdr.area2_key[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.area2_key[i], isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.area2_key[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.area3_key[i] != isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.area3_key[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wdr.p_wdr_attr.manual_wdr.area3_key[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.area3_key[i], isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.area3_key[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.area4_key[i] != isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.area4_key[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wdr.p_wdr_attr.manual_wdr.area4_key[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.area4_key[i], isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.area4_key[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.area5_key[i] != isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.area5_key[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wdr.p_wdr_attr.manual_wdr.area5_key[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.area5_key[i], isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.area5_key[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.area6_key[i] != isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.area6_key[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wdr.p_wdr_attr.manual_wdr.area6_key[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_wdr.p_wdr_attr.manual_wdr.area6_key[i], isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.area6_key[i]);
			pFile->WriteString(str);
		}

	}

	for (j = 0; j < 9; j++)
	{
		if (isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].hdr_uv_adjust_level != isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].hdr_uv_adjust_level)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wdr.p_wdr_attr.linkage_wdr[%d].hdr_uv_adjust_level = %d, %d\r\n"), j, isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].hdr_uv_adjust_level, isp_compare_2->p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_uv_adjust_level);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].hdr_cnoise_suppress_slop != isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].hdr_cnoise_suppress_slop)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wdr.p_wdr_attr.linkage_wdr[%d].hdr_cnoise_suppress_slop = %d, %d\r\n"),j,  isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].hdr_cnoise_suppress_slop, isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].hdr_cnoise_suppress_slop);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].wdr_enable != isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].wdr_enable)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wdr.p_wdr_attr.linkage_wdr[%d].wdr_enable = %d, %d\r\n"), j, isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].wdr_enable, isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].wdr_enable);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].wdr_th1 != isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].wdr_th1)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wdr.p_wdr_attr.linkage_wdr[%d].wdr_th1 = %d, %d\r\n"), j, isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].wdr_th1, isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].wdr_th1);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].wdr_th2 != isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].wdr_th2)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wdr.p_wdr_attr.linkage_wdr[%d].wdr_th2 = %d, %d\r\n"), j, isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].wdr_th2, isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].wdr_th2);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].wdr_th3 != isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].wdr_th3)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wdr.p_wdr_attr.linkage_wdr[%d].wdr_th3 = %d, %d\r\n"), j, isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].wdr_th3, isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].wdr_th3);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].wdr_th4 != isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].wdr_th4)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wdr.p_wdr_attr.linkage_wdr[%d].wdr_th4 = %d, %d\r\n"), j, isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].wdr_th4, isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].wdr_th4);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].wdr_th5 != isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].wdr_th5)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wdr.p_wdr_attr.linkage_wdr[%d].wdr_th5 = %d, %d\r\n"), j, isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].wdr_th5, isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].wdr_th5);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].hdr_uv_adjust_enable != isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].hdr_uv_adjust_enable)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wdr.p_wdr_attr.linkage_wdr[%d].hdr_uv_adjust_enable = %d, %d\r\n"), j, isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].hdr_uv_adjust_enable, isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].hdr_uv_adjust_enable);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].hdr_cnoise_suppress_yth1 != isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].hdr_cnoise_suppress_yth1)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wdr.p_wdr_attr.linkage_wdr[%d].hdr_cnoise_suppress_yth1 = %d, %d\r\n"),j,  isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].hdr_cnoise_suppress_yth1, isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].hdr_cnoise_suppress_yth1);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].hdr_cnoise_suppress_yth2 != isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].hdr_cnoise_suppress_yth2)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wdr.p_wdr_attr.linkage_wdr[%d].hdr_cnoise_suppress_yth2 = %d, %d\r\n"),j,  isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].hdr_cnoise_suppress_yth2, isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].hdr_cnoise_suppress_yth2);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].hdr_cnoise_suppress_gain != isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].hdr_cnoise_suppress_gain)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wdr.p_wdr_attr.linkage_wdr[%d].hdr_cnoise_suppress_gain = %d, %d\r\n"), j, isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].hdr_cnoise_suppress_gain, isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].hdr_cnoise_suppress_gain);
			pFile->WriteString(str);
		}
		
		
		for (i = 0; i < 65; i++)
		{
			if (isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area_tb1[i] != isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area_tb1[i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_wdr.p_wdr_attr.linkage_wdr[%d].area_tb1[%d] = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area_tb1[i], isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area_tb1[i]);
				pFile->WriteString(str);
			}
			
			if (isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area_tb2[i] != isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area_tb2[i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_wdr.p_wdr_attr.linkage_wdr[%d].area_tb2[%d] = %d, %d\r\n"), j , i, isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area_tb2[i], isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area_tb2[i]);
				pFile->WriteString(str);
			}
			
			if (isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area_tb3[i] != isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area_tb3[i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_wdr.p_wdr_attr.linkage_wdr[%d].area_tb3[%d] = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area_tb3[i], isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area_tb3[i]);
				pFile->WriteString(str);
			}

			if (isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area_tb4[i] != isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area_tb4[i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_wdr.p_wdr_attr.linkage_wdr[%d].area_tb4[%d] = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area_tb4[i], isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area_tb4[i]);
				pFile->WriteString(str);
			}

			if (isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area_tb5[i] != isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area_tb5[i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_wdr.p_wdr_attr.linkage_wdr[%d].area_tb5[%d] = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area_tb5[i], isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area_tb5[i]);
				pFile->WriteString(str);
			}

			if (isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area_tb6[i] != isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area_tb6[i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_wdr.p_wdr_attr.linkage_wdr[%d].area_tb6[%d] = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area_tb6[i], isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area_tb6[i]);
				pFile->WriteString(str);
			}
		}
		
		for (i = 0; i < 16; i++)
		{
			if (isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area1_key[i] != isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area1_key[i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_wdr.p_wdr_attr.linkage_wdr[%d].area1_key[%d] = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area1_key[i], isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area1_key[i]);
				pFile->WriteString(str);
			}
			
			if (isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area2_key[i] != isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area2_key[i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_wdr.p_wdr_attr.linkage_wdr[%d].area2_key[%d] = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area2_key[i], isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area2_key[i]);
				pFile->WriteString(str);
			}
			
			if (isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area3_key[i] != isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area3_key[i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_wdr.p_wdr_attr.linkage_wdr[%d].area3_key[%d] = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area3_key[i], isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area3_key[i]);
				pFile->WriteString(str);
			}

			if (isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area4_key[i] != isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area4_key[i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_wdr.p_wdr_attr.linkage_wdr[%d].area4_key[%d] = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area4_key[i], isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area4_key[i]);
				pFile->WriteString(str);
			}

			if (isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area5_key[i] != isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area5_key[i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_wdr.p_wdr_attr.linkage_wdr[%d].area5_key[%d] = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area5_key[i], isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area5_key[i]);
				pFile->WriteString(str);
			}

			if (isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area6_key[i] != isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area6_key[i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_wdr.p_wdr_attr.linkage_wdr[%d].area6_key[%d] = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area6_key[i], isp_compare_2->p_Isp_wdr.p_wdr_attr.linkage_wdr[j].area6_key[i]);
				pFile->WriteString(str);
			}
			
		}
	}
	
	//pFile->WriteString(_T("\r\n"));
}

/*
void CCompareDlg::Write_edge_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) 
{
	CString str;
	UINT i = 0;

	if (isp_compare_1->p_Isp_edge.param_id != isp_compare_2->p_Isp_edge.param_id)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_edge.param_id = %d, %d\r\n"), isp_compare_1->p_Isp_edge.param_id, isp_compare_2->p_Isp_edge.param_id);
		pFile->WriteString(str);
	}
	
	if (isp_compare_1->p_Isp_edge.length != isp_compare_2->p_Isp_edge.length)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_edge.length = %d, %d\r\n"), isp_compare_1->p_Isp_edge.length, isp_compare_2->p_Isp_edge.length);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_edge.p_edge_attr.edge_mode != isp_compare_2->p_Isp_edge.p_edge_attr.edge_mode)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_edge.p_edge_attr.edge_mode = %d, %d\r\n"), isp_compare_1->p_Isp_edge.p_edge_attr.edge_mode, isp_compare_2->p_Isp_edge.p_edge_attr.edge_mode);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_edge.p_edge_attr.manual_edge.enable != isp_compare_2->p_Isp_edge.p_edge_attr.manual_edge.enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_edge.p_edge_attr.manual_edge.enable = %d, %d\r\n"), isp_compare_1->p_Isp_edge.p_edge_attr.manual_edge.enable, isp_compare_2->p_Isp_edge.p_edge_attr.manual_edge.enable);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_edge.p_edge_attr.manual_edge.edge_th != isp_compare_2->p_Isp_edge.p_edge_attr.manual_edge.edge_th)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_edge.p_edge_attr.manual_edge.edge_th = %d, %d\r\n"), isp_compare_1->p_Isp_edge.p_edge_attr.manual_edge.edge_th, isp_compare_2->p_Isp_edge.p_edge_attr.manual_edge.edge_th);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_edge.p_edge_attr.manual_edge.edge_max_len != isp_compare_2->p_Isp_edge.p_edge_attr.manual_edge.edge_max_len)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_edge.p_edge_attr.manual_edge.edge_max_len = %d, %d\r\n"), isp_compare_1->p_Isp_edge.p_edge_attr.manual_edge.edge_max_len, isp_compare_2->p_Isp_edge.p_edge_attr.manual_edge.edge_max_len);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_edge.p_edge_attr.manual_edge.edge_gain_th != isp_compare_2->p_Isp_edge.p_edge_attr.manual_edge.edge_gain_th)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_edge.p_edge_attr.manual_edge.edge_gain_th = %d, %d\r\n"), isp_compare_1->p_Isp_edge.p_edge_attr.manual_edge.edge_gain_th, isp_compare_2->p_Isp_edge.p_edge_attr.manual_edge.edge_gain_th);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_edge.p_edge_attr.manual_edge.edge_gain_slop != isp_compare_2->p_Isp_edge.p_edge_attr.manual_edge.edge_gain_slop)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_edge.p_edge_attr.manual_edge.edge_gain_slop = %d, %d\r\n"), isp_compare_1->p_Isp_edge.p_edge_attr.manual_edge.edge_gain_slop, isp_compare_2->p_Isp_edge.p_edge_attr.manual_edge.edge_gain_slop);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_edge.p_edge_attr.manual_edge.edge_y_th != isp_compare_2->p_Isp_edge.p_edge_attr.manual_edge.edge_y_th)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_edge.p_edge_attr.manual_edge.edge_y_th = %d, %d\r\n"), isp_compare_1->p_Isp_edge.p_edge_attr.manual_edge.edge_y_th, isp_compare_2->p_Isp_edge.p_edge_attr.manual_edge.edge_y_th);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_edge.p_edge_attr.manual_edge.edge_gain != isp_compare_2->p_Isp_edge.p_edge_attr.manual_edge.edge_gain)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_edge.p_edge_attr.manual_edge.edge_gain = %d, %d\r\n"), isp_compare_1->p_Isp_edge.p_edge_attr.manual_edge.edge_gain, isp_compare_2->p_Isp_edge.p_edge_attr.manual_edge.edge_gain);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_edge.p_edge_attr.manual_edge.c_edge_enable != isp_compare_2->p_Isp_edge.p_edge_attr.manual_edge.c_edge_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_edge.p_edge_attr.manual_edge.c_edge_enable = %d, %d\r\n"), isp_compare_1->p_Isp_edge.p_edge_attr.manual_edge.c_edge_enable, isp_compare_2->p_Isp_edge.p_edge_attr.manual_edge.c_edge_enable);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_edge.p_edge_attr.manual_edge.edge_skin_detect != isp_compare_2->p_Isp_edge.p_edge_attr.manual_edge.edge_skin_detect)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_edge.p_edge_attr.manual_edge.edge_skin_detect = %d, %d\r\n"), isp_compare_1->p_Isp_edge.p_edge_attr.manual_edge.edge_skin_detect, isp_compare_2->p_Isp_edge.p_edge_attr.manual_edge.edge_skin_detect);
		pFile->WriteString(str);
	}

	for (i = 0; i < 9; i++)
	{
		if (isp_compare_1->p_Isp_edge.p_edge_attr.linkage_edge[i].enable != isp_compare_2->p_Isp_edge.p_edge_attr.linkage_edge[i].enable)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_edge.p_edge_attr.linkage_edge[%d].enable = %d, %d\r\n"), i, isp_compare_1->p_Isp_edge.p_edge_attr.linkage_edge[i].enable, isp_compare_2->p_Isp_edge.p_edge_attr.linkage_edge[i].enable);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_th != isp_compare_2->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_th)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_edge.p_edge_attr.linkage_edge[%d].edge_th = %d, %d\r\n"), i, isp_compare_1->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_th, isp_compare_2->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_th);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_max_len != isp_compare_2->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_max_len)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_edge.p_edge_attr.linkage_edge[%d].edge_max_len = %d, %d\r\n"), i, isp_compare_1->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_max_len, isp_compare_2->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_max_len);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_gain_th != isp_compare_2->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_gain_th)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_edge.p_edge_attr.linkage_edge[%d].edge_gain_th = %d, %d\r\n"), i, isp_compare_1->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_gain_th, isp_compare_2->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_gain_th);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_gain_slop != isp_compare_2->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_gain_slop)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_edge.p_edge_attr.linkage_edge[%d].edge_gain_slop = %d, %d\r\n"), i, isp_compare_1->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_gain_slop, isp_compare_2->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_gain_slop);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_y_th != isp_compare_2->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_y_th)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_edge.p_edge_attr.linkage_edge[%d].edge_y_th = %d, %d\r\n"), i, isp_compare_1->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_y_th, isp_compare_2->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_y_th);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_gain != isp_compare_2->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_gain)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_edge.p_edge_attr.linkage_edge[%d].edge_gain = %d, %d\r\n"), i, isp_compare_1->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_gain, isp_compare_2->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_gain);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_edge.p_edge_attr.linkage_edge[i].c_edge_enable != isp_compare_2->p_Isp_edge.p_edge_attr.linkage_edge[i].c_edge_enable)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_edge.p_edge_attr.linkage_edge[%d].c_edge_enable = %d, %d\r\n"), i, isp_compare_1->p_Isp_edge.p_edge_attr.linkage_edge[i].c_edge_enable, isp_compare_2->p_Isp_edge.p_edge_attr.linkage_edge[i].c_edge_enable);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_skin_detect != isp_compare_2->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_skin_detect)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_edge.p_edge_attr.linkage_edge[%d].edge_skin_detect = %d, %d\r\n"), i, isp_compare_1->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_skin_detect, isp_compare_2->p_Isp_edge.p_edge_attr.linkage_edge[i].edge_skin_detect);
			pFile->WriteString(str);
		}
	}


	if (isp_compare_1->p_Isp_edge.p_edge_ex_attr.edge_skin_max_th != isp_compare_2->p_Isp_edge.p_edge_ex_attr.edge_skin_max_th)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_edge.p_edge_ex_attr.edge_skin_max_th = %d, %d\r\n"), isp_compare_1->p_Isp_edge.p_edge_ex_attr.edge_skin_max_th, isp_compare_2->p_Isp_edge.p_edge_ex_attr.edge_skin_max_th);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_edge.p_edge_ex_attr.edge_skin_min_th != isp_compare_2->p_Isp_edge.p_edge_ex_attr.edge_skin_min_th)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_edge.p_edge_ex_attr.edge_skin_min_th = %d, %d\r\n"),  isp_compare_1->p_Isp_edge.p_edge_ex_attr.edge_skin_min_th, isp_compare_2->p_Isp_edge.p_edge_ex_attr.edge_skin_min_th);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_edge.p_edge_ex_attr.edge_skin_uv_max_th != isp_compare_2->p_Isp_edge.p_edge_ex_attr.edge_skin_uv_max_th)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_edge.p_edge_ex_attr.edge_skin_uv_max_th = %d, %d\r\n"),  isp_compare_1->p_Isp_edge.p_edge_ex_attr.edge_skin_uv_max_th, isp_compare_2->p_Isp_edge.p_edge_ex_attr.edge_skin_uv_max_th);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_edge.p_edge_ex_attr.edge_skin_uv_min_th != isp_compare_2->p_Isp_edge.p_edge_ex_attr.edge_skin_uv_min_th)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_edge.p_edge_ex_attr.edge_skin_uv_min_th = %d, %d\r\n"),  isp_compare_1->p_Isp_edge.p_edge_ex_attr.edge_skin_uv_min_th, isp_compare_2->p_Isp_edge.p_edge_ex_attr.edge_skin_uv_min_th);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_edge.p_edge_ex_attr.edge_skin_y_max_th != isp_compare_2->p_Isp_edge.p_edge_ex_attr.edge_skin_y_max_th)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_edge.p_edge_ex_attr.edge_skin_y_max_th = %d, %d\r\n"),  isp_compare_1->p_Isp_edge.p_edge_ex_attr.edge_skin_y_max_th, isp_compare_2->p_Isp_edge.p_edge_ex_attr.edge_skin_y_max_th);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_edge.p_edge_ex_attr.edge_skin_y_min_th != isp_compare_2->p_Isp_edge.p_edge_ex_attr.edge_skin_y_min_th)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_edge.p_edge_ex_attr.edge_skin_y_min_th = %d, %d\r\n"),  isp_compare_1->p_Isp_edge.p_edge_ex_attr.edge_skin_y_min_th, isp_compare_2->p_Isp_edge.p_edge_ex_attr.edge_skin_y_min_th);
		pFile->WriteString(str);
	}

	//pFile->WriteString(_T("\r\n"));
}
*/
void CCompareDlg::Write_sharp_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) 
{
	CString str;
	UINT i = 0, j = 0;

	if (isp_compare_1->p_Isp_sharp.param_id != isp_compare_2->p_Isp_sharp.param_id)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_sharp.param_id = %d, %d\r\n"), isp_compare_1->p_Isp_sharp.param_id, isp_compare_2->p_Isp_sharp.param_id);
		pFile->WriteString(str);
	}
	
	if (isp_compare_1->p_Isp_sharp.length != isp_compare_2->p_Isp_sharp.length)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_sharp.length = %d, %d\r\n"), isp_compare_1->p_Isp_sharp.length, isp_compare_2->p_Isp_sharp.length);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_sharp.p_sharp_attr.ysharp_mode != isp_compare_2->p_Isp_sharp.p_sharp_attr.ysharp_mode)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_sharp.p_sharp_attr.ysharp_mode = %d, %d\r\n"), isp_compare_1->p_Isp_sharp.p_sharp_attr.ysharp_mode, isp_compare_2->p_Isp_sharp.p_sharp_attr.ysharp_mode);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.mf_hpf_k != isp_compare_2->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.mf_hpf_k)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_sharp.p_sharp_attr.manual_sharp_attr.mf_hpf_k = %d, %d\r\n"), isp_compare_1->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.mf_hpf_k, isp_compare_2->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.mf_hpf_k);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.mf_hpf_shift != isp_compare_2->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.mf_hpf_shift)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_sharp.p_sharp_attr.manual_sharp_attr.mf_hpf_shift = %d, %d\r\n"), isp_compare_1->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.mf_hpf_shift, isp_compare_2->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.mf_hpf_shift);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.hf_hpf_k != isp_compare_2->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.hf_hpf_k)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_sharp.p_sharp_attr.manual_sharp_attr.hf_hpf_k = %d, %d\r\n"), isp_compare_1->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.hf_hpf_k, isp_compare_2->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.hf_hpf_k);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.hf_hpf_shift != isp_compare_2->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.hf_hpf_shift)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_sharp.p_sharp_attr.manual_sharp_attr.hf_hpf_shift = %d, %d\r\n"), isp_compare_1->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.hf_hpf_shift, isp_compare_2->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.hf_hpf_shift);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_method != isp_compare_2->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_method)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_method = %d, %d\r\n"), isp_compare_1->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_method, isp_compare_2->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_method);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_skin_gain_weaken != isp_compare_2->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_skin_gain_weaken)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_skin_gain_weaken = %d, %d\r\n"), isp_compare_1->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_skin_gain_weaken, isp_compare_2->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_skin_gain_weaken);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_skin_gain_th != isp_compare_2->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_skin_gain_th)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_skin_gain_th = %d, %d\r\n"), isp_compare_1->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_skin_gain_th, isp_compare_2->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_skin_gain_th);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_skin_detect_enable != isp_compare_2->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_skin_detect_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_skin_detect_enable = %d, %d\r\n"), isp_compare_1->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_skin_detect_enable, isp_compare_2->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_skin_detect_enable);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.ysharp_enable != isp_compare_2->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.ysharp_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_sharp.p_sharp_attr.manual_sharp_attr.ysharp_enable = %d, %d\r\n"), isp_compare_1->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.ysharp_enable, isp_compare_2->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.ysharp_enable);
		pFile->WriteString(str);
	}

	for (i = 0; i < 256; i++)
	{
		if (isp_compare_1->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.MF_HPF_LUT[i] != isp_compare_2->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.MF_HPF_LUT[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_sharp.p_sharp_attr.manual_sharp_attr.MF_HPF_LUT[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.MF_HPF_LUT[i], isp_compare_2->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.MF_HPF_LUT[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.HF_HPF_LUT[i] != isp_compare_2->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.HF_HPF_LUT[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_sharp.p_sharp_attr.manual_sharp_attr.HF_HPF_LUT[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.HF_HPF_LUT[i], isp_compare_2->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.HF_HPF_LUT[i]);
			pFile->WriteString(str);
		}
	}

	for (i = 0; i < 16; i++)
	{
		if (isp_compare_1->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.MF_LUT_KEY[i] != isp_compare_2->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.MF_LUT_KEY[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_sharp.p_sharp_attr.manual_sharp_attr.MF_LUT_KEY[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.MF_LUT_KEY[i], isp_compare_2->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.MF_LUT_KEY[i]);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.HF_LUT_KEY[i] != isp_compare_2->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.HF_LUT_KEY[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_sharp.p_sharp_attr.manual_sharp_attr.HF_LUT_KEY[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.HF_LUT_KEY[i], isp_compare_2->p_Isp_sharp.p_sharp_attr.manual_sharp_attr.HF_LUT_KEY[i]);
			pFile->WriteString(str);
		}
	}


	for (j = 0; j < 9; j++)
	{
		if (isp_compare_1->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].mf_hpf_k != isp_compare_2->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].mf_hpf_k)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[%d].mf_hpf_k = %d, %d\r\n"), j, isp_compare_1->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].mf_hpf_k, isp_compare_2->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].mf_hpf_k);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].mf_hpf_shift != isp_compare_2->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].mf_hpf_shift)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[%d].mf_hpf_shift = %d, %d\r\n"), j, isp_compare_1->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].mf_hpf_shift, isp_compare_2->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].mf_hpf_shift);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].hf_hpf_k != isp_compare_2->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].hf_hpf_k)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[%d].hf_hpf_k = %d, %d\r\n"), j, isp_compare_1->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].hf_hpf_k, isp_compare_2->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].hf_hpf_k);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].hf_hpf_shift != isp_compare_2->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].hf_hpf_shift)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[%d].hf_hpf_shift = %d, %d\r\n"), j, isp_compare_1->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].hf_hpf_shift, isp_compare_2->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].hf_hpf_shift);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].sharp_method != isp_compare_2->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].sharp_method)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[%d].sharp_method = %d, %d\r\n"), j, isp_compare_1->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].sharp_method, isp_compare_2->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].sharp_method);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].sharp_skin_gain_weaken != isp_compare_2->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].sharp_skin_gain_weaken)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[%d].sharp_skin_gain_weaken = %d, %d\r\n"), j, isp_compare_1->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].sharp_skin_gain_weaken, isp_compare_2->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].sharp_skin_gain_weaken);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].sharp_skin_gain_th != isp_compare_2->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].sharp_skin_gain_th)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[%d].sharp_skin_gain_th = %d, %d\r\n"), j, isp_compare_1->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].sharp_skin_gain_th, isp_compare_2->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].sharp_skin_gain_th);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].sharp_skin_detect_enable != isp_compare_2->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].sharp_skin_detect_enable)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[%d].sharp_skin_detect_enable = %d, %d\r\n"), j, isp_compare_1->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].sharp_skin_detect_enable, isp_compare_2->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].sharp_skin_detect_enable);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].ysharp_enable != isp_compare_2->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].ysharp_enable)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[%d].ysharp_enable = %d, %d\r\n"), j, isp_compare_1->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].ysharp_enable, isp_compare_2->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].ysharp_enable);
			pFile->WriteString(str);
		}
		
		for (i = 0; i < 256; i++)
		{
			if (isp_compare_1->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].MF_HPF_LUT[i] != isp_compare_2->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].MF_HPF_LUT[i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[%d].MF_HPF_LUT[%d] = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].MF_HPF_LUT[i], isp_compare_2->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].MF_HPF_LUT[i]);
				pFile->WriteString(str);
			}
			
			if (isp_compare_1->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].HF_HPF_LUT[i] != isp_compare_2->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].HF_HPF_LUT[i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[%d].HF_HPF_LUT[%d] = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].HF_HPF_LUT[i], isp_compare_2->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].HF_HPF_LUT[i]);
				pFile->WriteString(str);
			}
		}
		
		for (i = 0; i < 16; i++)
		{
			
			if (isp_compare_1->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].MF_LUT_KEY[i] != isp_compare_2->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].MF_LUT_KEY[i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[%d].MF_LUT_KEY[%d] = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].MF_LUT_KEY[i], isp_compare_2->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].MF_LUT_KEY[i]);
				pFile->WriteString(str);
			}
			
			if (isp_compare_1->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].HF_LUT_KEY[i] != isp_compare_2->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].HF_LUT_KEY[i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[%d].HF_LUT_KEY[%d] = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].HF_LUT_KEY[i], isp_compare_2->p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[j].HF_LUT_KEY[i]);
				pFile->WriteString(str);
			}
		}
	}

	for (i = 0; i < 6; i++)
	{
		if (isp_compare_1->p_Isp_sharp.p_sharp_ex_attr.mf_HPF[i] != isp_compare_2->p_Isp_sharp.p_sharp_ex_attr.mf_HPF[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_sharp.p_sharp_ex_attr.mf_HPF[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_sharp.p_sharp_ex_attr.mf_HPF[i], isp_compare_2->p_Isp_sharp.p_sharp_ex_attr.mf_HPF[i]);
			pFile->WriteString(str);
		}
	}

	for (i = 0; i < 3; i++)
	{
		if (isp_compare_1->p_Isp_sharp.p_sharp_ex_attr.hf_HPF[i] != isp_compare_2->p_Isp_sharp.p_sharp_ex_attr.hf_HPF[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_sharp.p_sharp_ex_attr.hf_HPF[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_sharp.p_sharp_ex_attr.hf_HPF[i], isp_compare_2->p_Isp_sharp.p_sharp_ex_attr.hf_HPF[i]);
			pFile->WriteString(str);
		}
	}

	if (isp_compare_1->p_Isp_sharp.p_sharp_ex_attr.sharp_skin_max_th != isp_compare_2->p_Isp_sharp.p_sharp_ex_attr.sharp_skin_max_th)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_sharp.p_sharp_ex_attr.sharp_skin_max_th = %d, %d\r\n"), isp_compare_1->p_Isp_sharp.p_sharp_ex_attr.sharp_skin_max_th, isp_compare_2->p_Isp_sharp.p_sharp_ex_attr.sharp_skin_max_th);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_sharp.p_sharp_ex_attr.sharp_skin_min_th != isp_compare_2->p_Isp_sharp.p_sharp_ex_attr.sharp_skin_min_th)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_sharp.p_sharp_ex_attr.sharp_skin_min_th = %d, %d\r\n"), isp_compare_1->p_Isp_sharp.p_sharp_ex_attr.sharp_skin_min_th, isp_compare_2->p_Isp_sharp.p_sharp_ex_attr.sharp_skin_min_th);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_sharp.p_sharp_ex_attr.sharp_skin_v_max_th != isp_compare_2->p_Isp_sharp.p_sharp_ex_attr.sharp_skin_v_max_th)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_sharp.p_sharp_ex_attr.sharp_skin_v_max_th = %d, %d\r\n"), isp_compare_1->p_Isp_sharp.p_sharp_ex_attr.sharp_skin_v_max_th, isp_compare_2->p_Isp_sharp.p_sharp_ex_attr.sharp_skin_v_max_th);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_sharp.p_sharp_ex_attr.sharp_skin_v_min_th != isp_compare_2->p_Isp_sharp.p_sharp_ex_attr.sharp_skin_v_min_th)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_sharp.p_sharp_ex_attr.sharp_skin_v_min_th = %d, %d\r\n"), isp_compare_1->p_Isp_sharp.p_sharp_ex_attr.sharp_skin_v_min_th, isp_compare_2->p_Isp_sharp.p_sharp_ex_attr.sharp_skin_v_min_th);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_sharp.p_sharp_ex_attr.sharp_skin_y_max_th != isp_compare_2->p_Isp_sharp.p_sharp_ex_attr.sharp_skin_y_max_th)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_sharp.p_sharp_ex_attr.sharp_skin_y_max_th = %d, %d\r\n"), isp_compare_1->p_Isp_sharp.p_sharp_ex_attr.sharp_skin_y_max_th, isp_compare_2->p_Isp_sharp.p_sharp_ex_attr.sharp_skin_y_max_th);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_sharp.p_sharp_ex_attr.sharp_skin_y_min_th != isp_compare_2->p_Isp_sharp.p_sharp_ex_attr.sharp_skin_y_min_th)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_sharp.p_sharp_ex_attr.sharp_skin_y_min_th = %d, %d\r\n"), isp_compare_1->p_Isp_sharp.p_sharp_ex_attr.sharp_skin_y_min_th, isp_compare_2->p_Isp_sharp.p_sharp_ex_attr.sharp_skin_y_min_th);
		pFile->WriteString(str);
	}
	

	//pFile->WriteString(_T("\r\n"));
}

void CCompareDlg::Write_saturation_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) 
{
	CString str;
	UINT i = 0;

	if (isp_compare_1->p_Isp_saturation.param_id != isp_compare_2->p_Isp_saturation.param_id)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_saturation.param_id = %d, %d\r\n"), isp_compare_1->p_Isp_saturation.param_id, isp_compare_2->p_Isp_saturation.param_id);
		pFile->WriteString(str);
	}
	
	if (isp_compare_1->p_Isp_saturation.length != isp_compare_2->p_Isp_saturation.length)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_saturation.length = %d, %d\r\n"), isp_compare_1->p_Isp_saturation.length, isp_compare_2->p_Isp_saturation.length);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_saturation.p_se_attr.SE_mode != isp_compare_2->p_Isp_saturation.p_se_attr.SE_mode)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_saturation.p_se_attr.SE_mode = %d, %d\r\n"), isp_compare_1->p_Isp_saturation.p_se_attr.SE_mode, isp_compare_2->p_Isp_saturation.p_se_attr.SE_mode);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_saturation.p_se_attr.manual_sat.SE_enable != isp_compare_2->p_Isp_saturation.p_se_attr.manual_sat.SE_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_saturation.p_se_attr.manual_sat.SE_enable = %d, %d\r\n"), isp_compare_1->p_Isp_saturation.p_se_attr.manual_sat.SE_enable, isp_compare_2->p_Isp_saturation.p_se_attr.manual_sat.SE_enable);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_saturation.p_se_attr.manual_sat.SE_th1 != isp_compare_2->p_Isp_saturation.p_se_attr.manual_sat.SE_th1)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_saturation.p_se_attr.manual_sat.SE_th1 = %d, %d\r\n"), isp_compare_1->p_Isp_saturation.p_se_attr.manual_sat.SE_th1, isp_compare_2->p_Isp_saturation.p_se_attr.manual_sat.SE_th1);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_saturation.p_se_attr.manual_sat.SE_th2 != isp_compare_2->p_Isp_saturation.p_se_attr.manual_sat.SE_th2)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_saturation.p_se_attr.manual_sat.SE_th2 = %d, %d\r\n"), isp_compare_1->p_Isp_saturation.p_se_attr.manual_sat.SE_th2, isp_compare_2->p_Isp_saturation.p_se_attr.manual_sat.SE_th2);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_saturation.p_se_attr.manual_sat.SE_th3 != isp_compare_2->p_Isp_saturation.p_se_attr.manual_sat.SE_th3)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_saturation.p_se_attr.manual_sat.SE_th3 = %d, %d\r\n"), isp_compare_1->p_Isp_saturation.p_se_attr.manual_sat.SE_th3, isp_compare_2->p_Isp_saturation.p_se_attr.manual_sat.SE_th3);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_saturation.p_se_attr.manual_sat.SE_th4 != isp_compare_2->p_Isp_saturation.p_se_attr.manual_sat.SE_th4)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_saturation.p_se_attr.manual_sat.SE_th4 = %d, %d\r\n"), isp_compare_1->p_Isp_saturation.p_se_attr.manual_sat.SE_th4, isp_compare_2->p_Isp_saturation.p_se_attr.manual_sat.SE_th4);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_saturation.p_se_attr.manual_sat.SE_scale_slop1 != isp_compare_2->p_Isp_saturation.p_se_attr.manual_sat.SE_scale_slop1)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_saturation.p_se_attr.manual_sat.SE_scale_slop1 = %d, %d\r\n"), isp_compare_1->p_Isp_saturation.p_se_attr.manual_sat.SE_scale_slop1, isp_compare_2->p_Isp_saturation.p_se_attr.manual_sat.SE_scale_slop1);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_saturation.p_se_attr.manual_sat.SE_scale_slop2 != isp_compare_2->p_Isp_saturation.p_se_attr.manual_sat.SE_scale_slop2)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_saturation.p_se_attr.manual_sat.SE_scale_slop2 = %d, %d\r\n"), isp_compare_1->p_Isp_saturation.p_se_attr.manual_sat.SE_scale_slop2, isp_compare_2->p_Isp_saturation.p_se_attr.manual_sat.SE_scale_slop2);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_saturation.p_se_attr.manual_sat.SE_scale1 != isp_compare_2->p_Isp_saturation.p_se_attr.manual_sat.SE_scale1)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_saturation.p_se_attr.manual_sat.SE_scale1 = %d, %d\r\n"), isp_compare_1->p_Isp_saturation.p_se_attr.manual_sat.SE_scale1, isp_compare_2->p_Isp_saturation.p_se_attr.manual_sat.SE_scale1);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_saturation.p_se_attr.manual_sat.SE_scale2 != isp_compare_2->p_Isp_saturation.p_se_attr.manual_sat.SE_scale2)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_saturation.p_se_attr.manual_sat.SE_scale2 = %d, %d\r\n"), isp_compare_1->p_Isp_saturation.p_se_attr.manual_sat.SE_scale2, isp_compare_2->p_Isp_saturation.p_se_attr.manual_sat.SE_scale2);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_saturation.p_se_attr.manual_sat.SE_scale3 != isp_compare_2->p_Isp_saturation.p_se_attr.manual_sat.SE_scale3)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_saturation.p_se_attr.manual_sat.SE_scale3 = %d, %d\r\n"), isp_compare_1->p_Isp_saturation.p_se_attr.manual_sat.SE_scale3, isp_compare_2->p_Isp_saturation.p_se_attr.manual_sat.SE_scale3);
		pFile->WriteString(str);
	}

	for (i = 0; i < 9; i++)
	{
		if (isp_compare_1->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_enable != isp_compare_2->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_enable)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_saturation.p_se_attr.linkage_sat[i].SE_enable = %d, %d\r\n"), isp_compare_1->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_enable, isp_compare_2->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_enable);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th1 != isp_compare_2->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th1)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th1 = %d, %d\r\n"), isp_compare_1->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th1, isp_compare_2->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th1);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th2 != isp_compare_2->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th2)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th2 = %d, %d\r\n"), isp_compare_1->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th2, isp_compare_2->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th2);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th3 != isp_compare_2->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th3)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th3 = %d, %d\r\n"), isp_compare_1->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th3, isp_compare_2->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th3);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th4 != isp_compare_2->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th4)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th4 = %d, %d\r\n"), isp_compare_1->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th4, isp_compare_2->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th4);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale_slop1 != isp_compare_2->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale_slop1)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale_slop1 = %d, %d\r\n"), isp_compare_1->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale_slop1, isp_compare_2->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale_slop1);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale_slop2 != isp_compare_2->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale_slop2)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale_slop2 = %d, %d\r\n"), isp_compare_1->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale_slop2, isp_compare_2->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale_slop2);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale1 != isp_compare_2->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale1)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale1 = %d, %d\r\n"), isp_compare_1->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale1, isp_compare_2->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale1);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale2 != isp_compare_2->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale2)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale2 = %d, %d\r\n"), isp_compare_1->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale2, isp_compare_2->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale2);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale3 != isp_compare_2->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale3)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale3 = %d, %d\r\n"), isp_compare_1->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale3, isp_compare_2->p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale3);
			pFile->WriteString(str);
		}
	}
	
	//pFile->WriteString(_T("\r\n"));
}

void CCompareDlg::Write_contrast_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) 
{
	CString str;
	UINT i = 0;

	if (isp_compare_1->p_Isp_contrast.param_id != isp_compare_2->p_Isp_contrast.param_id)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_contrast.param_id = %d, %d\r\n"), isp_compare_1->p_Isp_contrast.param_id, isp_compare_2->p_Isp_contrast.param_id);
		pFile->WriteString(str);
	}
	
	if (isp_compare_1->p_Isp_contrast.length != isp_compare_2->p_Isp_contrast.length)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_contrast.length = %d, %d\r\n"), isp_compare_1->p_Isp_contrast.length, isp_compare_2->p_Isp_contrast.length);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_contrast.p_contrast.cc_mode != isp_compare_2->p_Isp_contrast.p_contrast.cc_mode)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_contrast.p_contrast.cc_mode = %d, %d\r\n"), isp_compare_1->p_Isp_contrast.p_contrast.cc_mode, isp_compare_2->p_Isp_contrast.p_contrast.cc_mode);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_contrast.p_contrast.manual_contrast.y_contrast != isp_compare_2->p_Isp_contrast.p_contrast.manual_contrast.y_contrast)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_contrast.p_contrast.manual_contrast.y_contrast = %d, %d\r\n"), isp_compare_1->p_Isp_contrast.p_contrast.manual_contrast.y_contrast, isp_compare_2->p_Isp_contrast.p_contrast.manual_contrast.y_contrast);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_contrast.p_contrast.manual_contrast.y_shift != isp_compare_2->p_Isp_contrast.p_contrast.manual_contrast.y_shift)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_contrast.p_contrast.manual_contrast.y_shift = %d, %d\r\n"), isp_compare_1->p_Isp_contrast.p_contrast.manual_contrast.y_shift, isp_compare_2->p_Isp_contrast.p_contrast.manual_contrast.y_shift);
		pFile->WriteString(str);
	}

	for (i = 0; i < 9; i++)
	{
		if (isp_compare_1->p_Isp_contrast.p_contrast.linkage_contrast[i].dark_pixel_area != isp_compare_2->p_Isp_contrast.p_contrast.linkage_contrast[i].dark_pixel_area)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_contrast.p_contrast.linkage_contrast[%d].dark_pixel_area = %d, %d\r\n"), i, isp_compare_1->p_Isp_contrast.p_contrast.linkage_contrast[i].dark_pixel_area, isp_compare_2->p_Isp_contrast.p_contrast.linkage_contrast[i].dark_pixel_area);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_contrast.p_contrast.linkage_contrast[i].shift_max != isp_compare_2->p_Isp_contrast.p_contrast.linkage_contrast[i].shift_max)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_contrast.p_contrast.linkage_contrast[%d].shift_max = %d, %d\r\n"), i, isp_compare_1->p_Isp_contrast.p_contrast.linkage_contrast[i].shift_max, isp_compare_2->p_Isp_contrast.p_contrast.linkage_contrast[i].shift_max);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_contrast.p_contrast.linkage_contrast[i].dark_pixel_rate != isp_compare_2->p_Isp_contrast.p_contrast.linkage_contrast[i].dark_pixel_rate)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_contrast.p_contrast.linkage_contrast[%d].dark_pixel_rate = %d, %d\r\n"), i, isp_compare_1->p_Isp_contrast.p_contrast.linkage_contrast[i].dark_pixel_rate, isp_compare_2->p_Isp_contrast.p_contrast.linkage_contrast[i].dark_pixel_rate);
			pFile->WriteString(str);
		}
	}
	
	//pFile->WriteString(_T("\r\n"));
}

void CCompareDlg::Write_rgb2yuv_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) 
{
	CString str;
	UINT i = 0;

	if (isp_compare_1->p_Isp_rgb2yuv.param_id != isp_compare_2->p_Isp_rgb2yuv.param_id)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_rgb2yuv.param_id = %d, %d\r\n"), isp_compare_1->p_Isp_rgb2yuv.param_id, isp_compare_2->p_Isp_rgb2yuv.param_id);
		pFile->WriteString(str);
	}
	
	if (isp_compare_1->p_Isp_rgb2yuv.length != isp_compare_2->p_Isp_rgb2yuv.length)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_rgb2yuv.length = %d, %d\r\n"), isp_compare_1->p_Isp_rgb2yuv.length, isp_compare_2->p_Isp_rgb2yuv.length);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_rgb2yuv.p_rgb2yuv.mode != isp_compare_2->p_Isp_rgb2yuv.p_rgb2yuv.mode)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_rgb2yuv.p_rgb2yuv.mode = %d, %d\r\n"), isp_compare_1->p_Isp_rgb2yuv.p_rgb2yuv.mode, isp_compare_2->p_Isp_rgb2yuv.p_rgb2yuv.mode);
		pFile->WriteString(str);
	}

	
	//pFile->WriteString(_T("\r\n"));
}

void CCompareDlg::Write_effect_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) 
{
	CString str;
	UINT i = 0;

	if (isp_compare_1->p_Isp_effect.param_id != isp_compare_2->p_Isp_effect.param_id)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_effect.param_id = %d, %d\r\n"), isp_compare_1->p_Isp_effect.param_id, isp_compare_2->p_Isp_effect.param_id);
		pFile->WriteString(str);
	}
	
	if (isp_compare_1->p_Isp_effect.length != isp_compare_2->p_Isp_effect.length)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_effect.length = %d, %d\r\n"), isp_compare_1->p_Isp_effect.length, isp_compare_2->p_Isp_effect.length);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_effect.p_isp_effect.y_a != isp_compare_2->p_Isp_effect.p_isp_effect.y_a)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_effect.p_isp_effect.y_a = %d, %d\r\n"), isp_compare_1->p_Isp_effect.p_isp_effect.y_a, isp_compare_2->p_Isp_effect.p_isp_effect.y_a);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_effect.p_isp_effect.y_b != isp_compare_2->p_Isp_effect.p_isp_effect.y_b)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_effect.p_isp_effect.y_b = %d, %d\r\n"), isp_compare_1->p_Isp_effect.p_isp_effect.y_b, isp_compare_2->p_Isp_effect.p_isp_effect.y_b);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_effect.p_isp_effect.uv_a != isp_compare_2->p_Isp_effect.p_isp_effect.uv_a)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_effect.p_isp_effect.uv_a = %d, %d\r\n"), isp_compare_1->p_Isp_effect.p_isp_effect.uv_a, isp_compare_2->p_Isp_effect.p_isp_effect.uv_a);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_effect.p_isp_effect.uv_b != isp_compare_2->p_Isp_effect.p_isp_effect.uv_b)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_effect.p_isp_effect.uv_b = %d, %d\r\n"), isp_compare_1->p_Isp_effect.p_isp_effect.uv_b, isp_compare_2->p_Isp_effect.p_isp_effect.uv_b);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_effect.p_isp_effect.dark_margin_en != isp_compare_2->p_Isp_effect.p_isp_effect.dark_margin_en)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_effect.p_isp_effect.dark_margin_en = %d, %d\r\n"), isp_compare_1->p_Isp_effect.p_isp_effect.dark_margin_en, isp_compare_2->p_Isp_effect.p_isp_effect.dark_margin_en);
		pFile->WriteString(str);
	}
	
	//pFile->WriteString(_T("\r\n"));
}

void CCompareDlg::Write_dpc_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) 
{
	CString str;
	UINT i = 0;

	if (isp_compare_1->p_Isp_dpc.param_id != isp_compare_2->p_Isp_dpc.param_id)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_dpc.param_id = %d, %d\r\n"), isp_compare_1->p_Isp_dpc.param_id, isp_compare_2->p_Isp_dpc.param_id);
		pFile->WriteString(str);
	}
	
	if (isp_compare_1->p_Isp_dpc.length != isp_compare_2->p_Isp_dpc.length)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_dpc.length = %d, %d\r\n"), isp_compare_1->p_Isp_dpc.length, isp_compare_2->p_Isp_dpc.length);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_dpc.p_ddpc.ddpc_mode != isp_compare_2->p_Isp_dpc.p_ddpc.ddpc_mode)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_dpc.p_ddpc.ddpc_mode = %d, %d\r\n"), isp_compare_1->p_Isp_dpc.p_ddpc.ddpc_mode, isp_compare_2->p_Isp_dpc.p_ddpc.ddpc_mode);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_dpc.p_ddpc.manual_ddpc.ddpc_enable != isp_compare_2->p_Isp_dpc.p_ddpc.manual_ddpc.ddpc_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_dpc.p_ddpc.manual_ddpc.ddpc_enable = %d, %d\r\n"), isp_compare_1->p_Isp_dpc.p_ddpc.manual_ddpc.ddpc_enable, isp_compare_2->p_Isp_dpc.p_ddpc.manual_ddpc.ddpc_enable);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_dpc.p_ddpc.manual_ddpc.ddpc_th != isp_compare_2->p_Isp_dpc.p_ddpc.manual_ddpc.ddpc_th)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_dpc.p_ddpc.manual_ddpc.ddpc_th = %d, %d\r\n"), isp_compare_1->p_Isp_dpc.p_ddpc.manual_ddpc.ddpc_th, isp_compare_2->p_Isp_dpc.p_ddpc.manual_ddpc.ddpc_th);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_dpc.p_ddpc.manual_ddpc.white_dpc_enable != isp_compare_2->p_Isp_dpc.p_ddpc.manual_ddpc.white_dpc_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_dpc.p_ddpc.manual_ddpc.white_dpc_enable = %d, %d\r\n"), isp_compare_1->p_Isp_dpc.p_ddpc.manual_ddpc.white_dpc_enable, isp_compare_2->p_Isp_dpc.p_ddpc.manual_ddpc.white_dpc_enable);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_dpc.p_ddpc.manual_ddpc.black_dpc_enable != isp_compare_2->p_Isp_dpc.p_ddpc.manual_ddpc.black_dpc_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_dpc.p_ddpc.manual_ddpc.black_dpc_enable = %d, %d\r\n"), isp_compare_1->p_Isp_dpc.p_ddpc.manual_ddpc.black_dpc_enable, isp_compare_2->p_Isp_dpc.p_ddpc.manual_ddpc.black_dpc_enable);
		pFile->WriteString(str);
	}

	for (i = 0; i < 9; i++)
	{
		if (isp_compare_1->p_Isp_dpc.p_ddpc.linkage_ddpc[i].ddpc_enable != isp_compare_2->p_Isp_dpc.p_ddpc.linkage_ddpc[i].ddpc_enable)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_dpc.p_ddpc.linkage_ddpc[%d].ddpc_enable = %d, %d\r\n"), i, isp_compare_1->p_Isp_dpc.p_ddpc.linkage_ddpc[i].ddpc_enable, isp_compare_2->p_Isp_dpc.p_ddpc.linkage_ddpc[i].ddpc_enable);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_dpc.p_ddpc.linkage_ddpc[i].ddpc_th != isp_compare_2->p_Isp_dpc.p_ddpc.linkage_ddpc[i].ddpc_th)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_dpc.p_ddpc.linkage_ddpc[%d].ddpc_th = %d, %d\r\n"), i, isp_compare_1->p_Isp_dpc.p_ddpc.linkage_ddpc[i].ddpc_th, isp_compare_2->p_Isp_dpc.p_ddpc.linkage_ddpc[i].ddpc_th);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_dpc.p_ddpc.linkage_ddpc[i].white_dpc_enable != isp_compare_2->p_Isp_dpc.p_ddpc.linkage_ddpc[i].white_dpc_enable)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_dpc.p_ddpc.linkage_ddpc[%d].white_dpc_enable = %d, %d\r\n"), i, isp_compare_1->p_Isp_dpc.p_ddpc.linkage_ddpc[i].white_dpc_enable, isp_compare_2->p_Isp_dpc.p_ddpc.linkage_ddpc[i].white_dpc_enable);
			pFile->WriteString(str);
		}
		
		if (isp_compare_1->p_Isp_dpc.p_ddpc.linkage_ddpc[i].black_dpc_enable != isp_compare_2->p_Isp_dpc.p_ddpc.linkage_ddpc[i].black_dpc_enable)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_dpc.p_ddpc.linkage_ddpc[%d].black_dpc_enable = %d, %d\r\n"), i, isp_compare_1->p_Isp_dpc.p_ddpc.linkage_ddpc[i].black_dpc_enable, isp_compare_2->p_Isp_dpc.p_ddpc.linkage_ddpc[i].black_dpc_enable);
			pFile->WriteString(str);
		}
	}

	if (isp_compare_1->p_Isp_dpc.p_sdpc.sdpc_enable != isp_compare_2->p_Isp_dpc.p_sdpc.sdpc_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_dpc.p_sdpc.sdpc_enable = %d, %d\r\n"), isp_compare_1->p_Isp_dpc.p_sdpc.sdpc_enable, isp_compare_2->p_Isp_dpc.p_sdpc.sdpc_enable);
		pFile->WriteString(str);
	}

	for (i = 0; i < 1024; i++)
	{
		if (isp_compare_1->p_Isp_dpc.p_sdpc.sdpc_table[i] != isp_compare_2->p_Isp_dpc.p_sdpc.sdpc_table[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_dpc.p_sdpc.sdpc_table[i] = %d, %d\r\n"), isp_compare_1->p_Isp_dpc.p_sdpc.sdpc_table[i], isp_compare_2->p_Isp_dpc.p_sdpc.sdpc_table[i]);
			pFile->WriteString(str);
		}
	}
	
	//pFile->WriteString(_T("\r\n"));
}

void CCompareDlg::Write_weight_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) 
{
	CString str;
	UINT i = 0, j = 0;

	if (isp_compare_1->p_Isp_weight.param_id != isp_compare_2->p_Isp_weight.param_id)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_weight.param_id = %d, %d\r\n"), isp_compare_1->p_Isp_weight.param_id, isp_compare_2->p_Isp_weight.param_id);
		pFile->WriteString(str);
	}
	
	if (isp_compare_1->p_Isp_weight.length != isp_compare_2->p_Isp_weight.length)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_weight.length = %d, %d\r\n"), isp_compare_1->p_Isp_weight.length, isp_compare_2->p_Isp_weight.length);
		pFile->WriteString(str);
	}

	for (j = 0; j < 8; j++)
	{
		for (i = 0; i < 16; i++)
		{
			if (isp_compare_1->p_Isp_weight.p_weight.zone_weight[j][i] != isp_compare_2->p_Isp_weight.p_weight.zone_weight[j][i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_weight.p_weight.zone_weight[%d][%d] = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_weight.p_weight.zone_weight[j][i], isp_compare_2->p_Isp_weight.p_weight.zone_weight[j][i]);
				pFile->WriteString(str);
			}
		}
	}

	//pFile->WriteString(_T("\r\n"));
}

void CCompareDlg::Write_af_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) 
{
	CString str;
	UINT i = 0;

	if (isp_compare_1->p_Isp_af.param_id != isp_compare_2->p_Isp_af.param_id)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_af.param_id = %d, %d\r\n"), isp_compare_1->p_Isp_af.param_id, isp_compare_2->p_Isp_af.param_id);
		pFile->WriteString(str);
	}
	
	if (isp_compare_1->p_Isp_af.length != isp_compare_2->p_Isp_af.length)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_af.length = %d, %d\r\n"), isp_compare_1->p_Isp_af.length, isp_compare_2->p_Isp_af.length);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_af.p_af_attr.af_win0_left != isp_compare_2->p_Isp_af.p_af_attr.af_win0_left)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_af.p_af_attr.af_win0_left = %d, %d\r\n"), isp_compare_1->p_Isp_af.p_af_attr.af_win0_left, isp_compare_2->p_Isp_af.p_af_attr.af_win0_left);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_af.p_af_attr.af_win0_right != isp_compare_2->p_Isp_af.p_af_attr.af_win0_right)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_af.p_af_attr.af_win0_right = %d, %d\r\n"), isp_compare_1->p_Isp_af.p_af_attr.af_win0_right, isp_compare_2->p_Isp_af.p_af_attr.af_win0_right);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_af.p_af_attr.af_win0_top != isp_compare_2->p_Isp_af.p_af_attr.af_win0_top)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_af.p_af_attr.af_win0_top = %d, %d\r\n"), isp_compare_1->p_Isp_af.p_af_attr.af_win0_top, isp_compare_2->p_Isp_af.p_af_attr.af_win0_top);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_af.p_af_attr.af_win0_bottom != isp_compare_2->p_Isp_af.p_af_attr.af_win0_bottom)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_af.p_af_attr.af_win0_bottom = %d, %d\r\n"), isp_compare_1->p_Isp_af.p_af_attr.af_win0_bottom, isp_compare_2->p_Isp_af.p_af_attr.af_win0_bottom);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_af.p_af_attr.af_th != isp_compare_2->p_Isp_af.p_af_attr.af_th)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_af.p_af_attr.af_th = %d, %d\r\n"), isp_compare_1->p_Isp_af.p_af_attr.af_th, isp_compare_2->p_Isp_af.p_af_attr.af_th);
		pFile->WriteString(str);
	}

	
	if (isp_compare_1->p_Isp_af.p_af_attr.af_win1_left != isp_compare_2->p_Isp_af.p_af_attr.af_win1_left)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_af.p_af_attr.af_win1_left = %d, %d\r\n"), isp_compare_1->p_Isp_af.p_af_attr.af_win1_left, isp_compare_2->p_Isp_af.p_af_attr.af_win1_left);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_af.p_af_attr.af_win1_right != isp_compare_2->p_Isp_af.p_af_attr.af_win1_right)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_af.p_af_attr.af_win1_right = %d, %d\r\n"), isp_compare_1->p_Isp_af.p_af_attr.af_win1_right, isp_compare_2->p_Isp_af.p_af_attr.af_win1_right);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_af.p_af_attr.af_win1_top != isp_compare_2->p_Isp_af.p_af_attr.af_win1_top)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_af.p_af_attr.af_win1_top = %d, %d\r\n"), isp_compare_1->p_Isp_af.p_af_attr.af_win1_top, isp_compare_2->p_Isp_af.p_af_attr.af_win1_top);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_af.p_af_attr.af_win1_bottom != isp_compare_2->p_Isp_af.p_af_attr.af_win1_bottom)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_af.p_af_attr.af_win1_bottom = %d, %d\r\n"), isp_compare_1->p_Isp_af.p_af_attr.af_win1_bottom, isp_compare_2->p_Isp_af.p_af_attr.af_win1_bottom);
		pFile->WriteString(str);
	}


	if (isp_compare_1->p_Isp_af.p_af_attr.af_win2_left != isp_compare_2->p_Isp_af.p_af_attr.af_win2_left)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_af.p_af_attr.af_win2_left = %d, %d\r\n"), isp_compare_1->p_Isp_af.p_af_attr.af_win2_left, isp_compare_2->p_Isp_af.p_af_attr.af_win2_left);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_af.p_af_attr.af_win2_right != isp_compare_2->p_Isp_af.p_af_attr.af_win2_right)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_af.p_af_attr.af_win2_right = %d, %d\r\n"), isp_compare_1->p_Isp_af.p_af_attr.af_win2_right, isp_compare_2->p_Isp_af.p_af_attr.af_win2_right);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_af.p_af_attr.af_win2_top != isp_compare_2->p_Isp_af.p_af_attr.af_win2_top)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_af.p_af_attr.af_win2_top = %d, %d\r\n"), isp_compare_1->p_Isp_af.p_af_attr.af_win2_top, isp_compare_2->p_Isp_af.p_af_attr.af_win2_top);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_af.p_af_attr.af_win2_bottom != isp_compare_2->p_Isp_af.p_af_attr.af_win2_bottom)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_af.p_af_attr.af_win2_bottom = %d, %d\r\n"), isp_compare_1->p_Isp_af.p_af_attr.af_win2_bottom, isp_compare_2->p_Isp_af.p_af_attr.af_win2_bottom);
		pFile->WriteString(str);
	}


	if (isp_compare_1->p_Isp_af.p_af_attr.af_win3_left != isp_compare_2->p_Isp_af.p_af_attr.af_win3_left)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_af.p_af_attr.af_win3_left = %d, %d\r\n"), isp_compare_1->p_Isp_af.p_af_attr.af_win3_left, isp_compare_2->p_Isp_af.p_af_attr.af_win3_left);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_af.p_af_attr.af_win3_right != isp_compare_2->p_Isp_af.p_af_attr.af_win3_right)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_af.p_af_attr.af_win3_right = %d, %d\r\n"), isp_compare_1->p_Isp_af.p_af_attr.af_win3_right, isp_compare_2->p_Isp_af.p_af_attr.af_win3_right);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_af.p_af_attr.af_win3_top != isp_compare_2->p_Isp_af.p_af_attr.af_win3_top)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_af.p_af_attr.af_win3_top = %d, %d\r\n"), isp_compare_1->p_Isp_af.p_af_attr.af_win3_top, isp_compare_2->p_Isp_af.p_af_attr.af_win3_top);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_af.p_af_attr.af_win3_bottom != isp_compare_2->p_Isp_af.p_af_attr.af_win3_bottom)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_af.p_af_attr.af_win3_bottom = %d, %d\r\n"), isp_compare_1->p_Isp_af.p_af_attr.af_win3_bottom, isp_compare_2->p_Isp_af.p_af_attr.af_win3_bottom);
		pFile->WriteString(str);
	}


	if (isp_compare_1->p_Isp_af.p_af_attr.af_win4_left != isp_compare_2->p_Isp_af.p_af_attr.af_win4_left)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_af.p_af_attr.af_win4_left = %d, %d\r\n"), isp_compare_1->p_Isp_af.p_af_attr.af_win4_left, isp_compare_2->p_Isp_af.p_af_attr.af_win4_left);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_af.p_af_attr.af_win4_right != isp_compare_2->p_Isp_af.p_af_attr.af_win4_right)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_af.p_af_attr.af_win4_right = %d, %d\r\n"), isp_compare_1->p_Isp_af.p_af_attr.af_win4_right, isp_compare_2->p_Isp_af.p_af_attr.af_win4_right);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_af.p_af_attr.af_win4_top != isp_compare_2->p_Isp_af.p_af_attr.af_win4_top)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_af.p_af_attr.af_win4_top = %d, %d\r\n"), isp_compare_1->p_Isp_af.p_af_attr.af_win4_top, isp_compare_2->p_Isp_af.p_af_attr.af_win4_top);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_af.p_af_attr.af_win4_bottom != isp_compare_2->p_Isp_af.p_af_attr.af_win4_bottom)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_af.p_af_attr.af_win4_bottom = %d, %d\r\n"), isp_compare_1->p_Isp_af.p_af_attr.af_win4_bottom, isp_compare_2->p_Isp_af.p_af_attr.af_win4_bottom);
		pFile->WriteString(str);
	}
	
	//pFile->WriteString(_T("\r\n"));
}

void CCompareDlg::Write_wb_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) 
{
	CString str;
	UINT i = 0;

	if (isp_compare_1->p_Isp_wb.param_id != isp_compare_2->p_Isp_wb.param_id)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wb.param_id = %d, %d\r\n"), isp_compare_1->p_Isp_wb.param_id, isp_compare_2->p_Isp_wb.param_id);
		pFile->WriteString(str);
	}
	
	if (isp_compare_1->p_Isp_wb.length != isp_compare_2->p_Isp_wb.length)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wb.length = %d, %d\r\n"), isp_compare_1->p_Isp_wb.length, isp_compare_2->p_Isp_wb.length);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_wb.wb_type.wb_type != isp_compare_2->p_Isp_wb.wb_type.wb_type)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wb.wb_type.wb_type = %d, %d\r\n"), isp_compare_1->p_Isp_wb.wb_type.wb_type, isp_compare_2->p_Isp_wb.wb_type.wb_type);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_wb.p_mwb.r_gain != isp_compare_2->p_Isp_wb.p_mwb.r_gain)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wb.p_mwb.r_gain = %d, %d\r\n"), isp_compare_1->p_Isp_wb.p_mwb.r_gain, isp_compare_2->p_Isp_wb.p_mwb.r_gain);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_wb.p_mwb.g_gain != isp_compare_2->p_Isp_wb.p_mwb.g_gain)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wb.p_mwb.g_gain = %d, %d\r\n"), isp_compare_1->p_Isp_wb.p_mwb.g_gain, isp_compare_2->p_Isp_wb.p_mwb.g_gain);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_wb.p_mwb.b_gain != isp_compare_2->p_Isp_wb.p_mwb.b_gain)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wb.p_mwb.b_gain = %d, %d\r\n"), isp_compare_1->p_Isp_wb.p_mwb.b_gain, isp_compare_2->p_Isp_wb.p_mwb.b_gain);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_wb.p_mwb.r_offset != isp_compare_2->p_Isp_wb.p_mwb.r_offset)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wb.p_mwb.r_offset = %d, %d\r\n"), isp_compare_1->p_Isp_wb.p_mwb.r_offset, isp_compare_2->p_Isp_wb.p_mwb.r_offset);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_wb.p_mwb.g_offset != isp_compare_2->p_Isp_wb.p_mwb.g_offset)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wb.p_mwb.g_offset = %d, %d\r\n"), isp_compare_1->p_Isp_wb.p_mwb.g_offset, isp_compare_2->p_Isp_wb.p_mwb.g_offset);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_wb.p_mwb.b_offset != isp_compare_2->p_Isp_wb.p_mwb.b_offset)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wb.p_mwb.b_offset = %d, %d\r\n"), isp_compare_1->p_Isp_wb.p_mwb.b_offset, isp_compare_2->p_Isp_wb.p_mwb.b_offset);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_wb.p_awb.y_low != isp_compare_2->p_Isp_wb.p_awb.y_low)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wb.p_awb.y_low = %d, %d\r\n"), isp_compare_1->p_Isp_wb.p_awb.y_low, isp_compare_2->p_Isp_wb.p_awb.y_low);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_wb.p_awb.y_high != isp_compare_2->p_Isp_wb.p_awb.y_high)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wb.p_awb.y_high = %d, %d\r\n"), isp_compare_1->p_Isp_wb.p_awb.y_high, isp_compare_2->p_Isp_wb.p_awb.y_high);
		pFile->WriteString(str);
	}

	
	if (isp_compare_1->p_Isp_wb.p_awb.err_est != isp_compare_2->p_Isp_wb.p_awb.err_est)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wb.p_awb.err_est = %d, %d\r\n"), isp_compare_1->p_Isp_wb.p_awb.err_est, isp_compare_2->p_Isp_wb.p_awb.err_est);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_wb.p_awb.auto_wb_step != isp_compare_2->p_Isp_wb.p_awb.auto_wb_step)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wb.p_awb.auto_wb_step = %d, %d\r\n"), isp_compare_1->p_Isp_wb.p_awb.auto_wb_step, isp_compare_2->p_Isp_wb.p_awb.auto_wb_step);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_wb.p_awb.total_cnt_thresh != isp_compare_2->p_Isp_wb.p_awb.total_cnt_thresh)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wb.p_awb.total_cnt_thresh = %d, %d\r\n"), isp_compare_1->p_Isp_wb.p_awb.auto_wb_step, isp_compare_2->p_Isp_wb.p_awb.total_cnt_thresh);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_wb.p_awb.colortemp_stable_cnt_thresh != isp_compare_2->p_Isp_wb.p_awb.colortemp_stable_cnt_thresh)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wb.p_awb.colortemp_stable_cnt_thresh = %d, %d\r\n"), isp_compare_1->p_Isp_wb.p_awb.colortemp_stable_cnt_thresh, isp_compare_2->p_Isp_wb.p_awb.colortemp_stable_cnt_thresh);
		pFile->WriteString(str);
	}

	for (i = 0; i < 16; i++)
	{
		if (isp_compare_1->p_Isp_wb.p_awb.g_weight[i] != isp_compare_2->p_Isp_wb.p_awb.g_weight[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wb.p_awb.g_weight[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_wb.p_awb.g_weight[i], isp_compare_2->p_Isp_wb.p_awb.g_weight[i]);
			pFile->WriteString(str);
		}
	}

	for (i = 0; i < 10; i++)
	{
		if (isp_compare_1->p_Isp_wb.p_awb.gr_low[i] != isp_compare_2->p_Isp_wb.p_awb.gr_low[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wb.p_awb.gr_low[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_wb.p_awb.gr_low[i], isp_compare_2->p_Isp_wb.p_awb.gr_low[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_wb.p_awb.gb_low[i] != isp_compare_2->p_Isp_wb.p_awb.gb_low[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wb.p_awb.gb_low[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_wb.p_awb.gb_low[i], isp_compare_2->p_Isp_wb.p_awb.gb_low[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_wb.p_awb.gr_high[i] != isp_compare_2->p_Isp_wb.p_awb.gr_high[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wb.p_awb.gr_high[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_wb.p_awb.gr_high[i], isp_compare_2->p_Isp_wb.p_awb.gr_high[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_wb.p_awb.gb_high[i] != isp_compare_2->p_Isp_wb.p_awb.gb_high[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wb.p_awb.gb_high[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_wb.p_awb.gb_high[i], isp_compare_2->p_Isp_wb.p_awb.gb_high[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_wb.p_awb.rb_low[i] != isp_compare_2->p_Isp_wb.p_awb.rb_low[i])
		{
			
			diffrent_num++;
			str.Format(_T("p_Isp_wb.p_awb.rb_low[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_wb.p_awb.rb_low[i], isp_compare_2->p_Isp_wb.p_awb.rb_low[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_wb.p_awb.rb_high[i] != isp_compare_2->p_Isp_wb.p_awb.rb_high[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wb.p_awb.rb_high[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_wb.p_awb.rb_high[i], isp_compare_2->p_Isp_wb.p_awb.rb_high[i]);
			pFile->WriteString(str);
		}
	}

	for (i = 0; i < 10; i++)
	{
		if (isp_compare_1->p_Isp_wb.p_awb.colortemp_envi[i] != isp_compare_2->p_Isp_wb.p_awb.colortemp_envi[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wb.p_awb.colortemp_envi[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_wb.p_awb.colortemp_envi[i], isp_compare_2->p_Isp_wb.p_awb.colortemp_envi[i]);
			pFile->WriteString(str);
		}
	}

	if (isp_compare_1->p_Isp_wb.p_awb_ex.awb_ex_ctrl_enable != isp_compare_2->p_Isp_wb.p_awb_ex.awb_ex_ctrl_enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_wb.p_awb_ex.awb_ex_ctrl_enable = %d, %d\r\n"), isp_compare_1->p_Isp_wb.p_awb_ex.awb_ex_ctrl_enable, isp_compare_2->p_Isp_wb.p_awb_ex.awb_ex_ctrl_enable);
		pFile->WriteString(str);
	}

	for (i = 0; i < 10; i++)
	{
		if (isp_compare_1->p_Isp_wb.p_awb_ex.awb_ctrl[i].rgain_max != isp_compare_2->p_Isp_wb.p_awb_ex.awb_ctrl[i].rgain_max)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wb.p_awb_ex.awb_ctrl[%d].rgain_max = %d, %d\r\n"), i, isp_compare_1->p_Isp_wb.p_awb_ex.awb_ctrl[i].rgain_max, isp_compare_2->p_Isp_wb.p_awb_ex.awb_ctrl[i].rgain_max);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_wb.p_awb_ex.awb_ctrl[i].rgain_min != isp_compare_2->p_Isp_wb.p_awb_ex.awb_ctrl[i].rgain_min)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wb.p_awb_ex.awb_ctrl[%d].rgain_min = %d, %d\r\n"), i, isp_compare_1->p_Isp_wb.p_awb_ex.awb_ctrl[i].rgain_min, isp_compare_2->p_Isp_wb.p_awb_ex.awb_ctrl[i].rgain_min);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_wb.p_awb_ex.awb_ctrl[i].ggain_max != isp_compare_2->p_Isp_wb.p_awb_ex.awb_ctrl[i].ggain_max)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wb.p_awb_ex.awb_ctrl[%d].ggain_max = %d, %d\r\n"), i, isp_compare_1->p_Isp_wb.p_awb_ex.awb_ctrl[i].ggain_max, isp_compare_2->p_Isp_wb.p_awb_ex.awb_ctrl[i].ggain_max);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_wb.p_awb_ex.awb_ctrl[i].ggain_min != isp_compare_2->p_Isp_wb.p_awb_ex.awb_ctrl[i].ggain_min)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wb.p_awb_ex.awb_ctrl[%d].ggain_min = %d, %d\r\n"), i, isp_compare_1->p_Isp_wb.p_awb_ex.awb_ctrl[i].ggain_min, isp_compare_2->p_Isp_wb.p_awb_ex.awb_ctrl[i].ggain_min);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_wb.p_awb_ex.awb_ctrl[i].bgain_max != isp_compare_2->p_Isp_wb.p_awb_ex.awb_ctrl[i].bgain_max)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wb.p_awb_ex.awb_ctrl[%d].bgain_max = %d, %d\r\n"), i, isp_compare_1->p_Isp_wb.p_awb_ex.awb_ctrl[i].bgain_max, isp_compare_2->p_Isp_wb.p_awb_ex.awb_ctrl[i].bgain_max);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_wb.p_awb_ex.awb_ctrl[i].bgain_min != isp_compare_2->p_Isp_wb.p_awb_ex.awb_ctrl[i].bgain_min)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wb.p_awb_ex.awb_ctrl[%d].bgain_min = %d, %d\r\n"), i, isp_compare_1->p_Isp_wb.p_awb_ex.awb_ctrl[i].bgain_min, isp_compare_2->p_Isp_wb.p_awb_ex.awb_ctrl[i].bgain_min);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_wb.p_awb_ex.awb_ctrl[i].rgain_ex != isp_compare_2->p_Isp_wb.p_awb_ex.awb_ctrl[i].rgain_ex)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wb.p_awb_ex.awb_ctrl[%d].rgain_ex = %d, %d\r\n"), i, isp_compare_1->p_Isp_wb.p_awb_ex.awb_ctrl[i].rgain_ex, isp_compare_2->p_Isp_wb.p_awb_ex.awb_ctrl[i].rgain_ex);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_wb.p_awb_ex.awb_ctrl[i].bgain_ex != isp_compare_2->p_Isp_wb.p_awb_ex.awb_ctrl[i].bgain_ex)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_wb.p_awb_ex.awb_ctrl[%d].bgain_ex = %d, %d\r\n"), i, isp_compare_1->p_Isp_wb.p_awb_ex.awb_ctrl[i].bgain_ex, isp_compare_2->p_Isp_wb.p_awb_ex.awb_ctrl[i].bgain_ex);
			pFile->WriteString(str);
		}
	}


	//pFile->WriteString(_T("\r\n"));
}

void CCompareDlg::Write_exp_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) 
{
	CString str;
	UINT i = 0, j = 0;

	if (isp_compare_1->p_Isp_exp.param_id != isp_compare_2->p_Isp_exp.param_id)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_exp.param_id = %d, %d\r\n"), isp_compare_1->p_Isp_exp.param_id, isp_compare_2->p_Isp_exp.param_id);
		pFile->WriteString(str);
	}
	
	if (isp_compare_1->p_Isp_exp.length != isp_compare_2->p_Isp_exp.length)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_exp.length = %d, %d\r\n"), isp_compare_1->p_Isp_exp.length, isp_compare_2->p_Isp_exp.length);
		pFile->WriteString(str);
	}
	
	if (isp_compare_1->p_Isp_exp.p_raw_hist.enable != isp_compare_2->p_Isp_exp.p_raw_hist.enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_exp.p_raw_hist.enable = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_raw_hist.enable, isp_compare_2->p_Isp_exp.p_raw_hist.enable);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_exp.p_rgb_hist.enable != isp_compare_2->p_Isp_exp.p_rgb_hist.enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_exp.p_rgb_hist.enable = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_rgb_hist.enable, isp_compare_2->p_Isp_exp.p_rgb_hist.enable);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_exp.p_yuv_hist.enable != isp_compare_2->p_Isp_exp.p_yuv_hist.enable)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_exp.p_yuv_hist.enable = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_yuv_hist.enable, isp_compare_2->p_Isp_exp.p_yuv_hist.enable);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_exp.p_exp_type.exp_type != isp_compare_2->p_Isp_exp.p_exp_type.exp_type)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_exp.p_exp_type.exp_type = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_exp_type.exp_type, isp_compare_2->p_Isp_exp.p_exp_type.exp_type);
		pFile->WriteString(str);
	}

	if ((isp_compare_1->p_Isp_exp.p_frame_rate.hight_light_frame_rate & 0xffffff00)
		&& (isp_compare_2->p_Isp_exp.p_frame_rate.hight_light_frame_rate & 0xffffff00))
	{
		AK_ISP_FRAME_RATE_ATTR_EX* fr_ex1 = (AK_ISP_FRAME_RATE_ATTR_EX*)&isp_compare_1->p_Isp_exp.p_frame_rate;
		AK_ISP_FRAME_RATE_ATTR_EX* fr_ex2 = (AK_ISP_FRAME_RATE_ATTR_EX*)&isp_compare_2->p_Isp_exp.p_frame_rate;

		if (fr_ex1->fps1 != fr_ex2->fps1)
		{
			diffrent_num++;
			str.Format(_T("hight_light_frame_rate = %d, %d\r\n"), fr_ex1->fps1, fr_ex2->fps1);
			pFile->WriteString(str);
		}

		if (fr_ex1->max_exp_time1 != fr_ex2->max_exp_time1)
		{
			diffrent_num++;
			str.Format(_T("hight_light_max_exp_time = %d, %d\r\n"), fr_ex1->max_exp_time1, fr_ex2->max_exp_time1);
			pFile->WriteString(str);
		}

		if (fr_ex1->gain12 != fr_ex2->gain12)
		{
			diffrent_num++;
			str.Format(_T("hight_light_to_lower_light_gain = %d, %d\r\n"), fr_ex1->gain12, fr_ex2->gain12);
			pFile->WriteString(str);
		}

		if (fr_ex1->fps2 != fr_ex2->fps2)
		{
			diffrent_num++;
			str.Format(_T("mid_light_frame_rate = %d, %d\r\n"), fr_ex1->fps2, fr_ex2->fps2);
			pFile->WriteString(str);
		}

		if (fr_ex1->max_exp_time2 != fr_ex2->max_exp_time2)
		{
			diffrent_num++;
			str.Format(_T("mid_light_max_exp_time = %d, %d\r\n"), fr_ex1->max_exp_time2, fr_ex2->max_exp_time2);
			pFile->WriteString(str);
		}

		if (fr_ex1->gain22 != fr_ex2->gain22)
		{
			diffrent_num++;
			str.Format(_T("mid_light_to_low_light_gain = %d, %d\r\n"), fr_ex1->gain22, fr_ex2->gain22);
			pFile->WriteString(str);
		}

		if (fr_ex1->fps3 != fr_ex2->fps3)
		{
			diffrent_num++;
			str.Format(_T("low_light_frame_rate = %d, %d\r\n"), fr_ex1->fps3, fr_ex2->fps3);
			pFile->WriteString(str);
		}

		if (fr_ex1->max_exp_time3 != fr_ex2->max_exp_time3)
		{
			diffrent_num++;
			str.Format(_T("low_light_max_exp_time = %d, %d\r\n"), fr_ex1->max_exp_time3, fr_ex2->max_exp_time3);
			pFile->WriteString(str);
		}

		if (fr_ex1->gain31 != fr_ex2->gain31)
		{
			diffrent_num++;
			str.Format(_T("low_light_to_higher_light_gain = %d, %d\r\n"), fr_ex1->gain31, fr_ex2->gain31);
			pFile->WriteString(str);
		}
	}
	else if (!(isp_compare_1->p_Isp_exp.p_frame_rate.hight_light_frame_rate & 0xffffff00)
		&& (isp_compare_2->p_Isp_exp.p_frame_rate.hight_light_frame_rate & 0xffffff00))
	{
		AK_ISP_FRAME_RATE_ATTR_EX* fr_ex2 = (AK_ISP_FRAME_RATE_ATTR_EX*)&isp_compare_2->p_Isp_exp.p_frame_rate;

		if (isp_compare_1->p_Isp_exp.p_frame_rate.hight_light_frame_rate != fr_ex2->fps1)
		{
			diffrent_num++;
			str.Format(_T("hight_light_frame_rate = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_frame_rate.hight_light_frame_rate, fr_ex2->fps1);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_exp.p_frame_rate.hight_light_max_exp_time != fr_ex2->max_exp_time1)
		{
			diffrent_num++;
			str.Format(_T("hight_light_max_exp_time = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_frame_rate.hight_light_max_exp_time, fr_ex2->max_exp_time1);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_exp.p_frame_rate.hight_light_to_low_light_gain != fr_ex2->gain12)
		{
			diffrent_num++;
			str.Format(_T("hight_light_to_lower_light_gain = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_frame_rate.hight_light_to_low_light_gain, fr_ex2->gain12);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_exp.p_frame_rate.low_light_frame_rate != fr_ex2->fps3)
		{
			diffrent_num++;
			str.Format(_T("low_light_frame_rate = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_frame_rate.low_light_frame_rate, fr_ex2->fps3);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_exp.p_frame_rate.low_light_max_exp_time != fr_ex2->max_exp_time3)
		{
			diffrent_num++;
			str.Format(_T("low_light_max_exp_time = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_frame_rate.low_light_max_exp_time, fr_ex2->max_exp_time3);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_exp.p_frame_rate.low_light_to_hight_light_gain != fr_ex2->gain31)
		{
			diffrent_num++;
			str.Format(_T("low_light_to_higher_light_gain = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_frame_rate.low_light_to_hight_light_gain, fr_ex2->gain31);
			pFile->WriteString(str);
		}

		diffrent_num += 3;
		str.Format(_T("file 2 has 3 more fps params :\n"));
		pFile->WriteString(str);
		
		str.Format(_T("mid_light_fps = %d\r\n"), fr_ex2->fps2);
		pFile->WriteString(str);

		str.Format(_T("mid_light_max_exptime = %d\r\n"), fr_ex2->max_exp_time2);
		pFile->WriteString(str);

		str.Format(_T("mid_light_to_low_light_gain = %d\r\n"), fr_ex2->gain22);
		pFile->WriteString(str);
	}
	else if ((isp_compare_1->p_Isp_exp.p_frame_rate.hight_light_frame_rate & 0xffffff00)
		&& !(isp_compare_2->p_Isp_exp.p_frame_rate.hight_light_frame_rate & 0xffffff00))
	{
		AK_ISP_FRAME_RATE_ATTR_EX* fr_ex1 = (AK_ISP_FRAME_RATE_ATTR_EX*)&isp_compare_1->p_Isp_exp.p_frame_rate;

		if (fr_ex1->fps1 != isp_compare_2->p_Isp_exp.p_frame_rate.hight_light_frame_rate)
		{
			diffrent_num++;
			str.Format(_T("hight_light_frame_rate = %d, %d\r\n"), fr_ex1->fps1, isp_compare_2->p_Isp_exp.p_frame_rate.hight_light_frame_rate);
			pFile->WriteString(str);
		}

		if (fr_ex1->max_exp_time1 != isp_compare_2->p_Isp_exp.p_frame_rate.hight_light_max_exp_time)
		{
			diffrent_num++;
			str.Format(_T("hight_light_max_exp_time = %d, %d\r\n"), fr_ex1->max_exp_time1, isp_compare_2->p_Isp_exp.p_frame_rate.hight_light_max_exp_time);
			pFile->WriteString(str);
		}

		if (fr_ex1->gain12 != isp_compare_2->p_Isp_exp.p_frame_rate.hight_light_to_low_light_gain)
		{
			diffrent_num++;
			str.Format(_T("hight_light_to_lower_light_gain = %d, %d\r\n"), fr_ex1->gain12, isp_compare_2->p_Isp_exp.p_frame_rate.hight_light_to_low_light_gain);
			pFile->WriteString(str);
		}

		if (fr_ex1->fps3 != isp_compare_2->p_Isp_exp.p_frame_rate.low_light_frame_rate)
		{
			diffrent_num++;
			str.Format(_T("low_light_frame_rate = %d, %d\r\n"), fr_ex1->fps3, isp_compare_2->p_Isp_exp.p_frame_rate.low_light_frame_rate);
			pFile->WriteString(str);
		}

		if (fr_ex1->max_exp_time3 != isp_compare_2->p_Isp_exp.p_frame_rate.low_light_max_exp_time)
		{
			diffrent_num++;
			str.Format(_T("low_light_max_exp_time = %d, %d\r\n"), fr_ex1->max_exp_time3, isp_compare_2->p_Isp_exp.p_frame_rate.low_light_max_exp_time);
			pFile->WriteString(str);
		}

		if (fr_ex1->gain31 != isp_compare_2->p_Isp_exp.p_frame_rate.low_light_to_hight_light_gain)
		{
			diffrent_num++;
			str.Format(_T("low_light_to_higher_light_gain = %d, %d\r\n"), fr_ex1->gain31, isp_compare_2->p_Isp_exp.p_frame_rate.low_light_to_hight_light_gain);
			pFile->WriteString(str);
		}

		diffrent_num += 3;
		str.Format(_T("file 1 has 3 more fps params :\n"));
		pFile->WriteString(str);
		
		str.Format(_T("mid_light_fps = %d\r\n"), fr_ex1->fps2);
		pFile->WriteString(str);

		str.Format(_T("mid_light_max_exptime = %d\r\n"), fr_ex1->max_exp_time2);
		pFile->WriteString(str);


		str.Format(_T("mid_light_to_low_light_gain = %d\r\n"), fr_ex1->gain22);
		pFile->WriteString(str);
	}
	else
	{
		if (isp_compare_1->p_Isp_exp.p_frame_rate.hight_light_frame_rate != isp_compare_2->p_Isp_exp.p_frame_rate.hight_light_frame_rate)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_exp.p_frame_rate.hight_light_frame_rate = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_frame_rate.hight_light_frame_rate, isp_compare_2->p_Isp_exp.p_frame_rate.hight_light_frame_rate);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_exp.p_frame_rate.hight_light_max_exp_time != isp_compare_2->p_Isp_exp.p_frame_rate.hight_light_max_exp_time)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_exp.p_frame_rate.hight_light_max_exp_time = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_frame_rate.hight_light_max_exp_time, isp_compare_2->p_Isp_exp.p_frame_rate.hight_light_max_exp_time);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_exp.p_frame_rate.hight_light_to_low_light_gain != isp_compare_2->p_Isp_exp.p_frame_rate.hight_light_to_low_light_gain)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_exp.p_frame_rate.hight_light_to_low_light_gain = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_frame_rate.hight_light_to_low_light_gain, isp_compare_2->p_Isp_exp.p_frame_rate.hight_light_to_low_light_gain);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_exp.p_frame_rate.low_light_frame_rate != isp_compare_2->p_Isp_exp.p_frame_rate.low_light_frame_rate)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_exp.p_frame_rate.low_light_frame_rate = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_frame_rate.low_light_frame_rate, isp_compare_2->p_Isp_exp.p_frame_rate.low_light_frame_rate);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_exp.p_frame_rate.low_light_max_exp_time != isp_compare_2->p_Isp_exp.p_frame_rate.low_light_max_exp_time)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_exp.p_frame_rate.low_light_max_exp_time = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_frame_rate.low_light_max_exp_time, isp_compare_2->p_Isp_exp.p_frame_rate.low_light_max_exp_time);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_exp.p_frame_rate.low_light_to_hight_light_gain != isp_compare_2->p_Isp_exp.p_frame_rate.low_light_to_hight_light_gain)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_exp.p_frame_rate.low_light_to_hight_light_gain = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_frame_rate.low_light_to_hight_light_gain, isp_compare_2->p_Isp_exp.p_frame_rate.low_light_to_hight_light_gain);
			pFile->WriteString(str);
		}
	}

	

	if (isp_compare_1->p_Isp_exp.p_ae.exp_time_max != isp_compare_2->p_Isp_exp.p_ae.exp_time_max )
	{
		diffrent_num++;
		str.Format(_T("p_Isp_exp.p_ae.exp_time_max  = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_ae.exp_time_max, isp_compare_2->p_Isp_exp.p_ae.exp_time_max );
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_exp.p_ae.exp_time_min != isp_compare_2->p_Isp_exp.p_ae.exp_time_min )
	{
		diffrent_num++;
		str.Format(_T("p_Isp_exp.p_ae.exp_time_min  = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_ae.exp_time_min, isp_compare_2->p_Isp_exp.p_ae.exp_time_min );
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_exp.p_ae.d_gain_max != isp_compare_2->p_Isp_exp.p_ae.d_gain_max )
	{
		diffrent_num++;
		str.Format(_T("p_Isp_exp.p_ae.d_gain_max  = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_ae.d_gain_max, isp_compare_2->p_Isp_exp.p_ae.d_gain_max );
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_exp.p_ae.d_gain_min != isp_compare_2->p_Isp_exp.p_ae.d_gain_min )
	{
		diffrent_num++;
		str.Format(_T("p_Isp_exp.p_ae.d_gain_min  = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_ae.d_gain_min, isp_compare_2->p_Isp_exp.p_ae.d_gain_min );
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_exp.p_ae.isp_d_gain_min != isp_compare_2->p_Isp_exp.p_ae.isp_d_gain_min )
	{
		diffrent_num++;
		str.Format(_T("p_Isp_exp.p_ae.isp_d_gain_min  = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_ae.isp_d_gain_min, isp_compare_2->p_Isp_exp.p_ae.isp_d_gain_min );
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_exp.p_ae.isp_d_gain_max != isp_compare_2->p_Isp_exp.p_ae.isp_d_gain_max )
	{
		diffrent_num++;
		str.Format(_T("p_Isp_exp.p_ae.isp_d_gain_max  = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_ae.isp_d_gain_max, isp_compare_2->p_Isp_exp.p_ae.isp_d_gain_max );
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_exp.p_ae.a_gain_max != isp_compare_2->p_Isp_exp.p_ae.a_gain_max )
	{
		diffrent_num++;
		str.Format(_T("p_Isp_exp.p_ae.a_gain_max  = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_ae.a_gain_max, isp_compare_2->p_Isp_exp.p_ae.a_gain_max );
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_exp.p_ae.a_gain_min != isp_compare_2->p_Isp_exp.p_ae.a_gain_min )
	{
		diffrent_num++;
		str.Format(_T("p_Isp_exp.p_ae.a_gain_min  = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_ae.a_gain_min, isp_compare_2->p_Isp_exp.p_ae.a_gain_min );
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_exp.p_ae.exp_step != isp_compare_2->p_Isp_exp.p_ae.exp_step )
	{
		diffrent_num++;
		str.Format(_T("p_Isp_exp.p_ae.exp_step  = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_ae.exp_step, isp_compare_2->p_Isp_exp.p_ae.exp_step );
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_exp.p_ae.exp_stable_range != isp_compare_2->p_Isp_exp.p_ae.exp_stable_range )
	{
		diffrent_num++;
		str.Format(_T("p_Isp_exp.p_ae.exp_stable_range  = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_ae.exp_stable_range, isp_compare_2->p_Isp_exp.p_ae.exp_stable_range );
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_exp.p_ae.target_lumiance != isp_compare_2->p_Isp_exp.p_ae.target_lumiance )
	{
		diffrent_num++;
		str.Format(_T("p_Isp_exp.p_ae.target_lumiance  = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_ae.target_lumiance, isp_compare_2->p_Isp_exp.p_ae.target_lumiance );
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_exp.p_ae.OE_suppress_en != isp_compare_2->p_Isp_exp.p_ae.OE_suppress_en )
	{
		diffrent_num++;
		str.Format(_T("p_Isp_exp.p_ae.OE_suppress_en  = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_ae.OE_suppress_en, isp_compare_2->p_Isp_exp.p_ae.OE_suppress_en );
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_exp.p_ae.OE_detect_scope != isp_compare_2->p_Isp_exp.p_ae.OE_detect_scope )
	{
		diffrent_num++;
		str.Format(_T("p_Isp_exp.p_ae.OE_detect_scope  = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_ae.OE_detect_scope, isp_compare_2->p_Isp_exp.p_ae.OE_detect_scope );
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_exp.p_ae.OE_rate_max != isp_compare_2->p_Isp_exp.p_ae.OE_rate_max )
	{
		diffrent_num++;
		str.Format(_T("p_Isp_exp.p_ae.OE_rate_max  = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_ae.OE_rate_max, isp_compare_2->p_Isp_exp.p_ae.OE_rate_max );
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_exp.p_ae.OE_rate_min != isp_compare_2->p_Isp_exp.p_ae.OE_rate_min )
	{
		diffrent_num++;
		str.Format(_T("p_Isp_exp.p_ae.OE_rate_min  = %d, %d\r\n"), isp_compare_1->p_Isp_exp.p_ae.OE_rate_min, isp_compare_2->p_Isp_exp.p_ae.OE_rate_min );
		pFile->WriteString(str);
	}

	for (i= 0; i < 16; i++)
	{
		if (isp_compare_1->p_Isp_exp.p_ae.hist_weight[i] != isp_compare_2->p_Isp_exp.p_ae.hist_weight[i] )
		{
			diffrent_num++;
			str.Format(_T("p_Isp_exp.p_ae.hist_weight[%d]  = %d, %d\r\n"), i, isp_compare_1->p_Isp_exp.p_ae.hist_weight[i], isp_compare_2->p_Isp_exp.p_ae.hist_weight[i] );
			pFile->WriteString(str);
		}
	}

	for (j = 0; j < 10; j ++)
	{
		for (i= 0; i < 2; i++)
		{
			if (isp_compare_1->p_Isp_exp.p_ae.envi_gain_range[j][i] != isp_compare_2->p_Isp_exp.p_ae.envi_gain_range[j][i] )
			{
				diffrent_num++;
				str.Format(_T("p_Isp_exp.p_ae.envi_gain_range[%d][%d]  = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_exp.p_ae.envi_gain_range[j][i], isp_compare_2->p_Isp_exp.p_ae.envi_gain_range[j][i] );
				pFile->WriteString(str);
			}
		}
	}
	
	//pFile->WriteString(_T("\r\n"));
}

void CCompareDlg::Write_misc_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) 
{
	CString str;
	UINT i = 0;

	if (isp_compare_1->p_Isp_misc.param_id != isp_compare_2->p_Isp_misc.param_id)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_misc.param_id = %d, %d\r\n"), isp_compare_1->p_Isp_misc.param_id, isp_compare_2->p_Isp_misc.param_id);
		pFile->WriteString(str);
	}
	
	if (isp_compare_1->p_Isp_misc.length != isp_compare_2->p_Isp_misc.length)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_misc.length = %d, %d\r\n"), isp_compare_1->p_Isp_misc.length, isp_compare_2->p_Isp_misc.length);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_misc.p_misc.hsyn_pol != isp_compare_2->p_Isp_misc.p_misc.hsyn_pol)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_misc.p_misc.hsyn_pol = %d, %d\r\n"), isp_compare_1->p_Isp_misc.p_misc.hsyn_pol, isp_compare_2->p_Isp_misc.p_misc.hsyn_pol);
		pFile->WriteString(str);
	}
	if (isp_compare_1->p_Isp_misc.p_misc.vsync_pol != isp_compare_2->p_Isp_misc.p_misc.vsync_pol)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_misc.p_misc.vsync_pol = %d, %d\r\n"), isp_compare_1->p_Isp_misc.p_misc.vsync_pol, isp_compare_2->p_Isp_misc.p_misc.vsync_pol);
		pFile->WriteString(str);
	}
	if (isp_compare_1->p_Isp_misc.p_misc.pclk_pol != isp_compare_2->p_Isp_misc.p_misc.pclk_pol)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_misc.p_misc.pclk_pol = %d, %d\r\n"), isp_compare_1->p_Isp_misc.p_misc.pclk_pol, isp_compare_2->p_Isp_misc.p_misc.pclk_pol);
		pFile->WriteString(str);
	}
	
	if (isp_compare_1->p_Isp_misc.p_misc.test_pattern_en != isp_compare_2->p_Isp_misc.p_misc.test_pattern_en)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_misc.p_misc.test_pattern_en = %d, %d\r\n"), isp_compare_1->p_Isp_misc.p_misc.test_pattern_en, isp_compare_2->p_Isp_misc.p_misc.test_pattern_en);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_misc.p_misc.test_pattern_cfg != isp_compare_2->p_Isp_misc.p_misc.test_pattern_cfg)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_misc.p_misc.test_pattern_cfg = %d, %d\r\n"), isp_compare_1->p_Isp_misc.p_misc.test_pattern_cfg, isp_compare_2->p_Isp_misc.p_misc.test_pattern_cfg);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_misc.p_misc.cfa_mode != isp_compare_2->p_Isp_misc.p_misc.cfa_mode)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_misc.p_misc.cfa_mode = %d, %d\r\n"), isp_compare_1->p_Isp_misc.p_misc.cfa_mode, isp_compare_2->p_Isp_misc.p_misc.cfa_mode);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_misc.p_misc.inputdataw != isp_compare_2->p_Isp_misc.p_misc.inputdataw)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_misc.p_misc.inputdataw = %d, %d\r\n"), isp_compare_1->p_Isp_misc.p_misc.inputdataw, isp_compare_2->p_Isp_misc.p_misc.inputdataw);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_misc.p_misc.one_line_cycle != isp_compare_2->p_Isp_misc.p_misc.one_line_cycle)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_misc.p_misc.one_line_cycle = %d, %d\r\n"), isp_compare_1->p_Isp_misc.p_misc.one_line_cycle, isp_compare_2->p_Isp_misc.p_misc.one_line_cycle);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_misc.p_misc.hblank_cycle != isp_compare_2->p_Isp_misc.p_misc.hblank_cycle)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_misc.p_misc.hblank_cycle = %d, %d\r\n"), isp_compare_1->p_Isp_misc.p_misc.hblank_cycle, isp_compare_2->p_Isp_misc.p_misc.hblank_cycle);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_misc.p_misc.frame_start_delay_en != isp_compare_2->p_Isp_misc.p_misc.frame_start_delay_en)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_misc.p_misc.frame_start_delay_en = %d, %d\r\n"), isp_compare_1->p_Isp_misc.p_misc.frame_start_delay_en, isp_compare_2->p_Isp_misc.p_misc.frame_start_delay_en);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_misc.p_misc.frame_start_delay_num != isp_compare_2->p_Isp_misc.p_misc.frame_start_delay_num)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_misc.p_misc.frame_start_delay_num = %d, %d\r\n"), isp_compare_1->p_Isp_misc.p_misc.frame_start_delay_num, isp_compare_2->p_Isp_misc.p_misc.frame_start_delay_num);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_misc.p_misc.flip_en != isp_compare_2->p_Isp_misc.p_misc.flip_en)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_misc.p_misc.flip_en = %d, %d\r\n"), isp_compare_1->p_Isp_misc.p_misc.flip_en, isp_compare_2->p_Isp_misc.p_misc.flip_en);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_misc.p_misc.mirror_en != isp_compare_2->p_Isp_misc.p_misc.mirror_en)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_misc.p_misc.mirror_en = %d, %d\r\n"), isp_compare_1->p_Isp_misc.p_misc.mirror_en, isp_compare_2->p_Isp_misc.p_misc.mirror_en);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_misc.p_misc.twoframe_merge_en != isp_compare_2->p_Isp_misc.p_misc.twoframe_merge_en)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_misc.p_misc.twoframe_merge_en = %d, %d\r\n"), isp_compare_1->p_Isp_misc.p_misc.twoframe_merge_en, isp_compare_2->p_Isp_misc.p_misc.twoframe_merge_en);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_misc.p_misc.mipi_line_end_sel != isp_compare_2->p_Isp_misc.p_misc.mipi_line_end_sel)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_misc.p_misc.mipi_line_end_sel = %d, %d\r\n"), isp_compare_1->p_Isp_misc.p_misc.mipi_line_end_sel, isp_compare_2->p_Isp_misc.p_misc.mipi_line_end_sel);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_misc.p_misc.mipi_line_end_cnt_en_cfg != isp_compare_2->p_Isp_misc.p_misc.mipi_line_end_cnt_en_cfg)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_misc.p_misc.mipi_line_end_cnt_en_cfg = %d, %d\r\n"), isp_compare_1->p_Isp_misc.p_misc.mipi_line_end_cnt_en_cfg, isp_compare_2->p_Isp_misc.p_misc.mipi_line_end_cnt_en_cfg);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_misc.p_misc.mipi_count_time != isp_compare_2->p_Isp_misc.p_misc.mipi_count_time)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_misc.p_misc.mipi_count_time = %d, %d\r\n"), isp_compare_1->p_Isp_misc.p_misc.mipi_count_time, isp_compare_2->p_Isp_misc.p_misc.mipi_count_time);
		pFile->WriteString(str);
	}

	//pFile->WriteString(_T("\r\n"));
}

void CCompareDlg::Write_hue_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) 
{
	CString str;
	UINT i = 0, j = 0;

	if (isp_compare_1->p_Isp_hue.param_id != isp_compare_2->p_Isp_hue.param_id)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_hue.param_id = %d, %d\r\n"), isp_compare_1->p_Isp_hue.param_id, isp_compare_2->p_Isp_wdr.param_id);
		pFile->WriteString(str);
	}
	
	if (isp_compare_1->p_Isp_hue.length != isp_compare_2->p_Isp_hue.length)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_hue.length = %d, %d\r\n"), isp_compare_1->p_Isp_hue.length, isp_compare_2->p_Isp_wdr.length);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_hue.p_hue.hue_mode != isp_compare_2->p_Isp_hue.p_hue.hue_mode)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_hue.p_hue.hue_mode = %d, %d\r\n"), isp_compare_1->p_Isp_hue.p_hue.hue_mode, isp_compare_2->p_Isp_hue.p_hue.hue_mode);
		pFile->WriteString(str);
	}

	if (isp_compare_1->p_Isp_hue.p_hue.manual_hue.hue_sat_en != isp_compare_2->p_Isp_hue.p_hue.manual_hue.hue_sat_en)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_hue.p_hue.manual_hue.hue_sat_en = %d, %d\r\n"), isp_compare_1->p_Isp_hue.p_hue.manual_hue.hue_sat_en, isp_compare_2->p_Isp_hue.p_hue.manual_hue.hue_sat_en);
		pFile->WriteString(str);
	}

	for (i = 0; i < 65; i++)
	{
		if (isp_compare_1->p_Isp_hue.p_hue.manual_hue.hue_lut_a[i]!= isp_compare_2->p_Isp_hue.p_hue.manual_hue.hue_lut_a[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_hue.p_hue.manual_hue.hue_lut_a[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_hue.p_hue.manual_hue.hue_lut_a[i], isp_compare_2->p_Isp_hue.p_hue.manual_hue.hue_lut_a[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_hue.p_hue.manual_hue.hue_lut_b[i]!= isp_compare_2->p_Isp_hue.p_hue.manual_hue.hue_lut_b[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_hue.p_hue.manual_hue.hue_lut_b[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_hue.p_hue.manual_hue.hue_lut_b[i], isp_compare_2->p_Isp_hue.p_hue.manual_hue.hue_lut_b[i]);
			pFile->WriteString(str);
		}

		if (isp_compare_1->p_Isp_hue.p_hue.manual_hue.hue_lut_s[i]!= isp_compare_2->p_Isp_hue.p_hue.manual_hue.hue_lut_s[i])
		{
			diffrent_num++;
			str.Format(_T("p_Isp_hue.p_hue.manual_hue.hue_lut_s[%d] = %d, %d\r\n"), i, isp_compare_1->p_Isp_hue.p_hue.manual_hue.hue_lut_s[i], isp_compare_2->p_Isp_hue.p_hue.manual_hue.hue_lut_s[i]);
			pFile->WriteString(str);
		}
	}


	for (j = 0; j < 4; j++)
	{
		if (isp_compare_1->p_Isp_hue.p_hue.hue[j].hue_sat_en != isp_compare_2->p_Isp_hue.p_hue.hue[j].hue_sat_en)
		{
			diffrent_num++;
			str.Format(_T("p_Isp_hue.p_hue.hue[%d].hue_sat_en = %d, %d\r\n"), j, isp_compare_1->p_Isp_hue.p_hue.hue[j].hue_sat_en, isp_compare_2->p_Isp_hue.p_hue.hue[j].hue_sat_en);
			pFile->WriteString(str);
		}		
		
		for (i = 0; i < 65; i++)
		{
			if (isp_compare_1->p_Isp_hue.p_hue.hue[j].hue_lut_a[i]!= isp_compare_2->p_Isp_hue.p_hue.hue[j].hue_lut_a[i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_hue.p_hue.hue[%d].hue_lut_a[%d] = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_hue.p_hue.hue[j].hue_lut_a[i], isp_compare_2->p_Isp_hue.p_hue.hue[j].hue_lut_a[i]);
				pFile->WriteString(str);
			}

			if (isp_compare_1->p_Isp_hue.p_hue.hue[j].hue_lut_b[i]!= isp_compare_2->p_Isp_hue.p_hue.hue[j].hue_lut_b[i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_hue.p_hue.hue[%d].hue_lut_b[%d] = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_hue.p_hue.hue[j].hue_lut_b[i], isp_compare_2->p_Isp_hue.p_hue.hue[j].hue_lut_b[i]);
				pFile->WriteString(str);
			}

			if (isp_compare_1->p_Isp_hue.p_hue.hue[j].hue_lut_s[i]!= isp_compare_2->p_Isp_hue.p_hue.hue[j].hue_lut_s[i])
			{
				diffrent_num++;
				str.Format(_T("p_Isp_hue.p_hue.hue[%d].hue_lut_s[%d] = %d, %d\r\n"), j, i, isp_compare_1->p_Isp_hue.p_hue.hue[j].hue_lut_s[i], isp_compare_2->p_Isp_hue.p_hue.hue[j].hue_lut_s[i]);
				pFile->WriteString(str);
			}
		}
		
	}
	
	//pFile->WriteString(_T("\r\n"));
}

void CCompareDlg::Write_sensor_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2, UINT sensor_1_num, UINT sensor_2_num) 
{
	CString str;
	UINT i = 0;

	if (isp_compare_1->p_Isp_sensor.param_id != isp_compare_2->p_Isp_sensor.param_id)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_sensor.param_id = %d, %d\r\n"), isp_compare_1->p_Isp_sensor.param_id, isp_compare_2->p_Isp_sensor.param_id);
		pFile->WriteString(str);
	}
	
	if (isp_compare_1->p_Isp_sensor.length != isp_compare_2->p_Isp_sensor.length)
	{
		diffrent_num++;
		str.Format(_T("p_Isp_sensor.length = %d, %d\r\n"), isp_compare_1->p_Isp_sensor.length, isp_compare_2->p_Isp_sensor.length);
		pFile->WriteString(str);
	}

	if (sensor_1_num != sensor_2_num)
	{
		int compare_num = 0;
		int no_compare_num = 0;
	
		diffrent_num++;
		str.Format(_T("the sensor num is diffrent = %d, %d\r\n"), sensor_1_num, sensor_2_num);
		pFile->WriteString(str);

		compare_num = sensor_1_num > sensor_2_num ? sensor_2_num : sensor_1_num;
		no_compare_num = sensor_2_num - sensor_1_num;

		for (i = 0; i < compare_num; i++)
		{
			if (isp_compare_1->p_Isp_sensor.p_sensor[i].sensor_addr != isp_compare_2->p_Isp_sensor.p_sensor[i].sensor_addr)
			{
				diffrent_num++;
				str.Format(_T("p_Isp_sensor.p_sensor[%d].sensor_addr = 0x%04x, 0x%04x\r\n"), i, isp_compare_1->p_Isp_sensor.p_sensor[i].sensor_addr, isp_compare_2->p_Isp_sensor.p_sensor[i].sensor_addr);
				pFile->WriteString(str);
			}
			
			if (isp_compare_1->p_Isp_sensor.p_sensor[i].sensor_value != isp_compare_2->p_Isp_sensor.p_sensor[i].sensor_value)
			{
				diffrent_num++;
				str.Format(_T("p_Isp_sensor.p_sensor[%d].sensor_value = 0x%04x, 0x%04x\r\n"), i, isp_compare_1->p_Isp_sensor.p_sensor[i].sensor_value, isp_compare_2->p_Isp_sensor.p_sensor[i].sensor_value);
				pFile->WriteString(str);
			}
		}

		if (0 != no_compare_num)
		{		
			int num = 0;
			
			if (no_compare_num > 0)
			{
				num = no_compare_num;
				str.Format(_T("File 2 has %d more sensor params!\n"), num);
				pFile->WriteString(str);

				for (i=0; i<num; i++)
				{
					str.Format(_T("p_Isp_sensor.p_sensor[%d].sensor_addr = 0x%04x, value = 0x%04x\r\n"), compare_num + i, isp_compare_2->p_Isp_sensor.p_sensor[compare_num + i].sensor_addr, isp_compare_2->p_Isp_sensor.p_sensor[compare_num + i].sensor_value);
					pFile->WriteString(str);
				}
			}
			else
			{
				num = 0 - no_compare_num;
				str.Format(_T("File 1 has %d more sensor params!\n"), num);
				pFile->WriteString(str);

				for (i=0; i<num; i++)
				{
					str.Format(_T("p_Isp_sensor.p_sensor[%d].sensor_addr = 0x%04x, value = 0x%04x\r\n"), compare_num + i, isp_compare_1->p_Isp_sensor.p_sensor[compare_num + i].sensor_addr, isp_compare_1->p_Isp_sensor.p_sensor[compare_num + i].sensor_value);
					pFile->WriteString(str);
				}
			}			

			diffrent_num += num;
		}
	}
	else
	{
		for (i = 0; i < sensor_1_num; i++)
		{
			if (isp_compare_1->p_Isp_sensor.p_sensor[i].sensor_addr != isp_compare_2->p_Isp_sensor.p_sensor[i].sensor_addr)
			{
				diffrent_num++;
				str.Format(_T("p_Isp_sensor.p_sensor[%d].sensor_addr = 0x%04x, 0x%04x\r\n"), i, isp_compare_1->p_Isp_sensor.p_sensor[i].sensor_addr, isp_compare_2->p_Isp_sensor.p_sensor[i].sensor_addr);
				pFile->WriteString(str);
			}
			
			if (isp_compare_1->p_Isp_sensor.p_sensor[i].sensor_value != isp_compare_2->p_Isp_sensor.p_sensor[i].sensor_value)
			{
				diffrent_num++;
				str.Format(_T("p_Isp_sensor.p_sensor[%d].sensor_value = 0x%04x, 0x%04x\r\n"), i, isp_compare_1->p_Isp_sensor.p_sensor[i].sensor_value, isp_compare_2->p_Isp_sensor.p_sensor[i].sensor_value);
				pFile->WriteString(str);
			}
		}
	}

	

	
	//pFile->WriteString(_T("\r\n"));
}




void CCompareDlg::Write_info(CStdioFile *pFile, AK_ISP_COMPARE_INFO *isp_compare_1, AK_ISP_COMPARE_INFO *isp_compare_2) 
{
	CString strsub[SUBFILE_NUM_MAX];
	CString str;
	int subfile_diff_num[SUBFILE_NUM_MAX] = {0};
	int i = 0;
	int compare_subfilecnt = 0;
	int no_compare_subfilecnt = 0;

	strsub[0].Format(_T("day"));
	strsub[1].Format(_T("night"));
	strsub[2].Format(_T("2"));
	strsub[3].Format(_T("3"));
	strsub[4].Format(_T("4"));
	
	compare_subfilecnt = isp_compare_1->subfilecnt > isp_compare_2->subfilecnt ? isp_compare_2->subfilecnt : isp_compare_1->subfilecnt;
	no_compare_subfilecnt = isp_compare_2->subfilecnt - isp_compare_1->subfilecnt;

	for (i=0; i<compare_subfilecnt; i++)
	{
		str.Format(_T("%s conf compare:\n"), strsub[i]);
		pFile->WriteString(str);

		if (isp_compare_1->headinfo[i].sensorId != isp_compare_2->headinfo[i].sensorId)
		{
			diffrent_num++;
			str.Format(_T("%s sensorId = 0x%04x, 0x%04x\r\n"), strsub[i], isp_compare_1->headinfo[i].sensorId, isp_compare_2->headinfo[i].sensorId);
			pFile->WriteString(str);
		}
		if (memcmp(isp_compare_1->headinfo[i].file_version, isp_compare_2->headinfo[i].file_version, 15) != 0)
		{
			diffrent_num++;
			str.Format(_T("%s version = %s, %s\r\n"), strsub[i], isp_compare_1->headinfo[i].file_version, isp_compare_2->headinfo[i].file_version);
			pFile->WriteString(str);
		}

		Write_BLC_info(pFile, &isp_compare_1->buf_isp[i], &isp_compare_2->buf_isp[i]);
		Write_lsc_info(pFile, &isp_compare_1->buf_isp[i], &isp_compare_2->buf_isp[i]);
		Write_raw_lut_info(pFile, &isp_compare_1->buf_isp[i], &isp_compare_2->buf_isp[i]);
		Write_nr_info(pFile, &isp_compare_1->buf_isp[i], &isp_compare_2->buf_isp[i]);
		Write_3dnr_info(pFile, &isp_compare_1->buf_isp[i], &isp_compare_2->buf_isp[i]);
		Write_gb_info(pFile, &isp_compare_1->buf_isp[i], &isp_compare_2->buf_isp[i]);
		Write_demo_info(pFile, &isp_compare_1->buf_isp[i], &isp_compare_2->buf_isp[i]);
		Write_gamma_info(pFile, &isp_compare_1->buf_isp[i], &isp_compare_2->buf_isp[i]);
		Write_ccm_info(pFile, &isp_compare_1->buf_isp[i], &isp_compare_2->buf_isp[i]);
		Write_fcs_info(pFile, &isp_compare_1->buf_isp[i], &isp_compare_2->buf_isp[i]);
		Write_wdr_info(pFile, &isp_compare_1->buf_isp[i], &isp_compare_2->buf_isp[i]);
		//Write_edge_info(pFile, &isp_compare_1->buf_isp[i], &isp_compare_2->buf_isp[i]); 
		Write_sharp_info(pFile, &isp_compare_1->buf_isp[i], &isp_compare_2->buf_isp[i]);
		Write_saturation_info(pFile, &isp_compare_1->buf_isp[i], &isp_compare_2->buf_isp[i]); 
		Write_contrast_info(pFile, &isp_compare_1->buf_isp[i], &isp_compare_2->buf_isp[i]);
		Write_rgb2yuv_info(pFile, &isp_compare_1->buf_isp[i], &isp_compare_2->buf_isp[i]);
		Write_effect_info(pFile, &isp_compare_1->buf_isp[i], &isp_compare_2->buf_isp[i]);
		Write_dpc_info(pFile, &isp_compare_1->buf_isp[i], &isp_compare_2->buf_isp[i]); 
		Write_weight_info(pFile, &isp_compare_1->buf_isp[i], &isp_compare_2->buf_isp[i]);
		Write_af_info(pFile, &isp_compare_1->buf_isp[i], &isp_compare_2->buf_isp[i]);
		Write_wb_info(pFile, &isp_compare_1->buf_isp[i], &isp_compare_2->buf_isp[i]);
		Write_exp_info(pFile, &isp_compare_1->buf_isp[i], &isp_compare_2->buf_isp[i]);
		Write_misc_info(pFile, &isp_compare_1->buf_isp[i], &isp_compare_2->buf_isp[i]);
		Write_y_gamma_info(pFile, &isp_compare_1->buf_isp[i], &isp_compare_2->buf_isp[i]);
		Write_hue_info(pFile, &isp_compare_1->buf_isp[i], &isp_compare_2->buf_isp[i]);
		Write_sensor_info(pFile, &isp_compare_1->buf_isp[i], &isp_compare_2->buf_isp[i],isp_compare_1->sensor_num[i], isp_compare_2->sensor_num[i]);

		subfile_diff_num[i] = diffrent_num;

		if (0 == subfile_diff_num[i])
		{
			str.Format(_T("%s conf is same\n"), strsub[i]);
			pFile->WriteString(str);
		}
		
		diffrent_num = 0;

		str.Format(_T("\r\n"));
		pFile->WriteString(str);
		
	}

	for (i=0; i<compare_subfilecnt; i++)
		diffrent_num += subfile_diff_num[i];

	if (0 != no_compare_subfilecnt)
	{		
		if (no_compare_subfilecnt > 0)
		{
			str.Format(_T("File 2 has %d more subfile! Can't compare!\n"), no_compare_subfilecnt);
		}
		else
		{
			str.Format(_T("File 1 has %d more subfile! Can't compare!\n"), 0 - no_compare_subfilecnt);
		}

		diffrent_num++;

		pFile->WriteString(str);
	}


}
