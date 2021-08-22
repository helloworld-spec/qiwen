// Update.cpp: implementation of the CUpdate class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "burntool.h"
#include "Update.h"
#include "anyka_types.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define FILE_HEAD_INFO			"ANYKA_V2"
#define ADJUST_512(x)			(((x-1)/512+1)*512)
#define UPDATE_FOLDER			_T("Update_Files")
#define WRITE_BUF_LEN			(512*1024)
#define IMAGE_RES_NAME			_T("ImageRes") 
#define SPOTLIGHT_NAME			_T("PROG") 
#define CHECK_EXPORT_ID			"ANYKA_ID"

extern CBurnToolApp theApp;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUpdate::CUpdate()
{
	m_nand_file_offset = 0;
}

CUpdate::~CUpdate()
{
}

T_U16 g_xor1 = 0;
T_U16 g_xor2 = 0;
//Caclutate check sum
UINT CUpdate::Upd_file_CheckSum(PBYTE data, UINT len)
{
    UINT sum = 0;
    UINT i;

	g_xor2 = ((T_U16)data[0]<<8)|(T_U16)data[1];

	sum += g_xor1 ^ g_xor2;

	for(i = 0; i < len-2;)
    {
		//T_U16 xor1 = (T_U16)data[i];
		//T_U16 xor2 = (T_U16)data[i+2];
		g_xor1 = ((T_U16)data[i]<<8)|(T_U16)data[i+1];
		g_xor2 = ((T_U16)data[i+2]<<8)|(T_U16)data[i+3];
		
		sum += g_xor1 ^ g_xor2;

		g_xor1 = ((T_U16)data[i+2]<<8)|(T_U16)data[i+3];

		i +=2;
    }
	
    return sum;
}

//Caclutate check sum
UINT CUpdate::Cal_CheckSum(PBYTE data, UINT len)
{
    UINT sum = 0;
    UINT i;

     for(i = 0; i < len-2;)
    {
		T_U16 xor1 = (T_U16)data[i];
		T_U16 xor2 = (T_U16)data[i+2];
		
		sum += xor1 ^ xor2;
		i +=2;
    }

    return sum;
}
//read file to buffer
BOOL CUpdate::ReadBuffer(HANDLE hFile, LPVOID buf, UINT begin, UINT size)
{
	BOOL ret;
	DWORD read_len;

	if (INVALID_HANDLE_VALUE == hFile || NULL == buf)
	{
		return FALSE;
	}
	
	memset(buf, 0, size);
	SetFilePointer(hFile, begin, NULL, FILE_BEGIN);
	ret = ReadFile(hFile, buf, size, &read_len, NULL);	
	if(!ret || read_len != size)
	{
		return FALSE;
	}

	return TRUE;
}

//write buffer to file
BOOL CUpdate::WriteBuffer(HANDLE hFile, LPVOID buf, UINT begin, UINT size)
{
	BOOL ret;
	DWORD write_len;

	if (INVALID_HANDLE_VALUE == hFile || NULL == buf)
	{
		return FALSE;
	}

	SetFilePointer(hFile, begin, NULL, FILE_BEGIN);
	ret = WriteFile(hFile, buf, size, &write_len, NULL);	
	if(!ret || write_len != size)
	{
		return FALSE;
	}

	FlushFileBuffers(hFile);

	return TRUE;
}

// delelte files of update folder
BOOL CUpdate::Del_UpdateFiles(LPCTSTR file_path)
{	
	CString strPCPath = file_path;
	strPCPath = theApp.ConvertAbsolutePath(strPCPath);
	BOOL bFind;
	CFileFind* fd = new CFileFind();
	
	strPCPath += _T("\\*.*");
	bFind = fd->FindFile(strPCPath);
	while(bFind)
	{
		bFind = fd->FindNextFile();	
		if(!fd->IsDots())
		{
			strPCPath = fd->GetFilePath();
			if(!fd->IsDirectory())
			{
				if(!DeleteFile(strPCPath))
				{
					delete fd;
					fd = NULL;
					return FALSE;
				}
			}
			else
			{
				Del_UpdateFiles(strPCPath);
			}
		}
		RemoveDirectory(strPCPath);
	}

		
	
	delete fd;
	fd = NULL;
	return TRUE;
}

// create update folder
BOOL CUpdate::CreateUpdateDir()
{
	WIN32_FIND_DATA fd;
	CString strPCUpdatePath = theApp.ConvertAbsolutePath(UPDATE_FOLDER);	

	if(INVALID_HANDLE_VALUE != FindFirstFile(strPCUpdatePath, &fd))
	{
		if(FILE_ATTRIBUTE_DIRECTORY == GetFileAttributes(LPCTSTR(strPCUpdatePath)))
		{
			if(!Del_UpdateFiles(strPCUpdatePath))
			{
				return FALSE;
			}
		}
	}
	else
	{
		if(!CreateDirectory(strPCUpdatePath, NULL))
		{
			return FALSE;
		}
	}

	m_update_folder = strPCUpdatePath;
	m_update_folder += _T("\\");
	
	return TRUE;
}

// Unpacket update file
BOOL CUpdate::UnpacketFile(HANDLE hSourceFile, LPCTSTR file_path, UINT file_offset, UINT file_length)
{
	HANDLE hDstFile;
	BOOL ret;
	UINT i=0;
	BYTE* dataBuffer;
//	WIN32_FIND_DATA fd;
	TCHAR pathTemp[MAX_PATH+1] = {0};
	
	if(INVALID_HANDLE_VALUE == hSourceFile || NULL == file_path)
	{
		return FALSE;
	}

	// create path
	while('\0'!=file_path[i])
	{
		while('\\' != file_path[i])
		{
			if(_T('\0')==file_path[i])
			{
				break;
			}
			i++;
		}
		if('\0' !=file_path[i])
		{	
			wcsncpy(pathTemp, file_path, i);

		//	if(INVALID_HANDLE_VALUE == FindFirstFile(pathTemp, &fd))
		//	{
				CreateDirectory(pathTemp,NULL);
		//	}
			i++;
		}
	}
	
	//create file 
	hDstFile = CreateFile(file_path, GENERIC_WRITE, FILE_SHARE_WRITE, 
		NULL, CREATE_ALWAYS, 0, NULL);
	if(INVALID_HANDLE_VALUE == hDstFile)
	{
		return FALSE;
	}
	
	dataBuffer = new BYTE[WRITE_BUF_LEN];
	if (NULL == dataBuffer)
	{
		CloseHandle(hDstFile);
		return FALSE;
	}
	memset(dataBuffer, 0, WRITE_BUF_LEN);
	
	//SetFilePointer
	SetFilePointer(hSourceFile, file_offset, NULL, FILE_BEGIN);

	DWORD read_len, write_len;
	UINT nSpare = file_length % WRITE_BUF_LEN;
	UINT nTimes = file_length / WRITE_BUF_LEN;

	while(nTimes >0)
	{
		ret = ReadFile(hSourceFile, dataBuffer, WRITE_BUF_LEN, &read_len, NULL);
		if(!ret || read_len != WRITE_BUF_LEN)
		{
			delete[] dataBuffer;
			dataBuffer = NULL;
			CloseHandle(hDstFile);
			return FALSE;	
		}
		
		ret = WriteFile(hDstFile, dataBuffer, WRITE_BUF_LEN, &write_len, NULL);
		if (!ret || write_len != WRITE_BUF_LEN)
		{
			delete[] dataBuffer;
			dataBuffer = NULL;
			CloseHandle(hDstFile);
			return FALSE;
		}
		FlushFileBuffers(hDstFile);
		nTimes--;	
	}
	//ReadFile
	ret = ReadFile(hSourceFile, dataBuffer, nSpare, &read_len, NULL);
	if(!ret || read_len != nSpare)
	{
		delete[] dataBuffer;
		dataBuffer = NULL;
		CloseHandle(hDstFile);
		return FALSE;	
	}
	//WriteFile
	ret = WriteFile(hDstFile, dataBuffer, nSpare, &write_len, NULL);
	if (!ret || write_len != nSpare)
	{
		delete[] dataBuffer;
		dataBuffer = NULL;
		CloseHandle(hDstFile);
		return FALSE;
	}
	FlushFileBuffers(hDstFile);//FlushFileBuffers
	
	delete[] dataBuffer;
	dataBuffer = NULL;
	
	CloseHandle(hDstFile);//CloseHandle
	return TRUE;
}


BOOL CUpdate::Unpacket_Fs_Img_File(HANDLE hSourceFile, T_UPDATE_FILE_HEAD *pFileHead, T_DOWNLOAD_MTD* pDownload_fs_img)
{
	T_FS_IMG_FILE_INFO *pFileInfo = NULL;
	UINT data_size, i;
	CString strFilePath;
	
	//read nand files info
	pFileInfo = new T_FS_IMG_FILE_INFO[pFileHead->fs_img_count];
	data_size = sizeof(T_FS_IMG_FILE_INFO) * pFileHead->fs_img_count;
	if (NULL == pFileInfo)
	{
		return FALSE;
	}
	
	//ReadBuffer
	if (!ReadBuffer(hSourceFile,pFileInfo,pFileHead->fs_img_offset,data_size))
	{
		delete[] pFileInfo;
		pFileInfo = NULL;
		return FALSE;
	}
	
	USES_CONVERSION;
	
	// unpacket nand files
	for(i=0; i<pFileHead->fs_img_count; i++)
	{
		if(pFileInfo[i].check_sum != Cal_CheckSum((PBYTE)(&pFileInfo[i]), sizeof(pFileInfo[i])-sizeof(pFileInfo[i].check_sum)))
		{
			delete[] pFileInfo;
			pFileInfo = NULL;
			return FALSE;
		}
		
		//get download to nand info
		pDownload_fs_img[i].bCompare = (BOOL)pFileInfo[i].bCompare;

		wcsncpy(pDownload_fs_img[i].disk_name, A2T(pFileInfo[i].file_name), 8);
		
		strFilePath = m_update_folder + pDownload_fs_img[i].disk_name;
		strFilePath += ".bin";
		wcscpy(pDownload_fs_img[i].pc_path, strFilePath);
		
		//UnpacketFile
		if(!UnpacketFile(hSourceFile, pDownload_fs_img[i].pc_path, 
			pFileInfo[i].file_offset, pFileInfo[i].file_length))
			
		{
			delete[] pFileInfo;
			pFileInfo = NULL;
			return FALSE;
		}
	}
	
	delete[] pFileInfo;
	pFileInfo = NULL;
	return TRUE;
}

BOOL CUpdate::Unpacket_Spi_Img_File(HANDLE hSourceFile, T_UPDATE_FILE_HEAD *pFileHead, T_DOWNLOAD_SPIFLASH_IMG* pDownload_spi_img)
{
	T_SPI_IMG_FILE_INFO *pFileInfo = NULL;
	UINT data_size, i;
	CString strFilePath;
	
	//read nand files info
	pFileInfo = new T_SPI_IMG_FILE_INFO[pFileHead->spi_img_file_count];
	data_size = sizeof(T_SPI_IMG_FILE_INFO) * pFileHead->spi_img_file_count;
	if (NULL == pFileInfo)
	{
		return FALSE;
	}
	
	//ReadBuffer
	if (!ReadBuffer(hSourceFile,pFileInfo,pFileHead->spi_img_file_offset,data_size))
	{
		delete[] pFileInfo;
		pFileInfo = NULL;
		return FALSE;
	}
	
	USES_CONVERSION;
	
	// unpacket nand files
	for(i=0; i<pFileHead->spi_img_file_count; i++)
	{
		if(pFileInfo[i].check_sum != Cal_CheckSum((PBYTE)(&pFileInfo[i]), sizeof(pFileInfo[i])-sizeof(pFileInfo[i].check_sum)))
		{
			delete[] pFileInfo;
			pFileInfo = NULL;
			return FALSE;
		}
		
		//get download to nand info
		pDownload_spi_img[i].bCompare = (BOOL)pFileInfo[i].bCompare;
		
		strFilePath = m_update_folder + pFileInfo[i].file_name;
		strFilePath += ".bin";
		wcscpy(pDownload_spi_img[i].pc_path, strFilePath);
		
		//UnpacketFile
		if(!UnpacketFile(hSourceFile, pDownload_spi_img[i].pc_path, 
			pFileInfo[i].file_offset, pFileInfo[i].file_length))
			
		{
			delete[] pFileInfo;
			pFileInfo = NULL;
			return FALSE;
		}
	}
	
	delete[] pFileInfo;
	pFileInfo = NULL;
	return TRUE;
	
}


//Unpacket nand file
BOOL CUpdate::UnpacketNandFile(HANDLE hSourceFile, T_UPDATE_FILE_HEAD *pFileHead, T_DOWNLOAD_NAND* pDownloadNand)
{	
	T_FILE_INFO* pFileInfo = NULL;
	UINT data_size, i;
	CString strFilePath;

	//read nand files info
	pFileInfo = new T_FILE_INFO[pFileHead->bin_file_count];
	data_size = sizeof(T_FILE_INFO) * pFileHead->bin_file_count;
	if (NULL == pFileInfo)
	{
		return FALSE;
	}

	//ReadBuffer
	if (!ReadBuffer(hSourceFile,pFileInfo,pFileHead->bin_file_offset,data_size))
	{
		delete[] pFileInfo;
		pFileInfo = NULL;
		return FALSE;
	}

	USES_CONVERSION;

	// unpacket nand files
	for(i=0; i<pFileHead->bin_file_count; i++)
	{
		if(pFileInfo[i].check_sum != Cal_CheckSum((PBYTE)(&pFileInfo[i]), sizeof(pFileInfo[i])-sizeof(pFileInfo[i].check_sum)))
		{
			delete[] pFileInfo;
			pFileInfo = NULL;
			return FALSE;
		}
		
		//get download to nand info
		pDownloadNand[i].bCompare = (BOOL)pFileInfo[i].bCompare;
		pDownloadNand[i].bBackup = (BOOL)pFileInfo[i].bBack;
		pDownloadNand[i].ld_addr = pFileInfo[i].ld_addr;

		wcsncpy(pDownloadNand[i].file_name, A2T(pFileInfo[i].file_name), 8);
		
		strFilePath = m_update_folder + pDownloadNand[i].file_name;
		strFilePath += ".bin";
		wcscpy(pDownloadNand[i].pc_path, strFilePath);
	
		//UnpacketFile
		if(!UnpacketFile(hSourceFile, pDownloadNand[i].pc_path, 
			                pFileInfo[i].file_offset, pFileInfo[i].file_length))

		{
			delete[] pFileInfo;
			pFileInfo = NULL;
			return FALSE;
		}
	}

	delete[] pFileInfo;
	pFileInfo = NULL;
	return TRUE;
}

//unpacket udisk files
BOOL CUpdate::UnpacketUdiskFile(HANDLE hSourceFile, T_UPDATE_FILE_HEAD *pFileHead, T_DOWNLOAD_UDISK * pDownloadUdisk)
{
	UINT i, data_size, file_offset, file_len;
	TCHAR file_path[MAX_PATH];
	T_UDISK_UPDATE_FILE_INFO * pUdiskFileInfo = NULL;
	T_UDISK_INFO *pUdiskInfo = NULL;
	CString strUpdatePath;

	if(INVALID_HANDLE_VALUE == hSourceFile || NULL == pFileHead)
	{
		return FALSE;
	}

	pUdiskInfo = new T_UDISK_INFO[pFileHead->udisk_info_count];//pUdiskInfo分配
	data_size = pFileHead->udisk_info_count * sizeof(T_UDISK_INFO);
	if (NULL == pUdiskInfo)
	{
		return FALSE;
	}
	//ReadBuffer
	if(!ReadBuffer(hSourceFile, pUdiskInfo, pFileHead->udisk_info_offset, data_size))
	{
		delete[] pUdiskInfo;
		pUdiskInfo = NULL;
		return FALSE;
	}

	USES_CONVERSION;

	for(i=0; i<pFileHead->udisk_info_count; i++)
	{
		memset(file_path, 0, MAX_PATH);
		
		if(pUdiskInfo[i].check_sum != Cal_CheckSum(PBYTE(pUdiskInfo + i), sizeof(pUdiskInfo[i]) - sizeof(pUdiskInfo[i].check_sum)))
		{
			delete[] pUdiskInfo;
			pUdiskInfo = NULL;
			return FALSE;
		}

		pDownloadUdisk[i].bCompare = pUdiskInfo[i].bCompare;
		wcscpy(pDownloadUdisk[i].udisk_path, A2T(pUdiskInfo[i].udisk_path));//udisk_path

		wcscpy(file_path, A2T(pUdiskInfo[i].pc_path));//pc_path
		strUpdatePath = m_update_folder + file_path;
		wcscpy(file_path, strUpdatePath);
		wcscpy(pDownloadUdisk[i].pc_path, file_path);	
	}

	delete[] pUdiskInfo;//释放
	pUdiskInfo = NULL;

	// read udisk files info
	pUdiskFileInfo = new T_UDISK_UPDATE_FILE_INFO[pFileHead->udisk_file_count];
	data_size = pFileHead->udisk_file_count * sizeof(T_UDISK_UPDATE_FILE_INFO);
	if (NULL == pUdiskFileInfo)
	{
		return FALSE;
	}
	//ReadBuffer
	if (!ReadBuffer(hSourceFile, pUdiskFileInfo, pFileHead->udisk_file_info_offset, data_size))
	{	
		delete[] pUdiskFileInfo;
		pUdiskFileInfo = NULL;
		return FALSE;
	}

	//unpacket udisk files
	for(i=0; i<pFileHead->udisk_file_count; i++)
	{
		memset(file_path, 0, MAX_PATH);
		
		if(pUdiskFileInfo[i].check_sum != Cal_CheckSum(PBYTE(&pUdiskFileInfo[i]),
										sizeof(pUdiskFileInfo[i])-sizeof(pUdiskFileInfo[i].check_sum)))
		{
			delete[] pUdiskFileInfo;
			pUdiskFileInfo = NULL;
			return FALSE;
		}
		wcsncpy(file_path, A2T(pUdiskFileInfo[i].pc_file_path), MAX_PATH);
		strUpdatePath = m_update_folder + file_path;
		wcscpy(file_path, strUpdatePath);
		
		file_offset = pUdiskFileInfo[i].file_offset;
		file_len = pUdiskFileInfo[i].file_length;
		
		if(!UnpacketFile(hSourceFile, file_path, file_offset, file_len))//UnpacketFile
		{
			delete[] pUdiskFileInfo;
			pUdiskFileInfo = NULL;
			return FALSE;
		}
	}
	delete[] pUdiskFileInfo;
	pUdiskFileInfo = NULL;
	return TRUE;

}

//get file count
UINT CUpdate::GetFileCnt(CString path, UINT* pFileCnt)
{
	BOOL bFind;
	CString strPath = path;
	CFileFind *fd = new CFileFind();

	bFind = fd->FindFile(strPath);

	while(bFind)
	{
		bFind = fd->FindNextFile();//FindNextFile
		strPath = fd->GetFilePath();//GetFilePath
		if(!fd->IsDots())
		{
			if(fd->IsDirectory())//IsDirectory
			{		
				strPath +=_T("\\*.*");	
				GetFileCnt(strPath, pFileCnt);
			}
			else
			{
				(*pFileCnt)++;
			}
		}
	}

	delete fd;
	fd = NULL;
	return *pFileCnt;
}

//get download to udisk file count 
UINT CUpdate::GetUdiskFileCount(CConfig *pCFG)
{
	UINT i, nFileCnt=0;
	
	for (i=0; i<pCFG->download_udisk_count; i++)
	{
		CString strPath = theApp.ConvertAbsolutePath(pCFG->download_udisk_data[i].pc_path);
		GetFileCnt(strPath, &nFileCnt);		//GetFileCnt
	}
	return nFileCnt;
}

//set downdload to udisk file info
void CUpdate::SetUdiskFileInfo(CString strPath, T_UDISK_UPDATE_FILE_INFO *pUdisFileInfo, UINT *pFileCnt)
{
	CString strFilePath = theApp.ConvertAbsolutePath(strPath);
//	static i = 0;
	BOOL bFind;
	CFileFind fd;

	USES_CONVERSION;
	
    bFind = fd.FindFile(strFilePath);
	while(bFind)
	{
		bFind = fd.FindNextFile();//FindNextFile
		strFilePath = fd.GetFilePath();//GetFilePath
		if (!fd.IsDots())
		{	
			if (fd.IsDirectory())
			{	
				strFilePath += _T("\\*.*");
				SetUdiskFileInfo(strFilePath, pUdisFileInfo, pFileCnt);//SetUdiskFileInfo
			}
			else
			{
				//get file info
				strcpy(pUdisFileInfo[*pFileCnt].pc_file_path, T2A((LPCTSTR)strFilePath)); 
		
				pUdisFileInfo[*pFileCnt].file_length = fd.GetLength();//GetLength

				if(0 == *pFileCnt)
				{
					pUdisFileInfo[*pFileCnt].file_offset = m_spi_img_file_offset;
				}
				else
				{
					pUdisFileInfo[*pFileCnt].file_offset = pUdisFileInfo[*pFileCnt-1].file_offset 
						                                 + ADJUST_512(pUdisFileInfo[*pFileCnt-1].file_length);
				}

				(*pFileCnt)++;
			}
		}
	}
}

//store file
BOOL CUpdate::StoreFile(HANDLE hPacketFile, TCHAR* file_path, UINT file_offset, UINT file_len)
{
	HANDLE hDstFile = NULL;
	BOOL ret;
	PBYTE pBuf = NULL;

	if(NULL == file_path || NULL == hPacketFile)
	{
		return FALSE;
	}

	SetFilePointer(hPacketFile, file_offset, NULL, CFile::begin);//SetFilePointer
	hDstFile = CreateFile(theApp.ConvertAbsolutePath(file_path), GENERIC_READ , FILE_SHARE_READ , NULL , 
		    		OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);
	if(INVALID_HANDLE_VALUE == hDstFile)
	{
		return FALSE;
	}

	DWORD read_len, write_len;
	UINT nTimes = file_len / WRITE_BUF_LEN;
	UINT nSpare = file_len % WRITE_BUF_LEN;

	pBuf = new BYTE[WRITE_BUF_LEN];
    if(NULL == pBuf)
    {
		CloseHandle(hDstFile);//CloseHandle
		return FALSE;
    }
	memset(pBuf, 0, WRITE_BUF_LEN);
	
	while(nTimes >0)
	{
		ret = ReadFile(hDstFile, pBuf, WRITE_BUF_LEN, &read_len, NULL);//ReadFile
		if(!ret || read_len != WRITE_BUF_LEN)
		{
			delete[] pBuf;
			pBuf = NULL;
			CloseHandle(hDstFile);
			return FALSE;
		}

		ret = WriteFile(hPacketFile, pBuf, WRITE_BUF_LEN, &write_len,NULL);//WriteFile

		if (!ret || write_len != WRITE_BUF_LEN)
		{
			delete[] pBuf;
			pBuf = NULL;
			CloseHandle(hDstFile);
			return FALSE;
		}

		FlushFileBuffers(hPacketFile);
		nTimes--;
	}

	ret = ReadFile(hDstFile, pBuf, nSpare, &read_len, NULL);//ReadFile

	if(!ret || read_len != nSpare)
	{
		delete[] pBuf;
		pBuf = NULL;
		CloseHandle(hDstFile);
		return FALSE;
	}
	
	ret = WriteFile(hPacketFile, pBuf, nSpare,&write_len,NULL);//WriteFile
	
	if (!ret || write_len != nSpare)
	{
		delete[] pBuf;
		pBuf = NULL;
		CloseHandle(hDstFile);
		return FALSE;
	}
	
	FlushFileBuffers(hPacketFile);//FlushFileBuffers

	CloseHandle(hDstFile);//CloseHandle
	delete[] pBuf;
	pBuf = NULL;

	return TRUE;
}

//strore producer
BOOL CUpdate::StoreProducer(HANDLE hPacketFile, CConfig *pCFG, T_UPDATE_FILE_HEAD *pFileHead)
{
	pFileHead->producer_offset = pFileHead->spi_img_file_offset + ADJUST_512(pFileHead->spi_img_file_count * sizeof(T_DOWNLOAD_SPIFLASH_IMG));

	HANDLE hProducerFile = CreateFile(theApp.ConvertAbsolutePath(pCFG->path_producer), GENERIC_READ, FILE_SHARE_READ, 
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);//CreateFile

	if(hProducerFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwSize = GetFileSize(hProducerFile, NULL);
		if(dwSize != 0xFFFFFFFF)
		{
			pFileHead->producer_size = dwSize;//producer_size
		}
		else
		{
			CloseHandle(hProducerFile);//CloseHandle
			return FALSE;
		}
		CloseHandle(hProducerFile);//CloseHandle
	}
	else
	{
		CString str;
		str.Format(_T("打开文件:%s失败"),theApp.ConvertAbsolutePath(pCFG->path_producer));
		AfxMessageBox(str);
	}

	if(!StoreFile(hPacketFile, pCFG->path_producer, pFileHead->producer_offset, pFileHead->producer_size))
	{
		return FALSE;
	}

	return TRUE;
	
}

// store bios
BOOL CUpdate::StoreBios(HANDLE hPacketFile, CConfig *pCFG, T_UPDATE_FILE_HEAD *pFileHead)
{
	pFileHead->bios_offset = pFileHead->producer_offset + ADJUST_512(pFileHead->producer_size);;
	
	HANDLE hBiosFile = CreateFile(theApp.ConvertAbsolutePath(pCFG->path_nandboot), GENERIC_READ, FILE_SHARE_READ, 
		NULL, OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL, NULL);//CreateFile

	if(hBiosFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwSize = GetFileSize(hBiosFile, NULL);//GetFileSize
		if(dwSize != 0xFFFFFFFF)
		{
			pFileHead->bios_size = dwSize;//bios_size
		}
		else
		{
			CloseHandle(hBiosFile);//CloseHandle
			return FALSE;
		}
		CloseHandle(hBiosFile);//CloseHandle
	}
	else
	{
		CString str;
		str.Format(_T("打开文件:%s失败"),theApp.ConvertAbsolutePath(pCFG->path_nandboot));
		AfxMessageBox(str);
	}

	if(!StoreFile(hPacketFile, pCFG->path_nandboot, pFileHead->bios_offset, pFileHead->bios_size))
	{
		return FALSE;
	}

	return TRUE;
}

//store nand files info and files
BOOL CUpdate::StoreNandFile(HANDLE hPacketFile, CConfig *pCFG, T_UPDATE_FILE_HEAD *pFileHead)
{
    T_FILE_INFO *pfInfo = NULL;
    UINT file_offset = 0;
	UINT file_info_len;
    HANDLE hFile;
    UINT i;
    DWORD dwSize;

	if(NULL == hPacketFile)
    {
        return FALSE;
    }

    pfInfo = new T_FILE_INFO[pFileHead->bin_file_count];
    if(NULL == pfInfo)
    {
        return FALSE;
    }
	
	file_info_len = pFileHead->bin_file_count * sizeof(T_FILE_INFO);
	memset(pfInfo, 0, file_info_len);

    USES_CONVERSION;

    file_offset = pFileHead->bios_offset + ADJUST_512(pFileHead->bios_size);

    for(i = 0; i < pFileHead->bin_file_count; i++)
    {
        pfInfo[i].bCompare = (BYTE)pCFG->download_nand_data[i].bCompare;//bCompare
		pfInfo[i].bBack = (BYTE)pCFG->download_nand_data[i].bBackup;   //bBackup
		pfInfo[i].ld_addr = pCFG->download_nand_data[i].ld_addr;//ld_addr
        
        hFile = CreateFile(theApp.ConvertAbsolutePath(pCFG->download_nand_data[i].pc_path), GENERIC_READ , FILE_SHARE_READ , NULL , 
		    		OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);
		if(hFile != INVALID_HANDLE_VALUE)
		{
			dwSize = GetFileSize(hFile, NULL);
			if(dwSize != 0xFFFFFFFF)
			{
				pfInfo[i].file_length = dwSize;        // 
                pfInfo[i].file_offset = file_offset;//
                file_offset += ADJUST_512(dwSize);//
		    }
            else
            {
                CloseHandle(hFile);
                delete[] pfInfo;
				pfInfo = NULL;
                return FALSE;
            }
        }
        else
        {		
			CString str;
			str.Format(_T("打开文件:%s失败"),theApp.ConvertAbsolutePath(pCFG->download_nand_data[i].pc_path));
			AfxMessageBox(str);
			
            delete[] pfInfo;
            pfInfo = NULL;
            return FALSE;
        }

        CloseHandle(hFile);

        //file name
		/*
		if(0 == wcscmp(pCFG->download_nand_data[i].file_name, IMAGE_RES_NAME))
		{
			pFileHead->config_tool[0][0] = pfInfo[i].file_offset;//
			pFileHead->config_tool[0][1] = dwSize;//
		}

		if(0 == wcscmp(pCFG->download_nand_data[i].file_name, SPOTLIGHT_NAME))
		{
			pFileHead->config_tool[1][0] = pfInfo[i].file_offset;//
			pFileHead->config_tool[1][1] = dwSize;//
		}
		*/
		
        strncpy(pfInfo[i].file_name, T2A(pCFG->download_nand_data[i].file_name), 8);
		pfInfo[i].check_sum = Cal_CheckSum((PBYTE)(pfInfo+i), sizeof(T_FILE_INFO)-sizeof(UINT));
    }

	//get all nand file offset
	m_nand_file_offset = file_offset;

    //write file info
	if (!WriteBuffer(hPacketFile, pfInfo, pFileHead->bin_file_offset, file_info_len))
	{
		delete[] pfInfo;
		pfInfo = NULL;
		return FALSE;
	}

    //write all nand file
    for(i = 0; i < pFileHead->bin_file_count; i++)
    {
		//StoreFile
		if(!StoreFile(hPacketFile, pCFG->download_nand_data[i].pc_path, pfInfo[i].file_offset, pfInfo[i].file_length))
		{
			delete[] pfInfo;
			pfInfo = NULL;
			return FALSE;
		}
	}
    delete[] pfInfo;
	pfInfo = NULL;
	
    return TRUE;
}

//write download_to_udisk info and files
BOOL CUpdate::Store_fs_img_File(HANDLE hPacketFile, CConfig *pCFG, T_UPDATE_FILE_HEAD *pFileHead)
{
	T_FS_IMG_FILE_INFO *pfInfo = NULL;
    UINT file_offset = 0;
	UINT file_info_len;
    HANDLE hFile;
    UINT i;
    DWORD dwSize;

	if(NULL == hPacketFile)
    {
        return FALSE;
    }

    pfInfo = new T_FS_IMG_FILE_INFO[pFileHead->fs_img_count];
    if(NULL == pfInfo)
    {
        return FALSE;
    }
	
	file_info_len = pFileHead->fs_img_count * sizeof(T_FS_IMG_FILE_INFO);
	memset(pfInfo, 0, file_info_len);

    USES_CONVERSION;

    file_offset = m_nand_file_offset;

    for(i = 0; i < pFileHead->fs_img_count; i++)
    {
        pfInfo[i].bCompare = (BYTE)pCFG->download_mtd_data[i].bCompare;//bCompare

        hFile = CreateFile(theApp.ConvertAbsolutePath(pCFG->download_mtd_data[i].pc_path), GENERIC_READ , FILE_SHARE_READ , NULL , 
		    		OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);
		if(hFile != INVALID_HANDLE_VALUE)
		{
			dwSize = GetFileSize(hFile, NULL);
			if(dwSize != 0xFFFFFFFF)
			{
				pfInfo[i].file_length = dwSize;        // 
                pfInfo[i].file_offset = file_offset;//
                file_offset += ADJUST_512(dwSize);//
		    }
            else
            {
                CloseHandle(hFile);
                delete[] pfInfo;
				pfInfo = NULL;
                return FALSE;
            }
        }
        else
        {		
			CString str;
			str.Format(_T("打开文件:%s失败"),theApp.ConvertAbsolutePath(pCFG->download_mtd_data[i].pc_path));
			AfxMessageBox(str);
			
            delete[] pfInfo;
            pfInfo = NULL;
            return FALSE;
        }

        CloseHandle(hFile);
		
        strncpy(pfInfo[i].file_name, T2A(pCFG->download_mtd_data[i].disk_name), 8);
		pfInfo[i].check_sum = Cal_CheckSum((PBYTE)(pfInfo+i), sizeof(T_FS_IMG_FILE_INFO)-sizeof(UINT));
    }

	//get all nand file offset
	m_fs_img_file_offset = file_offset;

    //write file info
	if (!WriteBuffer(hPacketFile, pfInfo, pFileHead->fs_img_offset, file_info_len))
	{
		delete[] pfInfo;
		pfInfo = NULL;
		return FALSE;
	}

    //write all nand file
    for(i = 0; i < pFileHead->fs_img_count; i++)
    {
		//StoreFile
		if(!StoreFile(hPacketFile, pCFG->download_mtd_data[i].pc_path, pfInfo[i].file_offset, pfInfo[i].file_length))
		{
			delete[] pfInfo;
			pfInfo = NULL;
			return FALSE;
		}
	}
    delete[] pfInfo;
	pfInfo = NULL;

	return TRUE;

}

//write download_to_udisk info and files
BOOL CUpdate::Store_spi_img_File(HANDLE hPacketFile, CConfig *pCFG, T_UPDATE_FILE_HEAD *pFileHead)
{
	T_SPI_IMG_FILE_INFO *pfInfo = NULL;
    UINT file_offset = 0;
	UINT file_info_len;
    HANDLE hFile;
    UINT i;
    DWORD dwSize;
	
	if(NULL == hPacketFile)
    {
        return FALSE;
    }
	
    pfInfo = new T_SPI_IMG_FILE_INFO[pFileHead->spi_img_file_count];
    if(NULL == pfInfo)
    {
        return FALSE;
    }
	
	file_info_len = pFileHead->spi_img_file_count * sizeof(T_SPI_IMG_FILE_INFO);
	memset(pfInfo, 0, file_info_len);
	
    USES_CONVERSION;
	
    file_offset = m_fs_img_file_offset;
	
    for(i = 0; i < pFileHead->spi_img_file_count; i++)
    {
        pfInfo[i].bCompare = (BYTE)pCFG->download_spi_img_data[i].bCompare;//bCompare
		
        hFile = CreateFile(theApp.ConvertAbsolutePath(pCFG->download_spi_img_data[i].pc_path), GENERIC_READ , FILE_SHARE_READ , NULL , 
			OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);
		if(hFile != INVALID_HANDLE_VALUE)
		{
			dwSize = GetFileSize(hFile, NULL);
			if(dwSize != 0xFFFFFFFF)
			{
				pfInfo[i].file_length = dwSize;        // 
                pfInfo[i].file_offset = file_offset;//
                file_offset += ADJUST_512(dwSize);//
			}
            else
            {
                CloseHandle(hFile);
                delete[] pfInfo;
				pfInfo = NULL;
                return FALSE;
            }
        }
        else
        {		
			CString str;
			str.Format(_T("打开文件:%s失败"),theApp.ConvertAbsolutePath(pCFG->download_mtd_data[i].pc_path));
			AfxMessageBox(str);
			
            delete[] pfInfo;
            pfInfo = NULL;
            return FALSE;
        }
		
        CloseHandle(hFile);
		
        strncpy(pfInfo[i].file_name, "spidata.bin", 16);
		pfInfo[i].check_sum = Cal_CheckSum((PBYTE)(pfInfo+i), sizeof(T_FS_IMG_FILE_INFO)-sizeof(UINT));
    }
	
	//get all nand file offset
	m_spi_img_file_offset = file_offset;
	
    //write file info
	if (!WriteBuffer(hPacketFile, pfInfo, pFileHead->spi_img_file_offset, file_info_len))
	{
		delete[] pfInfo;
		pfInfo = NULL;
		return FALSE;
	}
	
    //write all nand file
    for(i = 0; i < pFileHead->spi_img_file_count; i++)
    {
		//StoreFile
		if(!StoreFile(hPacketFile, pCFG->download_spi_img_data[i].pc_path, pfInfo[i].file_offset, pfInfo[i].file_length))
		{
			delete[] pfInfo;
			pfInfo = NULL;
			return FALSE;
		}
	}
    delete[] pfInfo;
	pfInfo = NULL;
	return TRUE;
}


//write download_to_udisk info and files
BOOL CUpdate::StoreUdiskFile(HANDLE hPacketFile, CConfig *pCFG, T_UPDATE_FILE_HEAD *pFileHead)
{
	UINT i, dwSize, nUdiskFileTotal = 0;
	T_UDISK_INFO *pUdiskInfo = NULL;            //download_to_udisk info
	T_UDISK_UPDATE_FILE_INFO* pUdiskFileInfo = NULL;   //download_to_udisk file info
	T_DOWNLOAD_UDISK* pudisk = pCFG->download_udisk_data;

	if(NULL == hPacketFile)
	{
		return FALSE;
	}

	pUdiskFileInfo = new T_UDISK_UPDATE_FILE_INFO[pFileHead->udisk_file_count];//pUdiskFileInfo
	if(NULL == pUdiskFileInfo)
	{
		return FALSE;
	}
	memset(pUdiskFileInfo, 0, pFileHead->udisk_file_count * sizeof(T_UDISK_UPDATE_FILE_INFO));

	USES_CONVERSION;
	
	//fill struct pUdiskFileInfo to store udisk file
	for (i=0; i<pFileHead->udisk_info_count; i++)
	{
		SetUdiskFileInfo(pudisk[i].pc_path, pUdiskFileInfo, &nUdiskFileTotal);//SetUdiskFileInfo
	
		if ((pFileHead->udisk_info_count != 0) && (0 == nUdiskFileTotal))
		{
			delete[] pUdiskFileInfo;
			pUdiskFileInfo = NULL;
	//		AfxMessageBox(_T("没找到下载的优盘文件"));
            return FALSE;
		}
	}

	//write all file of download_to_udisk 
	for (i=0; i<pFileHead->udisk_file_count; i++)
	{
		//StoreFile
		if(!StoreFile(hPacketFile, A2T(pUdiskFileInfo[i].pc_file_path), pUdiskFileInfo[i].file_offset, pUdiskFileInfo[i].file_length))
        {
			delete[] pUdiskFileInfo;
			pUdiskFileInfo = NULL;
            return FALSE;
		}
    }

	pUdiskInfo = new T_UDISK_INFO[pFileHead->udisk_info_count];//pUdiskInfo
	if(NULL == pUdiskInfo)
	{
		delete[] pUdiskFileInfo;
		pUdiskFileInfo = NULL;
		return FALSE;
	}
	memset(pUdiskInfo, 0, pFileHead->udisk_info_count * sizeof(T_UDISK_INFO));

	
	UINT nindex, j, nFileCnt, nFileTotal=0;

	//fill struct pUdiskInfo and modify pc_file_path of struct pUdiskFileInfo to store
	for (i=0; i<pFileHead->udisk_info_count; i++)
	{
		CString strUdiskInfo, strFront, strtemp;
		BOOL bSame = FALSE;
		
		strFront.Format(_T("UDISK%d_"), i+1);

		CFileFind fd;
		if (fd.FindFile(theApp.ConvertAbsolutePath(pudisk[i].pc_path)))//FindFile
		{
			fd.FindNextFile();
			strUdiskInfo = fd.GetFilePath();
		}

		nFileCnt = 0;
		nFileCnt = GetFileCnt(strUdiskInfo, &nFileCnt);//GetFileCnt
			
		nindex = strUdiskInfo.ReverseFind('\\');
		strUdiskInfo = strUdiskInfo.Mid(nindex+1);
		
		if(i>0)
		{
			for(UINT k=0; k<i; k++)
			{
				strtemp = A2T(pUdiskInfo[i-1].pc_path);
				if(0 == strtemp.Compare(strUdiskInfo))
				{
					strUdiskInfo = strFront + strUdiskInfo;
					bSame = TRUE;
					break;
				}		
			}		
		}
	
		pUdiskInfo[i].bCompare = pudisk[i].bCompare;//bCompare
		strcpy(pUdiskInfo[i].udisk_path, T2A(pudisk[i].udisk_path));//udisk_path
		strcpy(pUdiskInfo[i].pc_path, T2A((LPCTSTR)strUdiskInfo)/*, strUdiskInfo.GetLength()*/);
		pUdiskInfo[i].check_sum = Cal_CheckSum(PBYTE(pUdiskInfo + i), sizeof(T_UDISK_INFO)-sizeof(UINT));
			
		for (j = 0; j<nFileCnt; j++)
		{	
			CString strUdiskFile;
			strUdiskFile = A2T(pUdiskFileInfo[j + nFileTotal].pc_file_path);//strUdiskFile
			strUdiskFile = strUdiskFile.Mid(nindex+1);
			
			if (bSame)
			{
				strUdiskFile = strFront + strUdiskFile;//strUdiskFile
			}
			
		
			memset(pUdiskFileInfo[j + nFileTotal].pc_file_path, 0, MAX_PATH);
			strcpy(pUdiskFileInfo[j + nFileTotal].pc_file_path, T2A((LPCTSTR)strUdiskFile)/*, strUdiskFile.GetLength()*/);

			pUdiskFileInfo[j + nFileTotal].check_sum = 0;
			pUdiskFileInfo[j + nFileTotal].check_sum	= Cal_CheckSum((PBYTE)(pUdiskFileInfo + j + nFileTotal), sizeof(T_UDISK_UPDATE_FILE_INFO)-sizeof(UINT));//
		}
		
		nFileTotal += nFileCnt;	
	}


	//write download_to_udisk info

	dwSize = pFileHead->udisk_info_count * sizeof(T_UDISK_INFO);//
	if(!WriteBuffer(hPacketFile, pUdiskInfo, pFileHead->udisk_info_offset, dwSize))
	{
		delete[] pUdiskInfo;
		pUdiskInfo = NULL;
		return FALSE;
	}

	//write download_to_udisk files info

	dwSize = pFileHead->udisk_file_count * sizeof(T_UDISK_UPDATE_FILE_INFO);//dwSize
	if(!WriteBuffer(hPacketFile, pUdiskFileInfo, pFileHead->udisk_file_info_offset, dwSize))
	{
		delete[] pUdiskInfo;
		delete[] pUdiskFileInfo;
		pUdiskInfo = NULL;
		pUdiskFileInfo = NULL;
		return FALSE;
	}


	return TRUE;	
}

BOOL CUpdate::ExportUpdateFile(CConfig *pCFG, CString file_path, CString strCheck)
{
	UINT temp_4 = 0;

    if(NULL == pCFG || file_path.IsEmpty())
    {
        return FALSE;
    }
	
	HANDLE hPacketFile = NULL;
	T_UPDATE_FILE_HEAD file_head;
    UINT tmp, data_size;

	hPacketFile = CreateFile(file_path, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_WRITE|FILE_SHARE_READ, 
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (NULL == hPacketFile)
	{
		return FALSE;
	}

    //prepare for file head struct
    memset(&file_head, 0, sizeof(T_UPDATE_FILE_HEAD));
    memcpy(file_head.head_info, FILE_HEAD_INFO, 8);

	file_head.usb_mode = (UINT)pCFG->bUsb2;//bUsb2
	file_head.chip_type = pCFG->chip_type;//chip_type
	file_head.burn_mode = pCFG->burn_mode;//burn_mode
	file_head.planform_tpye = pCFG->planform_tpye;//planform_tpye
	
    //format list
    file_head.partition_count = pCFG->partition_count;
    file_head.partition_offset = sizeof(T_UPDATE_FILE_HEAD);
	tmp = file_head.partition_count * sizeof(T_ALL_PARTITION_INFO);

    //nand list
	file_head.spinand_count = pCFG->spi_nandflash_parameter_count;
    file_head.spinand_offset = ADJUST_512(tmp);
    tmp = file_head.spinand_count * sizeof(T_NAND_PHY_INFO_TRANSC);

	//spi list
    file_head.spi_count = pCFG->spiflash_parameter_count;
    file_head.spi_offset = file_head.spinand_offset + ADJUST_512(tmp);
    tmp = file_head.spi_count * sizeof(T_SFLASH_PHY_INFO_TRANSC);

	//udisk info
	file_head.udisk_info_count = pCFG->download_udisk_count;
	file_head.udisk_info_offset = file_head.spi_offset + ADJUST_512(tmp);
	tmp = file_head.udisk_info_count * sizeof(T_UDISK_INFO);
	
	//udisk file
	file_head.udisk_file_count = GetUdiskFileCount(pCFG);
	file_head.udisk_file_info_offset = file_head.udisk_info_offset + ADJUST_512(tmp);
	tmp = file_head.udisk_file_count * sizeof(T_UDISK_UPDATE_FILE_INFO);

	//fs img list
    file_head.fs_img_count = pCFG->download_mtd_count;
    file_head.fs_img_offset = file_head.udisk_file_info_offset + ADJUST_512(tmp);
	tmp = file_head.fs_img_count * sizeof(T_DOWNLOAD_MTD);

	//bin list
    file_head.bin_file_count = pCFG->download_nand_count;
    file_head.bin_file_offset = file_head.fs_img_offset + ADJUST_512(tmp);
	tmp = file_head.bin_file_count * sizeof(T_FILE_INFO);

	//spi img list
    file_head.spi_img_file_count = pCFG->download_spi_img_count;
    file_head.spi_img_file_offset = file_head.bin_file_offset + ADJUST_512(tmp);
	tmp = file_head.spi_img_file_count * sizeof(T_DOWNLOAD_SPIFLASH_IMG);
	
    //store partition list
	data_size = file_head.partition_count * sizeof(T_ALL_PARTITION_INFO);
	//WriteBuffer
	if (!WriteBuffer(hPacketFile, pCFG->partition_data, file_head.partition_offset, data_size))
	{
		CloseHandle(hPacketFile);
		return FALSE;
	}

    //store nand list
	data_size = file_head.spinand_count * sizeof(T_NAND_PHY_INFO_TRANSC);
	if (!WriteBuffer(hPacketFile, pCFG->spi_nandflash_parameter, file_head.spinand_offset, data_size))
	{
		CloseHandle(hPacketFile);
		return FALSE;
	}

	//store spi list
	data_size = file_head.spi_count * sizeof(T_SFLASH_PHY_INFO_TRANSC);
	if (!WriteBuffer(hPacketFile, pCFG->spiflash_parameter, file_head.spi_offset, data_size))
	{
		CloseHandle(hPacketFile);
		return FALSE;
	}
	
	//store file
	if(!StoreProducer(hPacketFile, pCFG, &file_head) 
		|| !StoreBios(hPacketFile, pCFG, &file_head)
		|| !StoreNandFile(hPacketFile, pCFG, &file_head)
		|| !Store_fs_img_File(hPacketFile, pCFG, &file_head)
		|| !Store_spi_img_File(hPacketFile, pCFG, &file_head)
		|| !StoreUdiskFile(hPacketFile, pCFG, &file_head))
	{
		CloseHandle(hPacketFile);
		return FALSE;
	}
	

	//check sum
    file_head.check_sum = Cal_CheckSum((PBYTE)(&file_head), sizeof(file_head)-sizeof(file_head.check_sum));

	//store file head
	if (!WriteBuffer(hPacketFile, &file_head, 0, sizeof(T_UPDATE_FILE_HEAD)))
	{
		CloseHandle(hPacketFile);
		return FALSE;
	}

	//write user check string
	if (!WriteCheckExport(hPacketFile, strCheck))
	{
		CloseHandle(hPacketFile);
		return FALSE;
	}
	CloseHandle(hPacketFile);

    return TRUE;
}

BOOL CUpdate::ImportUpdateFile(CConfig *pCFG, CString file_path)
{
	HANDLE hSourceFile;
	UINT data_size;
	T_UPDATE_FILE_HEAD file_head;
	T_ALL_PARTITION_INFO *pPartitionData = NULL;
	T_SPIFLASH_PARTION_INFO* spi_pFormatData = NULL;
	T_VOLUME_LABLE* pVolumeLable = NULL;
	T_DOWNLOAD_NAND* pDownloadNand = NULL;
	T_NAND_PHY_INFO_TRANSC *pspiNandInfo = NULL;
	T_SFLASH_PHY_INFO_TRANSC *pspiInfo = NULL;
	T_DOWNLOAD_MTD *pDownload_fs_img = NULL;
	T_DOWNLOAD_SPIFLASH_IMG *pDownload_spi_img = NULL;
	
	T_DOWNLOAD_UDISK* pDownloadUdisk = NULL;
	T_UDISK_INFO *pUdiskInfo = NULL;
    BYTE *pCheckExport = NULL;
	UINT temp_4 = 0;


	if(pCFG == NULL || file_path.IsEmpty())
	{
		return FALSE;
	}

	hSourceFile = CreateFile(file_path, GENERIC_READ, FILE_SHARE_READ, 
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);//CreateFile
	if(INVALID_HANDLE_VALUE == hSourceFile)
	{
		return FALSE;
	}
	
	//read file_head and check
	if(!ReadBuffer(hSourceFile, &file_head, 0, sizeof(T_UPDATE_FILE_HEAD)))
	{
		CloseHandle(hSourceFile);
		return FALSE;
	}

	USES_CONVERSION;

	if((wcsncmp(_T(FILE_HEAD_INFO), A2T((LPSTR)file_head.head_info), 8) != 0))
	{	
		CloseHandle(hSourceFile);
		return FALSE;
	}
	
	if(file_head.check_sum != Cal_CheckSum((PBYTE)(&file_head), sizeof(file_head)-sizeof(file_head.check_sum)))//Cal_CheckSum
	{
		CloseHandle(hSourceFile);
		return FALSE;
	}

	//create empty update folder
	if(!CreateUpdateDir())//CreateUpdateDir
	{
		CloseHandle(hSourceFile);
		return FALSE;
	}

	pCFG->bUsb2 = file_head.usb_mode;//bUsb2
	pCFG->chip_type = (E_CHIP_TYPE)file_head.chip_type;//chip_type
    pCFG->burn_mode = file_head.burn_mode;//burn_mode
	pCFG->planform_tpye = file_head.planform_tpye;//planform_tpye

	CString strPath = m_update_folder + _T("producer.bin");
	if(!UnpacketFile(hSourceFile, strPath, file_head.producer_offset, file_head.producer_size))
	{
		CloseHandle(hSourceFile);
		return FALSE;
	}

	memset(pCFG->path_producer, 0, MAX_PATH+1);
	wcscpy(pCFG->path_producer, strPath);//produce

	strPath = m_update_folder + _T("BOOT.bin");
	if(!UnpacketFile(hSourceFile, strPath, file_head.bios_offset, file_head.bios_size))
	{
		CloseHandle(hSourceFile);
		return FALSE;
	}
	memset(pCFG->path_nandboot, 0, MAX_PATH+1);
	wcscpy(pCFG->path_nandboot, strPath);//boot path

	//download_to_nandflash struct
	pDownloadNand = new T_DOWNLOAD_NAND[file_head.bin_file_count];//分配
	if(NULL == pDownloadNand )
	{
		CloseHandle(hSourceFile);
		return FALSE;
	}	
	memset(pDownloadNand, 0, file_head.bin_file_count * sizeof(T_DOWNLOAD_NAND));
	//unpacket download_to_nandflash files and fill pDownloadNand
	if(!UnpacketNandFile(hSourceFile, &file_head, pDownloadNand))
	{
		goto FAIL;
	}
	
	//fs img
	pDownload_fs_img = new T_DOWNLOAD_MTD[file_head.fs_img_count];//分配
	if(NULL == pDownloadNand )
	{
		CloseHandle(hSourceFile);
		return FALSE;
	}	
	memset(pDownload_fs_img, 0, file_head.fs_img_count * sizeof(T_DOWNLOAD_MTD));
	if(!Unpacket_Fs_Img_File(hSourceFile, &file_head, pDownload_fs_img))
	{
		goto FAIL;
	}


	//spi img
	pDownload_spi_img = new T_DOWNLOAD_SPIFLASH_IMG[file_head.spi_img_file_count];//分配
	if(NULL == pDownloadNand )
	{
		CloseHandle(hSourceFile);
		return FALSE;
	}	
	memset(pDownload_spi_img, 0, file_head.spi_img_file_count * sizeof(T_DOWNLOAD_SPIFLASH_IMG));
	if(!Unpacket_Spi_Img_File(hSourceFile, &file_head, pDownload_spi_img))
	{
		goto FAIL;
	}

	//download_to_udisk struct
	pDownloadUdisk = new T_DOWNLOAD_UDISK[file_head.udisk_info_count];
	if (NULL == pDownloadUdisk)
	{
		goto FAIL;
	}
	memset(pDownloadUdisk, 0, file_head.udisk_info_count * sizeof(T_DOWNLOAD_UDISK));

	//unpacket Udisk files	
	if (!UnpacketUdiskFile(hSourceFile, &file_head, pDownloadUdisk))//UnpacketUdiskFile
	{
		goto FAIL;
	}

	//read FORMAT info
	pPartitionData = new T_ALL_PARTITION_INFO[file_head.partition_count];//分配
	if (NULL == pPartitionData)
	{
		goto FAIL;
	}

	data_size = file_head.partition_count * sizeof(T_ALL_PARTITION_INFO);
	//由于pFormatData是需要的，所以二个都要进行读取
	if(!ReadBuffer(hSourceFile, pPartitionData, file_head.partition_offset, data_size))
	{
		goto FAIL;
	}

	//导入时去掉nand参数导入
	//read nand param info
	pspiNandInfo = new T_NAND_PHY_INFO_TRANSC[file_head.spinand_count];
	if (NULL ==pspiNandInfo)
	{
		goto FAIL;
	}
	data_size = sizeof(T_NAND_PHY_INFO_TRANSC) * file_head.spinand_count;

	if (!ReadBuffer(hSourceFile, pspiNandInfo, file_head.spinand_offset, data_size)) 
	{
		goto FAIL;
	}

	//导入时去掉spi参数导入
	//read spi param info
	pspiInfo = new T_SFLASH_PHY_INFO_TRANSC[file_head.spi_count];
	if (NULL ==pspiInfo)
	{
		goto FAIL;
	}

	data_size = sizeof(T_SFLASH_PHY_INFO_TRANSC) * file_head.spi_count;
	if (!ReadBuffer(hSourceFile, pspiInfo, file_head.spi_offset, data_size)) 
	{
		goto FAIL;
	}

    pCheckExport = new BYTE[64];
	if (NULL ==pCheckExport)
	{
		goto FAIL;
	}
    memset(pCheckExport, 0, 64);
    if (!UnpacketCheckExport(hSourceFile, pCheckExport))
    {
        delete [] pCheckExport;//释放
        pCheckExport = NULL;
    }
	CloseHandle(hSourceFile);


	//write config 
	pCFG->partition_count = file_head.partition_count;
	if(pCFG->partition_data != NULL)
	{
		delete [] pCFG->partition_data;//释放
	}
	pCFG->partition_data = pPartitionData;


    //#if 0导入时去掉nand参数导入
	pCFG->spi_nandflash_parameter_count = file_head.spinand_count;
	if(pCFG->spi_nandflash_parameter != NULL)
	{
		delete [] pCFG->spi_nandflash_parameter;
	}
	pCFG->spi_nandflash_parameter = pspiNandInfo;

	// 导入时去掉spi参数导入
	pCFG->spiflash_parameter_count = file_head.spi_count;
	if(pCFG->spiflash_parameter != NULL)
	{
		delete [] pCFG->spiflash_parameter;
	}
	pCFG->spiflash_parameter = pspiInfo;

	//bin
	pCFG->download_nand_count = file_head.bin_file_count;
	if (pCFG->download_nand_data != NULL)
	{
		delete [] pCFG->download_nand_data;//释放
	}
	pCFG->download_nand_data = pDownloadNand;
	
	//fs img
	pCFG->download_mtd_count = file_head.fs_img_count;
	if (pCFG->download_mtd_data != NULL)
	{
		delete [] pCFG->download_mtd_data;//释放
	}
	pCFG->download_mtd_data = pDownload_fs_img;

	//spi img
	pCFG->download_spi_img_count = file_head.spi_img_file_count;
	if (pCFG->download_spi_img_data != NULL)
	{
		delete [] pCFG->download_spi_img_data;//释放
	}
	pCFG->download_spi_img_data = pDownload_spi_img;


	pCFG->download_udisk_count = file_head.udisk_info_count;
	if(pCFG->download_udisk_data != NULL)
	{
		delete [] pCFG->download_udisk_data;//释放
	}
	pCFG->download_udisk_data = pDownloadUdisk;

	if(pCFG->pCheckExport != NULL)
	{
		delete [] pCFG->pCheckExport;//释放
	}
	pCFG->pCheckExport = pCheckExport;

    pCFG->WriteConfig(theApp.m_config_file[theApp.m_config_index]);

	return true;

//release memory and exit
FAIL:
	if(pDownloadNand != NULL)
	{
		delete [] pDownloadNand;
		pDownloadNand = NULL;
	}

	if(pDownload_fs_img != NULL)
	{
		delete [] pDownload_fs_img;
		pDownload_fs_img = NULL;
	}

	if(pDownload_spi_img != NULL)
	{
		delete [] pDownload_spi_img;
		pDownload_spi_img = NULL;
	}

	if(pPartitionData != NULL)
	{
		delete [] pPartitionData;
		pPartitionData = NULL;
	}

    //导入时去掉nand参数导入
	if(pspiNandInfo != NULL)
	{
		delete pspiNandInfo;
		pspiNandInfo = NULL;
	}

    //导入时去掉nand参数导入
	if(pspiInfo != NULL)
	{
		delete pspiInfo;
		pspiInfo = NULL;
	}
	
	if(pDownloadUdisk != NULL)
	{
		delete [] pDownloadUdisk;
		pDownloadUdisk = NULL;
	}
	CloseHandle(hSourceFile);
	return FALSE;
}

BOOL CUpdate::WriteCheckExport(HANDLE hFile, CString strCheck)
{
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return FALSE;
	}
	
	UINT nLen = 0;
	BYTE strBuf[64] = {0};
	BOOL ret;
	DWORD write_len;
	UINT chksum = 0;
    UINT IDLen = strlen(CHECK_EXPORT_ID);
	BYTE *strBuf_temp = NULL;
	UINT strBuf_temp_len = 4*1024;
	UINT file_len = 0;
	UINT read_len = 0;
	UINT read_addr = 0;

	USES_CONVERSION;
    
    if (IDLen > 64)
    {
        return FALSE;
    }

	nLen = strCheck.GetLength();//GetLength
    if ((IDLen + nLen + 4) > 64)
    {
        nLen = 64 - IDLen - 4;
    }

	strBuf_temp = new BYTE[strBuf_temp_len];
	memset(strBuf_temp, 0, strBuf_temp_len);

	//获取upd文件的chksum
	//进行回读数据，统计chksum的值
	SetFilePointer(hFile, 0, NULL, FILE_END);//结
	file_len = GetFileSize(hFile, NULL);

	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);//SetFilePointer
	while (1)
	{
		if (file_len - read_addr > strBuf_temp_len)
		{
			read_len = strBuf_temp_len;
		}
		else
		{
			read_len = file_len - read_addr;
		}

		if(!ReadBuffer(hFile, strBuf_temp, read_addr, read_len))
		{
			delete [] strBuf_temp;
			strBuf_temp = NULL;
			return FALSE;
		}
		read_addr = read_addr + read_len;

		chksum += Upd_file_CheckSum(strBuf_temp, read_len);
		if (file_len == read_addr)
		{
			break;
		}
	}

	delete [] strBuf_temp;
	strBuf_temp = NULL;

    memcpy(strBuf, CHECK_EXPORT_ID, IDLen);
	memcpy(&strBuf[IDLen], &chksum, 4);
	memcpy(&strBuf[IDLen+4], T2A(strCheck), nLen);

	SetFilePointer(hFile, 0, NULL, FILE_END);//SetFilePointer
	ret = WriteFile(hFile, strBuf, 64, &write_len, NULL);	//WriteFile
	if(!ret || write_len != 64)
	{
		return FALSE;
	}

	FlushFileBuffers(hFile);//FlushFileBuffers

	return TRUE;
}

BOOL CUpdate::UnpacketCheckExport(HANDLE hSourceFile, BYTE str[])
{
	if (INVALID_HANDLE_VALUE == hSourceFile || NULL == str)
	{
		return FALSE;
	}
	
    BOOL ret;
	DWORD read_len;
    BYTE buf[64];
    UINT IDLen = strlen(CHECK_EXPORT_ID);//长度

    if (IDLen > 64)
    {
        return FALSE;
    }  
    
	memset(buf, 0, 64);
	SetFilePointer(hSourceFile, -64, NULL, FILE_END);//SetFilePointer
	ret = ReadFile(hSourceFile, buf, 64, &read_len, NULL);	//ReadFile
	if(!ret || read_len != 64)
	{
		return FALSE;
	}
    
    if (memcmp(buf, CHECK_EXPORT_ID, IDLen))//比较
    {
        return FALSE;
    }
    
    memcpy(str, &buf[IDLen], (64 - IDLen));
	return TRUE;
}
