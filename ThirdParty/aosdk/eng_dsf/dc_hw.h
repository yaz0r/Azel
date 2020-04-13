#ifndef _DC_HW_H_
#define _DC_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

extern uint8 dc_ram[8*1024*1024];

void dc_hw_init(void);

uint8 dc_read8(uint32 addr);
uint16 dc_read16(uint32 addr);
uint32 dc_read32(uint32 addr);
void dc_write8(uint32 addr, uint8 byte);
void dc_write16(uint32 addr, uint16 word);
void dc_write32(uint32 addr, uint32 dword);

#ifdef __cplusplus
}
#endif

#endif

