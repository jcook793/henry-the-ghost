Henry the Ghost
===============

An Arduino ghost project I made on a Sunday before Halloween.  He's pretty simple - he lights up, fading from orange to purple, then randomly will flash red and play a spooky laugh or scream.  You can see a very short clip of it in operation here: http://www.youtube.com/watch?v=G8SFG6cB0VQ or a longer version here: http://www.youtube.com/watch?v=AgGDIzZsZo8

I used an Arduino 2560, an Adafruit Wave Shield and 4 macetech ShiftBrites.  It fades from orange to purple anywhere from 5 to 10 times, then flashes red and randomly plays one of the WAV files on the SD card.  Rinse, repeat.

The code assumes you have 4 ShiftBrites, though this is easily changed by altering the numShiftBrites global.  Also it is assumed that the ShiftBrites are hooked up on pins 22, 23, 24 and 25 for the data, latch, enable and clock pins respectively.  Again, this is easily changed at the bottom of the setup() function.

You'll need the [WaveHC](http://code.google.com/p/wavehc/) and [ShiftBriteHughesy](http://code.google.com/p/shiftbritehughesyarduino/) libraries.
