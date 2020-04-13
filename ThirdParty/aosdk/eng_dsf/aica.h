/*

	Sega/Yamaha AICA emulation
*/

#ifndef _AICA_H_
#define _AICA_H_

#define MAX_AICA	(2)

#define COMBINE_DATA(varptr)	(*(varptr) = (*(varptr) & mem_mask) | (data & ~mem_mask))

// convert AO types
typedef int8 data8_t;
typedef int16 data16_t;
typedef int32 data32_t;
typedef int offs_t;

/*
    AICA features 64 programmable slots
    that can generate PCM and ADPCM (from ROM/RAM) sound
*/

//SLOT PARAMETERS
#define KEYONEX(slot)		((slot->udata.data[0x0]>>0x0)&0x8000)
#define KEYONB(slot)		((slot->udata.data[0x0]>>0x0)&0x4000)
#define SSCTL(slot)		((slot->udata.data[0x0]>>0xA)&0x0001)
#define LPCTL(slot)		((slot->udata.data[0x0]>>0x9)&0x0001)
#define PCMS(slot)		((slot->udata.data[0x0]>>0x7)&0x0003)

#define SA(slot)		(((slot->udata.data[0x0]&0x7F)<<16)|(slot->udata.data[0x4/2]))

#define LSA(slot)		(slot->udata.data[0x8/2])

#define LEA(slot)		(slot->udata.data[0xc/2])

#define D2R(slot)		((slot->udata.data[0x10/2]>>0xB)&0x001F)
#define D1R(slot)		((slot->udata.data[0x10/2]>>0x6)&0x001F)
#define AR(slot)		((slot->udata.data[0x10/2]>>0x0)&0x001F)

#define LPSLNK(slot)		((slot->udata.data[0x14/2]>>0x0)&0x4000)
#define KRS(slot)		((slot->udata.data[0x14/2]>>0xA)&0x000F)
#define DL(slot)		((slot->udata.data[0x14/2]>>0x5)&0x001F)
#define RR(slot)		((slot->udata.data[0x14/2]>>0x0)&0x001F)

#define TL(slot)		((slot->udata.data[0x28/2]>>0x8)&0x00FF)

#define OCT(slot)		((slot->udata.data[0x18/2]>>0xB)&0x000F)
#define FNS(slot)		((slot->udata.data[0x18/2]>>0x0)&0x03FF)

#define LFORE(slot)		((slot->udata.data[0x1c/2]>>0x0)&0x8000)
#define LFOF(slot)		((slot->udata.data[0x1c/2]>>0xA)&0x001F)
#define PLFOWS(slot)		((slot->udata.data[0x1c/2]>>0x8)&0x0003)
#define PLFOS(slot)		((slot->udata.data[0x1c/2]>>0x5)&0x0007)
#define ALFOWS(slot)		((slot->udata.data[0x1c/2]>>0x3)&0x0003)
#define ALFOS(slot)		((slot->udata.data[0x1c/2]>>0x0)&0x0007)

#define ISEL(slot)		((slot->udata.data[0x20/2]>>0x0)&0x000F)
#define IMXL(slot)		((slot->udata.data[0x20/2]>>0x4)&0x000F)

#define DISDL(slot)		((slot->udata.data[0x24/2]>>0x8)&0x000F)
#define DIPAN(slot)		((slot->udata.data[0x24/2]>>0x0)&0x001F)

#define EFSDL(slot)		((AICA->EFSPAN[slot*4]>>8)&0x000f)
#define EFPAN(slot)		((AICA->EFSPAN[slot*4]>>0)&0x001f)

INT32 EG_TABLE[0x400];

typedef enum {ATTACK,DECAY1,DECAY2,RELEASE} _STATE;
struct _EG
{
	int volume;	//
	_STATE state;
	int step;
	//step vals
	int AR;		//Attack
	int D1R;	//Decay1
	int D2R;	//Decay2
	int RR;		//Release

	int DL;		//Decay level
	UINT8 LPLINK;
};

struct _ADPCM_STATE
{
	UINT8 *base;
	int cur_sample; //current ADPCM sample
	int cur_quant; //current ADPCM step
	int cur_step;
};

struct _LFO
{
	unsigned short phase;
	UINT32 phase_step;
	int *table;
	int *scale;
};

struct _SLOT
{
	union
	{
		UINT16 data[0x40];	//only 0x1a bytes used
		UINT8 datab[0x80];
	} udata;
	UINT8 active;	//this slot is currently playing
	UINT8 *base;		//samples base address
	UINT32 prv_addr;    // previous play address (for ADPCM)
	UINT32 cur_addr;	//current play address (24.8)
	UINT32 nxt_addr;	//next play address
	UINT32 step;		//pitch step (24.8)
	UINT8 Backwards;	//the wave is playing backwards
	struct _EG EG;			//Envelope
	struct _LFO PLFO;		//Phase LFO
	struct _LFO ALFO;		//Amplitude LFO
	int slot;
	struct _ADPCM_STATE adpcm;
	struct _ADPCM_STATE adpcm_lp;
	UINT8 lpend;
	char midi_note; // MIDI Note at the last Note On event
};


#define MEM4B(aica)		((aica->udata.data[0]>>0x0)&0x0200)
#define DAC18B(aica)		((aica->udata.data[0]>>0x0)&0x0100)
#define MVOL(aica)		((aica->udata.data[0]>>0x0)&0x000F)
#define RBL(aica)		((aica->udata.data[2]>>0xD)&0x0003)
#define RBP(aica)		((aica->udata.data[2]>>0x0)&0x0fff)
#define MOFULL(aica)   		((aica->udata.data[4]>>0x0)&0x1000)
#define MOEMPTY(aica)		((aica->udata.data[4]>>0x0)&0x0800)
#define MIOVF(aica)		((aica->udata.data[4]>>0x0)&0x0400)
#define MIFULL(aica)		((aica->udata.data[4]>>0x0)&0x0200)
#define MIEMPTY(aica)		((aica->udata.data[4]>>0x0)&0x0100)

#define AFSEL(aica)		((aica->udata.data[0xc/2]>>0x0)&0x4000)
#define MSLC(aica)		((aica->udata.data[0xc/2]>>0x8)&0x3F)

#define SCILV0(aica)    	((aica->udata.data[0xa8/2]>>0x0)&0xff)
#define SCILV1(aica)    	((aica->udata.data[0xac/2]>>0x0)&0xff)
#define SCILV2(aica)    	((aica->udata.data[0xb0/2]>>0x0)&0xff)

#define MCIEB(aica)    	((aica->udata.data[0xb4/2]>>0x0)&0xff)
#define MCIPD(aica)    	((aica->udata.data[0xb8/2]>>0x0)&0xff)
#define MCIRE(aica)    	((aica->udata.data[0xbc/2]>>0x0)&0xff)

#define SCIEX0	0
#define SCIEX1	1
#define SCIEX2	2
#define SCIMID	3
#define SCIDMA	4
#define SCIIRQ	5
#define SCITMA	6
#define SCITMB	7

//the DSP Context
struct _AICADSP
{
//Config
	UINT16 *AICARAM;
	UINT32 AICARAM_LENGTH;
	UINT32 RBP;	//Ring buf pointer
	UINT32 RBL;	//Delay ram (Ring buffer) size in words

//context

	INT16 COEF[128*2];		//16 bit signed
	UINT16 MADRS[64*2];	//offsets (in words), 16 bit
	UINT16 MPRO[128*4*2*2];	//128 steps 64 bit
	INT32 TEMP[128];	//TEMP regs,24 bit signed
	INT32 MEMS[32];	//MEMS regs,24 bit signed
	UINT32 DEC;

//input
	INT32 MIXS[16];	//MIXS, 24 bit signed
	INT16 EXTS[2];	//External inputs (CDDA)    16 bit signed

//output
	INT16 EFREG[16];	//EFREG, 16 bit signed

	int Stopped;
	int LastStep;
};

void AICADSP_Init(struct _AICADSP *DSP);
void AICADSP_SetSample(struct _AICADSP *DSP, INT32 sample, INT32 SEL, INT32 MXL);
void AICADSP_Step(struct _AICADSP *DSP);
void AICADSP_Start(struct _AICADSP *DSP);

struct _AICA
{
	union
	{
		UINT16 data[0xc0/2];
		UINT8 datab[0xc0];
	} udata;
	UINT16 IRQL, IRQR;
	UINT16 EFSPAN[0x48];
	struct _SLOT Slots[64];
	unsigned char *AICARAM;
	UINT32 AICARAM_LENGTH, RAM_MASK, RAM_MASK16;
	char Master;
	void (*IntARMCB)(int irq);

	UINT32 IrqTimA;
	UINT32 IrqTimBC;
	UINT32 IrqMidi;

	UINT8 MidiOutW,MidiOutR;
	UINT8 MidiStack[16];
	UINT8 MidiW,MidiR;

	int LPANTABLE[0x20000];
	int RPANTABLE[0x20000];

	int TimPris[3];
	int TimCnt[3];

	// DMA stuff
	struct
	{
		UINT32 dmea;
		UINT16 drga;
		UINT16 dlg;
		UINT8 dgate;
		UINT8 ddir;
	} dma;

	int ARTABLE[64], DRTABLE[64];

	struct _AICADSP DSP;
};

extern struct _AICA AICA;

// AICA sample types (value of PCMS)
typedef enum
{
	ST_PCM_16 = 0,
	ST_PCM_8 = 1,
	ST_ADPCM = 2,
	ST_ADPCM_LONG = 3
} AICA_SAMPLE_TYPE;

struct AICAinterface
{
	int num;
	void *region[MAX_AICA];
	int mixing_level[MAX_AICA];			/* volume */
	void (*irq_callback[MAX_AICA])(int state);	/* irq callback */
};

void *aica_start(const void *config);
void AICA_Update(void *param, INT16 **inputs, stereo_sample_t *sample);
int AICA_DumpSample(const UINT8 *ram, uint32 SA, uint16 LSA, uint16 LEA, AICA_SAMPLE_TYPE PCMS);

#define READ16_HANDLER(name)	data16_t name(offs_t offset, data16_t mem_mask)
#define WRITE16_HANDLER(name)	void     name(offs_t offset, data16_t data, data16_t mem_mask)

// AICA register access
READ16_HANDLER( AICA_0_r );
WRITE16_HANDLER( AICA_0_w );
READ16_HANDLER( AICA_1_r );
WRITE16_HANDLER( AICA_1_w );

// MIDI I/O access (used for comms on Model 2/3)
WRITE16_HANDLER( AICA_MidiIn );
READ16_HANDLER( AICA_MidiOutR );

#endif
