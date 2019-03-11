xbeboot - *the original Xbox Linux bootloader*
==============================================
xbeboot is a Linux bootloader for the original Xbox. xbeboot was originally developed by the [Xbox Linux Project](https://web.archive.org/web/20100617000252/http://www.xbox-linux.org/wiki/Main_Page).

xbeboot is noteworthy for not only allowing booting from DVD/CD and Hard Drive, but also allowing the bundling of a kernel into the .xbe itself

Status
------
xbeboot has full graphics support and can boot into the Linux kernel, however it doesn't seem to correctly pass kernel command line arguments into the kernel quite yet. Also imagebld doesn't work so kernels can't be compiled into the .xbe quite yet

TODO
----
Fix command line argument passing

Fix imagebld

Allow booting from more storage mediums

Make it prettier

Support GRUB modules so MINIX3 can be booted

Getting Started
---------------
### Prerequisites
You will need the following tools:
- make
- clang
- lld

### Download & Build xbeboot
    git clone https://github.com/Xbox-Linux-2/xbeboot.git
    cd xbeboot
    make all
