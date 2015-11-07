CPUDEFS += -DHAS_CYCLONE=1 -DHAS_DRZ80=1
OBJDIRS += $(OBJ)/cpu/m68000_cyclone $(OBJ)/cpu/z80_drz80
CPUOBJS += $(OBJ)/cpu/m68000_cyclone/cyclone.o $(OBJ)/cpu/m68000_cyclone/c68000.o
CPUOBJS += $(OBJ)/cpu/z80_drz80/drz80.o $(OBJ)/cpu/z80_drz80/drz80_z80.o

#OBJDIRS += $(OBJ)/cpu/nec_armnec
#CPUDEFS += -DHAS_ARMNEC
#CPUOBJS += $(OBJ)/cpu/nec_armnec/armV30.o $(OBJ)/cpu/nec_armnec/armV33.o $(OBJ)/cpu/nec_armnec/armnecintrf.o

OSOBJS = $(OBJ)/wiz/wiz_lib.o $(OBJ)/wiz/uppermem.o \
    $(OBJ)/wiz/warm.o $(OBJ)/wiz/pollux_set.o $(OBJ)/wiz/sys_cacheflush.o \
	$(OBJ)/wiz/memcmp.o $(OBJ)/wiz/memcpy.o $(OBJ)/wiz/memset.o \
	$(OBJ)/wiz/strcmp.o $(OBJ)/wiz/strlen.o $(OBJ)/wiz/strncmp.o \
	$(OBJ)/wiz/wiz.o $(OBJ)/wiz/video.o $(OBJ)/wiz/blit.o \
	$(OBJ)/wiz/sound.o $(OBJ)/wiz/input.o $(OBJ)/wiz/fileio.o \
	$(OBJ)/wiz/config.o $(OBJ)/wiz/fronthlp.o

mame.gpe:
	$(LD) $(LDFLAGS) \
		src/wiz/wiz_lib.cpp src/wiz/uppermem.cpp src/wiz/pollux_set.cpp src/wiz/sys_cacheflush.S src/wiz/wiz_frontend.cpp $(LIBS) -o $@
