# cat-reward-system
Arduino implementation in C++ to train my fat cat to run on a cat wheel. Rewarding cat with a treat actuated by a servo motor every time the rotary encoder detects the wheel has been spinning for incrementing periods of time. 'Gambling states' are incorporated to mimic training stages when training pets manually.

## Photo

## How It Works
Rotary encoder with low pass filter for debouncing increments or decrements a counter from spinning the wheel. 'loop' function checks the state of the wheel spinning (clockwise, counter clockwise, no spin) every 0.25s by checking how the counter has changed.
If the wheel has been spinning the same direction for more than the threshold time, the state machine depending on 'gamblingState' is used to determine whether a treat will be dispensed. 
```markdown
gambling states:
 - countinuous reinforcment: when total treats dispensed <= 50, treat dispensed every time
 - fixed ratio: when 50 < total treats dispensed <= 100, treat dispensed every third time
 - variable: when total treats dispensed > 100, treat dispensed probabilistically, probability = 1 - exp(-time over threshold / treatCounter)
```
Upon release of a treat: a servo motor is actuated to open and close briefly and the time threshold is increased linearly.

## Project Structure
```markdown
 - 'main.cpp' - Source C++ code for program
 - 'platformio.ini' - Specifications for specific Arduino board and additional libraries used
```

## How to Run
Configure pin A of rotary encoder to pin 2 of Arduino Rev 4 Minima and pin B of encoder to pin 3 on Arduino board. Attach a small capacitor from pin A to ground and pin B to ground. Attach a small resistor between the rotary pins and Arduino. Connect Servo motor PWM to pin 9 on Arduino and 5V and GND accordingly. Upload the code onto the board using PlatformIO or Arduino IDE. 
