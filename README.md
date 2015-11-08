# MameAppleTV


This is something I've always wanted on the AppleTV. I was lucky enough to get hold of the AppleTV developers kit and here is the result using tvOS beta 2.

The tvOS app is based on Les Bird's code I found here :- http://www.lesbird.com/iMame4All/iMame4All_Xcode.html

The original work is based on iMAME4all which is a iOS universal app, port of MAME 0.37b5 emulator by Nicola Salmoria for all iOS devices (iPad HD, iPhone 4G , iPod touch and older) based on GP2X, WIZ MAME4ALL 2.5 by Franxis. 

Videos<br/>

Initial Test<br/>
<a href="http://www.youtube.com/watch?feature=player_embedded&v=VlO4nQGNFKU" target="_blank"><img src="http://img.youtube.com/vi/VlO4nQGNFKU/0.jpg" alt="Mame AppleTV" width="240" height="180" border="10" /></a>
<br/>

Apple TV - remote<br/>
<a href="http://www.youtube.com/watch?feature=player_embedded&v=zLY2Rkw6Rxk" target="_blank"><img src="http://img.youtube.com/vi/zLY2Rkw6Rxk/0.jpg" alt="Mame AppleTV" width="240" height="180" border="10" /></a>
<br/>


To get this to work you need 

- A real  Apple TV device, this won't work in the simulator.

- Drag the rom zip file into the resource folder in the xcode project so they are copied to the device (Should appear as files in Build Phases/ Copy Bundle resources)

Apple TV - Brief guide to using to play game (Tested with 1942,Donkey Kong, Raiden)

- Turn the controller on it's side (It will work both directions but I found this easier)
- Select rom using up down on dpad
- Press play to select
- Swipe left/right for OK message
- Hold play and press dpad for credit 
- Hold play and press dpad for start
- Play game :-)


The code needs some work but you can play basic games


Areas that need work

- Sound - fix arm64 assembler which has been commented out
- Apple TV remote support (WIP)

