/*
 * (C) Copyright 2003
 * Anyka Ltd.
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/anyka_cpu.h>


DECLARE_GLOBAL_DATA_PTR;

#define AK39_TIMER_CTRL1		TIMER1_CTRL1_REG
#define AK39_TIMER_CTRL2		TIMER1_CTRL2_REG

#define TIMER_USEC_SHIFT		16
#define TIMER_CNT_MASK			(0x3F<<26)

//define timer register bits
#define TIMER_CLEAR_BIT			(1<<30)
#define TIMER_FEED_BIT			(1<<29)
#define TIMER_ENABLE_BIT		(1<<28)
#define TIMER_STATUS_BIT		(1<<27)
#define TIMER_READ_SEL_BIT		(1<<26)

//define pwm/pwm mode
#define MODE_AUTO_RELOAD_TIMER	0x0
#define MODE_ONE_SHOT_TIMER		0x1
#define MODE_PWM				0x2   

#define timestamp gd->arch.tbl
#define lastdec gd->arch.lastinc

#define TIMER_SCALER 	1
#define TIMER_CLK_FREQ 	(CONFIG_SYS_CLK_FREQ/TIMER_SCALER)
#define TIMER_LOAD_VAL 	(TIMER_CLK_FREQ/CONFIG_SYS_HZ)


int timer_init(void)
{
	unsigned long regval;
	unsigned long timecnt = TIMER_LOAD_VAL; // cdh:12ms

	regval = (TIMER_ENABLE_BIT | TIMER_FEED_BIT | (MODE_AUTO_RELOAD_TIMER << 24));
	regval |= (TIMER_SCALER-1)<<16; /*pre divider*/
	writel(timecnt, AK39_TIMER_CTRL1);
	writel(regval, AK39_TIMER_CTRL2);
	return 0;
}

/*
 * timer without interrupts
 */
ulong get_timer (ulong base)
{
	return get_timer_masked() - base;
}

void reset_timer_masked(void)
{
	unsigned long regval;

	regval = readl(AK39_TIMER_CTRL2);
	writel(regval | TIMER_READ_SEL_BIT, AK39_TIMER_CTRL2);

	/* reset time */
	regval = readl(AK39_TIMER_CTRL1);
	lastdec = TIMER_LOAD_VAL - regval;
	timestamp = 0;	       /* start "advancing" time stamp from 0 */
}

unsigned long long get_ticks(void)
{
	unsigned long regval;
	ulong now;

	regval = __raw_readl(AK39_TIMER_CTRL2);
	__raw_writel(regval | TIMER_READ_SEL_BIT, AK39_TIMER_CTRL2);

	now = __raw_readl(AK39_TIMER_CTRL1);

	if (lastdec >= now) {		/* normal mode (non roll) */
		/* normal mode */
		timestamp += lastdec - now; /* move stamp fordward with absoulte diff ticks */
	} else {			/* we have overflow of the count down timer */
		/* nts = ts + ld + (TLV - now)
		 * ts=old stamp, ld=time that passed before passing through -1
		 * (TLV-now) amount of time after passing though -1
		 * nts = new "advancing time stamp"...it could also roll and cause problems.
		 */
		timestamp += lastdec + TIMER_LOAD_VAL - now;
	}
	lastdec = now;

	return timestamp;
}


ulong get_timer_masked(void)
{
	return get_ticks()/TIMER_LOAD_VAL;
}


/* waits specified delay value and resets timestamp */
void udelay_masked (unsigned long usec)
{
	u32 tmo;
	u32 endtime;
	signed long diff;

	tmo = TIMER_CLK_FREQ / 1000;
	tmo *= usec;
	tmo /= 1000;

	endtime = get_ticks() + tmo;

	do {
		u32 now = get_ticks();
		diff = endtime - now;
	} while (diff >= 0);
}


/* delay x useconds AND preserve advance timestamp value */
void __udelay (unsigned long usec)
{
	udelay_masked(usec);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk (void)
{
	ulong tbclk;

	tbclk = CONFIG_SYS_HZ;
	return tbclk;
}


