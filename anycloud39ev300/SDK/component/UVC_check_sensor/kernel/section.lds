OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	.text 0x80000000: {		        /* Real text segment		*/
		_text = .;		/* Text and read-only data	*/
		*(.text)
		. = ALIGN(4);
		_etext = .;		/* End of text section		*/
	}
	. = ALIGN(4);
	.rel.plt        :
     	 {
         *(.rel.plt)
      	}
      	.rela.plt       :
      	{
          *(.rela.plt)
     	 }
      	. = ALIGN(4);

	.rodata :
	{
		__rodata_start = .;
		*(.rodata)
		*(.rodata*)
		. = ALIGN(4);
		__rodata_end = .;
	}
	.ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) }
	PROVIDE_HIDDEN (__exidx_start = .);
	.ARM.exidx   : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) }
	PROVIDE_HIDDEN (__exidx_end = .);
	. = ALIGN(4);
	.preinit_array :
 	{
     	 PROVIDE_HIDDEN (__preinit_array_start = .);
		 KEEP (*(.preinit_array))
		 PROVIDE_HIDDEN (__preinit_array_end = .);
    }
    .init_array :
	{
	    PROVIDE_HIDDEN (__init_array_start = .);
	    KEEP (*(SORT(.init_array.*)))
	    KEEP (*(.init_array ))
	    PROVIDE_HIDDEN (__init_array_end = .);
    }
    .fini_array :
    {
     	PROVIDE_HIDDEN (__fini_array_start = .);
    	KEEP (*(SORT(.fini_array.*)))
       	KEEP (*(.fini_array ))
       	PROVIDE_HIDDEN (__fini_array_end = .);
    }
	. = ALIGN(4);
	.data : {
		__data_start = .;
		*(.data)
		. = ALIGN(4);
		_edata = .;
	}
	. = ALIGN(4);
    .initcall : {
	    __initcall_start = .;
		*(.initcall)
	    . = ALIGN(4);
		__initcall_end = .;
    }
    
	. = ALIGN(4);
    .dev_initcall : {
	    __dev_initcall_start = .;
		*(.dev_initcall)
	    . = ALIGN(4);
		__dev_initcall_end = .;
    }  
  
    
	. = ALIGN(4);
    .cmd_initcall : {
	    __cmd_initcall_start = .;
		*(.cmd_initcall)
	    . = ALIGN(4);
		__cmd_initcall_end = .;
    }
    
	. = ALIGN(4);
    .lock_init : {
	    __lock_init_start = .;
		*(.lock_init)
	    . = ALIGN(4);
		__lock_init_end = .;
    }
    
	. = ALIGN(4);
	.bss :
	{
		__bss_start = .;
		*(.bss) *(COMMON)
		. = ALIGN(4);
		__bss_end = .;
	}
	
}
