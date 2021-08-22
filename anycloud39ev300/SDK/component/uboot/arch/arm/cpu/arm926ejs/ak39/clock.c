#include <asm/io.h>
#include <asm/arch/anyka_cpu.h>

#define PLL_CLK_MIN	180

// cdh:check ok
unsigned long get_asic_pll_clk(void)
{
	unsigned long pll_m, pll_n, pll_od;
	unsigned long asic_pll_clk;
	unsigned long regval;

	regval = readl(CLK_ASIC_PLL_CTRL);
	pll_od = (regval & (0x3 << 12)) >> 12;
	pll_n = (regval & (0xf << 8)) >> 8;
	pll_m = regval & 0xff;

	asic_pll_clk = (12 * pll_m)/(pll_n * (1 << pll_od));

	if ((pll_od >= 1) && ((pll_n >= 2) && (pll_n <= 6)) 
		&& ((pll_m >= 84) && (pll_m <= 254)))
		return asic_pll_clk;

	return 0;
}

// cdh:check ok
unsigned long get_vclk(void)
{
	unsigned long regval;
	unsigned long div;
	
	regval = readl(CLK_ASIC_PLL_CTRL);
	div = (regval & (0x7 << 17)) >> 17;
	if (div == 0)
		regval = get_asic_pll_clk() >> 1;
	else
		regval = get_asic_pll_clk() >> div;

	return regval;
}

// cdh:check ok
unsigned long get_asic_freq(void)
{
	unsigned long regval;
	unsigned long div;

	regval = readl(CLK_ASIC_PLL_CTRL);
	div = regval & (1 << 24); // cdh:check vclk
	if (div == 0)
		regval =  get_vclk() * 1000000;
	else
		regval = (get_vclk() >> 1) * 1000000;

	return regval;
}


