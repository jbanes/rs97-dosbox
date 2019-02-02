# RS97-dosbox

This project is a fork of the dosbox code from https://github.com/steward-fu/rs97_emulator.

If you just want to start using the program, you can grab the "dosbox" executable from dosbox/bin/dosbox, install it onto your RS97 and start having fun!

In case you want to compile this yourself, note that you'll need a MIPS toolchain built to /opt/buildroot on Linux (I'm using Ubuntu.) Simply run "make" and then "./package" to populate the bin/ folder. 

If you wish to recreate the project clean, execute the following configure command:
```
./configure --host=mipsel-linux --disable-opengl --disable-alsa-midi --disable-dynamic-x86 --disable-fpu-x86 --enable-core-inline CXXFLAGS="-g -O2 -G0 -march=mips32 -mtune=mips32 -pipe -fno-builtin -fno-common -ffast-math -fomit-frame-pointer -fexpensive-optimizations -frename-registers" LIBS="-lSDL_gfx -lSDL_image" 
```
Follow the instructions in dosbox/README-GCW0.txt that explain how to change the config.h. This will enable the high performance dynamic core. Complete the build by running the following commands:

```
make
./package
```
If you wish to use the dynamic core, make sure your dosbox.conf file has "core=dynamic". You can change the core using the menu once DOSBox is started. If DOSBox is started without the dynamic core, the dynamic core will not be accessible.


