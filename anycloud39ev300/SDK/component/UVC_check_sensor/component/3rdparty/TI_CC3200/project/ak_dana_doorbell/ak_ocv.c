#include "ak_ocv.h"
#include "i2c_if.h"

#ifndef NOTERM
#include "uart_if.h"
#endif


#define DEV_ADDR  0x34
#define SUCCESS 	0

typedef unsigned short	uint16_t;
typedef unsigned char	uint8_t;

#define RET_IF_ERR(Func)          {int iRetVal = (Func); \
                                   if (SUCCESS != iRetVal) \
                                     return  iRetVal;}


 static  int axp_vbat_to_mV(uint16_t reg)
{
	return (reg) * 1100 / 1000;
}


static int axp_ibat_to_mA(uint16_t reg)
{
	return (reg) * 500 / 1000;
}



 static int axp_bat_vol(uint8_t dir,int vol,int cur,int rdc)
{
	if(dir)
		return (vol-cur*rdc/1000);
	else
		return (vol+cur*rdc/1000);
}


static uint8_t axp_vol2cap(int ocv)
{
    if(ocv >= OCVVOLF)
    {
        return OCVREGF;
    }
    else if(ocv < OCVVOL0)
    {
        return OCVREG0;
    }
    else if(ocv < OCVVOL1)
    {
        return OCVREG0 + (OCVREG1 - OCVREG0) * (ocv - OCVVOL0) / (OCVVOL1 - OCVVOL0);
    }
    else if(ocv < OCVVOL2)
    {
        return OCVREG1 + (OCVREG2 - OCVREG1) * (ocv - OCVVOL1) / (OCVVOL2 - OCVVOL1);
    }
    else if(ocv < OCVVOL3)
    {
        return OCVREG2 + (OCVREG3 - OCVREG2) * (ocv - OCVVOL2) / (OCVVOL3 - OCVVOL2);
    }
    else if(ocv < OCVVOL4)
    {
        return OCVREG3 + (OCVREG4 - OCVREG3) * (ocv - OCVVOL3) / (OCVVOL4 - OCVVOL3);
    }
    else if(ocv < OCVVOL5)
    {
        return OCVREG4 + (OCVREG5 - OCVREG4) * (ocv - OCVVOL4) / (OCVVOL5 - OCVVOL4);
    }
    else if(ocv < OCVVOL6)                               
    {
        return OCVREG5 + (OCVREG6 - OCVREG5) * (ocv - OCVVOL5) / (OCVVOL6 - OCVVOL5);
    }
    else if(ocv < OCVVOL7)
    {
        return OCVREG6 + (OCVREG7 - OCVREG6) * (ocv - OCVVOL6) / (OCVVOL7 - OCVVOL6);
    }
    else if(ocv < OCVVOL8)
    {
        return OCVREG7 + (OCVREG8 - OCVREG7) * (ocv - OCVVOL7) / (OCVVOL8 - OCVVOL7);
    }
    else if(ocv < OCVVOL9)
    {
        return OCVREG8 + (OCVREG9 - OCVREG8) * (ocv - OCVVOL8) / (OCVVOL9 - OCVVOL8);
    }
    else if(ocv < OCVVOLA)
    {
        return OCVREG9 + (OCVREGA - OCVREG9) * (ocv - OCVVOL9) / (OCVVOLA - OCVVOL9);
    }
    else if(ocv < OCVVOLB)
    {
        return OCVREGA + (OCVREGB - OCVREGA) * (ocv - OCVVOLA) / (OCVVOLB - OCVVOLA);
    }
    else if(ocv < OCVVOLC)
    {
        return OCVREGB + (OCVREGC - OCVREGB) * (ocv - OCVVOLB) / (OCVVOLC - OCVVOLB);
    }
    else if(ocv < OCVVOLD)
    {
        return OCVREGC + (OCVREGD - OCVREGC) * (ocv - OCVVOLC) / (OCVVOLD - OCVVOLC);
    }
    else if(ocv < OCVVOLE)
    {
        return OCVREGD + (OCVREGE - OCVREGD) * (ocv - OCVVOLD) / (OCVVOLE - OCVVOLD);
    }
    else if(ocv < OCVVOLF)
    {
        return OCVREGE + (OCVREGF - OCVREGE) * (ocv - OCVVOLE) / (OCVVOLF - OCVVOLE);
    }
    else
    {
        return 0;
    }
}







static int axp_read(unsigned char regaddr, unsigned char *data, unsigned char len)
{
	//
    // Write the register address to be read from.
    // Stop bit implicitly assumed to be 0.
    //
    RET_IF_ERR(I2C_IF_Write(DEV_ADDR,&regaddr,1,0));

	
    //
    // Read the specified length of data
    //
    RET_IF_ERR(I2C_IF_Read(DEV_ADDR, data, len));

    Report("I2C Read From address complete\n\r");
	return 0;
}



 //电量计量函数，返回值为电量
 unsigned char axp_ocv_restcap(void)
{
	 int ocv ,vbat,ibat ;
	 unsigned char val[2] , bat_current_direction; 
     unsigned short tmp[2];

	I2C_IF_Open(I2C_MASTER_MODE_FST);
		
//	 axp_read(0x00,val[0]);	 //	读00寄存器
	if(0 > axp_read(0x00, val, 1))
	{
		Report("read 0x00 error!\r\n");
		return -1;
	}
	bat_current_direction=	val[0] &&(1<<2) ;//取00寄存器的bit2

	
	  //	axp_reads(0x78,2,val);	 //	读78,79寄存器 ,计算电池电压
	 if(0 > axp_read(0x78, val, 2))
	{
		Report("read 0x78 error!\r\n");
		return -1;
	}
	tmp[0]=((uint16_t)val[0] << 4 )| (val[1] & 0x0f);	  //adc->vbat_res
	vbat=  axp_vbat_to_mV(tmp[0]);




	//	 axp_reads(0x7A,2,val);//读7A 7B 寄存器计算充电电流
	if(0 > axp_read(0x7A, val, 2))
	{
		Report("read 0x7A error!\r\n");
		return -1;
	}
	tmp[0]=((uint16_t) val[0] << 5 )| (val[1] & 0x1f);//adc->ichar_res
	
	//	 axp_reads(0x7C,2,val);	//读7C 7D 寄存器计算放电电流
	if(0 > axp_read(0x7C, val, 2))
	{
			Report("read 0x7C error!\r\n");
			return -1;
	}
	tmp[1]=((uint16_t) val[0] << 5 )| (val[1] & 0x1f);//adc->idischar_res
	
	ibat= ABS(axp_ibat_to_mA(tmp[0])-axp_ibat_to_mA(tmp[1]));

	ocv = axp_bat_vol(bat_current_direction,vbat,ibat,100);
	
	I2C_IF_Close();
	Report("dir:%d vbat:%d IA:%x OA:%x ibat:%d, ocv:%d\r\n",bat_current_direction, vbat, tmp[0], tmp[1], ibat, ocv);
	return axp_vol2cap(ocv);
}

