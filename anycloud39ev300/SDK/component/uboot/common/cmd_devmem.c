#include <common.h>
#include <command.h>
#include <asm/arch-ak39/anyka_cpu.h>

int do_devmem(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long addr = simple_strtol(argv[1], NULL, 16);
    void __iomem *base = (void *)addr;
    unsigned long value = 0;
    
    value = REG32(base);
	printf("value is 0x%08x, base 0x%08x\n", value, base);
}

U_BOOT_CMD(
	devmem, 3, 1, do_devmem,
	"read or write register, now just read",
	"    - devmem addr [witdh:value]"
);
