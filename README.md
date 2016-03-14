# LightDimmer - Arduino
Dims the light in a hallway depending on where people stand.

##Example:
######!!!Very unprecise and is actually using Gaussian function to dim the light!!!

	0: off, 1: low, 2: mid, 3: high, p: person, L: light and sensor
	
	  L L L L L L L L L L L
	  3 2 1 2 3 2 2 3 2 1 0
	  p       p     p
##Setup:
######Made with Arduino/Genuino Uno


Plug in one or more lights and the corresponding amount of light sensors

If have one, plug in an LCD screen(2x16) too. Not needed though.

Write the amount plugged in at line 12:

<code>const int cells = 3;     // number of units</code>

Then tell the program where you've plugged in the lights and sensors at lines 13, 14 and 15:

	int photocellPin[] = {0, 1, 2};   // the photocell connected to analog in
	int LED[] = {6, 9, 10}; // led pins write analog
	int PIR[] = {7, 8, 13}; // pir pins read digital

If you're using an LCD screen, tell the program where you've plugged it in at line 5:

<code>LiquidCrystal lcd(12, 11, 5, 4, 3, 2);</code>

, and which pixels you want the lights to be displayed on the LCD screen at line 19:

<code>int lcdPixels[cells] = {0, 6, 11};</code>
