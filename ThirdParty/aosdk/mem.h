//
// Audio Overload
// Emulated music player
//
// (C) 2000-2008 Richard F. Bannister
//

// mem.h

uint8 memory_read(uint16 addr);
uint8 memory_readop(uint16 addr);
uint8 memory_readport(uint16 addr);
void memory_write(uint16 addr, uint8 byte);
void memory_writeport(uint16 addr, uint8 byte);
