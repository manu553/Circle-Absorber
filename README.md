# Welcome to Circle Obsorber!
##Introduction
Circle Obsorber is a game developed for the orginal Pebble smartwatch (1st generation). The project is still currently a work in progress and since its a passion project there is no set deadlines or scheduled release dates. The game itself uses the Pebble's accelerometer to control a big circle and collect smaller circles, every smaller circle you collect you get a point. On the screen there are also bombs that you need to avoid. Think of the game "Snake" but in circle format. Circle Obsorber is currently being developed using Cloud Pebble, an online IDE, and is using Pebble C SDK V4.

##Reasons for project
The main reason I started this project is because of the total unknown. I wanted to challenge myself to build something with very little resources or external help and see how I could do. Since the Pebble is a niche market, finding help on stackoverflow or similar sites is near impossible. Throught the project, the only real help I have is the documentation provided for the Pebble C SDK. Reading through the documentation has taught me an incredible amount of how to build something from scratch and has helped in other projects/courses as well. I'm not sure if I'll ever manage to finish this project exactly how I want it but it's been a great learning experience so far! :D

##Current states of the project
####Possible Game Modes
- Time attack: 30 second timer in the top right corner, bombs spawing every 5 seconds (may have to adjust) try to score the highest     within those 30 seconds
- 3 Lives: probably have 3 little icons in the top right, bombs every 5 seconds, hit 3 bombs and game over
- hardcore: no timer, bombs every 3 seconds, hit a bomb and insta death game over

####Work In Progress
- give the bomb functionality (ie. if they hit the bomb, game over, maybe life system later?)
- well, the score gets reset to 0 when you hit the bomb and the bomb does move to a different location but the actual image of the bomb dosen't move, fine if were doing a 1 life system, not okay for more than 1 life game
- for multiple life system, in the hit function, I would need to destroy the old bitmap layer, create a new one and then spawn the bomb (GBitmap) on the new layer
            
####Incomplete Work
- find a way to spawn more bombs as time goes by (maybe every 8 seconds spawn a new bomb on screen, play with the timing)
- MAKE SURE BOMB DOES NOT SPAWN ON TOP OF POINT DOT!
- program the buttons for the gameover window to restart the game or quit (change later to menu)
    

####Completed Work
- figure out how to log and debug while the game is running (done)
- convert the int coord to a string to show where the circle is on the screen (probably make my own function) (done)
- adjust and finish hit function (done)
- add counter to one of the layers (maybe main window?) to keep track of the number of dots collected (put in top left corner) (done, counter working and function)
- once counter is displayed make sure the hit function actually works (done, working)
- if it does work and count goes up, erase the existing dot and RANDOMLY draw a new dot somewhere (done, randomly draws)
- find a way to randomly draw a new dot (% operator maybe?) (done, rand plus % operator used to keep dot spawing on screen)
- figure out all this bitmap image stuff (done)
- maybe add in a bomb icon instead of more default shapes ? (done)
- add the bomb image (done)
- game over sceen (done, but need to fix up bomb hit coords)
- figure out how to create a new window and switch to it (done)
- get that new window to load once a bomb is hit (done)
- new window will have a simple text layer that says game over (done, some lag when the bomb is hit, not sure why)
