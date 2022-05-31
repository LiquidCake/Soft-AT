## dos driver

This is an old dos "driver" for Soft-AT (actually its just an interrupt service routine, not a full format driver).  

*It should work but was not tested for some time*  

Should be working starting from 8088 CPU and dos 2.0  

settings like IO port, IRQ number, timer delay etc.s are hardcoded  

When `isr` is launched it loads itself into RAM and stays residential, waiting for interrupts. Once interrupt comes (from soft power button press) - driver sends shutdown signal to Soft-AT board and PC will be powered off after delay

`timer` binary may be used to send signal to Soft-AT board manually  
