# Soft-AT

This is an PC/AT (and PC/XT) compatible ISA board that provides some simple implementation of soft power controls (like we have on PC/ATX)  
Board should work on any PC with at least 8-bit ISA bus starting from original IBM-PC and up to Pentium machines  
Driver is tested on Windows 98 but probably may work on all Win9x  

### Functionality:
When properly installed & wired board allows power on/off for AT machine to mimic look&feel of power controls of ATX machine  
i.e. 
```
when PC is off - pressing (soft) power button starts it normally
when PC is on - pressing (soft) power button triggers windows shutdown and then poweroff after windows is unloaded (instead of instant hard poweroff of regular AT PSU)
when user clicks on shutdown in windows start menu - board receives signal and will turn power off after some delay (so windows will already be unloaded at that point)
```

**DISCLAIMER 1: this is a hobby project made by a person who is NOT and electronics engineer so it may be flawed in any possible way. It works though**

**DISCLAIMER 2: due to war i wasn't able to actually solder and test last revision of board, though it was just a minor tracks fix so should be fine**   

**For same reason i wasn't able to order and test board with actual 74LS series chips so it was tested only with 74LS's Soviet clones that i had easily available. They should be pretty much identical to original chips though**

**And this is also why there are no real pictures of assembled and properly installed board, only 3d renders**

### Description:
Board was developed using `Proteus 8`, project files are provided as well as schematics and model screenshots  

In order to make board:  
- produce (order) PCB using provided `Gerber` files  
- mount/solder components from `components.txt` (the list is numerated list, same numbers on PCB silk and on schematics screenshot/Proteus project)  
*check component mount guide picture*
- insert board into ISA slot
- **!!!WARNING - high voltage here** disconnect your AT PSU from PC power switch (4 cables) and connect cables to large power relay RL2 that you've mounted on board  
DOUBLE CHECK:  
AT switch connects `white-to-blue`, `black-to-brown` so relay RL2 must do the same
- connect soft power button that will be used as PC power button  
(e.g. ideally you may put your AT motherboard and AT PSU into ATX PC case which has soft power button with proper header by default, though it is possible only if holes on board match ATX case board mount points)  
- probably connect reset button as force shutdown button `1` (see `force shutdown buttons` details below)
- connect one of PSU molex power connector to molex header on board (use smaller one, like one used for 3.5 floppies)  
- pick free IRQ and IO port address and set it on board phisically (check driver readme for more info on IRQ and IO port picking)  
	1. pick IRQ that is free on your PC and set it by board jumper `JP1/JP2/JP3` (e.g. `IRQ 3`)
	2. pick IO port that is free on your PC, convert its hex number to binary and set higher 7 bits by board DIP switch DSW1 (e.g. if using IO port address HEX `220` - convert to binary: `1000100000` and take 7 higher bits - `1000100`)
- adjust R11 so board timer ticks ~1sec at a time when counting for shutdown delay - e.g. set it to middle value and change a bit to find proper position empirically during device testing, having set driver shutdown delay to known value in seconds :)
- insert batteries into mounted battery bay (tested with `AA` batteries)  
make sure not to forget batteries inside bay for a long time because of possible leakage. Also for that reason it makes sense to use a very bottom ISA slot for vertical PC cases  
- install driver (.vxd and config) using install.bat (tested to work on Windows 98) - check readme for driver    

#####
Shutdown process wont be triggered after pressing soft power button if windows is in fullscreen dos mode (like dos game)  
(workaround to somehow make it work is possible, but would require both hardware and sofrware changes)

##### force shutdown buttons
There are 2 force shutdown buttons:  
`1` for shutting down operating PC (resets power relay RL2) - on PCB this button's contacts are top pair  
it would make sense to connect PC reset button as this one

`2` for disabling RL1 that initially opens pre-start battery power for RL2 - may be needed if PC didnt start after RL1 opened e.g. due wall power issues - on PCB this button's contacts are bottom pair  
it would make sense to connect some external button as this one, e.g. ideally mount such button on back pannel bar of ISA board

##### BTW
it is possible to use board as just external low-precision ISA timer that sends interrupt when done but this is not supported in current driver version since no real usage for that mode  
(if timeout value is sent to board using IO port with 0 at the end, e.g. 220 instead of 221 - board wont shutdown and will just send interrupt to CPU instead)
