USB_SRCS := \
	$(TOPDIR)/lib/usb/host/ohci-hcd.c \
	$(TOPDIR)/lib/usb/core/message.c \
	$(TOPDIR)/lib/usb/core/hcd.c \
	$(TOPDIR)/lib/usb/core/hcd-pci.c \
	$(TOPDIR)/lib/usb/core/hub.c \
	$(TOPDIR)/lib/usb/core/usb.c \
	$(TOPDIR)/lib/usb/core/config.c \
	$(TOPDIR)/lib/usb/core/urb.c \
	$(TOPDIR)/lib/usb/core/buffer_simple.c \
	$(TOPDIR)/lib/usb/core/usb-debug.c \
	$(TOPDIR)/lib/usb/sys/BootUSB.c \
	$(TOPDIR)/lib/usb/sys/linuxwrapper.c \
	$(TOPDIR)/lib/usb/sys/xpad.c \
	$(TOPDIR)/lib/usb/sys/xremote.c \
	$(TOPDIR)/lib/usb/sys/usbkey.c \
	$(TOPDIR)/lib/usb/sys/usbmouse.c \
	$(TOPDIR)/lib/usb/misc/misc.c \
	$(TOPDIR)/lib/usb/misc/pci.c \
	$(TOPDIR)/lib/usb/misc/malloc.c

SRCS += \
	$(wildcard $(TOPDIR)/lib/xboxrt/*.c) \
	$(shell find $(TOPDIR)/lib/xlibc/src/ -name "*.c") \
	$(shell find $(TOPDIR)/lib/xlibc/src/ -name "*.s") \
	$(wildcard $(TOPDIR)/lib/xlibc-rt/*.c) \
	$(wildcard $(TOPDIR)/lib/xlibc-rt/*.s) \
	$(wildcard $(TOPDIR)/lib/hal/*.c) \
	$(wildcard $(TOPDIR)/lib/pbkit/*.c) \
	$(wildcard $(TOPDIR)/*.c) \
	$(wildcard $(TOPDIR)/*.S) \
	$(wildcard $(TOPDIR)/*.s) \
	$(USB_SRCS)
