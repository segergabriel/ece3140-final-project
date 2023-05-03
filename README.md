## Your Project Answers

### Project Description

For our project, we are taking inspiration from a minigame from Wii Party (2010) called “Time Bomb.”  

* Gameplay

* Each person must hold a specific button while passing the board (aka bomb)

* External Components

* Implementation: 

* to the next person as steadily as possible to avoid detonating the bomb. The timer displayed on the bomb resets every time it is successfully passed to the next person. As rounds increase, the timer decreases and the bomb becomes more sensitive to shaking.

* If the bomb shakes too much while passing it onto the next person, both the passer and the receiver lose

* If the receiver presses a wrong input, the receiver loses

* If the passer shakes it too much after receiving the the bomb and before the next input is announced, the passer loses

* Different game modes:

* Another game mode where the timer is shown at the beginning but is then hidden. The timer will continue to count down and players must avoid being the person holding the bomb when the timer reaches zero.

* Elimination game mode where the game continues after someone is out until one person remains
### Technical Approach

* External Components

* Speaker

* 2N3904 NPN BJT for amplification

* https://www.sparkfun.com/products/15350

* Screen (OLED, I2C)

* https://www.sparkfun.com/products/17153 

* Extra buttons

* https://www.sparkfun.com/products/14460

* Switch

* https://www.sparkfun.com/products/9276

* Implementation: 

* All C code/software contained on board. 

* With battery pack, the board does not need to be connected to computer

* OLED screen to display information with I2C.

* Speaker to play game sounds and give instructions

* On-board accelerometer/gyroscope

* Measures movement/shakiness of board and compares signal to a specific threshold.

* On-board buttons and lights to interact with and signal the state of the game.

* Look into extra buttons/inputs

* Encoders for extra difficulty

* Extra buttons

* Switches

* GPIO pins interacting with the board. Use interrupts for timing with PIT and detecting excessive movement/wrong input.
## Your page
You can access your place holder page on [https://pages.github.coecis.cornell.edu/ece3140-sp2023/dl634-kz85-lsl95/](https://pages.github.coecis.cornell.edu/ece3140-sp2023/dl634-kz85-lsl95/).

You can edit your page in the gh-page branch of this repo.