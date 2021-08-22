# Microsoft Developer Studio Project File - Name="ISPCTRL_TOOL" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=ISPCTRL_TOOL - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ISPCTRL_TOOL.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ISPCTRL_TOOL.mak" CFG="ISPCTRL_TOOL - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ISPCTRL_TOOL - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ISPCTRL_TOOL - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ISPCTRL_TOOL - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "ANYKA_DEVELOP" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386 /out:"Release/Anyka_ISP_Tool.exe"

!ELSEIF  "$(CFG)" == "ISPCTRL_TOOL - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "ANYKA_DEVELOP" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 gdiplus.lib /nologo /subsystem:windows /debug /machine:I386 /out:"Debug/Anyka_ISP_Tool.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "ISPCTRL_TOOL - Win32 Release"
# Name "ISPCTRL_TOOL - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\3DNRDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\3DStatDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AEOtherDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AEStatDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AFDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AFStatDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AWB_G_weightDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Awb_wb_statDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AwbStatDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\BasePage.cpp
# End Source File
# Begin Source File

SOURCE=.\BBDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CCM_ImgDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CCM_LinkageDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CCM_RGBvalDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CCMDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CHUEDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CompareDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ContrastDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CoordinateDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CurveDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CurveLargeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\DemosaicDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\DenoiseDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\DpcDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\EdgeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ExpDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FcsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FcsLinkageDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\GammaDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\GammaLargeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\GammaRgbDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\GBDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\HUE_CALCDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\HUE_ImgDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\HUE_lineDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ImgDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ISPCTRL_TOOL.cpp
# End Source File
# Begin Source File

SOURCE=.\ISPCTRL_TOOL.rc
# End Source File
# Begin Source File

SOURCE=.\ISPCTRL_TOOLDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\LineDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\LscBrightnessCalcDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\LSCCOEFDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\LscDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\LutRgbDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MEDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MISCDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MiscOtherDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NetCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\NrLinkageDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NrLutDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Public_TextDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RawlutDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RawLutLargeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Rgb2YuvDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SaturationDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SharpDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SharpLutDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StatDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\SysDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\WB_EX_Attr.cpp
# End Source File
# Begin Source File

SOURCE=.\WbDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\WdrDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\WdrLutDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Y_GammaDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Y_GammaLargeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Y_GammaParmDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\YuvEffectDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Zone_weightDlg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\3DNRDlg.h
# End Source File
# Begin Source File

SOURCE=.\3DStatDlg.h
# End Source File
# Begin Source File

SOURCE=.\AEOtherDlg.h
# End Source File
# Begin Source File

SOURCE=.\AEStatDlg.h
# End Source File
# Begin Source File

SOURCE=.\AFDlg.h
# End Source File
# Begin Source File

SOURCE=.\AFStatDlg.h
# End Source File
# Begin Source File

SOURCE=.\anyka_types.h
# End Source File
# Begin Source File

SOURCE=.\AWB_G_weightDlg.h
# End Source File
# Begin Source File

SOURCE=.\Awb_wb_statDlg.h
# End Source File
# Begin Source File

SOURCE=.\AwbStatDlg.h
# End Source File
# Begin Source File

SOURCE=.\BasePage.h
# End Source File
# Begin Source File

SOURCE=.\BBDlg.h
# End Source File
# Begin Source File

SOURCE=.\CCM_ImgDlg.h
# End Source File
# Begin Source File

SOURCE=.\CCM_LinkageDlg.h
# End Source File
# Begin Source File

SOURCE=.\CCM_RGBvalDlg.h
# End Source File
# Begin Source File

SOURCE=.\CCMDlg.h
# End Source File
# Begin Source File

SOURCE=.\CHUEDlg.h
# End Source File
# Begin Source File

SOURCE=.\CompareDlg.h
# End Source File
# Begin Source File

SOURCE=.\ContrastDlg.h
# End Source File
# Begin Source File

SOURCE=.\CoordinateDlg.h
# End Source File
# Begin Source File

SOURCE=.\CurveDlg.h
# End Source File
# Begin Source File

SOURCE=.\CurveLargeDlg.h
# End Source File
# Begin Source File

SOURCE=.\DemosaicDlg.h
# End Source File
# Begin Source File

SOURCE=.\DenoiseDlg.h
# End Source File
# Begin Source File

SOURCE=.\DpcDlg.h
# End Source File
# Begin Source File

SOURCE=.\EdgeDlg.h
# End Source File
# Begin Source File

SOURCE=.\ExpDlg.h
# End Source File
# Begin Source File

SOURCE=.\FcsDlg.h
# End Source File
# Begin Source File

SOURCE=.\FcsLinkageDlg.h
# End Source File
# Begin Source File

SOURCE=.\GammaDlg.h
# End Source File
# Begin Source File

SOURCE=.\GammaLargeDlg.h
# End Source File
# Begin Source File

SOURCE=.\GammaRgbDlg.h
# End Source File
# Begin Source File

SOURCE=.\GBDlg.h
# End Source File
# Begin Source File

SOURCE=.\HUE_CALCDlg.h
# End Source File
# Begin Source File

SOURCE=.\HUE_ImgDlg.h
# End Source File
# Begin Source File

SOURCE=.\HUE_lineDlg.h
# End Source File
# Begin Source File

SOURCE=.\ImgDlg.h
# End Source File
# Begin Source File

SOURCE=.\isp_struct.h
# End Source File
# Begin Source File

SOURCE=.\isp_struct_v2.h
# End Source File
# Begin Source File

SOURCE=.\isp_struct_v3.h
# End Source File
# Begin Source File

SOURCE=.\ISPCTRL_TOOL.h
# End Source File
# Begin Source File

SOURCE=.\ISPCTRL_TOOLDlg.h
# End Source File
# Begin Source File

SOURCE=.\LineDlg.h
# End Source File
# Begin Source File

SOURCE=.\LscBrightnessCalcDlg.h
# End Source File
# Begin Source File

SOURCE=.\LSCCOEFDlg.h
# End Source File
# Begin Source File

SOURCE=.\LscDlg.h
# End Source File
# Begin Source File

SOURCE=.\LutRgbDlg.h
# End Source File
# Begin Source File

SOURCE=.\MEDlg.h
# End Source File
# Begin Source File

SOURCE=.\MISCDlg.h
# End Source File
# Begin Source File

SOURCE=.\MiscOtherDlg.h
# End Source File
# Begin Source File

SOURCE=.\NetCtrl.h
# End Source File
# Begin Source File

SOURCE=.\NrLinkageDlg.h
# End Source File
# Begin Source File

SOURCE=.\NrLutDlg.h
# End Source File
# Begin Source File

SOURCE=.\Public_TextDlg.h
# End Source File
# Begin Source File

SOURCE=.\RawlutDlg.h
# End Source File
# Begin Source File

SOURCE=.\RawLutLargeDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\Rgb2YuvDlg.h
# End Source File
# Begin Source File

SOURCE=.\SaturationDlg.h
# End Source File
# Begin Source File

SOURCE=.\SharpDlg.h
# End Source File
# Begin Source File

SOURCE=.\SharpLutDlg.h
# End Source File
# Begin Source File

SOURCE=.\StatDlg.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\SysDlg.h
# End Source File
# Begin Source File

SOURCE=.\WB_EX_Attr.h
# End Source File
# Begin Source File

SOURCE=.\WbDlg.h
# End Source File
# Begin Source File

SOURCE=.\WdrDlg.h
# End Source File
# Begin Source File

SOURCE=.\WdrLutDlg.h
# End Source File
# Begin Source File

SOURCE=.\Y_GammaDlg.h
# End Source File
# Begin Source File

SOURCE=.\Y_GammaLargeDlg.h
# End Source File
# Begin Source File

SOURCE=.\Y_GammaParmDlg.h
# End Source File
# Begin Source File

SOURCE=.\YuvEffectDlg.h
# End Source File
# Begin Source File

SOURCE=.\Zone_weightDlg.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\anykatest.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\copyright.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ISPCTRL_TOOL.ico
# End Source File
# Begin Source File

SOURCE=.\res\ISPCTRL_TOOL.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
