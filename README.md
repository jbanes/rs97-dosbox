# RS97-dosbox

This project is a fork of the dosbox code from https://github.com/steward-fu/rs97_emulator.

If you just want to start using the program, you can grab the "dosbox" executable from dosbox/bin/dosbox, install it onto your RS97 and start having fun!

In case you want to compile this yourself, note that you'll need a MIPS toolchain built to /opt/buildroot on Linux (I'm using Ubuntu.) Simply run "make" and then "./package" to populate the bin/ folder. 

If you wish to recreate the project clean, use the following steps:
```
make clean
./configure --host=mipsel-linux --disable-opengl --disable-alsa-midi --disable-dynamic-x86 --disable-fpu-x86 --enable-core-inline CXXFLAGS="-g -O2 -G0 -march=mips32 -mtune=mips32 -pipe -fno-builtin -fno-common -fno-shared -ffast-math -fomit-frame-pointer -fexpensive-optimizations -frename-registers" LIBS="-lSDL_gfx -lSDL_image" 
make
./package
```
