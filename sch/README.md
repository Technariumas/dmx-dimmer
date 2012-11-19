Modularisation is best shown in a diagram.

         DMX-DIMMER
    panel
      |
      |
    master --+- slave --+- dimmer
             |          +- dimmer
             |          +- dimmer
             |          +- dimmer
             |
             +- slave --+- dimmer
             |          +- dimmer
             |          +- dimmer
             |          +- dimmer
             |
             +- slave --+- dimmer
                        +- dimmer
                        +- dimmer
                        +- dimmer

There are currently four non-optional modules. Further division can be
carried out, if desired.

Digital power and zero-cross detection are performed by each slave.
Currently, if the same phase is used for all slaves, there is triple
repetition of effort. This is unintended.
