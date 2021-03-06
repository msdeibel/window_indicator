# window_indicator
Code for blinking leds on an ESP8266 based chip using NTP, MQTT and DeepSleep

# What does it do
This little project is currently built on a NodeMCU development board.
On opening a window it receives an "open" or "closed" message via MQTT.
During the time the window is open it the NodeMCU blinks three connected LEDS 
(brighter is always better) as a reminder to shut the hole in the wall again.

# How does it do this
After connecting to the local wifi, the program gets the current time from the router
via the NTP protocol (using https://github.com/aharshac/EasyNTPClient)

If the current time is between the wake and sleep time, set in the respective constants,
the prog loops, waiting for messages on the MQTT topic (using https://github.com/knolleary/pubsubclient).

If the time falls outside the configured waking hours the system goes to DeepSleep to save energy.

The DS is currently limited to 30s only since I'm using a powerbank to power the whole thing and without
pulling some regular voltage every half minute it would completely shut off, leaving the NodeMCU dead.

Once the powerbank has been replace with regular rechargeable batteries the system can be sent to DeepSleep
outside of the waking hours, saving additional battery power.

# Where do we go
The next step is to move the prog off the NodeMCU and onto an ESP12-F. I have already done that actually but the ESP is not yet soldered to anything or properly tested.

Once this is done I'll try to get everything soldered together properly on a board with some sensible layout.

In the far future I hope to extend the prog to cover all windows in the house which will need some multiplexing I guess, since the ESP does not have enough GPIOs to handle all of them by itself. That's gonna be fun :)
