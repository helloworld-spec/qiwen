/**
 * @FILENAME: analog.c
 * @BRIEF the source code of analog controller
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @DATE 2010-07-30
 * @VERSION 1.0
 */
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "drv_api.h"
#include "adc.h"
#include "dac.h"
#include "arch_analog.h"
#include "drv_module.h"
#include "hal_sound.h"
#include "l2.h"
#include "akos_api.h"

//#define SELECT_AIN_CHANNEL   //if define AIN0, not AIN1


#define LIN_GAIN_MAX    0xf
#define MIC_GAIN_MAX    0x7
#define HP_GAIN_MAX     0x6
#define ADC_MAIN_CLK    (12 * 1000000)

#define SIGNAL_PASS     0xff
#define POWER_ON        0x01
#define POWER_OFF       0x00

static bool m_adc1_init_flag = false;   //ADC1 init flag

static unsigned long  hp_close_tick = 0;

static unsigned long sina_value[] = 
{
    #include "sina_value.h"
};


static void pnmos_open(void)
{
    //Open Nmos    
    REG32(AUDIO_CODEC_CFG1_REG) &= ~(PD_HP);

    //open Pmos
    REG32(AUDIO_CODEC_CFG1_REG) &= ~(PD2_HP);    
}

typedef struct 
{
    unsigned short charge_sel;
    unsigned short delay_ms;
}T_VCM2_CHARGE;

#define TOTAL_DELAY  360
static void VCM2_charging(void)
{    
    static T_VCM2_CHARGE value[] = 
    {
     //   {0x1E , 5 },   //5uA
     //   {0x10 , 2},   //7uA
        {0x0F , 2},   //12uA
        {0x0E , 2},   //15uA
        {0x07 , 2},   //17uA
        {0x0C , 2},   //20uA
        {0x06 , 2},   //24uA
        {0x03 , 2},   //29uA
        {0x08 , 2},   //36uA    
        {0x04 , 180},   //42uA        
    //    {0x02 , 1},   //50uA        
    //    {0x01 , 1},   //57uA
        
    //    {0x00 , 50},   //200uA
        
    //    {0x01 , 5},   //57uA        
    //    {0x02 , 5},   //50uA
    //    {0x04 , 5},   //42uA
        {0x08 , 5},   //36uA
        {0x03 , 5},   //29uA        
        {0x06 , 5 },   //24uA
        {0x0C , 5 },   //20uA        
        {0x07 , 5 },   //17uA
        {0x0E , 6},   //15uA
        {0x0F , 7 },   //12uA
   //     {0x10 , 8 },   //7uA
   //     {0x1E , 10 },   //5uA    

   //     {0x03 , 0}     //29uA , default value      
    };

    unsigned long  reg_value;
    unsigned long  uCount;
    unsigned long  i;


    //set the bias the max for reduce the setup time
    REG32(AUDIO_CODEC_CFG3_REG) = 0x0;  
    
    
    //5uA
    //REG32(AUDIO_CODEC_CFG2_REG) = reg_value | (value[0].charge_sel << PTM_CHG);
    REG32(AUDIO_CODEC_CFG1_REG) &= ~(PL_VCM2);
    REG32(AUDIO_CODEC_CFG1_REG) &= ~(PD_VCM2);     
        
    mini_delay(value[0].delay_ms);

    uCount = value[0].delay_ms;


    if(uCount <= TOTAL_DELAY)
    {
        mini_delay(TOTAL_DELAY - uCount);
    }
    else
    {
        akprintf(C3 , M_DRVSYS, "Delay time too less\n");
    }

    //Set the charging current to default.
    REG32(AUDIO_CODEC_CFG2_REG) = reg_value | (0x03 << PTM_CHG);

    //set the the min for reduce the power    
    REG32(AUDIO_CODEC_CFG3_REG) = 0xFFFFFFFF;     
	
}


static void VCM2_discharging(void)
{
	REG32(AUDIO_CODEC_CFG1_REG) |= (0x1<<6);
	mini_delay(200);
	
	REG32(AUDIO_CODEC_CFG1_REG) |= (0x1<<7);
	mini_delay(200);
	
	REG32(AUDIO_CODEC_CFG1_REG) |= (0x1<<8);
	mini_delay(200);
	REG32(AUDIO_CODEC_CFG1_REG) |= (0x1<<9);

	mini_delay(200);

	REG32(AUDIO_CODEC_CFG1_REG) |= (0x1<<10);
	mini_delay(500);


}


static void dac_output_sina(void)
{        
    unsigned long       i;    
    SOUND_INFO  stInfo;
    unsigned long       *addr_l2;        
    unsigned long       *datebuf = sina_value;
    unsigned long       data_len = sizeof(sina_value)/sizeof(sina_value[0]);
    unsigned long       uCount;
    unsigned char        bufid;

    
    bufid = l2_alloc(ADDR_DAC);    
    if(BUF_NULL == bufid)
    {
        akprintf(C2 , M_DRVSYS, "l2_alloc fail\n");
        return ;
    }

    //wait for all the music data having sended to DAC
    uCount = 0;
    while((REG32(L2_STAT_REG1) & (0xF << (bufid * 4))) != 0)
    {
#ifdef AKOS
        AK_Sleep(1);   
#else        
        mini_delay(5);
#endif        
        if(++uCount > 200)
        {
            akprintf(C3 , M_DRVSYS, "L2 not clear3\n");
            uCount = 0;
        }
    }
    

    stInfo.BitsPerSample = 16;
    stInfo.nChannel      = 2;
    stInfo.nSampleRate   = 8000;

    if(!dac_setinfo(&stInfo))
    {
        akprintf(C3 , M_DRVSYS, "dac_setinfo fail\n");
    }


    addr_l2 = (unsigned long *)(L2_BUF_MEM_BASE_ADDR + bufid*512); 


    
    //not mute
    REG32(DAC_CONFIG_REG) &= ~MUTE; 
    
    uCount = 0;     
    for(i = 0; i< data_len ; ++i)
    {        
        addr_l2[i & 127] = datebuf[i];    

        if((i & 0xF) == 0)
        {
            while(((REG32(L2_STAT_REG1) & (0xF << (bufid * 4))) >> (bufid * 4)) > 6)
            {
                mini_delay(1);
                if(++uCount > 1000)
                {
                    akprintf(C3 , M_DRVSYS, "L2 not clear1\n");
                    uCount = 0;
                }
            }
        }          
    }

    for(; (i & 0xF) != 0; ++i)
    {
        addr_l2[i & 127] = 0x80008000;  
    }

    uCount = 0;
    while((REG32(L2_STAT_REG1) & (0xF << (bufid * 4))) != 0)
    {
        mini_delay(1);
        if(++uCount > 1000)
        {
            akprintf(C3 , M_DRVSYS, "L2 not clear2\n");
            uCount = 0;
        }
    }     

    //mute
    REG32(DAC_CONFIG_REG) |= MUTE; 
    
}


static bool setconnect(ANALOG_SIGNAL_INPUT analog_in, 
                ANALOG_SIGNAL_OUTPUT analog_out,ANALOG_SIGNAL_STATE state)
{   
    unsigned long reg_value;
    unsigned long hp_gain;

    
    if(SIGNAL_CONNECT == state)     //connect
    {
        if(analog_out & OUTPUT_ADC)
        {
			
			if(analog_in & INPUT_MIC)//MIC connect
			{
				
				reg_value = REG32(AUDIO_CODEC_CFG2_REG);
				reg_value &= (~(7UL<<29));
				reg_value |= (0x2UL<<29);
				REG32(AUDIO_CODEC_CFG2_REG)  = reg_value;
			}
			else
			{
				if(analog_in & INPUT_LINEIN)// LINEIN connect
				{
					reg_value = REG32(AUDIO_CODEC_CFG2_REG);
					reg_value &= (~(7UL<<29));
					reg_value |= (0x4UL<<29);
					REG32(AUDIO_CODEC_CFG2_REG)  = reg_value;
		
				}
			}
			  
		
		}
        if(analog_out & OUTPUT_HP)
        {            
			//reserve the original hp gain
			hp_gain =( REG32(AUDIO_CODEC_CFG1_REG) & (0x1f<<HP_GAIN) );
	
			//set the hp gain to -20db
			REG32(AUDIO_CODEC_CFG1_REG) |= (0x10<<HP_GAIN);

			//link the input to hp in
			reg_value = REG32(AUDIO_CODEC_CFG1_REG);
			reg_value &= (~(7<<HP_IN));
			reg_value |= (analog_in<<HP_IN);
			REG32(AUDIO_CODEC_CFG1_REG)  = reg_value;

			//resume the hp gain
			reg_value = REG32(AUDIO_CODEC_CFG1_REG);	
			reg_value &= (~(0x1f<<HP_GAIN));
			REG32(AUDIO_CODEC_CFG1_REG) |= hp_gain;
        }

        if(analog_out & OUTPUT_LINEOUT)
        {
            //do nothing
        }
        
    }
    else                            //disconnect
    {
        if(analog_out & OUTPUT_ADC)
        {
        	if(analog_in & INPUT_MIC)//MIC connect
            REG32(AUDIO_CODEC_CFG2_REG) &= ~(0x2UL << ADC2_IN);
			else
			{
				if(analog_in & INPUT_LINEIN)// LINEIN connect
				REG32(AUDIO_CODEC_CFG2_REG) &= ~(0x4UL << ADC2_IN);
			}
        }

        if(analog_out & OUTPUT_HP)
        {
            hp_gain =( REG32(AUDIO_CODEC_CFG1_REG) & (0x1f<<HP_GAIN) );

			//set the hp gain to -20db
			REG32(AUDIO_CODEC_CFG1_REG) &= (~(0x1f<<HP_GAIN));

			//cut the input to hp in
			REG32(AUDIO_CODEC_CFG1_REG) &= (~(analog_in<<HP_IN));

			//resume the hp gain
			reg_value = REG32(AUDIO_CODEC_CFG1_REG);	
			reg_value &= (~(0x1f<<18));
			REG32(AUDIO_CODEC_CFG1_REG) |= hp_gain;
        }

        if(analog_out & OUTPUT_LINEOUT)
        {
            //do nothing
        }
        
    }

    return true;
}


static void input_power_manage(ANALOG_SIGNAL_INPUT analog_in, 
                ANALOG_SIGNAL_OUTPUT analog_out,ANALOG_SIGNAL_STATE state)
{
    unsigned long reg_value;
    unsigned long hp_input;
    unsigned long adc2_input;
    unsigned long lineout_input;
    unsigned long input;
	
    
    if(SIGNAL_CONNECT == state)     //connect
    {
		
        if(analog_in & INPUT_DAC)
        {
            //do nothing, power on dac in dac_open()
            akprintf(C3 , M_DRVSYS, "DAC ON in dac_open\n");
        }

		if(analog_in & INPUT_LINEIN)
		{			 
			// power on the linein interface
			REG32(AUDIO_CODEC_CFG2_REG) &= (~PD_LINEIN);

            akprintf(C3 , M_DRVSYS, "LINEIN ON\n");
        }

        if(analog_in & INPUT_MIC)
        {                  
			
			reg_value = REG32(AUDIO_CODEC_CFG2_REG);
			reg_value |= (0x3<<19) ;
			reg_value &= (~(0x1<<19));			//mono
			//power on the mic 
			REG32(AUDIO_CODEC_CFG2_REG) = reg_value;
	
			akprintf(C3 , M_DRVSYS, "MIC ON\n");
        }
        
    }
    else                            //disconnect
    {
        hp_input        = (REG32(AUDIO_CODEC_CFG1_REG) >> HP_IN) & 0x07;
        adc2_input      = (REG32(AUDIO_CODEC_CFG2_REG) >> ADC2_IN) & 0x07;
        lineout_input   = 0;        //no lineout in AK37

        //analog_in is going to disconnect, so ignore it;
        if(analog_out & OUTPUT_ADC)
        {
            adc2_input &= ~analog_in;
        }

        if(analog_out & OUTPUT_HP)
        {         
            hp_input &= ~analog_in;
        }

        if(analog_out & OUTPUT_LINEOUT)
        {         
            lineout_input &= ~analog_in;
        }

        //analog_in does not connect output device, so to power off it.
        input = (hp_input | adc2_input | lineout_input);
        if((analog_in & INPUT_DAC) && (0 == (input & INPUT_DAC)))
        {
            //do nothing, power off dac in dac_close()
            
            akprintf(C3 , M_DRVSYS, "DAC OFF in dac_close\n");
        }

        if((analog_in & INPUT_LINEIN) &&(0 == (input & INPUT_LINEIN)))
        {
            // power off the linein interface
            REG32(AUDIO_CODEC_CFG2_REG) |= PD_LINEIN; 
            akprintf(C3 , M_DRVSYS, "LINEIN OFF\n");
        }

        if((analog_in & INPUT_MIC) && (0 == (input & INPUT_MIC)))
        {
            // power on mic interface
            REG32(AUDIO_CODEC_CFG2_REG) |= (PD_MICP | PD_MICN);
            akprintf(C3 , M_DRVSYS, "MIC OFF\n");
        }                
    }
}



#define MS_AFTER_CLOSE    200
static void output_power_manage(ANALOG_SIGNAL_INPUT analog_in, 
                ANALOG_SIGNAL_OUTPUT analog_out,ANALOG_SIGNAL_STATE state)
{
    unsigned long reg_value;
    unsigned long hp_input;
    unsigned long adc2_input;
    unsigned long lineout_input;
    unsigned long input;
    unsigned long hp_gain;
    unsigned long tmp;

    hp_input        = (REG32(AUDIO_CODEC_CFG1_REG) >> HP_IN) & 0x07;
    adc2_input      = (REG32(AUDIO_CODEC_CFG2_REG) >> ADC2_IN) & 0x07;
    lineout_input   = 0;        //no lineout in AK37

    //analog_in is going to connect or disconnect, so ignore it;
    if(analog_out & OUTPUT_ADC)
    {
        adc2_input &= ~analog_in;
    }

    if(analog_out & OUTPUT_HP)
    {         
        hp_input &= ~analog_in;
    }

    if(analog_out & OUTPUT_LINEOUT)
    {         
        lineout_input &= ~analog_in;
    }

    if(SIGNAL_CONNECT == state) //connect
    {
        //power on ADC2
        //因为所传入的参数analog_in是1，所以2 == adc2_input
        
        if((analog_out & OUTPUT_ADC) && (2 == adc2_input))
		{
			REG32( AUDIO_CODEC_CFG2_REG) &= (~(0x1<<26));
            akprintf(C3 , M_DRVSYS, "ADC2 ON\n");
        }
        
        //power on hp
        if((analog_out & OUTPUT_HP) && (0 == hp_input))
        { 
            //if hp has power on ,do nothing
            reg_value = REG32(AUDIO_CODEC_CFG1_REG);
            if(0 == (reg_value & (PD_HP | PD2_HP)))
            {
                akprintf(C2 , M_DRVSYS, "HP has been powered on\n");
                return ;
            }
	
			//open VREF1.5V
			REG32(AUDIO_CODEC_CFG2_REG) &= ~(0xF<<VREF_1_5V);
			
            //power on bias
            REG32(AUDIO_CODEC_CFG1_REG) &= ~(PD_BIAS); 
			
			//input DAC
            if(INPUT_DAC & analog_in)
            {
           	 	//Disable the pull-down 2Kohm resistor to VCM3
           		REG32(AUDIO_CODEC_CFG1_REG) &= ~(PL_VCM3);
          	  	
         	   	//Power on VCM3
         	   	REG32(AUDIO_CODEC_CFG1_REG) &= ~( PD_VCM3);             
          	}
         	//power on NMOS and PMOS
         	pnmos_open(); 
			//open VCM2
			REG32(AUDIO_CODEC_CFG1_REG) &= ~(Dischg_VCM2);
			if(INPUT_DAC & analog_in)	
            {
                //power on the integrator in DAC and DAC CLK, after VCM2 chareging fully
                REG32(AUDIO_CODEC_CFG1_REG) &= ~(PD_CK | PD_OP);
				//reset DAC output to middle volatge
                REG32(AUDIO_CODEC_CFG1_REG) |= RST_DAC_MID;  
				mini_delay(1000); //must  VCM2 ->2.5V,很重要，解决vcm2到达2.5V
                REG32(AUDIO_CODEC_CFG1_REG) &= ~RST_DAC_MID;              
            }
            
            akprintf(C3 , M_DRVSYS, "HP ON\n");
        }

        //power on lineout
        if((analog_out & OUTPUT_LINEOUT) && (0 == lineout_input))
        {
            //do nothing
        }
    }
    else                        //disconnect
    {
        //power off ADC2
         //因为所传入的参数analog_in是1，所以2 == adc2_input
        if((analog_out & OUTPUT_ADC) && (2 == adc2_input))
        {
            REG32(AUDIO_CODEC_CFG2_REG) |= PD_ADC2;
            akprintf(C3 , M_DRVSYS, "ADC2 OFF\n");
        }
        
        //power off hp
        if((analog_out & OUTPUT_HP) && (0 == hp_input))
        {
            //if hp has power off ,do nothing
            reg_value = REG32(AUDIO_CODEC_CFG1_REG);
            if((PD_HP | PD2_HP) == (reg_value & (PD_HP | PD2_HP)))
            {
                akprintf(C2 , M_DRVSYS, "HP is not power on\n");
                return ;
            }
			if(INPUT_DAC & analog_in)
			{
				//power offthe integrator in DAC and DAC CLK
				REG32(AUDIO_CODEC_CFG1_REG) |= (PD_CK | PD_OP);
			}
			//power off VCM2 and discharge it.After this, HP no need to discharge
			VCM2_discharging();

   
            //power off  bias
            REG32(AUDIO_CODEC_CFG1_REG) |= PD_BIAS; 
			mini_delay(500);
			
			//power off Pmos
			REG32(AUDIO_CODEC_CFG1_REG) |= PD2_HP;
			mini_delay(500);
			//power off Nmos
			REG32(AUDIO_CODEC_CFG1_REG) |= PD_HP;
			mini_delay(500);
			if(INPUT_DAC & analog_in)	
			{
				//pull-down 2Kohm resistor to VCM3
				REG32(AUDIO_CODEC_CFG1_REG) |= PL_VCM3;
				mini_delay(500);
				//power off VCM3
				REG32(AUDIO_CODEC_CFG1_REG) |= PD_VCM3;
			}
			//restore VREF
			REG32(AUDIO_CODEC_CFG2_REG) |= (1<<VREF_1_5V);

            akprintf(C3 , M_DRVSYS, "HP OFF\n");
        }
        
        //power off lineout
        if((analog_out & OUTPUT_LINEOUT) && (0 == lineout_input))
        {
            //do nothing
            akprintf(C3 , M_DRVSYS, "LINEOUT OFF\n");
        }
    }
    
}


/**
 * @brief   connect or disconnect the signal between input and output signal. 
 * @author  LianGenhui
 * @date    2010-06-30
 * @param[in] analog_in refer to ANALOG_SIGNAL_INPUT
 * @param[in] analog_out refer to ANALOG_SIGNAL_OUTPUT
 * @param[in] state SIGNAL_OPEN or SIGNAL_CLOSE
 * @return  bool
 * @retval  true  operation successful
 * @retval  false operation failed
 */
bool  analog_setsignal(ANALOG_SIGNAL_INPUT analog_in, 
                ANALOG_SIGNAL_OUTPUT analog_out, ANALOG_SIGNAL_STATE state)
{
    if((analog_in > INPUT_ALL) || (analog_in < INPUT_MIC) || 
       (analog_out > OUTPUT_ALL) || (analog_out < OUTPUT_ADC))
    {
        akprintf(C2, M_DRVSYS, "analog_in or analog_out is error\n");
        return false;
    }

    DrvModule_Protect(DRV_MODULE_DA); 

    if(SIGNAL_CONNECT == state)     //connect
    {
        input_power_manage(analog_in, analog_out, state);   
        //because linein interface occur a mistake state while vcm2 charging,
        //so do specially.
        if((INPUT_LINEIN & analog_in))
        {
            output_power_manage(analog_in, analog_out, state);            
            setconnect(analog_in, analog_out, state);        
        }
        else
        {            
            setconnect(analog_in, analog_out, state);
            output_power_manage(analog_in, analog_out, state);            
        }
    }
    else
    {
        output_power_manage(analog_in, analog_out, state);
        setconnect(analog_in, analog_out, state);
        input_power_manage(analog_in, analog_out, state);
    }

    DrvModule_UnProtect(DRV_MODULE_DA); 
    return true;    
}


/**
 * @brief   connect or disconnect the signal between input and output signal. 
 * @author  WangGuotian
 * @date    2012-05-14
 * @param[in] analog_in refer to ANALOG_SIGNAL_INPUT
 * @param[in] analog_out refer to ANALOG_SIGNAL_OUTPUT
 * @param[in] state SIGNAL_OPEN or SIGNAL_CLOSE
 * @return  bool
 * @retval  true  operation successful
 * @retval  false operation failed
 */
bool analog_setconnect(ANALOG_SIGNAL_INPUT analog_in, 
                ANALOG_SIGNAL_OUTPUT analog_out,ANALOG_SIGNAL_STATE state)
{
    if((analog_in > INPUT_ALL) || (analog_in < INPUT_DAC) || 
       (analog_out > OUTPUT_ALL) || (analog_out < OUTPUT_ADC))
    {
        akprintf(C2, M_DRVSYS, "analog_in or analog_out is error\n");
        return false;
    }

    DrvModule_Protect(DRV_MODULE_DA); 

    setconnect(analog_in, analog_out, state);

    DrvModule_UnProtect(DRV_MODULE_DA); 
    
    return true;
}




/**
 * @brief   get the signal connection state between input and output source
 * @author  WangGuotian
 * @date    2012-05-14
 * @param[in] analog_in refer to ANALOG_SIGNAL_INPUT
 * @param[in] analog_out refer to ANALOG_SIGNAL_OUTPUT 
 * @param[out] state SIGNAL_OPEN or SIGNAL_CLOSE
 * @return  bool
 * @retval  true  operation successful
 * @retval  false operation failed
 */
bool analog_getsignal(ANALOG_SIGNAL_INPUT analog_in, ANALOG_SIGNAL_OUTPUT analog_out, ANALOG_SIGNAL_STATE *state)
{
    unsigned long input;
    
    if(analog_in > 0x07)//0~0x07
    {
        akprintf(C2, M_DRVSYS, "signal input value > 7!\n");
        return false;
    }

    if(analog_out > 0x07)//0~0x07
    {
        akprintf(C2, M_DRVSYS, "signal output value > 7!\n");
        return false;
    }

    DrvModule_Protect(DRV_MODULE_DA); 

    *state = SIGNAL_DISCONNECT;

    if(OUTPUT_ADC == analog_out)
    {
        input = (REG32(ANALOG_CTRL_REG3) >> HP_IN) & 0x07;
    }
    else if(OUTPUT_HP == analog_out)
    {
        input = (REG32(AUDIO_CODEC_CFG2_REG) >> ADC2_IN) & 0x07;
    }
    else if(OUTPUT_LINEOUT == analog_out)
    {
        input = 0;
    }
    else
    {
        input = 0;
    }    

    if(input & analog_in)
    {
        *state = SIGNAL_CONNECT;
    } 

    DrvModule_UnProtect(DRV_MODULE_DA); 
    
    return true;
}


/**
 * @brief   set analog module channel to be MONO or STEREO
 * @author  LianGenhui
 * @date    2010-07-30
 * @param[in] module refer to ANALOG_CHANNEL
 * @param[in] state CHANNEL_MONO or CHANNEL_STEREO
 * @return  bool
 * @retval  true  operation successful
 * @retval  false operation failed
 */
bool    analog_setchannel(ANALOG_CHANNEL module, ANALOG_CHANNEL_STATE    state)
{
    akprintf(C3, M_DRVSYS, "not supply now\n");
    return false;
}

/**
 * @brief   get signal channel state, MONO or STEREO
 * @author  LianGenhui
 * @date    2010-07-30
 * @param[in]  module refer to ANALOG_CHANNEL
 * @param[out] state CHANNEL_MONO or CHANNEL_STEREO
 * @return  bool
 * @retval  true  operation successful
 * @retval  false operation failed
 */
bool analog_getchannel(ANALOG_CHANNEL module, ANALOG_CHANNEL_STATE *state)
{
    akprintf(C3, M_DRVSYS, "not supply now\n");
    return false;
}

/**
 * @brief   Set headphone gain,available for aspen3s later
 * @author  LianGenhui
 * @date    2010-07-30
 * @param[in] gain for normal mode, must be 0~8.0 for mute,1~8 for 0.1 time to 0.8 time
 * @return  bool
 * @retval  true  operation successful
 * @retval  false operation failed
 */
bool analog_setgain_hp (unsigned char gain)
{
    unsigned long reg_value;
    unsigned long gain_table[6] = {0x10, 0x08, 0x04, 0x02, 0x01, 0x00};

    if(gain >= HP_GAIN_MAX)
    {
        akprintf(C3, M_DRVSYS, "set gain bigger than %d\n", HP_GAIN_MAX);
        return false;
    }
    
    DrvModule_Protect(DRV_MODULE_DA);

    reg_value = REG32(AUDIO_CODEC_CFG1_REG);
    reg_value &= ~(0x1F << HP_GAIN);
    reg_value |= (gain_table[gain] << HP_GAIN);
    REG32(AUDIO_CODEC_CFG1_REG) = reg_value;

    DrvModule_UnProtect(DRV_MODULE_DA); 
    
    return true;
}

/**
 * @brief   Set headphone mute
 * @author Jiankui
 * @date    2017-03-20
 * @param[in] enable: 1:mute, 0 :volume
 * @return  bool
 * @retval  true  operation successful
 * @retval  false operation failed
 */
bool analog_setmute_hp(bool enable)
{
	unsigned long reg_value;
	DrvModule_Protect(DRV_MODULE_DA);

	reg_value = REG32(AUDIO_CODEC_CFG1_REG);
	if(enable)
	{
		reg_value &= (~(7<<HP_IN));
	}
	else
	{
		reg_value &= (~(7<<HP_IN));
		reg_value |= (INPUT_DAC<<HP_IN);
	}
    REG32(AUDIO_CODEC_CFG1_REG)  = reg_value;
	
    DrvModule_UnProtect(DRV_MODULE_DA); 
	return 0;
}
/**
 * @brief   Set mic gain
 * @author  LianGenhui
 * @date    2010-07-30
 * @param[in] gain must be 0~3,(aspen3s:0~7).
 * @return  bool
 * @retval  true  operation successful
 * @retval  false operation failed
 */
bool analog_setgain_mic(unsigned char gain)
{
    unsigned long reg_value;
    
    if(gain > MIC_GAIN_MAX)
    {
        akprintf(C3, M_DRVSYS, "set gain bigger than %d\n", MIC_GAIN_MAX);
        return false;
    }
    
    DrvModule_Protect(DRV_MODULE_DA); 

    reg_value = REG32(AUDIO_CODEC_CFG2_REG);

	reg_value &= (~((1<<18) | (0x7<<15)));
	reg_value |= ((gain<<15) | (0<<18));
	REG32(AUDIO_CODEC_CFG2_REG) = reg_value;
	
    DrvModule_UnProtect(DRV_MODULE_DA); 
    
    return true;
}

/**
 * @brief   Set linein gain
 * @author  LianGenhui
 * @date    2010-07-30
 * @param[in] gain must be 0~3,1 is 0db(aspen3s:0~15,6 is 0db)
 * @return  bool
 * @retval  true  operation successful
 * @retval  false operation failed
 */
bool analog_setgain_linein(unsigned char gain)
{
    unsigned long reg_value;
    if(gain > LIN_GAIN_MAX)
    {
        akprintf(C3, M_DRVSYS, "set gain bigger than %d\n", LIN_GAIN_MAX);
        return false;
    }

    DrvModule_Protect(DRV_MODULE_DA); 

	reg_value = REG32(AUDIO_CODEC_CFG2_REG);

	reg_value &= (~(0xf<<22));
	reg_value |= (gain<<22) ;
	REG32(AUDIO_CODEC_CFG2_REG) = reg_value;

    DrvModule_UnProtect(DRV_MODULE_DA); 
    
    return true;
}


void analog_adc1_init(unsigned long ADCClock, unsigned long SampleRate)
{
    unsigned long ClkDiv = 0;
    unsigned long bitcycle = 0;
    unsigned long WaitTime;
    unsigned long HoldTime;
    static unsigned long s_ADCClock, s_SampleRate;

    //if first init, save param setting
    if (!m_adc1_init_flag)
    {
        s_ADCClock = ADCClock;
        s_SampleRate = SampleRate;
    }

    //if setting changes, reinitial ADC1
    if (s_ADCClock != ADCClock || s_SampleRate != SampleRate)
    {
        m_adc1_init_flag = false;
        s_ADCClock = ADCClock;
        s_SampleRate = SampleRate;
    }        
    
    if(!m_adc1_init_flag)
    {
        akprintf(C3, M_DRVSYS, "init adc1 clock:%d, samplerate:%d\n", ADCClock, SampleRate);

		//reset saradc
		REG32(RESET_CTRL_REG) &= (~(1<<30));
		//mini_delay(100);//delay for reset
		//mini_delay(2);//delay for reset
		REG32(RESET_CTRL_REG) |= ((1<<30));
		//disable module chief driven by sar adc clk 
		REG32(ADC_SAMP_CTR_REG) &= (~(1<<0));
		//disable Ain0_sampling   Ain1_sampling   Bat_sampling
		REG32(ADC_SAMP_CTR_REG) &= (~(0x7<<5));
		//close sar adc clk
		REG32(ADC_CLK_DIV) &= (~(0x1<<3));
		
		//power on saradc 
		REG32(AUDIO_CODEC_CFG2_REG) &= (~(1<<0));
		//mini_delay(100);

		
        ClkDiv = (ADC_MAIN_CLK / ADCClock) - 1;
        ClkDiv &= 0x7;
		//cofig div 
		REG32(ADC_CLK_DIV) &= (~0xf);
		REG32(ADC_CLK_DIV) |= (ClkDiv);
		//mini_delay(100);
       

        /* because ADC1 is 5 channel multiplex*/
        SampleRate = SampleRate * 5;
        bitcycle = (unsigned long)(ADCClock / SampleRate);
        HoldTime = bitcycle - 1;
        WaitTime = bitcycle - 20;
        if (WaitTime > 0xff)
            WaitTime = 0xff;
            
        REG32(ADC_SAMP_RATE_REG) = (HoldTime << 16) | bitcycle;
		//mini_delay(100);
		REG32(ADC_SAMP_CTR_REG) |=  (WaitTime << 14);

      
    }

    //init over, keep it.
    m_adc1_init_flag = true;
}

/**
 * @brief get adc1 ad4 value. if input voltage from 0 to AVDD, it will return the value from 0 to 1023 
 * @author  Liangenhui 
 * @date 2010-07-30
 * @return unsigned long
 */
unsigned long analog_getvalue_bat(void)
{
    unsigned long ad4_value = 0;
    unsigned long count;
	

    DrvModule_Protect(DRV_MODULE_AD); 

    analog_adc1_init(1500000,  1000);
    

	//enable module chief driven by sar adc clk 
	REG32(ADC_SAMP_CTR_REG) |= ((1<<0));
	
	//open sar adc clk
	REG32(ADC_CLK_DIV) |= (0x1<<3);
	//mini_delay(2);
	//enable  Bat_sampling   
	REG32(ADC_SAMP_CTR_REG) |= ((1<<7));
	//mini_delay(100);
	

	//reset SAR ADC
	REG32(AUDIO_CODEC_CFG2_REG) &= (~(1<<1));
	REG32(AUDIO_CODEC_CFG2_REG) |= (1<<1);
	//use HPVDD(3.3)
	REG32(AUDIO_CODEC_CFG2_REG) &= (~(1<<3));
	//mini_delay(100);
	//enable VBAT div
	REG32(AUDIO_CODEC_CFG2_REG) |=(1<<7);//1/2ratio
		
    count = 5;
    //sample 5 times
    while ( count-- )
    {
        us_delay(1600);//delay for get next point ad4
        ad4_value += ((REG32(ADC_INIT_REG) >> 20) & 0xfff);//0xfff for 12 bit adc
    }

    //REG32(ANALOG_CTRL_REG1) &= ~(1<<3) | (1<<11);
    
    //disable ad4
    REG32(ADC_SAMP_CTR_REG) |= (~(1<<0));

    DrvModule_UnProtect(DRV_MODULE_AD); 

    return ad4_value / 5;
}

/**
 * @brief get adc1 ad5 value. if input voltage from 0 to AVDD, it will return the value from 0 to 1023 
 * @author  Liangenhui 
 * @date 2010-07-30
 * @return unsigned long
 */
unsigned long analog_getvalue_ain(void)
{
    unsigned long ad5_value = 0;
    unsigned long temp;
    unsigned long reg_data;
	unsigned long count;
    
    DrvModule_Protect(DRV_MODULE_AD); 
    
    //it set define if touch scheen not initial
    analog_adc1_init(4000000, 1000);

    //enable module chief driven by sar adc clk 
	REG32(ADC_SAMP_CTR_REG) |= ((1<<0));
	
	//open sar adc clk
	REG32(ADC_CLK_DIV) |= (0x1<<3);
	
	//reset SAR ADC
	REG32(AUDIO_CODEC_CFG2_REG) &= (~(1<<1));
	REG32(AUDIO_CODEC_CFG2_REG) |= (1<<1);
	//mini_delay(100);
	//use HPVDD(3.3)
	REG32(AUDIO_CODEC_CFG2_REG) &= (~(1<<3));

	
	
	//VREF_TEST_en
	//REG32(AUDIO_CODEC_CFG3_REG) |= (1<<31);
	
	//REG32(0x0800009c) &= (~(0x1f<<6));
	
	//mini_delay(100);

	#ifdef SELECT_AIN_CHANNEL
	{
		//enable AIN1WK
		REG32(AUDIO_CODEC_CFG2_REG) |= (1<<9);
		mini_delay(10);
		//enable  AIN1_sampling   
		REG32(ADC_SAMP_CTR_REG) |= ((1<<6));
		//mini_delay(100);
		
		count = 5;
    	//sample 5 times
    	while ( count-- )
    	{
        	us_delay(2500);//delay for get next point ad4
        	ad5_value += ((REG32(ADC_SAMP_DATA_REG)) & 0xfff);//0xfff for 12 bit adc
   		}
	}
	
	#else
	{
		//enable AIN0WK
		REG32(AUDIO_CODEC_CFG2_REG) |= (1<<8);
		
		mini_delay(10);
		//enable  AIN0_sampling   
		REG32(ADC_SAMP_CTR_REG) |= ((1<<5));
		//mini_delay(100);
				
		count = 5;
		//sample 5 times
		while ( count-- )
		{
	    	us_delay(2500);//delay for get next point ad4
	    	ad5_value += ((REG32(ADC_SAMP_DATA_REG)) & 0xfff);//0xfff for 12 bit adc
		}

	}

	#endif


	//disable ad
    REG32(ADC_SAMP_CTR_REG) |= (~(1<<0));
    
    DrvModule_UnProtect(DRV_MODULE_AD); 

    return ad5_value / 5;
}

/**
 * @brief set the mode of DAC analog voltage, it can be set to AC mode or DC mode
 * @author  Liangenhui 
 * @date 2010-07-30
 * @param[in] mode:must be MODE_AC or MODE_DC
 * @return bool  
 */
bool analog_setmode_voltage(ANALOG_VOLTAGE_MODE mode)
{
#if 0
    if(MODE_DC == mode)
    {
        REG32(TS_CONTROL_REG1) &= ~(1 << 22);
    }
    else if(MODE_AC == mode)
    {
        REG32(TS_CONTROL_REG1) |= (1 << 22);
    }
    else
    {
        return false;
    }
#endif
    return true;
}   


