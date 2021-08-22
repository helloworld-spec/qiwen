/*
 * Copyright (c) 2017 iComm Semiconductor Ltd.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
struct padpd_table padpd_am_table[]=
                    {{ADR_WIFI_IQ_CAL_DPD_GAIN_REG0,0xfffffc00,0xfc00ffff}, 
                    {ADR_WIFI_IQ_CAL_DPD_GAIN_REG1,0xfffffc00,0xfc00ffff}, 
                    {ADR_WIFI_IQ_CAL_DPD_GAIN_REG2,0xfffffc00,0xfc00ffff}, 
                    {ADR_WIFI_IQ_CAL_DPD_GAIN_REG3,0xfffffc00,0xfc00ffff}, 
                    {ADR_WIFI_IQ_CAL_DPD_GAIN_REG4,0xfffffc00,0xfc00ffff}, 
                    {ADR_WIFI_IQ_CAL_DPD_GAIN_REG5,0xfffffc00,0xfc00ffff}, 
                    {ADR_WIFI_IQ_CAL_DPD_GAIN_REG6,0xfffffc00,0xfc00ffff}, 
                    {ADR_WIFI_IQ_CAL_DPD_GAIN_REG7,0xfffffc00,0xfc00ffff}, 
                    {ADR_WIFI_IQ_CAL_DPD_GAIN_REG8,0xfffffc00,0xfc00ffff}, 
                    {ADR_WIFI_IQ_CAL_DPD_GAIN_REG9,0xfffffc00,0xfc00ffff}, 
                    {ADR_WIFI_IQ_CAL_DPD_GAIN_REGA,0xfffffc00,0xfc00ffff}, 
                    {ADR_WIFI_IQ_CAL_DPD_GAIN_REGB,0xfffffc00,0xfc00ffff}, 
                    {ADR_WIFI_IQ_CAL_DPD_GAIN_REGC,0xfffffc00,0xfc00ffff}};
                        
struct padpd_table padpd_pm_table[]=
                    {{ADR_WIFI_IQ_CAL_DPD_PHASE_REG0,0xffffe000,0xe000ffff},
                    {ADR_WIFI_IQ_CAL_DPD_PHASE_REG1,0xffffe000,0xe000ffff},
                    {ADR_WIFI_IQ_CAL_DPD_PHASE_REG2,0xffffe000,0xe000ffff},
                    {ADR_WIFI_IQ_CAL_DPD_PHASE_REG3,0xffffe000,0xe000ffff},
                    {ADR_WIFI_IQ_CAL_DPD_PHASE_REG4,0xffffe000,0xe000ffff},
                    {ADR_WIFI_IQ_CAL_DPD_PHASE_REG5,0xffffe000,0xe000ffff},
                    {ADR_WIFI_IQ_CAL_DPD_PHASE_REG6,0xffffe000,0xe000ffff},
                    {ADR_WIFI_IQ_CAL_DPD_PHASE_REG7,0xffffe000,0xe000ffff},
                    {ADR_WIFI_IQ_CAL_DPD_PHASE_REG8,0xffffe000,0xe000ffff},
                    {ADR_WIFI_IQ_CAL_DPD_PHASE_REG9,0xffffe000,0xe000ffff},
                    {ADR_WIFI_IQ_CAL_DPD_PHASE_REGA,0xffffe000,0xe000ffff},
                    {ADR_WIFI_IQ_CAL_DPD_PHASE_REGB,0xffffe000,0xe000ffff},
                    {ADR_WIFI_IQ_CAL_DPD_PHASE_REGC,0xffffe000,0xe000ffff}};  