# lifebox

```
$ make
$ sudo ./lifebox
usage: ./lifebox <options> 
Options:
        -t <seconds>              : Run for these number of seconds, then exit.
        -m <milliseconds>         : Lifebox speed. Default 15.
        -V <r,g,b>                : Species-Comp-Color. Default 255,0,255
        -W <r,g,b>                : Plantes-Color. Default 255,255,255
        -X <r,g,b>                : Species1-Color, Default 255,255,0
        -Y <r,g,b>                : Species2-Color. Default 0,255,255
        -Z <r,g,b>                : Nothing-Color. Default 0,0,0
        
        --led-gpio-mapping=<name> : Name of GPIO mapping used. Default "regular"
        --led-rows=<rows>         : Panel rows. Typically 8, 16, 32 or 64. (Default: 32).
        --led-cols=<cols>         : Panel columns. Typically 32 or 64. (Default: 32).
        --led-chain=<chained>     : Number of daisy-chained panels. (Default: 1).
        --led-parallel=<parallel> : Parallel chains. range=1..3 (Default: 1).
        --led-multiplexing=<0..6> : Mux type: 0=direct; 1=Stripe; 2=Checkered; 3=Spiral; 4=ZStripe; 5=ZnMirrorZStripe; 
                                    6=coreman (Default: 0)
        --led-pixel-mapper        : Semicolon-separated list of pixel-mappers to arrange pixels.
                                    Optional params after a colon e.g. "U-mapper;Rotate:90"
                                    Available: "Rotate", "U-mapper". Default: ""
        --led-pwm-bits=<1..11>    : PWM bits (Default: 11).
        --led-brightness=<percent>: Brightness in percent (Default: 100).
        --led-scan-mode=<0..1>    : 0 = progressive; 1 = interlaced (Default: 0).
        --led-row-addr-type=<0..2>: 0 = default; 1 = AB-addressed panels; 2 = direct row select(Default: 0).
        --led-show-refresh        : Show refresh rate.
        --led-inverse             : Switch if your matrix has inverse colors on.
        --led-rgb-sequence        : Switch if your matrix has led colors swapped (Default: "RGB")
        --led-pwm-lsb-nanoseconds : PWM Nanoseconds for LSB (Default: 130)
        --led-pwm-dither-bits=<0..2> : Time dithering of lower bits (Default: 0)
        --led-no-hardware-pulse   : Don't use hardware pin-pulse generation.
        --led-slowdown-gpio=<0..2>: Slowdown GPIO. Needed for faster Pis/slower panels (Default: 1).
        --led-daemon              : Make the process run in the background as daemon.
        --led-no-drop-privs       : Don't drop privileges from 'root' after initializing the hardware.

Examples:
        ./lifebox -V201,80,76 -X240,230,165 -Y222,199,75 -W10,172,130 -Z0,0,0 
        ./lifebox -t 10
        ./lifebox 
```
<div style="background:black">
<p align="center">
<img src="https://i.imgur.com/MsjFOf8.jpg" alt="lifebox" /> 
</p>
</div>


Hardware:
- Raspberry Pi 2 Model B v1.1
- RGB LED Panel 32x32 COM-12584
- Power supply MeanWell, RS-15-5
- [SHUTDOWN CIRCUIT (USE YOUR OWN SWITCH)](https://mausberry-circuits.myshopify.com/products/shutdown-circuit-use-your-own-switch)
- [Adapter PCB OSH Park](https://github.com/hzeller/rpi-rgb-led-matrix/tree/master/adapter/passive-rpi1)
- [Physical custom switch](https://www.conrad.de/de/wippschalter-250-vac-6-a-1-x-ausein-r13-112a-02-bb-onoff-rastend-1-st-700339.html)

