/////////////////////////////////////////////////////////////////////////////
// cpsr/spsr
#define Mode_USR		0x10
#define Mode_FIQ		0x11
#define Mode_IRQ		0x12
#define Mode_SVC		0x13
#define Mode_ABT		0x17
#define Mode_UNDEF		0x1B
#define Mode_SYS		0x1F		
#define I_Bit			0x80
#define F_Bit			0x40

//////////////////////////////////////////////////////////////////////////////


// raw read/write
//#define HAL_READ_UINT8( _register_, _value_ )        ((_value_) = *((volatile char *)(_register_)))

//#define HAL_WRITE_UINT8( _register_, _value_ )       (*((volatile  char *)(_register_)) = (_value_))
       
//#define HAL_READ_UINT16( _register_, _value_ )      ((_value_) = *((volatile unsigned short *)(_register_)))

//#define HAL_WRITE_UINT16( _register_, _value_ )     (*((volatile  unsigned short *)(_register_)) = (_value_))
       
//#define HAL_READ_UINT32( _register_, _value_ )      ((_value_) = *((volatile unsigned long *)(_register_)))

//#define HAL_WRITE_UINT32( _register_, _value_ )     (*((volatile  unsigned long *)(_register_)) = (_value_))

/*/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////&*/
// end
