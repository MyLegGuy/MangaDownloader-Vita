TITLE_ID = VSDK00001
TITLE_ID = LUAMANGAS
TARGET   = MangaDownloader
OBJS     = src/main.o src/fpsCapper.o src/keyboardCode.o src/linkedList.o src/netCode.o src/openBSDstrcharstr.o

LIBS = -lGeneralGood -lvita2dfixed -lSceAppUtil_stub -lSceNet_stub -lSceHttp_stub -lSceNetCtl_stub \
	-lSceDisplay_stub -lSceGxm_stub -lSceSysmodule_stub -lSceCtrl_stub \
	-lSceCommonDialog_stub -lfreetype -lpng -ljpeg -lz -lm -lc -llua \
	-lm -lcurl -lssl -lcrypto -lz -lpthread

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CXX = $(PREFIX)-g++
CFLAGS  = -Wl,-q -Wall -O3 -g -Llib/Vita -IInclude
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
