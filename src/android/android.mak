CPUDEFS += -DHAS_CYCLONE=1 -DHAS_DRZ80=1
OBJDIRS += $(OBJ)/cpu/m68000_cyclone $(OBJ)/cpu/z80_drz80
CPUOBJS += $(OBJ)/cpu/m68000_cyclone/cyclone.o $(OBJ)/cpu/m68000_cyclone/c68000.o
CPUOBJS += $(OBJ)/cpu/z80_drz80/drz80.o $(OBJ)/cpu/z80_drz80/drz80_z80.o

#OBJDIRS += $(OBJ)/cpu/nec_armnec
#CPUDEFS += -DHAS_ARMNEC
#CPUOBJS += $(OBJ)/cpu/nec_armnec/armV30.o $(OBJ)/cpu/nec_armnec/armV33.o $(OBJ)/cpu/nec_armnec/armnecintrf.o


OBJC = $(OBJ)/android/minimal.o \
	$(OBJ)/android/android_main.o $(OBJ)/android/video.o $(OBJ)/android/blit.o \
	$(OBJ)/android/sound.o $(OBJ)/android/input.o $(OBJ)/android/fileio.o \
	$(OBJ)/android/config.o $(OBJ)/android/fronthlp.o \
	$(OBJ)/android/memcmp.o $(OBJ)/android/memcpy.o $(OBJ)/android/memset.o \
	$(OBJ)/android/strcmp.o $(OBJ)/android/strlen.o $(OBJ)/android/strncmp.o \
	$(OBJ)/android/shared.o $(OBJ)/android/android_frontend.o $(OBJ)/android/wiimote.o
	

	


