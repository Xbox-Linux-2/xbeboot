TOPDIR  := $(shell /bin/pwd)

XBE_TITLE = xbeboot

ifeq ($(OUTPUT_DIR),)
OUTPUT_DIR = bin
endif

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
LD           = ld.lld-8 -flavor link
AS           = clang-8
CC           = clang-8
CXX          = clang++-8
CGC          = $(TOPDIR)/tools/cg/linux/cgc
endif

TARGET       = $(OUTPUT_DIR)/default.xbe
CXBE         = $(TOPDIR)/tools/cxbe/cxbe
TOOLS        = cxbe
CFLAGS  = -target i386-pc-win32 -march=pentium3 -fsave-optimization-record \
               -ffreestanding -nostdlib -fno-builtin -fno-exceptions \
               -I$(TOPDIR)/lib -I$(TOPDIR)/lib/xboxrt \
               -isystem $(TOPDIR)/lib/xlibc/include \
               -Wno-ignored-attributes -DNXDK
ASFLAGS = -target i386-pc-win32 -march=pentium3 \
               -nostdlib -I$(TOPDIR)/lib -I$(TOPDIR)/lib/xboxrt

include $(TOPDIR)/lib/Makefile
OBJS = $(addsuffix .obj, $(basename $(SRCS)))

OPTRECORDS = $(addsuffix .opt.yaml, $(basename $(SRCS)))
LIBS = $(addsuffix .lib, $(basename $(SRCS)))

QUIET=>/dev/null

DEPS := $(filter %.c.d, $(SRCS:.c=.c.d))

all: $(TARGET)

$(OUTPUT_DIR)/default.xbe: main.exe $(OUTPUT_DIR) $(CXBE)
	@echo "[ CXBE ] $@"
	@$(CXBE) -OUT:$@ -TITLE:$(XBE_TITLE) $< $(QUIET)

$(OUTPUT_DIR):
	@mkdir -p $(OUTPUT_DIR);

main.exe: $(OBJS) $(TOPDIR)/lib/xboxkrnl/libxboxkrnl.lib
	@echo "[ LD ] $@"
	@$(LD) $(LDFLAGS) -subsystem:windows -dll -out:'$@' -entry:XboxCRT $^

%.obj: %.c
	@echo "[ CC ] $@"
	@$(CC) $(CFLAGS) -c -o '$@' '$<'

%.obj: %.s
	@echo "[ AS ] $@"
	@$(AS) $(ASFLAGS) -c -o '$@' '$<'

%.obj: %.S
	@echo "[ AS ] $@"
	@$(AS) $(ASFLAGS) -c -o '$@' '$<'

tools: $(TOOLS)
.PHONY: tools $(TOOLS)

cxbe: $(CXBE)
$(CXBE):
	@echo "[ CXBE ] $@"
	@$(MAKE) -C $(TOPDIR)/tools/cxbe $(QUIET)

vp20compiler: $(VP20COMPILER)
$(VP20COMPILER):
	@echo "[ VP20 ] $@"
	@$(MAKE) -C $(TOPDIR)/tools/vp20compiler $(QUIET)

fp20compiler: $(FP20COMPILER)
$(FP20COMPILER):
	@echo "[ FP20 ] $@"
	@$(MAKE) -C $(TOPDIR)/tools/fp20compiler $(QUIET)

extract-xiso: $(EXTRACT_XISO)
$(EXTRACT_XISO):
	@echo "[ XISO ] $@"
	@$(MAKE) -C $(TOPDIR)/tools/extract-xiso $(QUIET)

.PHONY: clean 
clean:
	@rm -f $(TARGET) \
	           main.exe main.exe.manifest \
	           $(OBJS) $(SHADER_OBJS) \
	           $(GEN_XISO) ${DEPS} ${OPTRECORDS} \
                   ${LIBS} 
	@rm -r ${OUTPUT_DIR}

.PHONY: distclean 
distclean: clean
	@$(MAKE) -C $(TOPDIR)/tools/extract-xiso clean $(QUIET)
	@$(MAKE) -C $(TOPDIR)/tools/fp20compiler distclean $(QUIET)
	@$(MAKE) -C $(TOPDIR)/tools/vp20compiler distclean $(QUIET)
	@$(MAKE) -C $(TOPDIR)/tools/cxbe clean $(QUIET)
	@bash -c "if [ -d $(OUTPUT_DIR) ]; then rmdir $(OUTPUT_DIR); fi"
	@rm -f $(DEPS)

-include $(DEPS)
