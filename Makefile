TOPDIR  := $(shell /bin/pwd)

XBE_TITLE = xbeboot

ifeq ($(OUTPUT_DIR),)
OUTPUT_DIR = bin
endif

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
LD           = ld.lld-8 -flavor link
#LDFLAGS      = /EXPORT:__imp__main=boot
AS           = clang-8
CC           = clang-8
CXX          = clang++-8
CGC          = $(TOPDIR)/tools/cg/linux/cgc
endif
ifeq ($(UNAME_S),Darwin)
LD           = /usr/local/opt/llvm/bin/lld -flavor link
AS           = /usr/local/opt/llvm/bin/clang
CC           = /usr/local/opt/llvm/bin/clang
CXX          = /usr/local/opt/llvm/bin/clang++
CGC          = $(TOPDIR)/tools/cg/linux/cgc
endif
ifneq (,$(findstring MSYS_NT,$(UNAME_S)))
$(error Please use a MinGW64 shell)
endif
ifneq (,$(findstring MINGW,$(UNAME_S)))
LD           = lld-link
AS           = clang
CC           = clang
CXX          = clang++
CGC          = $(TOPDIR)/tools/cg/win/cgc
endif

TARGET       = $(OUTPUT_DIR)/default.xbe
CXBE         = $(TOPDIR)/tools/cxbe/cxbe
VP20COMPILER = $(TOPDIR)/tools/vp20compiler/vp20compiler
FP20COMPILER = $(TOPDIR)/tools/fp20compiler/fp20compiler
EXTRACT_XISO = $(TOPDIR)/tools/extract-xiso/extract-xiso
TOOLS        = cxbe vp20compiler fp20compiler extract-xiso
CFLAGS  = -target i386-pc-win32 -march=pentium3 -fsave-optimization-record \
               -ffreestanding -nostdlib -fno-builtin -fno-exceptions \
               -I$(TOPDIR)/lib -I$(TOPDIR)/lib/xboxrt \
               -isystem $(TOPDIR)/lib/xlibc/include \
               -Wno-ignored-attributes -DNXDK
ASFLAGS = -target i386-pc-win32 -march=pentium3 \
               -nostdlib -I$(TOPDIR)/lib -I$(TOPDIR)/lib/xboxrt

ifeq ($(DEBUG),y)
DEBUG_CFLAGS += -g
LDFLAGS += /debug
endif

CFLAGS += $(DEBUG_CFLAGS)

include $(TOPDIR)/lib/Makefile
OBJS = $(addsuffix .obj, $(basename $(SRCS)))

OPTRECORDS = $(addsuffix .opt.yaml, $(basename $(SRCS)))
LIBS = $(addsuffix .lib, $(basename $(SRCS)))

ifneq ($(NXDK_SDL),)
include $(TOPDIR)/lib/sdl/Makefile
endif

V = 0
VE_0 := @
VE_1 :=
VE = $(VE_$(V))

ifeq ($(V),1)
QUIET=
else
QUIET=>/dev/null
endif

DEPS := $(filter %.c.d, $(SRCS:.c=.c.d))

all: $(TARGET)

$(OUTPUT_DIR)/default.xbe: main.exe $(OUTPUT_DIR) $(CXBE)
	@echo "[ CXBE ] $@"
	$(VE)$(CXBE) -OUT:$@ -TITLE:$(XBE_TITLE) $< $(QUIET)
	#@echo "[ ImageBLD ] $@"
	#$(TOPDIR)/imagebld/image -build $(TOPDIR)/bin/default.xbe  $(TOPDIR)/vmlinuz $(TOPDIR)/initramfs.cpio.gz  $(TOPDIR)/linuxboot.cfg
	@ls -l $@

$(OUTPUT_DIR):
	@mkdir -p $(OUTPUT_DIR);

main.exe: $(OBJS) $(TOPDIR)/lib/xboxkrnl/libxboxkrnl.lib
	#@$(CC) $(TOPDIR)/imagebld/imagebld.c $(TOPDIR)/imagebld/sha1.c -o $(TOPDIR)/imagebld/image
	@echo "[ LD ] $@"
	$(VE) $(LD) $(LDFLAGS) -subsystem:windows -dll -out:'$@' -entry:XboxCRT $^

%.obj: %.c
	@echo "[ CC ] $@"
	$(VE) $(CC) $(CFLAGS) -c -o '$@' '$<'

%.obj: %.s
	@echo "[ AS ] $@"
	$(VE) $(AS) $(ASFLAGS) -c -o '$@' '$<'

%.obj: %.S
	@echo "[ AS ] $@"
	$(VE) $(AS) $(ASFLAGS) -c -o '$@' '$<'

%.c.d: %.c
	@echo "[ DEP ] $@"
	$(VE) set -e; rm -f $@; \
	$(CC) -M -MM -MG -MT '$*.obj' -MF $@ $(CFLAGS) $<; \
	echo "\n$@ : $^\n" >> $@

tools: $(TOOLS)
.PHONY: tools $(TOOLS)

cxbe: $(CXBE)
$(CXBE):
	@echo "[ CXBE ] $@"
	$(VE)$(MAKE) -C $(TOPDIR)/tools/cxbe $(QUIET)

vp20compiler: $(VP20COMPILER)
$(VP20COMPILER):
	@echo "[ VP20 ] $@"
	$(VE)$(MAKE) -C $(TOPDIR)/tools/vp20compiler $(QUIET)

fp20compiler: $(FP20COMPILER)
$(FP20COMPILER):
	@echo "[ FP20 ] $@"
	$(VE)$(MAKE) -C $(TOPDIR)/tools/fp20compiler $(QUIET)

extract-xiso: $(EXTRACT_XISO)
$(EXTRACT_XISO):
	@echo "[ XISO ] $@"
	$(VE)$(MAKE) -C $(TOPDIR)/tools/extract-xiso $(QUIET)

.PHONY: clean 
clean:
	$(VE)rm -f $(TARGET) \
	           main.exe main.exe.manifest \
	           $(OBJS) $(SHADER_OBJS) \
	           $(GEN_XISO) ${DEPS} ${OPTRECORDS} \
                   ${LIBS} 
	${VE}rm -r ${OUTPUT_DIR}

.PHONY: distclean 
distclean: clean
	$(VE)$(MAKE) -C $(TOPDIR)/tools/extract-xiso clean $(QUIET)
	$(VE)$(MAKE) -C $(TOPDIR)/tools/fp20compiler distclean $(QUIET)
	$(VE)$(MAKE) -C $(TOPDIR)/tools/vp20compiler distclean $(QUIET)
	$(VE)$(MAKE) -C $(TOPDIR)/tools/cxbe clean $(QUIET)
	$(VE)bash -c "if [ -d $(OUTPUT_DIR) ]; then rmdir $(OUTPUT_DIR); fi"
	$(VE)rm -f $(DEPS)

-include $(DEPS)
