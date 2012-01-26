This is an open-source 12-channel DMX512 dimmer, currently known as
dmx-dimmer. The name will change to something less meaningful by
production time. It is based on [Hendrik Hoelscher's dimmer][hoelscher].

The goal is to produce schematics, PCB layouts and code, all available
in a form that allows modification and homebrew manufacture. This is
important, since tool/material/component availability varies from place
to place.

Many modifications have been made, but most importantly:

- support up to 12 channels;
- use 3-phase mains power, 4 channels per phase;
- more modern AVR microcontrollers, ATMega168 and ATTiny2313;
- complete code rewrite in C.

This is a work in progress, currently in early testing stage.
Documentation is basically unavailable, but might become so if anyone is
interested. Most of what exists is in the git log.

It is probably impossible to use this git project for now, since some
custom symbols are missing from the tree. I will try to include them ASAP.
Which means, when someone asks.

[gEDA][geda] was chosen to do the design work. As to why is a topic for a
whole article. Mainly, I wanted to see whether it's good enough for
medium-sized projects. The short answer is maybe, if you're stubborn as I.

[hoelscher]: http://www.hoelscher-hi.de/hendrik/english/dimmer.htm
[geda]: http://www.gpleda.org/index.html
