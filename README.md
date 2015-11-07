# MameAppleTV


This is something I've always wanted on the AppleTV. I was lucky enough to get hold of the AppleTV developers kit and here is the result using tvOS beta 2.

The tvOS app is based on Les Bird's code I found here :- http://www.lesbird.com/iMame4All/iMame4All_Xcode.html

The original work is based on iMAME4all which is a iOS universal app, port of MAME 0.37b5 emulator by Nicola Salmoria for all iOS devices (iPad HD, iPhone 4G , iPod touch and older) based on GP2X, WIZ MAME4ALL 2.5 by Franxis. 

I created a target for tvOS and set about getting the code to compile for arm64 (Mandatory for AppleTV), fixed a varierty of compiler and linker errors. Removed code which was incompatible with tvOS frameworks and simplified code to work on tvOS. Added a basic icon compatible with tvOS. I added some tweaks to the source  to allow the pause button to exit the game and supporting the resolution for the 1080p display.

The controller being used is the SteelSeries Stratus XL  and paired to the AppleTV via the Bluetooth settings.

Most games are running well though there are some sound issues with Metal Slug and Street Fighter which I suspect are arm64 related which I still need to investigate.

Don't expect to see this on the App Store any time soon as Apple doesn't allow emulators of any kind.


As promised, here is a zip of the xcodeproject, tested with GM build of the Apple TV and Xcode 7.1 final.

Download Xcode project : http://bit.ly/1LQybD0

To get this to work you need 

- A real  Apple TV device, this won't work in the simulator.

- Drag the rom zip file into the resource folder in the xcode project so they are copied to the device (Should appear as files in Build Phases/ Copy Bundle resources)


Includes initial support for AppleTV remote if no controller available.

Areas that need work

- Sound - fix arm64 assembler which has been commented out
- Apple TV remote support
