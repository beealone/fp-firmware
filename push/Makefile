arm := y
#mips26:= y

ifdef arm
CC=arm-linux-gcc
LD=arm-linux-ld
AR=arm-linux-ar
else
ifdef mips26
CC = mipsel-linux-gcc
LD = mipsel-linux-ld
AR = mipsel-linux-ar
else
CC = mipsel-gcc
LD = mipsel-ld
AR = mipsel-ar
endif
endif

OBJCAT = *.o
COMPILE_FILE = *.c
all:	libhttppush.so
#libhttppush.a

libhttppush.so:
	$(CC) -Wall  -c -fPIC -O2 $(COMPILE_FILE)
	$(CC) -Wall --shared   -o $@ *.o
	cp $@ /tftpboot/. -rf
	cp $@ ../M880trunk/M880trunk/zem600/. -rf
#	cp $@ /home/work/src/iclock400mul/trunk_lib/zem510/.
#	cp $@ /home/work/src/iclock2800mul/trunk_lib/zem600/.
#cp $@  /mnt/hgfs/WorkPrj/telnet/ -f



libhttppush.a:
	$(CC) -c -fPIC -O2 $(COMPILE_FILE)
	$(AR)  -rcs $@  $(OBJCAT)

clean :
	rm *.o *.c~ *.d *.so *.a -rf 

