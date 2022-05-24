# Safe project

## Instructions for starting the application:
- open project in Microchip studio program

## Equipment:
- ATMega 16A 8-bit microrontroller
- LCD modul 1602A
- HC-543 keypad
- mini analog servo motor - 9G
- potentiometer B25K
- active buzzer module high level trigger 5V

## Project description:
The safe is initially locked with the help of a servo motor that locks the door and three red LEDs are turned on as initial state. The user must enter the correct password using the keypad and confirm it by pressing the "A" key. If the wrong password is entered three times, it turn on
an alarm that is performed with buzzer and all LEDs flash at the same time and stop only by pressing the "A" key. If the user enters the correct password, he receives a confirmation
sound,a message will be displayed on the LCD module and one red LED turn off while one green is
turn on. After that, it is possible to use two potentiometers and they are needed
set to exactly the set value which is 50 and has a tolerance of +/- 1. User must set the first potentiometer to the correct value and confirm it with the "A" key. If the value is correct, one red LED turn off while one green LED turn on. Then it is allowed to use another potentiometer that is also need to set to the correct value and if it is also correct, one red LED is turn off while one green LED is turn on. Once all three conditions are fulfilled, the safe door is unlocked and the user receives a message on the LCD screen.
Also, the user can lock the safe by enter the correct password for
a lock which is different from the unlock password. If the lock password is correct, the servo motor locks the door.

## Project picture
![slika1](https://user-images.githubusercontent.com/78230349/170139538-161b35cc-f1b1-4972-9b3b-cee25fc23f73.jpg)

![slika2](https://user-images.githubusercontent.com/78230349/170139599-a6fba3f9-9877-4a5f-b3d9-f428441cdb8b.jpg)
