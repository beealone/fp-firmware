EXECUTABLE := main


######以下是编译平台的开关

#zem600		:= y    #3.5寸屏支持ZEM600平台
#tft35		:= y    #3.5寸屏支持ZEM510平台
#tft300		:= y    #3寸屏支持510平台
#tft300_26	:= y    #3寸屏支持560平台（2.6内核）
#tft300_800	:= y    #3寸屏支持800平台（2.6内核）
#u8		:= y    #3寸屏U8系列支持510平台（屏幕倒装）
#u8_26		:= y    #3寸屏U8系列支持560平台（2.6内核屏幕倒装）
#u8_800		:= y    #3寸屏U8系列支持800平台（2.6内核屏幕倒装）
zem800_35       := y    #3.5寸屏支持ZEM800平台


#######3.5寸屏支持ZEM600平台
ifdef zem600
	ZEM600 := -DZEM600
	PLATFORM := zem600
	CROSS_COMPILE := arm-linux-
	DEVICENAME := zem600s
	ZEM600LIB := 1
endif	

#######3.5寸屏支持ZEM800平台
ifdef zem800_35
	ZEM600 := -DZEM600 -DZEM800
	PLATFORM := zem600
	CROSS_COMPILE := arm-linux-
	DEVICENAME := zem600s
	ZEM600LIB := 1
endif	


#######支持3.5寸屏支持ZEM510平台
ifdef tft35
	ZEM510 := -DZEM510
	PLATFORM := zem510
	CROSS_COMPILE := mipsel-
	DEVICENAME := 400.zk
endif	


#######3寸屏支持510平台
ifdef tft300
	ZEM510 := -DZEM510
	PLATFORM := zem510
	CROSS_COMPILE := mipsel-
	DEVICENAME := mul300_auto
endif	


#######3寸屏支持560平台
ifdef tft300_26
	ZEM510 := -DZEM510
	KERNEL:=-DLINUX_26KERNEL
	PLATFORM := zem560
	CROSS_COMPILE := mipsel-linux-
	DEVICENAME := mul300_auto_zem560
	ZEM560LIB := 1
endif


#######3寸屏支持800平台
ifdef tft300_800
	ZEM600 := -DZEM600 -DZEM800
	PLATFORM := zem600
	CROSS_COMPILE := arm-linux-
	DEVICENAME := zem600ds10
	ZEM600LIB := 1
endif


#######3寸屏U8系列支持510平台（屏幕倒装）
ifdef u8
	ZEM510 := -DZEM510
	PLATFORM := zem510
	CROSS_COMPILE := mipsel-
	DEVICE := -DNew_U8	
	DEVICENAME := u8_auto
endif	


#######3寸屏U8系列支持560平台（屏幕倒装）
ifdef u8_26
	ZEM510 := -DZEM510
	KERNEL:=-DLINUX_26KERNEL
	PLATFORM := zem560
	CROSS_COMPILE := mipsel-linux-
	DEVICE := -DNew_U8	
	DEVICENAME := u8_auto_zem560
	ZEM560LIB := 1
endif


#######3寸屏U8系列支持800平台（屏幕倒装）
ifdef u8_800
	ZEM600 := -DZEM600
	PLATFORM := zem600
	CROSS_COMPILE := arm-linux-
	DEVICE := -DNew_U8	
	DEVICENAME := U8.zem600ds10
	ZEM600LIB := 1
endif

#######公共参数
ZEM300 := -DZEM300

#######仅仅支持中文的时候需要打开此开关，这个针对国内不支持多国语言的机器
chinese_only:=y
ifdef chinese_only
	ZEM300 += -DCHINESE_ONLY
endif	

#######Support TTS Function
#TTS_SUPPORT := -D_TTS_
#TTSLIB := -ltts

#######webserver功能，默认是打开的，自动选择的方式,实际上这个已经没有用
#libdl.a   #this lib for webserver, only zem510 use this lib

#MP3PLAY := -DMP3PLAY
#MP3LIB := ./$(PLATFORM)/mpg123.a


MATHLIB := -lm
MATHLIB := ./$(PLATFORM)/libm.a
#URULIB := ./$(PLATFORM)/libusbdpfp.a

####### FACE Function  ----- only ZEM600
#FACE := face

ifdef FACE
	ZEM300 += -DFACE
endif

#wifi, gprs use thread Mode. only ZEM800
ifdef zem800_35
	ZEM600 += -DTHREAD_VERSION
endif

MINIGUI := -I /usr/local/kenny.$(DEVICENAME)/include/

#-msoft-float -mips32 -Wall -O6
CFLAGS :=-g -I. -I./$(PLATFORM) -Wall -D__GNU__ $(WEBSERVER) $(MP3PLAY) $(ZEM300) $(SENSOR) $(ZEM510) $(ZEM600) $(DEVICE) -I./$(FACE) 
CFLAGS := -O2 -I. -I./$(PLATFORM) -I./fpsensor -I./iclock  -I./modem $(MINIGUI) -Wall -D__GNU__ $(WEBSERVER) $(MP3PLAY) $(ZEM300) $(SENSOR) $(ZEM510) $(ZEM600) $(DEVICE) $(TTS_SUPPORT) $(KERNEL) -I ./$(FACE) -I./wifi

CXXFLAGS := $(CFLAGS)
CC := $(CROSS_COMPILE)gcc
STRIP := $(CROSS_COMPILE)strip
FPDIR := em_source_42

#FPLIB := -L./$(PLATFORM) -ldlcl $(WEBLIB) $(MP3LIB) -L /usr/local/kenny.$(DEVICENAME)/lib/ -lminigui -lmgext $(PLATFORM)/libcam.a -lttf -lhttppush
FPLIB := -L./$(PLATFORM) -ldlcl $(WEBLIB) $(MP3LIB) -L /usr/local/kenny.$(DEVICENAME)/lib/ -lminigui -lmgext -lttf -lhttppush

ifdef ZEM600LIB
	FPLIB += -L /usr/local/arm-linux-4.1.1/arm-iwmmxt-linux-gnueabi/lib/ -ljpeg -L $(PLATFORM) -lasound  -L./$(PLATFORM) -lcam
else
ifdef ZEM560LIB
       FPLIB += -L /opt/mipseltools-gcc412-glibc261_kernel_2.6/mipsel-linux/lib/ -ljpeg $(PLATFORM)/libshttpd.a  $(PLATFORM)/libnandflashstatic.a -lpthread  -L./$(PLATFORM) -ldl  $(PLATFORM)/libcx.a $(PLATFORM)/libcam.a
else
	FPLIB += -L /usr/local/mips-gcc3.2.3/mipsel-redhat-linux/lib/ -ljpeg $(PLATFORM)/libshttpd.a $(PLATFORM)/libdl.a $(PLATFORM)/libnandflashstatic.a -lpthread  -L./$(PLATFORM)  $(PLATFORM)/libcx.a $(PLATFORM)/libcam.a
endif
endif

#SOURCE := $(wildcard *.c) $(wildcard *.cc) $(wildcard $(PLATFORM)/*.c fpsensor/*.c) $(wildcard iclock/*.c)
SOURCE := $(wildcard *.c) $(wildcard *.cc) $(wildcard $(PLATFORM)/*.c fpsensor/*.c) $(wildcard iclock/*.c) $(wildcard wifi/*.c)

FPLIB +=  -L./modem  -lmodem
SOURCE += $(wildcard modem/*.c)

ifdef FACE
	FPLIB += -lZKFace
	SOURCE += $(wildcard face/*.c)
endif

OBJS := $(patsubst %.c,%.o,$(patsubst %.cc,%.o, $(SOURCE)))
DEPS := $(patsubst %.o,%.d,$(OBJS))
MISSING_DEPS := $(filter-out $(wildcard $(DEPS)),$(DEPS))
MISSING_DEPS_SOURCES := $(wildcard $(patsubst %.d,%.c,$(MISSING_DEPS)) \
$(patsubst %.d,%.cc,$(MISSING_DEPS)))
CPPFLAGS += -MD

dest : $(OBJS)
	$(CROSS_COMPILE)gcc -I. -I./$(PLATFORM)  -c -o ./$(PLATFORM)/$(PLATFORM).o ./$(PLATFORM)/$(PLATFORM).c 
	$(CROSS_COMPILE)gcc -o $(EXECUTABLE) $(OBJS) $(FPLIB) $(URULIB) -lcrypt -L./$(PLATFORM) $(MATHLIB) $(TTSLIB) 

	$(STRIP) --strip-all $(EXECUTABLE)
#	cp $(EXECUTABLE) /var/tftpboot/$(EXECUTABLE)_$(LOGNAME) -f
	cp $(EXECUTABLE) /tftpboot/$(EXECUTABLE) -f

clean:
	rm  -f $(OBJS) 
	rm -f $(EXECUTABLE)

cleanall:
	rm  -f $(OBJS) $(EXECUTABLE)
	rm  *.o $(PLATFORM)/*.o fpsensor/*.o iclock/*.o modem/*.o wifi/*.o -f
	rm  *.d $(PLATFORM)/*.d fpsensor/*.d iclock/*.d modem/*.d wifi/*.d -f
	rm  *.h~ -f
	rm  *.c~ -f
	
