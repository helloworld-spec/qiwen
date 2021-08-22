#ifndef _AK_ISP2_REG_H_
#define _AK_ISP2_REG_H_

#define     ISP_BASE_ADDR           0 //0x40000
#define     GEN_CTRL_ADDR          	(ISP_BASE_ADDR + 0x0)
#define     DATA_WIDTH              0
#define     DATA_FORMAT             2
#define     RAW_SEQ                 4
#define     VSYNC_POL               6
#define     HERF_POL                7
#define     GO_UPLOAD               8
#define     GO_YUV                  9
#define     MAIN_UV_MODE            10
#define     SUB_UV_MODE             11
#define     POWER_SAVE_EN           12
#define     VIDEO_MODE              13
#define     SD_BUF_MODE             14
#define     MAIN_ADDR1_EN           15
#define     MAIN_ADDR2_EN           16
#define     MAIN_ADDR3_EN           17
#define     MAIN_ADDR4_EN           18
#define     FORCE_STOP              19
#define     ISP_ENABLE              20
#define     SUB_ADDR1_EN            21
#define     SUB_ADDR2_EN            22
#define     SUB_ADDR3_EN            23
#define     SUB_ADDR4_EN            24
#define     TEST_PATTERN            25
#define     TEST_PATTERN_EN         29

#define     CSI_ERR_CHECK_1         (ISP_BASE_ADDR + 0x4)
#define     UV_SWITCH               31
#define     UV_SUB128               30
#define     PCLK_ERR_CHECK          29
#define     PCLK_ERR_CLR            28
#define     EXP_VERT                14
#define     EXP_HOR                 0

#define     CSI_ERR_CHECK_2         (ISP_BASE_ADDR + 0x8)
#define     ONE_LINE_CYCLE          16

#define     CSI_HANDLE_ADDR         (ISP_BASE_ADDR + 0xC)
#define		FRAME_START_DELAY_EN    31
#define     LINE_END_CNT_EN         30
#define     LINE_END_CNT            24
#define     HBLANK_YUV_CYCLE        20
#define     HBLANK_RAW_CYCLE        8
#define		FRAME_START_DELAY_NUM   2
#define     HSYNC_ADJUST_EN         1
#define     VSYNC_NEW               0   

#define     STATUS_REG_ADDR         (ISP_BASE_ADDR + 0x10)
#define     DMA_DONE_CNT            24
#define     MAIN_FRAME_BUF_ID       22
#define     SUB_FRAME_BUF_ID        20
#define     EXP_HOR_ERR             13
#define     EXP_VERT_ERR            12
#define     REG_DMA_CONFLICT        11
#define     TNR_DMA_OVERFLOW        10
#define     GO_UPLOAD_DMA_OVERFLOW  9
#define     MAIN_DMA_OVERFLOW       8
#define     SDB_DMA_ALL_DONE        7
#define     SDB562_DMA_DONE         6
#define     SDB4_DMA_DONE           5
#define     SDB3_DMA_DONE           4
#define     SDB1_DMA_DONE           3
#define     SUB_FRAME_DONE          2
#define     MAIN_FRAME_DONE         1
#define     FRAME_ALL_DONE          0

#define     TNTERRUPT_ADDR          (ISP_BASE_ADDR + 0x14)
#define      EXP_HOR_ERR_INTEN           13
#define      EXP_VERT_ERR_INTEN          12
#define      DMA_CFLCT_INTEN             11
#define      TNR_OVERFLOW_INTEN          10
#define      GO_UPLOAD_OVERFLOW_INTEN    9
#define      MAIN_OVERFLOW_INTEN         8
#define      SDB_ALL_DONE_INTEN          7
#define      SDB562_DONE_INTEN           6
#define      SDB4_DONE_INTEN             5
#define      SDB3_DONE_INTEN             4
#define      SDB1_DONE_INTEN             3
#define      SUB_FRAME_DONE_INTEN        2
#define      MAIN_FRAME_DONE_INTEN       1
#define      FRAME_ALL_DONE_INTEN        0
#define      FRAME_CNT                   24

#define     DMA_REG_ADDR            (ISP_BASE_ADDR + 0x18)
#define     DMAREG_ADDR             0

#define     REG_UPDATE              (ISP_BASE_ADDR + 0x1C)
#define     UPDATE_DMAREG_LEAST     2
#define     UPDATE_DMAREG_MOST      1
#define     UPDATE_DMAREG_ALL       0

#define     DMA_SDPC_ADDR           (ISP_BASE_ADDR + 0x20)
#define     SDPC_EN                 31
#define     SDPC_TAB_ADDR           0 

/* register block offset based on REGBA */
#define     REG_BLOCK_1             (ISP_BASE_ADDR + 0x24)
#define     REG_BLOCK_1_CNT_CLR     0

#define     REG_BLOCK_2             (ISP_BASE_ADDR + 0x28)
#define     REG_BLOCK_2_CNT_CLR     0

#define     REG_BLOCK_3             (ISP_BASE_ADDR + 0x2C)
#define     REG_BLOCK_3_CNT_CLR     0

#define     REG_BLOCK_4             (ISP_BASE_ADDR + 0x30)
#define     REG_BLOCK_4_CNT_CLR     0

#define     REG_BLOCK_5             (ISP_BASE_ADDR + 0x34)
#define     REG_BLOCK_5_CNT_CLR     0

/* memory block offset based on REGBA */
#define     MEM_BLOCK_1             (ISP_BASE_ADDR + 0x38)
#define     MEM_BLOCK_1_CNT_CLR     0

#define     MEM_BLOCK_2             (ISP_BASE_ADDR + 0x3C)
#define     MEM_BLOCK_2_CNT_CLR     0

#define     MEM_BLOCK_3             (ISP_BASE_ADDR + 0x40)
#define     MEM_BLOCK_3_CNT_CLR     0

#define     MEM_BLOCK_4             (ISP_BASE_ADDR + 0x44)
#define     MEM_BLOCK_4_CNT_CLR     0

#define     MEM_BLOCK_5             (ISP_BASE_ADDR + 0x48)
#define     MEM_BLOCK_5_CNT_CLR     0

#define     MEM_BLOCK_6             (ISP_BASE_ADDR + 0x4C)
#define     MEM_BLOCK_6_CNT_CLR     0

/* remain appointed number of low bits, and mask the other high bits */
#define LOW_BITS(var, bits)	((var) & ((0xFFFFFFFF) >> ((32-(bits)))))

/* set var bits [from, from+bits] */
#define SET_BITS(var, from, bits)	\
	((var) |= (((0xFFFFFFFF) >> (32-(bits))) << (from)))

/* clear var bits [from, from+bits] */
#define CLEAR_BITS(var, from, bits)	\
	((var) &= (~(((0xFFFFFFFF) >> (32-(bits))) << (from))))


/************************************************************************/
/*                          Register Block 1                            */
/************************************************************************/
//CUT_REG_1
#define   CUT_STR_XPOS            0
#define   CUT_STR_YPOS            16
//CUT_REG_2
#define   CUT_WIN_WIDTH           0
#define   CUT_WIN_HEIGHT          16
#define   DOWNSAMPLE              29
//LENC_REG_1              
#define   LENS_COEF0_BB_R         0
#define   LENS_COEF0_BB_GR        8
#define   LENS_COEF0_BB_GB        16
#define   LENS_COEF0_BB_B         24
//      LENC_REG_2
#define   LENS_COEF1_BB_R         0
#define   LENS_COEF1_BB_GR        8
#define   LENS_COEF1_BB_GB        16
#define   LENS_COEF1_BB_B         24
//      LENC_REG_3              
#define   LENS_COEF2_BB_R         0
#define   LENS_COEF2_BB_GR        8
#define   LENS_COEF2_BB_GB        16
#define   LENS_COEF2_BB_B         24
//      LENC_REG_4              
#define   LENS_COEF3_BB_R         0
#define   LENS_COEF3_BB_GR        8
#define   LENS_COEF3_BB_GB        16
#define   LENS_COEF3_BB_B         24
//      LENC_REG_5              
#define   LENS_COEF4_BB_R         0
#define   LENS_COEF4_BB_GR        8
#define   LENS_COEF4_BB_GB        16
#define   LENS_COEF4_BB_B         24
//      LENC_REG_6              
#define   LENS_COEF5_BB_R         0
#define   LENS_COEF5_BB_GR        8
#define   LENS_COEF5_BB_GB        16
#define   LENS_COEF5_BB_B         24
//      LENC_REG_7              
#define   LENS_COEF6_BB_R         0
#define   LENS_COEF6_BB_GR        8
#define   LENS_COEF6_BB_GB        16
#define   LENS_COEF6_BB_B         24
//      LENC_REG_8              
#define   LENS_COEF7_BB_R         0
#define   LENS_COEF7_BB_GR        8
#define   LENS_COEF7_BB_GB        16
#define   LENS_COEF7_BB_B         24
//      LENC_REG_9              
#define   LENS_COEF8_BB_R         0
#define   LENS_COEF8_BB_GR        8
#define   LENS_COEF8_BB_GB        16
#define   LENS_COEF8_BB_B         24
//      LENC_REG_10         
#define   LENS_COEF9_BB_R         0
#define   LENS_COEF9_BB_GR        8
#define   LENS_COEF9_BB_GB        16
#define   LENS_COEF9_BB_B         24
//      LENC_REG_11             
#define   LENS_COEF0_CC_R         0
#define   LENS_COEF0_CC_GR        10
#define   LENS_COEF0_CC_GB        20
//      LENC_REG_12             
#define   LENS_COEF0_CC_B         0
#define   LENS_COEF1_CC_R         10
#define   LENS_COEF1_CC_GR        20
//      LENC_REG_13             
#define   LENS_COEF1_CC_GB        0
#define   LENS_COEF1_CC_B         10
#define   LENS_COEF2_CC_R         20
//      LENC_REG_14             
#define   LENS_COEF2_CC_GR        0
#define   LENS_COEF2_CC_GB        10
#define   LENS_COEF2_CC_B         20
//      LENC_REG_15             
#define   LENS_COEF3_CC_R         0
#define   LENS_COEF3_CC_GR        10
#define   LENS_COEF3_CC_GB        20
//      LENC_REG_16             
#define   LENS_COEF3_CC_B         0
#define   LENS_COEF4_CC_R         10
#define   LENS_COEF4_CC_GR        20
//      LENC_REG_17         
#define   LENS_COEF4_CC_GB        0
#define   LENS_COEF4_CC_B         10
#define   LENS_COEF5_CC_R         20
//      LENC_REG_18             
#define   LENS_COEF5_CC_GR        0
#define   LENS_COEF5_CC_GB        10
#define   LENS_COEF5_CC_B         20
//      LENC_REG_19             
#define   LENS_COEF6_CC_R         0
#define   LENS_COEF6_CC_GR        10
#define   LENS_COEF6_CC_GB        20
//      LENC_REG_20             
#define   LENS_COEF6_CC_B         0
#define   LENS_COEF7_CC_R         10
#define   LENS_COEF7_CC_GR        20
//      LENC_REG_21             
#define   LENS_COEF7_CC_GB        0
#define   LENS_COEF7_CC_B         10
#define   LENS_COEF8_CC_R         20
//      LENC_REG_22             
#define   LENS_COEF8_CC_GR        0
#define   LENS_COEF8_CC_GB        10
#define   LENS_COEF8_CC_B         20
//      LENC_REG_23             
#define   LENS_COEF9_CC_R         0
#define   LENS_COEF9_CC_GR        10
#define   LENS_COEF9_CC_GB        20
//      LENC_REG_24             
#define   LENS_COEF9_CC_B         0
#define   LENS_RANGE0_RR          10
#define   LENS_RANGE1_RR          20
//      LENC_REG_25             
#define   LENS_RANGE2_RR          0
#define   LENS_RANGE3_RR          10
#define   LENS_RANGE4_RR          20
//      LENC_REG_26             
#define   LENS_RANGE5_RR          0
#define   LENS_RANGE6_RR          10
#define   LENS_RANGE7_RR          20
//      LENC_REG_27             
#define   LENS_RANGE8_RR          0
#define   LENS_RANGE9_RR          10
//      LENC_REG_28             
#define   LENS_XREF               0
#define   LENS_YREF               16
//      LENC_REG_29             
#define   LENS_1STSQR             0
#define   LENS_LSC_SHIFT          28

//      ADDR_REG_1              
#define   CH1_YADDR2              0
//      ADDR_REG_2              
#define   CH1_YADDR3              0 
//      ADDR_REG_3              
#define   CH1_YADDR4              0 
//      ADDR_REG_4              
#define   CH1_UADDR2              0
//      ADDR_REG_5              
#define   CH1_UADDR3              0 
//      ADDR_REG_6              
#define   CH1_UADDR4              0 
//      ADDR_REG_7              
#define   CH1_VADDR2              0
//      ADDR_REG_8              
#define   CH1_VADDR3              0 
//      ADDR_REG_9              
#define   CH1_VADDR4              0 

//      ADDR_REG_10             
#define   CH2_YADDR2              0
//      ADDR_REG_11             
#define   CH2_YADDR3              0 
//      ADDR_REG_12             
#define   CH2_YADDR4              0 
//      ADDR_REG_13             
#define   CH2_UADDR2              0
//      ADDR_REG_14             
#define   CH2_UADDR3              0 
//      ADDR_REG_15             
#define   CH2_UADDR4              0 
//      ADDR_REG_16             
#define   CH2_VADDR2              0
//      ADDR_REG_17             
#define   CH2_VADDR3              0 
//      ADDR_REG_18             
#define   CH2_VADDR4              0 

//      ADDR_REG_19             
#define   STAT_ADDR2              0
//      ADDR_REG_20             
#define   STAT_ADDR3              0
//      ADDR_REG_21             
#define   STAT_ADDR4              0
//      ADDR_REG_22             
//  #define SDPC_TABLE_SADDR        0
//      ADDR_REG_23             
#define   TNR_YADDR               0
//      ADDR_REG_24             
#define   TNR_YSIZE               0
//      ADDR_REG_25             
#define   TNR_UADDR               0
//      ADDR_REG_26             
#define   TNR_USIZE               0
//      ADDR_REG_27             
#define   TNR_VADDR               0
//      ADDR_REG_28         
#define   TNR_VSIZE               0

/************************************************************************/
/*                             Register Block 2                         */
/************************************************************************/
//      RAW_REG_1               
#define   BL_A_R                  0
#define   BL_A_GR                 10
#define   BL_A_GB                 20
#define   BL_A_B_2                30
//      RAW_REG_2               
#define   BL_A_B_8                0
#define   BL_B_R                  8
#define   BL_B_GR                 20
//      RAW_REG_3               
#define   BL_B_GB                 0
#define   BL_B_B                  12
//      RAW_REG_4               
#define   ZONE_WEIGHT_C0          0
//      RAW_REG_5               
#define   ZONE_WEIGHT_C1          0
//      RAW_REG_6               
#define   ZONE_WEIGHT_C2          0
//      RAW_REG_7               
#define   ZONE_WEIGHT_C3          0
//      RAW_REG_8               
#define   ZONE_WEIGHT_C4          0
//      RAW_REG_9               
#define   ZONE_WEIGHT_C5          0
//      RAW_REG_10              
#define   ZONE_WEIGHT_C6          0
//      RAW_REG_11              
#define   ZONE_WEIGHT_C7          0
//      RAW_REG_12              
#define   ZONE_WEIGHT_C8          0
//      RAW_REG_13              
#define   ZONE_WEIGHT_C9          0
//      RAW_REG_14              
#define   ZONE_WEIGHT_C10         0
//      RAW_REG_15              
#define   ZONE_WEIGHT_C11         0
//      RAW_REG_16              
#define   ZONE_WEIGHT_C12         0
//      RAW_REG_17              
#define   ZONE_WEIGHT_C13         0
//      RAW_REG_18              
#define   ZONE_WEIGHT_C14         0
//      RAW_REG_19              
#define   ZONE_WEIGHT_C15         0                   

//      RAW_REG_20              
#define     DPC_TH                  0
#define     BLACK_DPC_EN            16
#define     WHITE_DPC_EN            17


//      RAW_REG_21              
#define     NR_WEIGHT_TBL_R0        0
#define     NR_WEIGHT_TBL_R1        10
#define     NR_WEIGHT_TBL_R2        20
//      RAW_REG_22              
#define     NR_WEIGHT_TBL_R3        0
#define     NR_WEIGHT_TBL_R4        10
#define     NR_WEIGHT_TBL_R5        20
//      RAW_REG_23              
#define     NR_WEIGHT_TBL_R6        0
#define     NR_WEIGHT_TBL_R7        10
#define     NR_WEIGHT_TBL_R8        20
//      RAW_REG_24              
#define     NR_WEIGHT_TBL_R9        0
#define     NR_WEIGHT_TBL_R10       10
#define     NR_WEIGHT_TBL_R11       20
//      RAW_REG_25              
#define     NR_WEIGHT_TBL_R12       0
#define     NR_WEIGHT_TBL_R13       10
#define     NR_WEIGHT_TBL_R14       20
//      RAW_REG_26              
#define     NR_WEIGHT_TBL_R15       0
#define     NR_WEIGHT_TBL_R16       10
#define     NR_WEIGHT_TBL_G0        20
//      RAW_REG_27              
#define     NR_WEIGHT_TBL_G1        0
#define     NR_WEIGHT_TBL_G2        10
#define     NR_WEIGHT_TBL_G3        20
//      RAW_REG_28              
#define     NR_WEIGHT_TBL_G4        0
#define     NR_WEIGHT_TBL_G5        10
#define     NR_WEIGHT_TBL_G6        20
//      RAW_REG_29              
#define     NR_WEIGHT_TBL_G7        0
#define     NR_WEIGHT_TBL_G8        10
#define     NR_WEIGHT_TBL_G9        20
//      RAW_REG_30          
#define     NR_WEIGHT_TBL_G10       0
#define     NR_WEIGHT_TBL_G11       10
#define     NR_WEIGHT_TBL_G12       20
//      RAW_REG_31              
#define     NR_WEIGHT_TBL_G13       0
#define     NR_WEIGHT_TBL_G14       10
#define     NR_WEIGHT_TBL_G15       20
//      RAW_REG_32              
#define     NR_WEIGHT_TBL_G16       0
#define     NR_WEIGHT_TBL_B0        10
#define     NR_WEIGHT_TBL_B1        20
//      RAW_REG_33              
#define     NR_WEIGHT_TBL_B2        0
#define     NR_WEIGHT_TBL_B3        10
#define     NR_WEIGHT_TBL_B4        20
//      RAW_REG_34              
#define     NR_WEIGHT_TBL_B5        0
#define     NR_WEIGHT_TBL_B6        10
#define     NR_WEIGHT_TBL_B7        20
//      RAW_REG_35              
#define     NR_WEIGHT_TBL_B8        0
#define     NR_WEIGHT_TBL_B9        10
#define     NR_WEIGHT_TBL_B10       20
//      RAW_REG_36              
#define     NR_WEIGHT_TBL_B11       0
#define     NR_WEIGHT_TBL_B12       10
#define     NR_WEIGHT_TBL_B13       20
//      RAW_REG_37              
#define     NR_WEIGHT_TBL_B14       0
#define     NR_WEIGHT_TBL_B15       10
#define     NR_WEIGHT_TBL_B16       20
//      RAW_REG_38              
#define     NR_LC_LUT_0             0
#define     NR_LC_LUT_1             10
#define     NR_LC_LUT_2             20
//      RAW_REG_39          
#define     NR_LC_LUT_3             0
#define     NR_LC_LUT_4             10
#define     NR_LC_LUT_5             20
//      RAW_REG_40              
#define     NR_LC_LUT_6             0
#define     NR_LC_LUT_7             10
#define     NR_LC_LUT_8             20
//      RAW_REG_41              
#define     NR_LC_LUT_9             0
#define     NR_LC_LUT_10            10
#define     NR_LC_LUT_11            20
//      RAW_REG_42              
#define     NR_LC_LUT_12            0
#define     NR_LC_LUT_13            10
#define     NR_LC_LUT_14            20
//      RAW_REG_43              
#define     NR_LC_LUT_15            0
#define     NR_LC_LUT_16            10
#define     NR_K                    20
#define     GB_EN_TH                24
//      RAW_REG_44              
#define     GB_TH                   0
#define     GB_KSTEP                10
#define     DM_RG_TH                14
#define     DM_RG_GAIN              24
//      RAW_REG_45              
#define     DM_HF_TH1               0
#define     DM_HF_TH2               10
#define     DM_BG_TH                20
//      RAW_REG_46              
#define     DM_BG_GAIN              0
#define     DM_HV_TH                8
#define     DM_GR_GAIN              16
#define     DM_GB_GAIN              24
//      RAW_REG_47              
#define     CC_CC_CNOISE_TH         0
#define     CC_CC_CNOISE_GAIN       8
#define     CC_CC_CNOISE_SLOP       16

//      RAW_REG_48              
#define     CC_CCM_RR               12
#define     CC_CCM_RG_L8            24
//      RAW_REG_49              
#define     CC_CCM_RG_H4            0
#define     CC_CCM_RB               4
#define     CC_CCM_GR               16
#define     CC_CCM_GG_L4            28  
//      RAW_REG_50              
#define     CC_CCM_GG_H8            0
#define     CC_CCM_GB               8
#define     CC_CCM_BR               20
//      RAW_REG_51          
#define     CC_CCM_BG               0
#define     CC_CCM_BB               12

//      RAW_REG_52                           
#define     RGB2YUV_Y_CONSTRAST     0
#define     RGB2YUV_Y_SHIFT         8
#define     RGB2YUV_FORMULA         24

/************************************************************************/
/*                        Register Block 3                              */
/************************************************************************/
//      YUV_REG_1         
#define     YGAMMA_CNOISE_TH1       0
#define     YGAMMA_CNOISE_TH2       10
#define     YGAMMA_CNOISE_GAIN      20

//      YUV_REG_2 
#define     YGAMMA_CNOISE_SLOP      0
#define     YGAMMA_UV_ADJUST_LEVEL  8

//      YUV_REG_3 
#define     HDR_TH1                 0
#define     HDR_TH2                 8
#define     HDR_TH3                 16
#define     HDR_TH4                 24

//      YUV_REG_4    
#define     HDR_TH5                 0
#define     HDR_REVERSEW_G          8
#define     HDR_REVERSEW_SHIFT      17
#define     HDR_REVERSEH_G          20
#define     HDR_REVERSEH_SHIFT      29

//      YUV_REG_5               
#define     HDR_WEIGHT_G            0
#define     HDR_WEIGHT_SHIFT        9
#define     HDR_CNOISE_SUPPRESS_SLOP    12            
#define     HDR_CNOISE_SUPPRESS_YTH1    20

//      YUV_REG_6  
#define     HDR_CNOISE_SUPPRESS_YTH2    0
#define     HDR_CNOISE_SUPPRESS_GAIN    10           
#define     HDR_UV_ADJUST_LEVEL         20
#define     HDR_WEIGHT_K                28

//      YUV_REG_7               
#define     SE_YTH1                 0
#define     SE_YTH2                 10
#define     SE_YTH3                 20
#define     SE_YTH4_L2              30

//      YUV_REG_8               
#define     SE_YTH4_H_8             0
#define     SE_SCALE1               8
#define     SE_SCALE2               16
#define     SE_SCALE3               24

//      YUV_REG_9  
#define     SE_SCALE_SLOP_1         0
#define     SE_SCALE_SLOP_2         8
#define     TNR_UV_MIN_EN           16
#define     TNR_COMPRESS_FORMAT     17

//      YUV_REG_10          
#define     TNR_YNR_WEIGHT_TBL_1        0
#define     TNR_YNR_WEIGHT_TBL_2        10
#define     TNR_YNR_WEIGHT_TBL_3        20

//      YUV_REG_11              
#define     TNR_YNR_WEIGHT_TBL_4        0
#define     TNR_YNR_WEIGHT_TBL_5        10
#define     TNR_YNR_WEIGHT_TBL_6        20

//      YUV_REG_12              
#define     TNR_YNR_WEIGHT_TBL_7        0
#define     TNR_YNR_WEIGHT_TBL_8        10
#define     TNR_YNR_WEIGHT_TBL_9        20

//      YUV_REG_13              
#define     TNR_YNR_WEIGHT_TBL_10       0
#define     TNR_YNR_WEIGHT_TBL_11       10
#define     TNR_YNR_WEIGHT_TBL_12       20

//      YUV_REG_14              
#define     TNR_YNR_WEIGHT_TBL_13       0
#define     TNR_YNR_WEIGHT_TBL_14       10
#define     TNR_YNR_WEIGHT_TBL_15       20

//      YUV_REG_15          
#define     TNR_YNR_WEIGHT_TBL_16       0
#define     TNR_YNR_DIFF_SHIFT          12
#define     TNR_YNR_K                   16
#define     TNR_YLP_K                   20
#define     TNR_UVNR_K              24
#define     TNR_UVLP_K              28

//      YUV_REG_16
#define     TNR_T_Y_TH1             0
#define     TNR_T_Y_DIFFTH_K1       8
#define     TNR_T_Y_DIFFTH_K2       16
#define     TNR_T_Y_DIFFTH_SLOP     24

//      YUV_REG_17
#define     TNR_T_Y_K1              0
#define     TNR_T_Y_K2              7
#define     TNR_T_Y_KSLOP           14
#define     TNR_T_Y_AC_TH           21

//      YUV_REG_18 
#define     TNR_T_Y_MINSTEP         0
#define     TNR_T_Y_MF_TH1          5
#define     TNR_T_Y_MF_TH2          18

//      YUV_REG_19
#define     TNR_T_Y_MC_K           0
#define     TNR_T_UV_K             5
#define     TNR_T_UV_MINSTEP       12
#define     TNR_T_UV_MC_K          17
#define     TNR_T_UV_AC_TH         22

//      YUV_REG_20
#define     TNR_T_UV_MF_TH1        0
#define     TNR_T_UV_MF_TH2        16

//      YUV_REG_21
#define     TNR_T_UV_DIFFTH_K1     0
#define     TNR_T_UV_DIFFTH_K2     8
#define     TNR_T_UV_DIFFTH_SLOP   16

//      YUV_REG_22
#define     TNR_MOTION_START_MAX   0
#define     TNR_REVW               16
#define     TNR_REVH               24

//      YUV_REG_23
#define     YNR_WEIGHT_TBL_1       0
#define     YNR_WEIGHT_TBL_2       10
#define     YNR_WEIGHT_TBL_3       20

//      YUV_REG_24
#define     YNR_WEIGHT_TBL_4       0
#define     YNR_WEIGHT_TBL_5       10
#define     YNR_WEIGHT_TBL_6       20

//      YUV_REG_25             
#define     YNR_WEIGHT_TBL_7       0
#define     YNR_WEIGHT_TBL_8       10
#define     YNR_WEIGHT_TBL_9       20

//      YUV_REG_26              
#define     YNR_WEIGHT_TBL_10      0
#define     YNR_WEIGHT_TBL_11      10
#define     YNR_WEIGHT_TBL_12      20

//      YUV_REG_27              
#define     YNR_WEIGHT_TBL_13       0
#define     YNR_WEIGHT_TBL_14       10
#define     YNR_WEIGHT_TBL_15       20

//      YUV_REG_28
#define     YNR_WEIGHT_TBL_16       0
#define     YNR_Y_DPC_TH            10
#define     YNR_K                   20
#define     YUV_SHARPEN_M10         24
#define     YNR_Y_BLACK_DPC_EN      30
#define     YNR_Y_WHITE_DPC_EN      31

//      YUV_REG_29
#define     YUV_SHARPEN_M11         0
#define     YUV_SHARPEN_M12         4
#define     YUV_SHARPEN_M13         8
#define     YUV_SHARPEN_M14         12
#define     YUV_SHARPEN_M15         16
#define     YUV_SHARPEN_M20         20
#define     YUV_SHARPEN_M21         26

//      YUV_REG_30              
#define     YUV_SHARPEN_M22         0
#define     YUV_SHARPEN_MF_HPF_K    6
#define     YUV_SHARPEN_METHOD      14
#define     YUV_SHARPEN_SKIN_MAX_TH 16
#define     YUV_SHARPEN_SKIN_MIN_TH 24

//      YUV_REG_31            
#define     YUV_SHARPEN_SKIN_V_MAX_TH   0
#define     YUV_SHARPEN_SKIN_V_MIN_TH   8
#define     YUV_SHARPEN_SKIN_Y_MAX_TH   16
#define     YUV_SHARPEN_SKIN_Y_MIN_TH   24

//      YUV_REG_32              
#define     YUV_SHARPEN_MF_HPF_SHIFT    0
#define     YUV_SHARPEN_HF_HPF_K        4
#define     YUV_SHARPEN_HF_HPF_SHIFT    11
#define     YUV_SHARPEN_SKIN_GAIN_TH    16
#define     YUV_SHARPEN_SKIN_GAIN_WEEKEN    24

//      YUV_REG_33              
#define     AF_WIN0_LEFT             0
#define     AF_WIN0_RIGHT            11
#define     AF_WIN0_TOP              22

//      YUV_REG_34            
#define     AF_WIN0_BOTTOM           0
#define     AF_WIN1_LEFT             10
#define     AF_WIN1_RIGHT            21

//      YUV_REG_35  
#define     AF_WIN1_TOP              0
#define     AF_WIN1_BOTTOM           10
#define     AF_WIN2_LEFT             20

//      YUV_REG_36
#define     AF_WIN2_RIGHT            0
#define     AF_WIN2_TOP              11
#define     AF_WIN2_BOTTOM           21

//      YUV_REG_37              
#define     AF_WIN3_LEFT             0
#define     AF_WIN3_RIGHT            11
#define     AF_WIN3_TOP              22

//      YUV_REG_38             
#define     AF_WIN3_BOTTOM           0
#define     AF_WIN4_LEFT             10
#define     AF_WIN4_RIGHT            21

//      YUV_REG_39
#define     AF_WIN4_TOP              0
#define     AF_WIN4_BOTTOM           10
#define     AF_TH                    20

//      YUV_REG_40             
#define     FCS_TH                  0
#define     FCS_SLOP                8
#define     FCS_UV_NR_TH            16

//      YUV_REG_41             
#define     ENHANCE_Y_A             0
#define     ENHANCE_Y_B             8
#define     ENHANCE_UV_A            16

//      YUV_REG_42   
#define     ENHANCE_UV_B            0

/************************************************************************/
/*                         Register Block 4                             */
/************************************************************************/
//      PP_REG_1                
#define     PP_MASK1_S_XPOS         0
#define     PP_MASK1_E_XPOS         11
#define     PP_MASK1_S_YPOS         22

//      PP_REG_2                
#define     PP_MASK1_E_YPOS         0
#define     PP_MASK2_S_XPOS         10
#define     PP_MASK2_E_XPOS         21

//      PP_REG_3                
#define     PP_MASK2_S_YPOS         0
#define     PP_MASK2_E_YPOS         10
#define     PP_MASK3_S_XPOS         20

//      PP_REG_4                
#define     PP_MASK3_E_XPOS         0
#define     PP_MASK3_S_YPOS         11
#define     PP_MASK3_E_YPOS         21

//      PP_REG_5                
#define     PP_MASK4_S_XPOS         0
#define     PP_MASK4_E_XPOS         11
#define     PP_MASK4_S_YPOS         22

//      PP_REG_6                
#define     PP_MASK4_E_YPOS         0
#define     PP_MASK5_S_XPOS         10
#define     PP_MASK5_E_XPOS         21

//      PP_REG_7                
#define     PP_MASK5_S_YPOS         0
#define     PP_MASK5_E_YPOS         10
#define     PP_MASK6_S_XPOS         20

//      PP_REG_8                
#define     PP_MASK6_E_XPOS         0
#define     PP_MASK6_S_YPOS         11
#define     PP_MASK6_E_YPOS         21

//      PP_REG_9                
#define     PP_MASK7_S_XPOS         0
#define     PP_MASK7_E_XPOS         11
#define     PP_MASK7_S_YPOS         22

//      PP_REG_10               
#define     PP_MASK7_E_YPOS         0
#define     PP_MASK8_S_XPOS         10
#define     PP_MASK8_E_XPOS         21

//      PP_REG_11               
#define     PP_MASK8_S_YPOS         0
#define     PP_MASK8_E_YPOS         10
#define     PP_MASK1_EN             24
#define     PP_MASK2_EN             25
#define     PP_MASK3_EN             26
#define     PP_MASK4_EN             27
#define     PP_MASK5_EN             28
#define     PP_MASK6_EN             29
#define     PP_MASK7_EN             30
#define     PP_MASK8_EN             31

//      PP_REG_12               
#define     PP_OSD_COLOR1_V         8
#define     PP_OSD_COLOR2_V         16
#define     PP_OSD_COLOR3_V         24

//      PP_REG_13               
#define     PP_OSD_COLOR1_U         8
#define     PP_OSD_COLOR2_U         16
#define     PP_OSD_COLOR3_U         24

//      PP_REG_14               
#define     PP_OSD_COLOR1_Y         8
#define     PP_OSD_COLOR2_Y         16
#define     PP_OSD_COLOR3_Y         24

//      PP_REG_15               
#define     PP_OSD_COLOR4_V         0
#define     PP_OSD_COLOR5_V         8
#define     PP_OSD_COLOR6_V         16
#define     PP_OSD_COLOR7_V         24

//      PP_REG_16               
#define     PP_OSD_COLOR4_U         0
#define     PP_OSD_COLOR5_U         8
#define     PP_OSD_COLOR6_U         16
#define     PP_OSD_COLOR7_U         24

//      PP_REG_17               
#define     PP_OSD_COLOR4_Y         0
#define     PP_OSD_COLOR5_Y         8
#define     PP_OSD_COLOR6_Y         16
#define     PP_OSD_COLOR7_Y         24

//      PP_REG_18               
#define     PP_OSD_COLOR8_V         0
#define     PP_OSD_COLOR9_V         8
#define     PP_OSD_COLOR10_V        16
#define     PP_OSD_COLOR11_V        24

//      PP_REG_19               
#define     PP_OSD_COLOR8_U         0
#define     PP_OSD_COLOR9_U         8
#define     PP_OSD_COLOR10_U        16
#define     PP_OSD_COLOR11_U        24

//      PP_REG_20               
#define     PP_OSD_COLOR8_Y         0
#define     PP_OSD_COLOR9_Y         8
#define     PP_OSD_COLOR10_Y        16
#define     PP_OSD_COLOR11_Y        24

//      PP_REG_21               
#define     PP_OSD_COLOR12_V        0
#define     PP_OSD_COLOR13_V        8
#define     PP_OSD_COLOR14_V        16
#define     PP_OSD_COLOR15_V        24

//      PP_REG_22               
#define     PP_OSD_COLOR12_U        0
#define     PP_OSD_COLOR13_U        8
#define     PP_OSD_COLOR14_U        16
#define     PP_OSD_COLOR15_U        24

//      PP_REG_23               
#define     PP_OSD_COLOR12_Y        0
#define     PP_OSD_COLOR13_Y        8
#define     PP_OSD_COLOR14_Y        16
#define     PP_OSD_COLOR15_Y        24

//      PP_REG_24               
#define     PP_MASK_COLOR           0
#define     PP_MASK_ALPHA           24
#define     PP_MASK_FORMAT          28

//      PP_REG_25               
#define     PP_OSD1_ADDR            0

//      PP_REG_26               
#define     PP_OSD1_START_X         0
#define     PP_OSD1_START_Y         13

//      PP_REG_27               
#define     PP_OSD1_END_X           0
#define     PP_OSD1_END_Y           13
#define     PP_OSD1_ALPHA           28

//      PP_REG_28               
#define     PP_OSD2_ADDR            0

//      PP_REG_29           
#define     PP_OSD2_START_X         0
#define     PP_OSD2_START_Y         13

//      PP_REG_30               
#define     PP_OSD2_END_X           0
#define     PP_OSD2_END_Y           13
#define     PP_OSD2_ALPHA           28

//      PP_REG_31               
#define     PP_WIDTH1_PARA          0
#define     PP_HEIGHT1_PARA         12

//      PP_REG_32               
#define     PP_DST1_WIDTH           0
#define     pp_DST1_HEIGHT          12

//      PP_REG_33               
#define     PP_WIDTH2_PARA          0
#define     PP_HEIGHT2_PARA         12

//      PP_REG_34               
#define     PP_DST2_WIDTH           0
#define     pp_DST2_HEIGHT          12

//      PP_REG_35               
#define     PP_FRAME1_CTRL          0
#define     PP_FRAME2_CTRL          8

/************************************************************************/
/*                         Register Block 5                             */
/************************************************************************/
//      WB_REG_1                
#define     WB_GR_LOW_0             0
#define     WB_GR_HIGH_0            10
#define     WB_GB_LOW_0             20
#define     WB_GB_HIGH_0_L2         30

//      WB_REG_2                
#define     WB_GB_HIGH_0_H8         0
#define     WB_RB_LOW_0             8
#define     WB_RB_HIGH_0            18
#define     WB_GR_LOW_1_L4          28

//      WB_REG_3                
#define     WB_GR_LOW_1_H6          0
#define     WB_GR_HIGH_1            6
#define     WB_GB_LOW_1             16
#define     WB_GB_HIGH_1_L6         26

//      WB_REG_4                
#define     WB_GB_HIGH_1_H4         0
#define     WB_RB_LOW_1             4
#define     WB_RB_HIGH_1            14
#define     WB_GR_LOW_2_L8          24

//      WB_REG_5                
#define     WB_GR_LOW_2_H2          0
#define     WB_GR_HIGH_2            2
#define     WB_GB_LOW_2             12
#define     WB_GB_HIGH_2            22

//      WB_REG_6                
#define     WB_RB_LOW_2             0
#define     WB_RB_HIGH_2            10
#define     WB_GR_LOW_3             20
#define     WB_GR_HIGH_3_L2         30

//      WB_REG_7                
#define     WB_GR_HIGH_3_H8         0
#define     WB_GB_LOW_3             8
#define     WB_GB_HIGH_3            18
#define     WB_RB_LOW_3_L4          28

//      WB_REG_8       
#define     WB_RB_LOW_3_H6          0
#define     WB_RB_HIGH_3            6
#define     WB_GR_LOW_4             16
#define     WB_GR_HIGH_4_L6         26

//      WB_REG_9
#define     WB_GR_HIGH_4_H4         0
#define     WB_GB_LOW_4             4
#define     WB_GB_HIGH_4            14
#define     WB_RB_LOW_4_L8          24

//      WB_REG_10   
#define     WB_RB_LOW_4_H2          0               
#define     WB_RB_HIGH_4            2
#define     WB_GR_LOW_5             12
#define     WB_GR_HIGH_5            22

//      WB_REG_11               
#define     WB_GB_LOW_5             0
#define     WB_GB_HIGH_5            10
#define     WB_RB_LOW_5             20
#define     WB_RB_HIGH_5_L2         30

//      WB_REG_12
#define     WB_RB_HIGH_5_H8         0
#define     WB_GR_LOW_6             8
#define     WB_GR_HIGH_6            18
#define     WB_GB_LOW_6_L4          28

//      WB_REG_13        
#define     WB_GB_LOW_6_H6          0
#define     WB_GB_HIGH_6            6
#define     WB_RB_LOW_6             16
#define     WB_RB_HIGH_6_L6         26

//      WB_REG_14 
#define     WB_RB_HIGH_6_H4         0
#define     WB_GR_LOW_7             4
#define     WB_GR_HIGH_7            14
#define     WB_GB_LOW_7_L8          24

//      WB_REG_15
#define     WB_GB_LOW_7_H2          0
#define     WB_GB_HIGH_7            2
#define     WB_RB_LOW_7             12
#define     WB_RB_HIGH_7            22

//      WB_REG_16
#define     WB_GR_LOW_8             0
#define     WB_GR_HIGH_8            10
#define     WB_GB_LOW_8             20
#define     WB_GB_HIGH_8_L2         30

//      WB_REG_17
#define     WB_GB_HIGH_8_H8         0
#define     WB_RB_LOW_8             8
#define     WB_RB_HIGH_8            18
#define     WB_GR_LOW_9_L4          28

//      WB_REG_18
#define     WB_GR_LOW_9_H6          0
#define     WB_GR_HIGH_9            6
#define     WB_GB_LOW_9             16
#define     WB_GB_HIGH_9_L6         26

//      WB_REG_19
#define     WB_GB_HIGH_9_H4         0
#define     WB_RB_LOW_9             4
#define     WB_RB_HIGH_9            14
#define     WB_Y_LOW                24

//      WB_REG_20
#define     WB_Y_HIGH               0 
#define     WB_ERR_EST              8

//      WB_REG_21
#define     WB_G_WEIGHT_0           0
#define     WB_G_WEIGHT_1           4
#define     WB_G_WEIGHT_2           8
#define     WB_G_WEIGHT_3           12
#define     WB_G_WEIGHT_4           16
#define     WB_G_WEIGHT_5           20
#define     WB_G_WEIGHT_6           24
#define     WB_G_WEIGHT_7           28

//      WB_REG_22               
#define     WB_G_WEIGHT_8           0
#define     WB_G_WEIGHT_9           4
#define     WB_G_WEIGHT_10          8
#define     WB_G_WEIGHT_11          12
#define     WB_G_WEIGHT_12          16
#define     WB_G_WEIGHT_13          20
#define     WB_G_WEIGHT_14          24
#define     WB_G_WEIGHT_15          28

//      WB_REG_23              
#define     WBC_WR                  0
#define     WBC_WG                  16

//      WB_REG_24              
#define     WBC_WB                  0
#define     WBC_WR_OFFSET           16

//      WB_REG_25              
#define     WBC_WG_OFFSET           0
#define     WBC_WB_OFFSET           16

/*/////////////////////////////////////
Old 3DNR Para
//      TNR_REG_1               
#define     TNR_S_FILTER_Y_TH1      0
#define     TNR_S_FILTER_Y_TH2      10  
#define     TNR_T_FILTER_Y_K        20
#define     TNR_S_FILTER_Y_STEP     27

//      TNR_REG_2               
#define     TNR_S_FILTER_Y_K1       0
#define     TNR_S_FILTER_Y_K2       8   
#define     TNR_MC_SAD_UV_TH        16
#define     TUV_MIN_EN              29
#define     TNR_COMPRESS_FORMAT     30

//      TNR_REG_3           
#define     TNR_S_FILTER_UV_TH1     0
#define     TNR_S_FILTER_UV_TH2     10  
#define     TNR_T_FILTER_UV_K       20
#define     TNR_S_FILTER_UV_STEP    27

//      TNR_REG_4               
#define     TNR_S_FILTER_UV_K1      0
#define     TNR_S_FILTER_UV_K2      8   
#define     TNR_MC_VAR_Y_TH         16
#define     TNR_S_FILTER_VAR_GAIN   28

//      TNR_REG_5               
#define     TNR_S_FILTER_VAR_TH     0
#define     TNR_SKIN_VAR_TH         8   
#define     TNR_SKIN_MAX_TH         16
#define     TNR_SKIN_MIN_TH         24

//      TNR_REG_6               
#define     TNR_SKIN_V_MAX_TH       0
#define     TNR_SKIN_V_MIN_TH       8   
#define     TNR_SKIN_Y_MAX_TH       16
#define     TNR_SKIN_Y_MIN_TH       24

//      TNR_REG_7               
#define     TNR_SKIN_K              0
#define     TNR_MD_TH               8   
*/
/////////////////////////////////////


//      FADDR_REG_1             
#define     CH1_YADDR1              0
//      FADDR_REG_2             
#define     CH1_UADDR1              0
//      FADDR_REG_3             
#define     CH1_VADDR1              0
//      FADDR_REG_4             
#define     CH2_YADDR1              0
//      FADDR_REG_5             
#define     CH2_UADDR1              0
//      FADDR_REG_6             
#define     CH2_VADDR1              0
//      FADDR_REG_7             
#define     STATISTICS_ADDR1        0

//      ENABLE_REG              
#define     LEN_ADJUST_EN           0
#define     DDPC_EN                 1
#define     NR_EN                   2
#define     GB_EN                   3
#define     CC_EN                   4
#define     YGAMMA_UV_ADJUST_EN     5
#define     HDR_UV_ADJUST_EN        6
#define     HDR_EN                  7
#define     SE_EN                   8
#define     TNR_Y_EN                9
#define     TNR_UPDATA_REF_Y        10
#define     TNR_UV_EN               11
#define     TNR_UPDATA_REF_UV       12
#define     TNR_Y_2DNR_EN           13
#define     TNR_UV_2DNR_EN          14
#define     Y_DPC_EN                15
#define     YNR_EN                  16
#define     YUV_SHARPEN_SKIN_DETECT 17
#define     YUV_SHARPEN_EN          18
#define     FCS_EN                  19
#define     FCS_UV_NR_EN            20
#define     FCS_HUE_SAT_EN          21

#define     OSD1_EN                 22
#define     OSD2_EN                 23
#define     DARK_MARGIN_EN          24
#define     HFILP_EN                25
#define     VFILP_EN                26

#define     SDB3_UPLOAD_EN          28
#define     SDB4_UPLOAD_EN          29
#define     SDB5_UPLOAD_EN          30

#endif
