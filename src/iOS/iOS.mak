#CPUDEFS += -DHAS_CYCLONE=1 -DHAS_DRZ80=1
#OBJDIRS += $(OBJ)/cpu/m68000_cyclone $(OBJ)/cpu/z80_drz80
#CPUOBJS += $(OBJ)/cpu/m68000_cyclone/cyclone.o $(OBJ)/cpu/m68000_cyclone/c68000.o
#CPUOBJS += $(OBJ)/cpu/z80_drz80/drz80.o $(OBJ)/cpu/z80_drz80/drz80_z80.o

#OBJDIRS += $(OBJ)/cpu/nec_armnec
#CPUDEFS += -DHAS_ARMNEC
#CPUOBJS += $(OBJ)/cpu/nec_armnec/armV30.o $(OBJ)/cpu/nec_armnec/armV33.o $(OBJ)/cpu/nec_armnec/armnecintrf.o


OBJC = $(OBJ)/iOS/minimal.o \
	$(OBJ)/iOS/iOS.o $(OBJ)/iOS/video.o $(OBJ)/iOS/blit.o \
	$(OBJ)/iOS/sound.o $(OBJ)/iOS/input.o $(OBJ)/iOS/fileio.o \
	$(OBJ)/iOS/config.o $(OBJ)/iOS/fronthlp.o \
	$(OBJ)/iOS/shared.o $(OBJ)/iOS/iOS_frontend.o $(OBJ)/iOS/wiimote.o
	

	


