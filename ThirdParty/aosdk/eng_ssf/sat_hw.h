#ifndef _SAT_HW_H_
#define _SAT_HW_H_

extern unsigned char sat_ram[512*1024];

void sat_hw_init(void);

#if !LSB_FIRST
INLINE unsigned short mem_readword_swap(unsigned short *addr)
{
	return ((*addr&0x00ff)<<8)|((*addr&0xff00)>>8);
}

INLINE void mem_writeword_swap(unsigned short *addr, unsigned short value)
{
	*addr = ((value&0x00ff)<<8)|((value&0xff00)>>8);
}
#else	// big endian
INLINE unsigned short mem_readword_swap(unsigned short *addr)
{
	unsigned long retval;

	retval = *addr;

	return retval;
}

INLINE void mem_writeword_swap(unsigned short *addr, unsigned short value)
{
	*addr = value;
}
#endif


#endif
