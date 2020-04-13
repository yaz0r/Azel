#
# Makefile for Audio Overload SDK
#
# Copyright (c) 2007-2008, R. Belmont and Richard Bannister.
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
# * Neither the names of R. Belmont and Richard Bannister nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

# NOTE: this makefile will auto-detect Linux and MinGW and work appropriately.  Other OSes will likely
#       need some help.

CC   ?= gcc
LD   = $(CC)
CFLAGS += -c -DPATH_MAX=1024 -DHAS_PSXCPU=1 -I. -I.. -Ieng_ssf -Ieng_qsf  -Ieng_dsf -Izlib -fdata-sections -ffunction-sections
# set for little-endian, make "0" for big-endian
CFLAGS += -DLSB_FIRST=1
LDFLAGS += -Wl,--gc-sections

MACHINE = $(shell $(CC) -dumpmachine)

EXE  = aosdk-$(MACHINE)
LIBS += -lm

# main objects
OBJS = main.o ao.o corlett.o m1sdr.o utils.o mididump.o sampledump.o wavedump.o argparse/argparse.o

# port objects
ifeq ($(OSTYPE),linux)
ifeq ($(shell uname -m),x86_64)
CFLAGS += -DLONG_IS_64BIT=1
endif
ifneq (,$(wildcard /dev/dsp))
    $(info Using OSS via /dev/dsp for playback.)
    OBJS += oss.o
else
    $(info /dev/dsp not found. Playback will be unavailable.)
    CFLAGS += -DNOPLAY
endif
else
CFLAGS += -DWIN32_UTF8_NO_API
OBJS += dsnd.o win32_utf8/win32_utf8_build_static.o
LIBS += -ldsound -ldxguid
# Windows DLLs referenced by win32_utf8, which are hopefully eliminated by
# -gc-sections one day…
LIBS += -lcomdlg32 -lgdi32 -lole32 -lpsapi -lshlwapi -lversion -lwininet
endif

# DSF engine
OBJS += eng_dsf/eng_dsf.o eng_dsf/dc_hw.o eng_dsf/aica.o eng_dsf/aicadsp.o eng_dsf/arm7.o eng_dsf/arm7i.o

# SSF engine
OBJS += eng_ssf/m68kcpu.o eng_ssf/m68kopac.o eng_ssf/m68kopdm.o eng_ssf/m68kopnz.o eng_ssf/m68kops.o
OBJS += eng_ssf/scsp.o eng_ssf/scspdsp.o eng_ssf/sat_hw.o eng_ssf/eng_ssf.o

# QSF engine
OBJS += eng_qsf/eng_qsf.o eng_qsf/kabuki.o eng_qsf/qsound.o eng_qsf/z80.o eng_qsf/z80dasm.o

# PSF engine
OBJS += eng_psf/eng_psf.o eng_psf/psx.o eng_psf/psx_hw.o eng_psf/peops/spu.o

# PSF2 extentions
OBJS += eng_psf/eng_psf2.o eng_psf/peops2/spu.o eng_psf/peops2/dma.o eng_psf/peops2/registers.o

# SPU engine (requires PSF engine)
OBJS += eng_psf/eng_spu.o

# zlib (included for max portability)
OBJS += zlib/adler32.o zlib/compress.o zlib/crc32.o zlib/gzio.o zlib/uncompr.o zlib/deflate.o zlib/trees.o
OBJS += zlib/zutil.o zlib/inflate.o zlib/infback.o zlib/inftrees.o zlib/inffast.o

# ImGui
ifndef NOGUI
PKG_CONFIG = $(shell which pkg-config 2>/dev/null)
ifeq ($(PKG_CONFIG),)
$(error pkg-config not found)
endif

GLFW3_LIBS = $(shell $(PKG_CONFIG) --static --libs-only-l glfw3 2>/dev/null)

ifeq ($(GLFW3_LIBS),)
$(warning GLFW3 development files not installed, debug GUI can not be built.)
$(error To build without the debug GUI, set NOGUI=1 before calling make)
endif

CFLAGS += -Iimgui/ -DIMGUI_INCLUDE_IMGUI_USER_H -DIMGUI_INCLUDE_IMGUI_USER_INL
LIBS += $(GLFW3_LIBS) -lstdc++

OBJS += imgui/imgui.o imgui/imgui_draw.o imgui/imgui_demo.o imgui/examples/opengl_example/imgui_impl_glfw.o
OBJS += eng_dsf/dc_debug.o
OBJS += debug.o

ifneq ($(OSTYPE),linux)
LIBS += -limm32
endif
else
CFLAGS += -DNOGUI
endif

MACHINE_OBJS = $(OBJS:%.o=obj/$(MACHINE)/%.o)

all: release
.PHONY: debug release

debug: CFLAGS += -g -DDEBUG
debug: $(EXE)

release: CFLAGS += -O3 -DNDEBUG
release: $(EXE)

obj/$(MACHINE)/%.o: %.c
	@echo Compiling $<...
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -o $@

obj/$(MACHINE)/%.o: %.cpp
	@echo Compiling $<...
	@mkdir -p $(@D)
	@$(CC) -std=c++1y $(CFLAGS) $< -o $@

$(EXE): $(MACHINE_OBJS)
	@echo Linking $(EXE)...
	@$(LD) $(LDFLAGS) -g -o $(EXE) $(MACHINE_OBJS) $(LIBS)

clean:
	rm -f $(MACHINE_OBJS) $(EXE)
	rm -r obj/$(MACHINE)
	rm -d obj/ 2>/dev/null
