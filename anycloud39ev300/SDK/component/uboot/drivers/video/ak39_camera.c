#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/anyka_cpu.h>
#include <asm/arch/sysctl.h>
#include <asm/arch/module_reset.h>


#define CLOCK_GATE_CTRL1		(0x0800001C)
#define AK_SHAREPIN_CON2		(0x08000078)

 /**
  * @brief reset camera 
  * @author xia_wenting  
  * @date 2010-12-06
  * @return void
  */
void camctrl_enable(void)
{
	sysctl_clock(CLOCK_CAMERA_ENABLE);
	/*
	注释下面这句的原因: 因为camera 的data0~data12在系统上电时就是这个配置，
	所以为了兼顾部分data引脚用作gpio的情况才注释掉。
	*/
	//gpio_pin_group_cfg(ePIN_AS_CAMERA);
	REG32(AK_SHAREPIN_CON2) &= ~(0xf);
	
	//reset camera interface
	sysctl_reset(AK_SRESET_CAMERA);
	
	//enbale PLL2
	//camctrl_open(24); 	  
}


 /**
 * @brief open camera, should be done the after reset camera to initialize 
 * @author xia_wenting  
 * @date 2010-12-06
 * @param[in] mclk send to camera mclk 
 * @return bool
 * @retval true if successed
 * @retval false if failed
 */
int camctrl_open(unsigned long mclk)
{
	unsigned long regval;
	unsigned long mclk_div;

	unsigned long peri_pll = get_peri_pll()/1000000;

	printf("peri_pll:%ld\n",peri_pll);
	//set mclk, the sensor present working 24MHz
	mclk_div = peri_pll/mclk - 1;
	
	regval = REG32(PERI_CLOCK_DIV_REG_2);
	regval &= ~(0x3f << 10);
	regval |= (mclk_div << 10);
	REG32(PERI_CLOCK_DIV_REG_2) = (1 << 19)|(1 << 18)|regval;	//not the same as PG

	// enable isp clock, pclk
	REG32(CLOCK_GATE_CTRL1) &= ~(1 << 19);

	REG32(PERI_CLOCK_DIV_REG_1) &= ~(1 << 25);

#if 0
	//alloc memory
	if (!isp.ispdma_addr)
	{
		isp.bytes = 1024;
	    isp.ispdma_addr = drv_malloc(isp.bytes);
	    if(NULL == isp.ispdma_addr)
	    {
	        akprintf(C1, M_DRVSYS, "malloc fail in isp dma\r\n");
	        return false;
	    }
		memset(isp.ispdma_addr, 0, isp.bytes);
	}

	init_isp_mode(&isp);
#endif
	return 0;
}

