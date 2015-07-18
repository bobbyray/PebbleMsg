# PebbleMsg
This is a C program for the Pebble watch, using Pebble SDK 2.

Receives a message from an Android app using the Pebble AppMessage api.
Currently a message has two dictionary elements with keys 0 and 1:
* Key = 0, string of text displayed on Pebble face.
* Key = 1, string of digits for an ordinal number, 0, 1, 2, etc. The digits specify the number of vibrations to issue when the text (Key 0 element) is displayed. For example, specify '0' for no vibration. Specify '2' for two vibrations.

The time of day in minutes is shown at top of the watch face and is updated every minute.
The text received follows below the time of day.
