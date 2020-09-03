TITLE_ID = VSDK00001
TITLE_ID = LUAMANGAS
TARGET   = MangaDownloader
OBJS     = src/main.o src/fpsCapper.o src/keyboardCode.o src/linkedList.o src/netCode.o src/openBSDstrcharstr.o src/decrypt.o

LIBS = -lgoodbrewvita -lvita2dplusbloat -lSceAppUtil_stub -lSceNet_stub -lSceHttp_stub -lSceNetCtl_stub \
	-lSceDisplay_stub -lSceGxm_stub -lSceSysmodule_stub -lSceCtrl_stub \
	-lSceCommonDialog_stub -lfreetype -lpng -ljpeg -lz -lm -lc -llua \
	-lm -lcurl -lssl -lcrypto -lz -lpthread -lSceAppMgr_stub -lSceTouch_stub -lcrypto

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CXX = $(PREFIX)-g++
CFLAGS  = -Wl,-q -Wall -O3 -g -Llib/Vita -IInclude -Wno-format-overflow -Wno-pointer-sign
CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)

all: $(TARGET).vpk

%.vpk: eboot.bin
	vita-mksfoex -s TITLE_ID=$(TITLE_ID) "Manga Downloader" param.sfo
	vita-pack-vpk -s param.sfo -b eboot.bin \
	--add VpkContents/sce_sys/icon0.png=sce_sys/icon0.png \
	--add VpkContents/sce_sys/livearea/contents/bg.png=sce_sys/livearea/contents/bg.png \
	--add VpkContents/sce_sys/livearea/contents/startup.png=sce_sys/livearea/contents/startup.png \
	--add VpkContents/sce_sys/livearea/contents/template.xml=sce_sys/livearea/contents/template.xml \
	MangaDownloader.vpk

eboot.bin: $(TARGET).velf
	vita-make-fself -s $< $@

%.velf: %.elf
	vita-elf-create $< $@

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

%.o: %.png
	$(PREFIX)-ld -r -b binary -o $@ $^

clean:
	@rm -rf $(TARGET).vpk $(TARGET).velf $(TARGET).elf $(OBJS) \
		eboot.bin param.sfo

vpksend: $(TARGET).vpk
	curl -T $(TARGET).vpk ftp://192.168.1.229:1337/ux0:/_stuffz/
	@echo "Sent."

send: eboot.bin
	curl -T eboot.bin ftp://192.168.1.229:1337/ux0:/app/$(TITLE_ID)/
	@echo "Sent."

stuffvpk:
	@7z a MangaDownloader.vpk ./VpkContents/*
# DO NOT DELETE

./src/decrypt.o: ./src/cryptshared.h
./src/linkedList.o: ./src/linkedList.h
./src/main.o: ./src/main.h ./src/main.gfx.h ./src/fpsCapper.h
./src/main.o: ./src/keyboardCode.h ./src/openBSDstrcharstr.h
./src/main.o: ./src/linkedList.h ./src/photo.h ./src/decrypt.h
./src/main.o: ./src/photoExtended.h ./src/Downloader.h ./src/netCode.h
./src/netCode.o: ./src/main.h
