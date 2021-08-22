/**@file  arch_analog.h
 * @brief  analog operation interface
 *
 * Copyright (C) 2010 Anyka (Guangzhou) Software Technology Co., LTD
 * @author  Liangenhui 
 * @date  2010-06-30
 * @version 1.0
 */

#ifndef         __ARCH_ALALOG_H
#define         __ARCH_ALALOG_H

/** @defgroup ANALOG analog control
 *  @ingroup Drv_Lib 
 */
/*@{*/
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief audio input module
 */
typedef enum
{
    INPUT_MIC = 1,       ///< = 1
    INPUT_LINEIN = 2,    ///< = 2
    INPUT_DAC = 4,       ///< = 4
    INPUT_ALL = 7        ///< = 7, select all three input signal
}ANALOG_SIGNAL_INPUT;

/**
 * @brief audio output module
 */
typedef enum
{
    OUTPUT_ADC = 1,       ///< = 1
    OUTPUT_HP = 2,        ///< = 2
    OUTPUT_LINEOUT = 4,   ///< = 4
    OUTPUT_ALL = 7        ///< = 7, select all three output signal
}ANALOG_SIGNAL_OUTPUT;

/**
 * @brief connect or disconnect the signal between input and output source
 */
typedef enum
{
    SIGNAL_CONNECT = 0,   ///< open one or all input signal to one or all output signal
    SIGNAL_DISCONNECT     ///< close input signal to output signal
}ANALOG_SIGNAL_STATE;

/**
 * @brief set a signal to be mono or stereo
 */
typedef enum
{
    ANALOG_HP = 0,        ///< if chose mono, use left channel
    ANALOG_LINEOUT,       ///< if chose mono, use left channel
    ANALOG_LINEIN,        ///< if chose mono, use left channel
    ANALOG_MIC            ///< only one channel
}ANALOG_CHANNEL;

/**
 * @brief mono or stereo
 */
typedef enum
{
    CHANNEL_MONO = 0,    ///< one channel(use left channel)
    CHANNEL_STEREO       ///< two channel.
}ANALOG_CHANNEL_STATE;

/**
 * @brief set mode of reference voltage generator
 */
typedef enum
{
    MODE_AC = 0,         ///< AC mode
    MODE_DC              ///< DC mode
}ANALOG_VOLTAGE_MODE;

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
bool analog_setsignal(ANALOG_SIGNAL_INPUT analog_in, ANALOG_SIGNAL_OUTPUT analog_out, ANALOG_SIGNAL_STATE    state);    

/**
 * @brief   connect or disconnect the signal between input and output signal. 
 * @author  LHD
 * @date    2011-08-16
 * @param[in] analog_in refer to ANALOG_SIGNAL_INPUT
 * @param[in] analog_out refer to ANALOG_SIGNAL_OUTPUT
 * @param[in] state SIGNAL_OPEN or SIGNAL_CLOSE
 * @return  bool
 * @retval  true  operation successful
 * @retval  false operation failed
 */
bool analog_setconnect(ANALOG_SIGNAL_INPUT analog_in, ANALOG_SIGNAL_OUTPUT analog_out,ANALOG_SIGNAL_STATE state);

/**
 * @brief   get the signal connection state between input and output source
 * @author  LianGenhui
 * @date    2010-06-30
 * @param[in] analog_in refer to ANALOG_SIGNAL_INPUT
 * @param[in] analog_out refer to ANALOG_SIGNAL_OUTPUT 
 * @param[out] state SIGNAL_OPEN or SIGNAL_CLOSE
 * @return  bool
 * @retval  true  operation successful
 * @retval  false operation failed
 */
bool analog_getsignal(ANALOG_SIGNAL_INPUT analog_in, ANALOG_SIGNAL_OUTPUT analog_out, ANALOG_SIGNAL_STATE *state);

/**
 * @brief   set analog module channel to be MONO or STEREO
 * @author  LianGenhui
 * @date    2010-06-30
 * @param[in] module refer to ANALOG_CHANNEL
 * @param[in] state CHANNEL_MONO or CHANNEL_STEREO
 * @return  bool
 * @retval  true  operation successful
 * @retval  false operation failed
 */
bool analog_setchannel(ANALOG_CHANNEL module, ANALOG_CHANNEL_STATE    state);    

/**
 * @brief   get signal channel state, MONO or STEREO
 * @author  LianGenhui
 * @date    2010-06-30
 * @param[in]  module refer to ANALOG_CHANNEL
 * @param[out] state CHANNEL_MONO or CHANNEL_STEREO
 * @return  bool
 * @retval  true  operation successful
 * @retval  false operation connection failed
 */
bool analog_getchannel(ANALOG_CHANNEL module, ANALOG_CHANNEL_STATE *state);    

/**
 * @brief   Set headphone gain,available for aspen3s later
 * @author  LianGenhui
 * @date    2010-07-30
 * @param[in] gain for normal mode, 
            AK98XX: must be 0~8.0 for mute,1~8 for 0.1 time to 0.8 time
            AK37XX: must be 0~5, 5 is max gain
 * @return  bool
 * @retval  true  operation successful
 * @retval  false operation failed
 */
bool analog_setgain_hp (unsigned char gain);

/**
 * @brief   Set headphone mute
 * @author Jiankui
 * @date    2017-03-20
 * @param[in] enable: 1:mute, 0 :volume
 * @return  bool
 * @retval  true  operation successful
 * @retval  false operation failed
 */
bool analog_setmute_hp(bool enable);

/**
 * @brief   Set mic gain
 * @author  LianGenhui
 * @date    2010-06-30
 * @param[in] gain must be 0~3,(aspen3s:0~7).
 * @return  bool
 * @retval  true operation successful
 * @retval  false operation failed
 */
bool analog_setgain_mic(unsigned char gain);

/**
 * @brief   Set linein gain
 * @author  LianGenhui
 * @date    2010-06-30
 * @param[in] gain must be 0~3,1 is 0db(aspen3s:0~15,6 is 0db)
 * @return  bool
 * @retval  true operation successful
 * @retval  false operation failed
 */
bool analog_setgain_linein(unsigned char gain);

/**
 * @brief get adc1 ad4 value. if input voltage from 0 to AVDD, it will return the value from 0 to 1023 
 * @author  Liangenhui 
 * @date 2010-06-30
 * @return unsigned long
 */
unsigned long analog_getvalue_bat(void);

/**
 * @brief get adc1 ad5 value. if input voltage from 0 to AVDD, it will return the value from 0 to 1023 
 * @author  Liangenhui 
 * @date 2010-06-30
 * @return unsigned long
 */
unsigned long analog_getvalue_ain(void );

/**
 * @brief set the mode of DAC analog voltage, it can be set to AC mode or DC mode
 * @author  Liangenhui 
 * @date 2010-06-30
 * @param[in] mode must be MODE_AC or MODE_DC
 * @return bool
 */
bool analog_setmode_voltage(ANALOG_VOLTAGE_MODE mode);

#ifdef __cplusplus
}
#endif

/*@}*/

#endif//__ALALOG_H__

