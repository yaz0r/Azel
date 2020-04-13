//
// Audio Overload
// Emulated music player
//
// (C) 2000-2007 Richard F. Bannister
//

//
// eng_protos.h
//

#ifdef __cplusplus
extern "C" {
#endif

int32 psf_start(uint8 *, uint32 length);
int32 psf_sample(stereo_sample_t *);
int32 psf_frame(void);
int32 psf_stop(void);
int32 psf_command(int32, int32);
int32 psf_fill_info(ao_display_info *);

int32 psf2_start(uint8 *, uint32 length);
int32 psf2_sample(stereo_sample_t *);
int32 psf2_frame(void);
int32 psf2_stop(void);
int32 psf2_command(int32, int32);
int32 psf2_fill_info(ao_display_info *);

int32 qsf_start(uint8 *, uint32 length);
int32 qsf_sample(stereo_sample_t *);
int32 qsf_frame(void);
int32 qsf_stop(void);
int32 qsf_command(int32, int32);
int32 qsf_fill_info(ao_display_info *);

int32 ssf_start(uint8 *, uint32 length);
int32 ssf_sample(stereo_sample_t *);
int32 ssf_frame(void);
int32 ssf_stop(void);
int32 ssf_command(int32, int32);
int32 ssf_fill_info(ao_display_info *);

int32 spu_start(uint8 *, uint32 length);
int32 spu_sample(stereo_sample_t *);
int32 spu_frame(void);
int32 spu_stop(void);
int32 spu_command(int32, int32);
int32 spu_fill_info(ao_display_info *);

uint8 qsf_memory_read(uint16 addr);
uint8 qsf_memory_readop(uint16 addr);
uint8 qsf_memory_readport(uint16 addr);
void qsf_memory_write(uint16 addr, uint8 byte);
void qsf_memory_writeport(uint16 addr, uint8 byte);

int32 dsf_start(uint8 *, uint32 length);
int32 dsf_sample(stereo_sample_t *);
int32 dsf_frame(void);
int32 dsf_stop(void);
int32 dsf_command(int32, int32);
int32 dsf_fill_info(ao_display_info *);

void dc_debug(void);

#ifdef __cplusplus
}
#endif
