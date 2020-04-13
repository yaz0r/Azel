/*
    Sega/Yamaha AICA emulation
    By ElSemi, kingshriek, and R. Belmont

    This is effectively a 64-voice SCSP, with the following differences:
    - No FM mode
    - A third sample format (ADPCM) has been added
    - Some minor other tweeks
*/

#include <assert.h>
#include <math.h>
#include <string.h>
#include "ao.h"
#include "cpuintrf.h"
#include "aica.h"
#include "dc_hw.h"
#include "mididump.h"

#define ICLIP16(x) (x<-32768)?-32768:((x>32767)?32767:x)

#define SHIFT	12
#define FIX(v)	((UINT32) ((float) (1<<SHIFT)*(v)))


#define EG_SHIFT	16

#define USEDSP

// include the LFO handling code
#include "aicalfo.c"

//Envelope times in ms
static const double ARTimes[64]={100000/*infinity*/,100000/*infinity*/,8100.0,6900.0,6000.0,4800.0,4000.0,3400.0,3000.0,2400.0,2000.0,1700.0,1500.0,
					1200.0,1000.0,860.0,760.0,600.0,500.0,430.0,380.0,300.0,250.0,220.0,190.0,150.0,130.0,110.0,95.0,
					76.0,63.0,55.0,47.0,38.0,31.0,27.0,24.0,19.0,15.0,13.0,12.0,9.4,7.9,6.8,6.0,4.7,3.8,3.4,3.0,2.4,
					2.0,1.8,1.6,1.3,1.1,0.93,0.85,0.65,0.53,0.44,0.40,0.35,0.0,0.0};
static const double DRTimes[64]={100000/*infinity*/,100000/*infinity*/,118200.0,101300.0,88600.0,70900.0,59100.0,50700.0,44300.0,35500.0,29600.0,25300.0,22200.0,17700.0,
					14800.0,12700.0,11100.0,8900.0,7400.0,6300.0,5500.0,4400.0,3700.0,3200.0,2800.0,2200.0,1800.0,1600.0,1400.0,1100.0,
					920.0,790.0,690.0,550.0,460.0,390.0,340.0,270.0,230.0,200.0,170.0,140.0,110.0,98.0,85.0,68.0,57.0,49.0,43.0,34.0,
					28.0,25.0,22.0,18.0,14.0,12.0,11.0,8.5,7.1,6.1,5.4,4.3,3.6,3.1};

/// MIDI dumping code
/// -----------------
static double AICA_MIDI_FNSToPitch(struct _SLOT *slot)
{
	double fn = FNS(slot);
	fn /= 1024.0;
	fn += 1;
	fn = log(fn) / log(2.0);
	fn *= 1200.0;
	return fn;
}

static char AICA_MIDI_TLToVelocity(struct _SLOT *slot)
{
	/*
	 * Following the General MIDI System Level 1 developer guidelines
	 * (gmguide2.pdf), the preferred formula for mapping velocities to
	 * decibels is
	 *
	 *	L(dB) = 40 log (V/127)
	 */
	double db = -(TL(slot) / 256.0);
	db *= 96.0; // Dynamic range of TL[7:0]
	return rint(pow(10.0, db / 40.0) * 127);
}

// Returns the MIDI note currently playing in the given slot
static char AICA_MIDI_SlotNote(struct _SLOT *slot, double *bend)
{
	int octave = (OCT(slot) ^ 8) - 6;
	double pitch = AICA_MIDI_FNSToPitch(slot);
	char chromatic = pitch / 100;

	assert(bend);

	*bend = pitch - (chromatic * 100);
	// Round up the nominal note value
	if(*bend > 50.0)
	{
		chromatic++;
		*bend = -(100.0 - *bend);
		if(chromatic == 12)
		{
			octave++;
			chromatic = 0;
		}
	}
	return ((octave + 4) * 12) + chromatic;
}

static void AICA_MIDI_NoteOn(struct _SLOT *slot)
{
	if(!nomidi) {
		double bend;
		unsigned char velocity = AICA_MIDI_TLToVelocity(slot);
		char midi_note = AICA_MIDI_SlotNote(slot, &bend);
#ifdef DEBUG
		// Can't move this to mididump.c as long as it doesn't cover pitch bends
		static const char *PITCHES[12] =
		{
			"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
		};
		int octave = midi_note / 12;
		int chromatic = midi_note - (octave * 12);

		printf(
			"[MIDI] Note  On %04x %2s%d%+02.2f (V:%3d)\n",
			SA(slot), PITCHES[chromatic], octave - 2, bend, velocity
		);
#endif
		mididump_vchan_note_on(SA(slot), midi_note, velocity);
		slot->midi_note = midi_note;
	}
}

static void AICA_MIDI_NoteOff(struct _SLOT *slot)
{
	if(!nomidi && slot->midi_note != 0) {
		mididump_vchan_note_off(
			SA(slot), slot->midi_note, AICA_MIDI_TLToVelocity(slot)
		);
		slot->midi_note = 0;
	}
}
/// -----------------

struct _AICA AICA;

static void aica_exec_dma(struct _AICA *aica);       /*state DMA transfer function*/

static const float SDLT[16]={-1000000.0,-42.0,-39.0,-36.0,-33.0,-30.0,-27.0,-24.0,-21.0,-18.0,-15.0,-12.0,-9.0,-6.0,-3.0,0.0};

static unsigned char DecodeSCI(struct _AICA *AICA, unsigned char irq)
{
	unsigned char SCI=0;
	unsigned char v;
	v=(SCILV0((AICA))&(1<<irq))?1:0;
	SCI|=v;
	v=(SCILV1((AICA))&(1<<irq))?1:0;
	SCI|=v<<1;
	v=(SCILV2((AICA))&(1<<irq))?1:0;
	SCI|=v<<2;
	return SCI;
}

static void ResetInterrupts(struct _AICA *AICA)
{
	#if 0
	UINT32 reset = AICA->udata.data[0xa4/2];
	if (reset & 0x40)
		AICA->IntARMCB(-AICA->IrqTimA);
	if (reset & 0x180)
		AICA->IntARMCB(-AICA->IrqTimBC);
	#endif
}

static void CheckPendingIRQ(struct _AICA *AICA)
{
	UINT32 pend=AICA->udata.data[0xa0/2];
	UINT32 en=AICA->udata.data[0x9c/2];
	if(AICA->MidiW!=AICA->MidiR)
	{
		AICA->IRQL = AICA->IrqMidi;
		AICA->IntARMCB(1);
		return;
	}
	if(!pend)
		return;
	if(pend&0x40)
		if(en&0x40)
		{
			AICA->IRQL = AICA->IrqTimA;
			AICA->IntARMCB(1);
			return;
		}
	if(pend&0x80)
		if(en&0x80)
		{
			AICA->IRQL = AICA->IrqTimBC;
			AICA->IntARMCB(1);
			return;
		}
	if(pend&0x100)
		if(en&0x100)
		{
			AICA->IRQL = AICA->IrqTimBC;
			AICA->IntARMCB(1);
			return;
		}
}

static int Get_AR(struct _AICA *AICA,int base,int R)
{
	int Rate=base+(R<<1);
	if(Rate>63)	Rate=63;
	if(Rate<0) Rate=0;
	return AICA->ARTABLE[Rate];
}

static int Get_DR(struct _AICA *AICA,int base,int R)
{
	int Rate=base+(R<<1);
	if(Rate>63)	Rate=63;
	if(Rate<0) Rate=0;
	return AICA->DRTABLE[Rate];
}

static int Get_RR(struct _AICA *AICA,int base,int R)
{
	int Rate=base+(R<<1);
	if(Rate>63)	Rate=63;
	if(Rate<0) Rate=0;
	return AICA->DRTABLE[Rate];
}

static void Compute_EG(struct _AICA *AICA,struct _SLOT *slot)
{
	int octave=(OCT(slot)^8)-8;
	int rate;
	if(KRS(slot)!=0xf)
		rate=octave+2*KRS(slot)+((FNS(slot)>>9)&1);
	else
		rate=0; //rate=((FNS(slot)>>9)&1);

	slot->EG.volume=0x17f<<EG_SHIFT;
	slot->EG.AR=Get_AR(AICA,rate,AR(slot));
	slot->EG.D1R=Get_DR(AICA,rate,D1R(slot));
	slot->EG.D2R=Get_DR(AICA,rate,D2R(slot));
	slot->EG.RR=Get_RR(AICA,rate,RR(slot));
	slot->EG.DL=0x1f-DL(slot);
}

static void AICA_StopSlot(struct _SLOT *slot,int keyoff);

static int EG_Update(struct _SLOT *slot)
{
	switch(slot->EG.state)
	{
		case ATTACK:
			slot->EG.volume+=slot->EG.AR;
			if(slot->EG.volume>=(0x3ff<<EG_SHIFT))
			{
				if (!LPSLNK(slot) && slot->EG.D1R)
				{
					slot->EG.state=DECAY1;
					if(slot->EG.D1R>=(1024<<EG_SHIFT) && slot->EG.D2R) //Skip DECAY1, go directly to DECAY2
						slot->EG.state=DECAY2;
				}
				slot->EG.volume=0x3ff<<EG_SHIFT;
			}
			break;
		case DECAY1:
			slot->EG.volume-=slot->EG.D1R;
			if(slot->EG.volume<=0)
				slot->EG.volume=0;
			if(slot->EG.volume>>(EG_SHIFT+5)<=slot->EG.DL)
				slot->EG.state=DECAY2;
			break;
		case DECAY2:
			if(D2R(slot)==0)
				return (slot->EG.volume>>EG_SHIFT)<<(SHIFT-10);
			slot->EG.volume-=slot->EG.D2R;
			if(slot->EG.volume<=0)
				slot->EG.volume=0;

			break;
		case RELEASE:
			slot->EG.volume-=slot->EG.RR;
			if(slot->EG.volume<=0)
			{
				slot->EG.volume=0;
				AICA_StopSlot(slot,0);
//				slot->EG.volume=0x17f<<EG_SHIFT;
//				slot->EG.state=ATTACK;
			}
			break;
		default:
			return 1<<SHIFT;
	}
	return (slot->EG.volume>>EG_SHIFT)<<(SHIFT-10);
}

static UINT32 AICA_Step(struct _SLOT *slot)
{
	int octave=(OCT(slot)^8)-8+SHIFT-10;
	UINT32 Fn=FNS(slot) + (0x400);
	if (octave >= 0)
		Fn<<=octave;
	else
		Fn>>=-octave;
	return Fn;
}


static void Compute_LFO(struct _SLOT *slot)
{
	if(PLFOS(slot)!=0)
		AICALFO_ComputeStep(&(slot->PLFO),LFOF(slot),PLFOWS(slot),PLFOS(slot),0);
	if(ALFOS(slot)!=0)
		AICALFO_ComputeStep(&(slot->ALFO),LFOF(slot),ALFOWS(slot),ALFOS(slot),1);
}

#define ADPCMSHIFT	8
#define ADFIX(f)	(int) ((float) f*(float) (1<<ADPCMSHIFT))

const int TableQuant[8]={ADFIX(0.8984375),ADFIX(0.8984375),ADFIX(0.8984375),ADFIX(0.8984375),ADFIX(1.19921875),ADFIX(1.59765625),ADFIX(2.0),ADFIX(2.3984375)};
const int quant_mul[16]= { 1, 3, 5, 7, 9, 11, 13, 15, -1, -3, -5, -7, -9, -11, -13, -15};

void InitADPCM(struct _ADPCM_STATE *adpcm)
{
	adpcm->cur_sample=0;
	adpcm->cur_quant=0x7f;
}

INLINE signed short DecodeADPCM(struct _ADPCM_STATE *adpcm, unsigned char Delta)
{
	int x = adpcm->cur_quant * quant_mul [Delta & 15];
	x = adpcm->cur_sample + ((int)(x + ((UINT32)x >> 29)) >> 3);
	adpcm->cur_sample=ICLIP16(x);
	adpcm->cur_quant=(adpcm->cur_quant*TableQuant[Delta&7])>>ADPCMSHIFT;
	adpcm->cur_quant=(adpcm->cur_quant<0x7f)?0x7f:((adpcm->cur_quant>0x6000)?0x6000:adpcm->cur_quant);
	return adpcm->cur_sample;
}

int AICA_DumpSample(const UINT8 *ram, uint32 SA, uint16 LSA, uint16 LEA, AICA_SAMPLE_TYPE PCMS)
{
	wavedump_t wave;
	struct _ADPCM_STATE adpcm;
	char fn[18];
	int byterate;	// 1 = 8-bit, 2 = 16-bit
	UINT32 step = 0;
	uint32 SA_real = (SA)&0x7fffff;
	const UINT8* ram_sample = ram+SA_real;
	int extradata = 0;	// size of cue chunk data

	if (!sampledump_is_new(SA_real))
	{
		return 0;
	}

	sprintf(fn, "[0x%04x]_0x%06x", LEA, SA_real);
	if (!wavedump_open(&wave, fn))
	{
		return 0;
	}
	wavedump_loop_set(&wave, LSA);

	switch (PCMS)
	{
		case ST_PCM_16:
			byterate = 2;
			break;
		case ST_PCM_8:
			byterate = 1;
			break;
		default:
			byterate = 2;
			InitADPCM(&adpcm);
	}

	// just copy the sample if it's saved as raw PCM
	if(PCMS < ST_ADPCM)
	{
		INT16 local_sample[0x10000];
		UINT32 size = LEA * byterate;
		memcpy(local_sample, ram_sample, size);
		while(step < LEA)
		{
			if (PCMS == ST_PCM_8)
			{
				// 8-bit signed.
				// RIFF WAVE only has an unsigned 8-bit mode,
				// so we have to convert the sample data
				*((INT8*)(local_sample) + step) += 0x80;
			}
			else if(PCMS == ST_PCM_16)
			{
				// 16 bit signed.
				// Do an endianness conversion here in case we
				// happen to run under big-endian
				local_sample[step] = LE16(local_sample[step]);
			}
			step++;
		}
		wavedump_append(&wave, size, local_sample);
	}
	else
	{
		const UINT8* base = ram_sample;
		// We're decoding 4 bits at a time
		while (step < LEA)
		{
			int shift1 = 4*((step&1));
			int delta1 = (*base>>shift1)&0xf;
			DecodeADPCM(&adpcm,delta1);
			step++;
			if (!(step & 1))
			{
				base++;
			}
			wavedump_append(&wave, byterate, &adpcm.cur_sample);
		}
	}
	wavedump_finish(&wave, 44100, byterate * 8, 1);
	return 1;
}

static void AICA_StartSlot(struct _AICA *AICA, struct _SLOT *slot)
{
	UINT64 start_offset;

	slot->active=1;
	slot->Backwards=0;
	slot->cur_addr=0;
	slot->nxt_addr=1<<SHIFT;
	slot->prv_addr=-1;
	start_offset = SA(slot);	// AICA can play 16-bit samples from any boundry
	slot->base=&AICA->AICARAM[start_offset];
	slot->step=AICA_Step(slot);
	Compute_EG(AICA,slot);
	slot->EG.state=ATTACK;
	slot->EG.volume=0x17f<<EG_SHIFT;
	Compute_LFO(slot);
	AICA_DumpSample(AICA->AICARAM, SA(slot), LSA(slot), LEA(slot), PCMS(slot));
	AICA_MIDI_NoteOn(slot);

	if (PCMS(slot) >= 2)
	{
		slot->adpcm.cur_step = 0;
		slot->adpcm.base = (unsigned char *) (AICA->AICARAM+((SA(slot))&0x7fffff));
		InitADPCM(&slot->adpcm);
		InitADPCM(&slot->adpcm_lp);

		// on real hardware this creates undefined behavior.
		if (LSA(slot) > LEA(slot))
		{
			slot->udata.data[0xc/2] = 0xffff;
		}
	}
}

static void AICA_StopSlot(struct _SLOT *slot,int keyoff)
{
	if(keyoff /*&& slot->EG.state!=RELEASE*/)
	{
		slot->EG.state=RELEASE;
	}
	else
	{
		slot->active=0;
		slot->lpend = 1;
	}
	AICA_MIDI_NoteOff(slot);
	slot->udata.data[0]&=~0x4000;

}

#define log_base_2(n) (log((float) n)/log((float) 2))

static void AICA_Init(struct _AICA *AICA, const struct AICAinterface *intf)
{
	int i=0;

	AICA->IrqTimA = AICA->IrqTimBC = AICA->IrqMidi = 0;
	AICA->MidiR=AICA->MidiW=0;
	AICA->MidiOutR=AICA->MidiOutW=0;

	// get AICA RAM
	{
		memset(AICA,0,sizeof(*AICA));

		if (!i)
		{
			AICA->Master=1;
		}
		else
		{
			AICA->Master=0;
		}

		if (intf->region)
		{
			AICA->AICARAM = &dc_ram[0];
			AICA->AICARAM_LENGTH = 2*1024*1024;
			AICA->RAM_MASK = AICA->AICARAM_LENGTH-1;
			AICA->RAM_MASK16 = AICA->RAM_MASK & 0x7ffffe;
			AICA->DSP.AICARAM = (UINT16 *)AICA->AICARAM;
			AICA->DSP.AICARAM_LENGTH =  (2*1024*1024)/2;
		}
	}

	for(i=0; i<0x400; ++i)
	{
		float envDB=((float)(3*(i-0x3ff)))/32.0;
		float scale=(float)(1<<SHIFT);
		EG_TABLE[i]=(INT32)(pow(10.0,envDB/20.0)*scale);
	}

	for(i=0; i<0x20000; ++i)
	{
		int iTL =(i>>0x0)&0xff;
		int iPAN=(i>>0x8)&0x1f;
		int iSDL=(i>>0xD)&0x0F;
		float TL=1.0;
		float SegaDB=0;
		float fSDL=1.0;
		float PAN=1.0;
		float LPAN,RPAN;

		if(iTL&0x01) SegaDB-=0.4;
		if(iTL&0x02) SegaDB-=0.8;
		if(iTL&0x04) SegaDB-=1.5;
		if(iTL&0x08) SegaDB-=3;
		if(iTL&0x10) SegaDB-=6;
		if(iTL&0x20) SegaDB-=12;
		if(iTL&0x40) SegaDB-=24;
		if(iTL&0x80) SegaDB-=48;

		TL=pow(10.0,SegaDB/20.0);

		SegaDB=0;
		if(iPAN&0x1) SegaDB-=3;
		if(iPAN&0x2) SegaDB-=6;
		if(iPAN&0x4) SegaDB-=12;
		if(iPAN&0x8) SegaDB-=24;

		if((iPAN&0xf)==0xf) PAN=0.0;
		else PAN=pow(10.0,SegaDB/20.0);

		if(iPAN<0x10)
		{
			LPAN=PAN;
			RPAN=1.0;
		}
		else
		{
			RPAN=PAN;
			LPAN=1.0;
		}

		if(iSDL)
			fSDL=pow(10.0,(SDLT[iSDL])/20.0);
		else
			fSDL=0.0;

		AICA->LPANTABLE[i]=FIX((4.0*LPAN*TL*fSDL));
		AICA->RPANTABLE[i]=FIX((4.0*RPAN*TL*fSDL));
	}

	AICA->ARTABLE[0]=AICA->DRTABLE[0]=0;	//Infinite time
	AICA->ARTABLE[1]=AICA->DRTABLE[1]=0;	//Infinite time
	for(i=2; i<64; ++i)
	{
		double t,step,scale;
		t=ARTimes[i];	//In ms
		if(t!=0.0)
		{
			step=(1023*1000.0)/((float) 44100.0f*t);
			scale=(double) (1<<EG_SHIFT);
			AICA->ARTABLE[i]=(int) (step*scale);
		}
		else
			AICA->ARTABLE[i]=1024<<EG_SHIFT;

		t=DRTimes[i];	//In ms
		step=(1023*1000.0)/((float) 44100.0f*t);
		scale=(double) (1<<EG_SHIFT);
		AICA->DRTABLE[i]=(int) (step*scale);
	}

	// make sure all the slots are off
	for(i=0; i<64; ++i)
	{
		AICA->Slots[i].slot=i;
		AICA->Slots[i].active=0;
		AICA->Slots[i].base=NULL;
		AICA->Slots[i].EG.state=RELEASE;
		AICA->Slots[i].lpend=1;
	}

	AICALFO_Init();

	// no "pend"
	AICA[0].udata.data[0xa0/2] = 0;
	//AICA[1].udata.data[0x20/2] = 0;
	AICA->TimCnt[0] = 0xffff;
	AICA->TimCnt[1] = 0xffff;
	AICA->TimCnt[2] = 0xffff;
}

static void AICA_UpdateSlotReg(struct _AICA *AICA,int s,int r)
{
	struct _SLOT *slot=AICA->Slots+s;
	int sl;
	switch(r&0x7f)
	{
		case 0:
		case 1:
			if(KEYONEX(slot))
			{
				for(sl=0; sl<64; ++sl)
				{
					struct _SLOT *s2=AICA->Slots+sl;
					{
						if(KEYONB(s2) && s2->EG.state==RELEASE/*&& !s2->active*/)
						{
							s2->lpend = 0;
							AICA_StartSlot(AICA, s2);
							#ifdef DEBUG
							printf("StartSlot[%02X]:   SSCTL %01X SA %06X LSA %04X LEA %04X PCMS %01X LPCTL %01X\n",sl,SSCTL(s2),SA(s2),LSA(s2),LEA(s2),PCMS(s2),LPCTL(s2));
							printf("                 AR %02X D1R %02X D2R %02X RR %02X DL %02X KRS %01X LPSLNK %01X\n",AR(s2),D1R(s2),D2R(s2),RR(s2),DL(s2),KRS(s2),LPSLNK(s2)>>14);
							printf("                 TL %02X OCT %01X FNS %03X\n",TL(s2),OCT(s2),FNS(s2));
							printf("                 LFORE %01X LFOF %02X ALFOWS %01X ALFOS %01X PLFOWS %01X PLFOS %01X\n",LFORE(s2),LFOF(s2),ALFOWS(s2),ALFOS(s2),PLFOWS(s2),PLFOS(s2));
							printf("                 IMXL %01X ISEL %01X DISDL %01X DIPAN %02X\n",IMXL(s2),ISEL(s2),DISDL(s2),DIPAN(s2));
							printf("\n");
							fflush(stdout);
							#endif
						}
						if(!KEYONB(s2) /*&& s2->active*/)
						{
							AICA_StopSlot(s2,1);
						}
					}
				}
				slot->udata.data[0]&=~0x8000;
			}
			break;
		case 0x18:
		case 0x19:
			slot->step=AICA_Step(slot);
			break;
		case 0x14:
		case 0x15:
			slot->EG.RR=Get_RR(AICA,0,RR(slot));
			slot->EG.DL=0x1f-DL(slot);
			break;
		case 0x1c:
		case 0x1d:
			Compute_LFO(slot);
			break;
		case 0x24:
//			printf("[%02d]: %x to DISDL/DIPAN (PC=%x)\n", s, slot->udata.data[0x24/2], arm7_get_register(15));
			break;
	}
}

static void AICA_UpdateReg(struct _AICA *AICA, int reg)
{
	switch(reg&0xff)
	{
		case 0x4:
		case 0x5:
			{
				unsigned int v=RBL(AICA);
				AICA->DSP.RBP=RBP(AICA);
				if(v==0)
					AICA->DSP.RBL=8*1024;
				else if(v==1)
					AICA->DSP.RBL=16*1024;
				else if(v==2)
					AICA->DSP.RBL=32*1024;
				else if(v==3)
					AICA->DSP.RBL=64*1024;
			}
			break;
		case 0x8:
		case 0x9:
			AICA_MidiIn(0, AICA->udata.data[0x8/2]&0xff, 0);
			break;
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
			break;

		case 0x80:
		case 0x81:
			AICA->dma.dmea = ((AICA->udata.data[0x80/2] & 0xfe00) << 7) | (AICA->dma.dmea & 0xfffc);
			/* TODO: $TSCD - MRWINH regs */
			break;

		case 0x84:
		case 0x85:
			AICA->dma.dmea = (AICA->udata.data[0x84/2] & 0xfffc) | (AICA->dma.dmea & 0x7f0000);
			break;

		case 0x88:
		case 0x89:
			AICA->dma.drga = (AICA->udata.data[0x88/2] & 0x7ffc);
			AICA->dma.dgate = (AICA->udata.data[0x88/2] & 0x8000) >> 15;
			break;

		case 0x8c:
		case 0x8d:
			AICA->dma.dlg = (AICA->udata.data[0x8c/2] & 0x7ffc);
			AICA->dma.ddir = (AICA->udata.data[0x8c/2] & 0x8000) >> 15;
			if(AICA->udata.data[0x8c/2] & 1) // dexe
				aica_exec_dma(AICA);
			break;

		case 0x90:
		case 0x91:
			if(AICA->Master)
			{
				AICA->TimPris[0]=1<<((AICA->udata.data[0x90/2]>>8)&0x7);
				AICA->TimCnt[0]=(AICA->udata.data[0x90/2]&0xff)<<8;
			}
			break;
		case 0x94:
		case 0x95:
			if(AICA->Master)
			{
				AICA->TimPris[1]=1<<((AICA->udata.data[0x94/2]>>8)&0x7);
				AICA->TimCnt[1]=(AICA->udata.data[0x94/2]&0xff)<<8;
			}
			break;
		case 0x98:
		case 0x99:
			if(AICA->Master)
			{
				AICA->TimPris[2]=1<<((AICA->udata.data[0x98/2]>>8)&0x7);
				AICA->TimCnt[2]=(AICA->udata.data[0x98/2]&0xff)<<8;
			}
			break;
		case 0xa4:	//SCIRE
		case 0xa5:

			if(AICA->Master)
			{
				AICA->udata.data[0xa0/2] &= ~AICA->udata.data[0xa4/2];
				ResetInterrupts(AICA);

				// behavior from real hardware (SCSP, assumed to carry over): if you SCIRE a timer that's expired,
				// it'll immediately pop up again
				if (AICA->TimCnt[0] >= 0xff00)
				{
					AICA->udata.data[0xa0/2] |= 0x40;
				}
				if (AICA->TimCnt[1] >= 0xff00)
				{
					AICA->udata.data[0xa0/2] |= 0x80;
				}
				if (AICA->TimCnt[2] >= 0xff00)
				{
					AICA->udata.data[0xa0/2] |= 0x100;
				}
			}
			break;
		case 0xa8:
		case 0xa9:
		case 0xac:
		case 0xad:
		case 0xb0:
		case 0xb1:
			if(AICA->Master)
			{
				AICA->IrqTimA=DecodeSCI(AICA,SCITMA);
				AICA->IrqTimBC=DecodeSCI(AICA,SCITMB);
				AICA->IrqMidi=DecodeSCI(AICA,SCIMID);
			}
			break;

		case 0xb4:
		case 0xb5:
		case 0xb6:
		case 0xb7:
			if (MCIEB(AICA) & 0x20)
			{
				logerror("AICA: Interrupt requested on SH-4!\n");
			}
			break;
	}
}

static void AICA_UpdateSlotRegR(struct _AICA *AICA, int slot,int reg)
{

}

static void AICA_UpdateRegR(struct _AICA *AICA, int reg)
{
	switch(reg&0xff)
	{
		case 8:
		case 9:
			{
				unsigned short v=AICA->udata.data[0x8/2];
				v&=0xff00;
				v|=AICA->MidiStack[AICA->MidiR];
				AICA->IntARMCB(0);	// cancel the IRQ
				if(AICA->MidiR!=AICA->MidiW)
				{
					++AICA->MidiR;
					AICA->MidiR&=15;
				}
				AICA->udata.data[0x8/2]=v;
			}
			break;

		case 0x10:	// LP check
		case 0x11:
			{
				int slotnum = MSLC(AICA);
				struct _SLOT *slot=AICA->Slots + slotnum;
				UINT16 LP = 0;
				if (!(AFSEL(AICA)))
				{
					UINT16 SGC;
					int EG;

					LP = slot->lpend ? 0x8000 : 0x0000;
					slot->lpend = 0;
					SGC = (slot->EG.state << 13) & 0x6000;
					EG = slot->active ? slot->EG.volume : 0;
					EG >>= (EG_SHIFT - 13);
					EG = 0x1FFF - EG;
					if (EG < 0) EG = 0;

					AICA->udata.data[0x10/2] = (EG & 0x1FF8) | SGC | LP;
				}
				else
				{
					LP = slot->lpend ? 0x8000 : 0x0000;
					AICA->udata.data[0x10/2] = LP;
				}
			}
			break;

		case 0x14:	// CA (slot address)
		case 0x15:
			{
				int slotnum = MSLC(AICA);
				struct _SLOT *slot=AICA->Slots+slotnum;
				unsigned int CA = 0;

				if (PCMS(slot) == 0)   	// 16-bit samples
				{
					CA = (slot->cur_addr>>(SHIFT-1))&AICA->RAM_MASK16;
				}
				else	// 8-bit PCM and 4-bit ADPCM
				{
					CA = (slot->cur_addr>>SHIFT)&AICA->RAM_MASK;
				}

				AICA->udata.data[0x14/2] = CA;
			}
			break;
	}
}

static void AICA_w16(struct _AICA *AICA,unsigned int addr,unsigned short val)
{
	addr&=0xffff;
	if(addr<0x2000)
	{
		int slot=addr/0x80;
		addr&=0x7f;
//		printf("%x to slot %d offset %x\n", val, slot, addr);
		*((unsigned short *) (AICA->Slots[slot].udata.datab+(addr))) = val;
		AICA_UpdateSlotReg(AICA,slot,addr&0x7f);
	}
	else if (addr < 0x2800)
	{
		if (addr <= 0x2044)
		{
//			printf("%x to EFSxx slot %d (addr %x)\n", val, (addr-0x2000)/4, addr&0x7f);
			AICA->EFSPAN[addr&0x7f] = val;
		}
	}
	else if(addr<0x3000)
	{
		if (addr < 0x28be)
		{
//			printf("%x to AICA global @ %x\n", val, addr & 0xff);
			*((unsigned short *) (AICA->udata.datab+((addr&0xff)))) = val;
			AICA_UpdateReg(AICA, addr&0xff);
		}
		else if (addr == 0x2d00)
		{
			AICA->IRQL = val;
			logerror("AICA: write to IRQL?\n");
		}
		else if (addr == 0x2d04)
		{
			AICA->IRQR = val;

			if (val & 1)
			{
				AICA->IntARMCB(0);
			}
			if (val & 0x100)
				logerror("AICA: SH-4 write protection enabled!\n");

			if (val & 0xfefe)
				logerror("AICA: IRQR %04x!\n",val);
		}
	}
	else
	{
		//DSP
		if(addr<0x3200)	//COEF
			*((unsigned short *) (AICA->DSP.COEF+(addr-0x3000)/2))=val;
		else if(addr<0x3400)
			*((unsigned short *) (AICA->DSP.MADRS+(addr-0x3200)/2))=val;
		else if(addr<0x3c00)
		{
			*((unsigned short *) (AICA->DSP.MPRO+(addr-0x3400)/2))=val;

			if (addr == 0x3bfe)
			{
				AICADSP_Start(&AICA->DSP);
			}
		}
		else if(addr<0x4000)
		{
			logerror("AICADSP write to undocumented reg %04x -> %04x",addr,val);
		}
		else if(addr<0x4400)
		{
			if(addr & 4)
				AICA->DSP.TEMP[(addr >> 3) & 0x7f] = (AICA->DSP.TEMP[(addr >> 3) & 0x7f] & 0xffff0000) | (val & 0xffff);
			else
				AICA->DSP.TEMP[(addr >> 3) & 0x7f] = (AICA->DSP.TEMP[(addr >> 3) & 0x7f] & 0xffff) | (val << 16);
		}
		else if(addr<0x4500)
		{
			if(addr & 4)
				AICA->DSP.MEMS[(addr >> 3) & 0x1f] = (AICA->DSP.MEMS[(addr >> 3) & 0x1f] & 0xffff0000) | (val & 0xffff);
			else
				AICA->DSP.MEMS[(addr >> 3) & 0x1f] = (AICA->DSP.MEMS[(addr >> 3) & 0x1f] & 0xffff) | (val << 16);
		}
		else if(addr<0x4580)
		{
			if(addr & 4)
				AICA->DSP.MIXS[(addr >> 3) & 0xf] = (AICA->DSP.MIXS[(addr >> 3) & 0xf] & 0xffff0000) | (val & 0xffff);
			else
				AICA->DSP.MIXS[(addr >> 3) & 0xf] = (AICA->DSP.MIXS[(addr >> 3) & 0xf] & 0xffff) | (val << 16);
		}
		else if(addr<0x45c0)
			*((unsigned short *) (AICA->DSP.EFREG+(addr-0x4580)/4))=val;
		else if(addr<0x45c8)
			*((unsigned short *) (AICA->DSP.EXTS+(addr-0x45c0)/2))=val;
	}
}

static unsigned short AICA_r16(struct _AICA *AICA, unsigned int addr)
{
	unsigned short v=0;
	addr&=0xffff;
	if(addr<0x2000)
	{
		int slot=addr/0x80;
		addr&=0x7f;
		AICA_UpdateSlotRegR(AICA, slot,addr&0x7f);
		v=*((unsigned short *) (AICA->Slots[slot].udata.datab+(addr)));
	}
	else if(addr<0x3000)
	{
		if (addr <= 0x2044)
		{
			v = AICA->EFSPAN[addr&0x7f];
		}
		else if (addr < 0x28be)
		{
			AICA_UpdateRegR(AICA, addr&0xff);
			v= *((unsigned short *) (AICA->udata.datab+((addr&0xff))));
			if((addr&0xfffe)==0x2810) AICA->udata.data[0x10/2] &= 0x7FFF;	// reset LP on read
		}
		else if (addr == 0x2d00)
		{
			return AICA->IRQL;
		}
		else if (addr == 0x2d04)
		{
			return AICA->IRQR;
		}
	}
	else
	{
		if(addr<0x3200) //COEF
			v= *((unsigned short *) (AICA->DSP.COEF+(addr-0x3000)/2));
		else if(addr<0x3400)
			v= *((unsigned short *) (AICA->DSP.MADRS+(addr-0x3200)/2));
		else if(addr<0x3c00)
			v= *((unsigned short *) (AICA->DSP.MPRO+(addr-0x3400)/2));
		else if(addr<0x4000)
		{
			v= 0xffff;
			logerror("AICADSP read to undocumented reg %04x\n",addr);
		}
		else if(addr<0x4400)
		{
			if(addr & 4)
				v= AICA->DSP.TEMP[(addr >> 3) & 0x7f] & 0xffff;
			else
				v= AICA->DSP.TEMP[(addr >> 3) & 0x7f] >> 16;
		}
		else if(addr<0x4500)
		{
			if(addr & 4)
				v= AICA->DSP.MEMS[(addr >> 3) & 0x1f] & 0xffff;
			else
				v= AICA->DSP.MEMS[(addr >> 3) & 0x1f] >> 16;
		}
		else if(addr<0x4580)
		{
			if(addr & 4)
				v= AICA->DSP.MIXS[(addr >> 3) & 0xf] & 0xffff;
			else
				v= AICA->DSP.MIXS[(addr >> 3) & 0xf] >> 16;
		}
		else if(addr<0x45c0)
			v = *((unsigned short *) (AICA->DSP.EFREG+(addr-0x4580)/4));
		else if(addr<0x45c8)
			v = *((unsigned short *) (AICA->DSP.EXTS+(addr-0x45c0)/2));
	}
	return v;
}


#define REVSIGN(v) ((~v)+1)

void AICA_TimersAddTicks(struct _AICA *AICA, int ticks)
{
	if(AICA->TimCnt[0]<=0xff00)
	{
		AICA->TimCnt[0] += ticks << (8-((AICA->udata.data[0x90/2]>>8)&0x7));
		if (AICA->TimCnt[0] >= 0xFF00)
		{
			AICA->TimCnt[0] = 0xFFFF;
			AICA->udata.data[0xa0/2]|=0x40;
		}
		AICA->udata.data[0x90/2]&=0xff00;
		AICA->udata.data[0x90/2]|=AICA->TimCnt[0]>>8;
	}

	if(AICA->TimCnt[1]<=0xff00)
	{
		AICA->TimCnt[1] += ticks << (8-((AICA->udata.data[0x94/2]>>8)&0x7));
		if (AICA->TimCnt[1] >= 0xFF00)
		{
			AICA->TimCnt[1] = 0xFFFF;
			AICA->udata.data[0xa0/2]|=0x80;
		}
		AICA->udata.data[0x94/2]&=0xff00;
		AICA->udata.data[0x94/2]|=AICA->TimCnt[1]>>8;
	}

	if(AICA->TimCnt[2]<=0xff00)
	{
		AICA->TimCnt[2] += ticks << (8-((AICA->udata.data[0x98/2]>>8)&0x7));
		if (AICA->TimCnt[2] >= 0xFF00)
		{
			AICA->TimCnt[2] = 0xFFFF;
			AICA->udata.data[0xa0/2]|=0x100;
		}
		AICA->udata.data[0x98/2]&=0xff00;
		AICA->udata.data[0x98/2]|=AICA->TimCnt[2]>>8;
	}
}

INLINE INT32 AICA_UpdateSlot(struct _AICA *AICA, struct _SLOT *slot)
{
	INT32 sample, fpart;
	int cur_sample;       //current sample
	int nxt_sample;       //next sample
	int step=slot->step;
	UINT32 addr1,addr2;                                   // current and next sample addresses

	if(SSCTL(slot)!=0)	//no FM or noise yet
		return 0;

	if(PLFOS(slot)!=0)
	{
		step=step*AICAPLFO_Step(&(slot->PLFO));
		step>>=SHIFT;
	}

	if(PCMS(slot) == 0)
	{
		addr1=(slot->cur_addr>>(SHIFT-1))&0x7ffffe;
		addr2=(slot->nxt_addr>>(SHIFT-1))&0x7ffffe;
	}
	else
	{
		addr1=slot->cur_addr>>SHIFT;
		addr2=slot->nxt_addr>>SHIFT;
	}

	if(PCMS(slot) == 1)	// 8-bit signed
	{
		INT8 *p1=(signed char *) (AICA->AICARAM+(((SA(slot)+addr1))&0x7fffff));
		INT8 *p2=(signed char *) (AICA->AICARAM+(((SA(slot)+addr2))&0x7fffff));
		cur_sample = p1[0] << 8;
		nxt_sample = p2[0] << 8;
	}
	else if (PCMS(slot) == 0)	//16 bit signed
	{
		INT16 *p1=(signed short *) (AICA->AICARAM+((SA(slot)+addr1)&0x7fffff));
		INT16 *p2=(signed short *) (AICA->AICARAM+((SA(slot)+addr2)&0x7fffff));
		cur_sample = LE16(p1[0]);
		nxt_sample = LE16(p2[0]);
	}
	else	// 4-bit ADPCM
	{
		UINT8 *base= slot->adpcm.base;
		UINT32 steps_to_go = addr2, curstep = slot->adpcm.cur_step;

		if (base)
		{
			cur_sample = slot->adpcm.cur_sample; // may already contains current decoded sample

			// seek to the interpolation sample
			while (curstep < steps_to_go)
			{
				int shift1 = 4 & (curstep << 2);
				unsigned char delta1 = (*base>>shift1)&0xf;
				DecodeADPCM(&slot->adpcm,delta1);
				if (!(++curstep & 1))
					base++;
				if (curstep == addr1)
					cur_sample = slot->adpcm.cur_sample;
				if (curstep == LSA(slot))
				{
					slot->adpcm_lp.cur_sample = slot->adpcm.cur_sample;
					slot->adpcm_lp.cur_quant = slot->adpcm.cur_quant;
				}
			}
			nxt_sample = slot->adpcm.cur_sample;

			slot->adpcm.base = base;
			slot->adpcm.cur_step = curstep;
		}
		else
		{
			cur_sample = nxt_sample = 0;
		}
	}
	fpart = slot->cur_addr & ((1<<SHIFT)-1);
	sample=cur_sample*((1<<SHIFT)-fpart)+nxt_sample*fpart;
	sample>>=SHIFT;

	slot->prv_addr=slot->cur_addr;
	slot->cur_addr+=step;
	slot->nxt_addr=slot->cur_addr+(1<<SHIFT);

	addr1=slot->cur_addr>>SHIFT;
	addr2=slot->nxt_addr>>SHIFT;

	if(addr1>=LSA(slot))
	{
		if(LPSLNK(slot) && slot->EG.state==ATTACK && slot->EG.D1R)
			slot->EG.state = DECAY1;
	}

	switch(LPCTL(slot))
	{
		case 0:	//no loop
			if(addr2>=LSA(slot) && addr2>=LEA(slot)) // if next sample exceed then current must exceed too
			{
				AICA_StopSlot(slot,0);
			}
			break;
		case 1: //normal loop
			if(addr2>=LEA(slot))
			{
				INT32 rem_addr;
				slot->lpend = 1;
				rem_addr = slot->nxt_addr - (LEA(slot)<<SHIFT);
				slot->nxt_addr = (LSA(slot)<<SHIFT) + rem_addr;
				if(addr1>=LEA(slot))
				{
					rem_addr = slot->cur_addr - (LEA(slot)<<SHIFT);
					slot->cur_addr = (LSA(slot)<<SHIFT) + rem_addr;
				}

				if(PCMS(slot)>=2)
				{
					// restore the state @ LSA - the sampler will naturally walk to (LSA + remainder)
					slot->adpcm.base = &AICA->AICARAM[SA(slot)+(LSA(slot)/2)];
					slot->adpcm.cur_step = LSA(slot);
					if (PCMS(slot) == 2)
					{
						slot->adpcm.cur_sample = slot->adpcm_lp.cur_sample;
						slot->adpcm.cur_quant = slot->adpcm_lp.cur_quant;
					}
//printf("Looping: slot_addr %x LSA %x LEA %x step %x base %x\n", slot->cur_addr>>SHIFT, LSA(slot), LEA(slot), slot->adpcm.cur_step, slot->adpcm.base);
				}
			}
			break;
	}

	if(ALFOS(slot)!=0)
	{
		sample=sample*AICAALFO_Step(&(slot->ALFO));
		sample>>=SHIFT;
	}

	if(slot->EG.state==ATTACK)
		sample=(sample*EG_Update(slot))>>SHIFT;
	else
		sample=(sample*EG_TABLE[EG_Update(slot)>>(SHIFT-10)])>>SHIFT;

	return sample;
}

static void AICA_DoMasterSample(struct _AICA *AICA, stereo_sample_t *sample)
{
	int sl, i;
	INT32 smpl, smpr;

	smpl = smpr = 0;

	// mix slots' direct output
	for(sl=0; sl<64; ++sl)
	{
		struct _SLOT *slot=AICA->Slots+sl;
		if(AICA->Slots[sl].active)
		{
			unsigned int Enc;
			signed int sample;

			sample=AICA_UpdateSlot(AICA, slot);

			Enc=((TL(slot))<<0x0)|((IMXL(slot))<<0xd);
			AICADSP_SetSample(&AICA->DSP,(sample*AICA->LPANTABLE[Enc])>>(SHIFT-2),ISEL(slot),IMXL(slot));
			Enc=((TL(slot))<<0x0)|((DIPAN(slot))<<0x8)|((DISDL(slot))<<0xd);
			{
				smpl+=(sample*AICA->LPANTABLE[Enc])>>SHIFT;
				smpr+=(sample*AICA->RPANTABLE[Enc])>>SHIFT;
			}
		}
	}

	// process the DSP
	AICADSP_Step(&AICA->DSP);

	// mix DSP output
	for(i=0; i<16; ++i)
	{
		if(EFSDL(i))
		{
			unsigned int Enc=((EFPAN(i))<<0x8)|((EFSDL(i))<<0xd);
			smpl+=(AICA->DSP.EFREG[i]*AICA->LPANTABLE[Enc])>>SHIFT;
			smpr+=(AICA->DSP.EFREG[i]*AICA->RPANTABLE[Enc])>>SHIFT;
		}
	}

	sample->l = ICLIP16(smpl>>3);
	sample->r = ICLIP16(smpr>>3);

	AICA_TimersAddTicks(AICA, 1);
	CheckPendingIRQ(AICA);
}

/* TODO: this needs to be timer-ized */
static void aica_exec_dma(struct _AICA *aica)
{
	static UINT16 tmp_dma[4];
	int i;

	printf("AICA: DMA transfer START\n"
				"DMEA: %08x DRGA: %08x DLG: %04x\n"
				"DGATE: %d  DDIR: %d\n",aica->dma.dmea,aica->dma.drga,aica->dma.dlg,aica->dma.dgate,aica->dma.ddir);

	/* Copy the dma values in a temp storage for resuming later */
	/* (DMA *can't* overwrite its parameters).                  */
	if(!(aica->dma.ddir))
	{
		for(i=0; i<4; i++)
			tmp_dma[i] = aica->udata.data[(0x80+(i*4))/2];
	}

	/* note: we don't use space.read_word / write_word because it can happen that SH-4 enables the DMA instead of ARM like in DCLP tester. */
	/* TODO: don't know if params auto-updates, I guess not ... */
	if(aica->dma.ddir)
	{
		if(aica->dma.dgate)
		{
			for(i=0; i < aica->dma.dlg; i+=2)
			{
				aica->AICARAM[aica->dma.dmea] = 0;
				aica->AICARAM[aica->dma.dmea+1] = 0;
				aica->dma.dmea+=2;
			}
		}
		else
		{
			for(i=0; i < aica->dma.dlg; i+=2)
			{
				UINT16 tmp;
				tmp = AICA_r16(aica, aica->dma.drga);;
				aica->AICARAM[aica->dma.dmea] = tmp & 0xff;
				aica->AICARAM[aica->dma.dmea+1] = tmp>>8;
				aica->dma.dmea+=4;
				aica->dma.drga+=4;
			}
		}
	}
	else
	{
		if(aica->dma.dgate)
		{
			for(i=0; i < aica->dma.dlg; i+=2)
			{
				AICA_w16(aica, aica->dma.drga, 0);
				aica->dma.drga+=4;
			}
		}
		else
		{
			for(i=0; i < aica->dma.dlg; i+=2)
			{
				UINT16 tmp;
				tmp = aica->AICARAM[aica->dma.dmea];
				tmp|= aica->AICARAM[aica->dma.dmea+1]<<8;
				AICA_w16(aica, aica->dma.drga, tmp);
				aica->dma.dmea+=4;
				aica->dma.drga+=4;
			}
		}
	}

	/*Resume the values*/
	if(!(aica->dma.ddir))
	{
		for(i=0; i<4; i++)
			aica->udata.data[(0x80+(i*4))/2] = tmp_dma[i];
	}

	/* Job done, clear DEXE */
	aica->udata.data[0x8c/2] &= ~1;
	/* request a dma end irq (TBD) */
	// ...
}

int AICA_IRQCB(void *param)
{
	CheckPendingIRQ(param);
	return -1;
}

void AICA_Update(void *param, INT16 **inputs, stereo_sample_t *sample)
{
	AICA_DoMasterSample(&AICA, sample);
}

void *aica_start(const void *config)
{
	const struct AICAinterface *intf = config;

	memset(&AICA, 0, sizeof(struct _AICA));

	// init the emulation
	AICA_Init(&AICA, intf);

	// set up the IRQ callbacks
	AICA.IntARMCB = intf->irq_callback[0];
	// AICA.stream = stream_create(0, 2, 44100, AICA, AICA_Update);

	return &AICA;
}

void AICA_set_ram_base(int which, void *base)
{
	AICA.AICARAM = base;
	AICA.RAM_MASK = AICA.AICARAM_LENGTH-1;
	AICA.RAM_MASK16 = AICA.RAM_MASK & 0x7ffffe;
	AICA.DSP.AICARAM = base;
}

READ16_HANDLER( AICA_0_r )
{
	UINT16 res = AICA_r16(&AICA, offset*2);

//	printf("Read AICA @ %x => %x (PC=%x, R5=%x)\n", offset*2, res, arm7_get_register(15), arm7_get_register(5));

	return res;
}

extern UINT32* stv_scu;

WRITE16_HANDLER( AICA_0_w )
{
	UINT16 tmp = AICA_r16(&AICA, offset*2);
	COMBINE_DATA(&tmp);
	AICA_w16(&AICA,offset*2, tmp);
}

WRITE16_HANDLER( AICA_MidiIn )
{
	AICA.MidiStack[AICA.MidiW++]=data;
	AICA.MidiW &= 15;
}

READ16_HANDLER( AICA_MidiOutR )
{
	unsigned char val = AICA.MidiStack[AICA.MidiR++];
	AICA.MidiR&=7;
	return val;
}

