CON

  _clkmode = xtal1 + pll16x
  _xinfreq = 5_000_000
 
OBJ

  ser   : "Parallax Serial Terminal"
  
VAR

  byte ticks
  byte total
  
PUB main

  dira[8]  := 0                 '' Set to input
  dira[16] := 1                 '' Set to output
  ticks    := 0
  total    := 0
  ser.Start(115200)             '' Start the serial terminal  
  repeat
    repeat until ticks > 60
      outa[16] := ina[8]          '' Write the input to one LED
      total := total + ina[8]
      ticks := ticks + 1
      waitcnt (clkfreq + cnt)     '' Wait 1 second
    ser.Bin(total, 1)
    ticks := 0
    total := 0
