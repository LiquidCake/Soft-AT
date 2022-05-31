# driver config file
driver reads first 11 bytes of **C:\\softat.cfg** file (Three 3-digit hex numbers split by semi-colon). Format is strict - allways 3 digits (start from 0 if needed e.g. 07f)

e.g. 
```
--file starts
221;003;005
--file ends
```

## numbers meaning: 

### first - IO port 
e.g. you can set generally-free IO port "221". Then 220 must be  selected on board DIP switch, 1 means least significant bit is set which triggers shutdown mode.  

Used IO port may conflict with other devices IO address space! Make sure it is not assigned to other device and make it "reserved" in Windows Device Manager, "Computer" properties.  

If this port is assigned in Windows to other device - some values will be eventually written to this port which will lead to random shutdown request to SoftAT board  

### second - IRQ number 
IRQ3-IRQ5 are supported by board. Choose IRQ that is less probale to be used by other devices in system and put its number in config file, then set corresponding jumper on board (JP1 is IRQ3, JP2 is IRQ4, JP3 is IRQ5).  

*Attention: IRQ you chose may conflict with one of COM/PARALLEL ports!*  

Suggested value is 003 (IRQ3) - commonly used by COM2 port which could be easily disabled in BIOS (and then IRQ 3 is free to use)  

Also, this IRQ must be "reserved" in Windows Device Manager (in "Computer" properties)  

### third - shutdown delay in seconds (counts seconds after system exit event is received)
Value is in 1 byte hex (0-FF), so 0-255sec  
e.g. "005" is 5sec


# notes:
Driver wont catch interrupts (soft power button presses) when running fullscreen DOS app (like dos game) - you need to exit that first  