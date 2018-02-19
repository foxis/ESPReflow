# ESPReflow
ESP8266 reflow controller

# Purpose

# How it Works

# Installation

## Install as ZIP

* [Click here to download the ZIP file](https://github.com/foxis/EasyOTA/archive/master.zip)
* Start the Arduino IDE
* Go to _Sketch > Include Library > Add .ZIP Library..._
* Select the ZIP file you just downloaded

## Install using Git (OSX / Linux)

* Open a terminal window
* Go to your Arduino libraries dir: `cd ~/Documents/Arduino/libraries`
* Execute: `git clone https://github.com/foxis/EasyOTA.git`
* (Re)start your Arduino IDE

# Usage

## Building

## Flashing

# Problem Solving

## Network port is not showing in IDE

* It sometimes takes a few minutes for the port to show in the Arduino IDE
* Check if the port is found, this also seems to speed up detection in the IDE:
  * In OSX on console: `dns-sd -B _arduino._tcp`
  * In Windows use [Bonjour browser](http://hobbyistsoftware.com/bonjourBrowser)
* Try to restart the Arduino board, wait 5 minutes, check if the _Network port_ shows up
* Try to restart your Arduino IDE, wait 5 minutes, check if the _Network port_ shows up
* On Windows 8.1 or older, mDNS does may not work, or only when you install [Bonjour Services](https://support.apple.com/kb/DL999?locale=en_US).
* Your may need op open 'mDNS' on your firewall: UDP port 5353
