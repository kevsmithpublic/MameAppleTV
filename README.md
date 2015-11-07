# MameAppleTV


This is something I've always wanted on the AppleTV. I was lucky enough to get hold of the AppleTV developers kit and here is the result using tvOS beta 2.

The tvOS app is based on Les Bird's code I found here :- http://www.lesbird.com/iMame4All/iMame4All_Xcode.html

The original work is based on iMAME4all which is a iOS universal app, port of MAME 0.37b5 emulator by Nicola Salmoria for all iOS devices (iPad HD, iPhone 4G , iPod touch and older) based on GP2X, WIZ MAME4ALL 2.5 by Franxis. 

YouTube videos of it running

https://www.youtube.com/watch?v=VlO4nQGNFKU


To get this to work you need 

- A real  Apple TV device, this won't work in the simulator.

- Drag the rom zip file into the resource folder in the xcode project so they are copied to the device (Should appear as files in Build Phases/ Copy Bundle resources)

Areas that need work

- Sound - fix arm64 assembler which has been commented out
- Apple TV remote support (WIP)

