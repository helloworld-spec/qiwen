#include "stdafx.h"
#include "ServerInfo.h"

WCHAR * strFunction[COMM_TYPE_CNT][10] = 
			{{L"录像", L"对讲", L"拍照", L"移动侦测", L"停止录像", NULL, NULL, NULL, NULL, NULL},
			 {L"图像设置", L"ISP参数设置", L"隐私遮挡", L"缩放设置", L"音量设置", L"云台", NULL, NULL, NULL, NULL},
			 {L"录像文件获取", L"服务基本信息获取", L"隐私遮挡区域获取", L"移动侦测区域获取", L"图像设置获取", L"录像文件列表获取", L"ISP参数获取", NULL, NULL, NULL}};

WCHAR * strError[ERROR_MAX] = {L"未知原因错误", L"无存储设备", L"无采集设备", L"无输出设备", L"SD卡挂载失败",
							   L"SD卡只读", L"文件打开错误", L"文件写错误", L"文件读错误"};

int GetStringFromRetInfo(RETINFO & retInfo, CString & strServerInfo)
{
	strServerInfo.Empty();

	if ((retInfo.nCommandType >= COMM_TYPE_CNT) || (retInfo.nSubCommandType >= 10))
		return -1;

	WCHAR * pstrFun = strFunction[retInfo.nCommandType][retInfo.nSubCommandType];
	if (pstrFun) {
		strServerInfo.Format(L"%s", pstrFun);
	}

	if (retInfo.retType == RET_WARN_CODE) {
		strServerInfo.Append(L"[警告]:");
	}else if (retInfo.retType == RET_SUCCESS) {
		strServerInfo.Append(L":处理成功!");
		return 0;
	}else if (retInfo.retType == RET_ERROR_CODE) {
		strServerInfo.Append(L"[错误]:");
	}

	if (retInfo.retType == RET_ERROR_CODE) {
		if (retInfo.nCode >= ERROR_MAX)
			return -1;

		strServerInfo.Append(strError[retInfo.nCode]);
	}

	return 0;
}