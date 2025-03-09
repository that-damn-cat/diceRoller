// Commented values are from the Teensy
// MAX7219 Control Pins
#define MAXDATA 8    // 10
#define MAXCLOCK 9   // 11
#define MAXLATCH 10  // 12

// Other IO
#define ROLL_PIN 2  // 9
#define SEL_PIN 3   // 8

// milliseconds for the button debounce -- the button is considered to be pressed/released when the signal stops changing for this long.
#define BUTTON_DEBOUNCE_DELAY 175

// In case some logic cares about how many 7-segs we're using, this define remembers it.
#define NUM_DIGITS 4

// displayState values -- State Machine!
#define DISP_MENU 0
#define DISP_SLIDEOUT 1
#define DISP_ROLLANIM 2
#define DISP_RESULT 3
#define DISP_CRITANIM 4
#define DISP_FAILANIM 5
#define DISP_SLIDEIN 6

// Starting display is the main menu
volatile int displayState = DISP_MENU;

// MAX7219 Addresses...
const byte decodeMode  = 0x09;   // The address for setting which digits have the automatic decode mode. Used to turn this off.
const byte brightness  = 0x0A;   // The address for setting the brightness of the display
const byte scanLimit   = 0x0B;   // The address for setting the number of 7 segment displays that we have
const byte shutdown    = 0x0C;   // The address for putting it into sleep mode or waking it up
const byte displayTest = 0x0F;   // The address for starting the test mode (turns all lights on at max brightness)

// The array of different characters possible. The last entry (charListSize - 1) should always be the blank character.
const int charListSize = 32;
const byte blankChar = charListSize - 1;

const byte charList[charListSize] = {
  0b01111110,  // 0    0x00
  0b00110000,  // 1    0x01
  0b01101101,  // 2    0x02
  0b01111001,  // 3    0x03
  0b00110011,  // 4	   0x04
  0b01011011,  // 5	   0x05
  0b00011111,  // 6	   0x06
  0b01110000,  // 7	   0x07
  0b01111111,  // 8	   0x08
  0b01110011,  // 9	   0x09
  0b01110111,  // A	   0x0A
  0b00011111,  // b	   0x0B
  0b01001110,  // C	   0x0C
  0b00111101,  // d	   0x0D
  0b01001111,  // E	 	 0x0E
  0b01000111,  // F	   0x0F
  0b00011101,  // o	   0x10
  0b01100011,  // deg	 0x11
  0b00110111,  // H	   0x12
  0b00010101,  // n	   0x13
  0b00111011,  // Y	   0x14
  0b01100111,  // P	   0x15
  0b00001111,  // t	   0x16
  0b00001110,  // L	   0x17
  0b00000000,  //      0x18
  0b00000000,  //      0x19
  0b00000000,  //      0x1A
  0b00000000,  //      0x1B
  0b00000000,  //      0x1C
  0b00000000,  //      0x1D
  0b00000000,  //      0x1E
  0b00000000   //      0x1F
};

// The different types of dice we can roll.
const int diceTypes[8] = { 2, 4, 6, 8, 10, 12, 20, 100 };
volatile int currentDie = 6;    // d20

// Constant framerate of 200ms/frame = 5fps
const int animDelay = 200;

// The roll animation
const int rollAnimFrames = 11;
const byte rollAnimation[rollAnimFrames][NUM_DIGITS] = {
  { blankChar, 0x11     , blankChar, blankChar },  // Frame 0
  { 0x11     , blankChar, blankChar, blankChar },  // Frame 1
  { blankChar, 0x10     , blankChar, blankChar },  // Frame 2
  { blankChar, blankChar, 0x11     , blankChar },  // Frame 3
  { blankChar, blankChar, blankChar, 0x10      },  // Frame 4
  { blankChar, blankChar, 0x11     , blankChar },  // Frame 5
  { blankChar, 0x10     , blankChar, blankChar },  // Frame 6
  { 0x10     , blankChar, blankChar, blankChar },  // Frame 7
  { 0x10     , blankChar, blankChar, blankChar },  // Frame 8
  { blankChar, 0x10     , blankChar, blankChar },  // Frame 9
  { blankChar, 0x10     , blankChar, blankChar }   // Frame 10
};

// YIPPIE (Crit animation)
const int critAnimFrames = 12;
const byte critAnimation[critAnimFrames][NUM_DIGITS] = {
  { blankChar, blankChar, blankChar, blankChar },  // Frame 0
  { 0x14     , blankChar, blankChar, blankChar },  // Frame 1
  { 0x01     , 0x14     , blankChar, blankChar },  // Frame 2
  { 0x15     , 0x01     , 0x14     , blankChar },  // Frame 3
  { 0x15     , 0x15     , 0x01     , 0x14      },  // Frame 4
  { 0x01     , 0x15     , 0x15     , 0x01      },  // Frame 5
  { 0x0E     , 0x01     , 0x15     , 0x15      },  // Frame 6
  { blankChar, 0x0E     , 0x01     , 0x15      },  // Frame 7
  { blankChar, blankChar, 0x0E     , 0x01      },  // Frame 8
  { blankChar, blankChar, blankChar, 0x0E      },  // Frame 9
  { blankChar, blankChar, blankChar, blankChar },  // Frame 10
  { blankChar, blankChar, blankChar, blankChar }   // Frame 11
};

// oh no (Crit Fail animation)
const int failAnimFrames = 10;
const byte failAnimation[failAnimFrames][NUM_DIGITS] = {
  { blankChar, blankChar, blankChar, blankChar },  // Frame 0
  { 0x10     , blankChar, blankChar, blankChar },  // Frame 1
  { 0x12     , 0x10     , blankChar, blankChar },  // Frame 2
  { blankChar, 0x12     , 0x10     , blankChar },  // Frame 3
  { 0x13     , blankChar, 0x12     , 0x10      },  // Frame 4
  { 0x10     , 0x13     , blankChar, 0x12      },  // Frame 5
  { blankChar, 0x10     , 0x13     , blankChar },  // Frame 6
  { blankChar, blankChar, 0x10     , 0x13      },  // Frame 7
  { blankChar, blankChar, blankChar, 0x10      },  // Frame 8
  { blankChar, blankChar, blankChar, blankChar }   // Frame 9
};

// The slide in/out animation. Not constant, because this gets generated on the fly later.
const int slideAnimFrames = NUM_DIGITS + 2;
byte slideAnimation[slideAnimFrames][NUM_DIGITS] = {
  { blankChar, blankChar, blankChar, blankChar },  // Frame 0
  { blankChar, blankChar, blankChar, blankChar },  // Frame 1
  { blankChar, blankChar, blankChar, blankChar },  // Frame 2
  { blankChar, blankChar, blankChar, blankChar },  // Frame 3
  { blankChar, blankChar, blankChar, blankChar },  // Frame 4
  { blankChar, blankChar, blankChar, blankChar }   // Frame 5
};

// This will take a byte for the address, and a byte for the data, and send them out to the MAX7219, then toggle the latch.
void sendMaxCommand(byte address, byte data) {
  shiftOut(MAXDATA, MAXCLOCK, MSBFIRST, address);
  shiftOut(MAXDATA, MAXCLOCK, MSBFIRST, data);
  digitalWrite(MAXLATCH, HIGH);
  digitalWrite(MAXLATCH, LOW);
}

// The memory of what was last commanded to the display.
// Init with 0xFF, which doesn't exist so the first write will always update.
static byte lastDisplay[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

// This will send the appropriate command for the given digit, then update our memory of what is on the display.
void setDigit(int tgtDigit, byte tgtChar) {
  if (lastDisplay[tgtDigit] != tgtChar) {                    // Only send the update if the character is actually different from what's already there.
    sendMaxCommand(tgtDigit + 1, charList[tgtChar]);         // Send the command. The address for a digit is just the digit number + 1
    lastDisplay[tgtDigit] = tgtChar;                         // Update our memory of what's there.
  }
}

// Clear the display
void clearDisplay() {
  for (int thisDigit = 0; thisDigit < NUM_DIGITS; thisDigit++) {
    setDigit(thisDigit, blankChar);
  }
}

// Rolls a dice!
int rollDice(int diceSize) {
  return (random(1, diceSize + 1));  // Get a random number between 1 and diceSize + 1 (so it includes the upper bound).
}

// If you give this a number between 0 and 100, it will display that number using the rightmost 3 digits.
void displayNumber(int value) {
  int hundredsPlace = value / 100;                                   // Integer division cuts off the decimal place. So this just gets us how many hundreds there are.
  int tensPlace = (value - (hundredsPlace * 100)) / 10;              // Take our value and remove those hundreds. Then divide by ten to get how many tens there are.
  int onesPlace = value - (hundredsPlace * 100) - (tensPlace * 10);  // Take our value and remove the hundreds and the tens, to get the ones place.
  setDigit(0, onesPlace);                                            // Send the ones place value to the right digit...

  if (tensPlace > 0 || hundredsPlace > 0) {  // If either the tens or hundreds place is > 0, we should show something in the 10s spot.
    setDigit(1, tensPlace);
  } else {
    setDigit(1, blankChar);
  }

  if (hundredsPlace > 0) {  // If the hundreds place has something in it, it should show too.
    setDigit(2, hundredsPlace);
  } else {
    setDigit(2, blankChar);
  }
}

// Steps through an animation array and displays it at the set framerate
void runAnimation(byte animArray[][NUM_DIGITS], int numFrames) {
  clearDisplay();
  
  for (int thisFrame = 0; thisFrame < numFrames; thisFrame++) {
    for (int thisDigit = 0; thisDigit < NUM_DIGITS; thisDigit++) {
      setDigit(thisDigit, animArray[thisFrame][thisDigit]);
    }
    delay(animDelay);
  }

  clearDisplay();
}

// Create the animation for sliding right off screen
void generateSlideAnimation() {
  // Clear out the slide animation array
  for (int frame = 0; frame < slideAnimFrames; frame++) {
    for (int disp = 0; disp < NUM_DIGITS; disp++) {
      slideAnimation[frame][disp] = blankChar;
    }
  }

  // Load lastDisplay into frame 0
  slideAnimation[0][0] = lastDisplay[0];
  slideAnimation[0][1] = lastDisplay[1];
  slideAnimation[0][2] = lastDisplay[2];
  slideAnimation[0][3] = lastDisplay[3];

  // Frame 0, 1, 2 of this frame = 1, 2, 3 of the last frame. Repeat this process for every frame.
  for (int frame = 1; frame < slideAnimFrames; frame++) {
    slideAnimation[frame][0] = slideAnimation[frame - 1][1];
    slideAnimation[frame][1] = slideAnimation[frame - 1][2];
    slideAnimation[frame][2] = slideAnimation[frame - 1][3];
    slideAnimation[frame][3] = blankChar;
  }
}

// Takes the slide animation array and flips it around for sliding back in.
void flipSlideAnimation() {
  // slideArray is the temp holder for the copy
  byte slideArray[slideAnimFrames][NUM_DIGITS];

  // Copy slideAnimation into slideArray
  for (int frame = 0; frame < slideAnimFrames; frame++) {
    for (int digit = 0; digit < NUM_DIGITS; digit++) {
      slideArray[frame][digit] = slideAnimation[frame][digit];
    }
  }

  // Because C is stupid and doesn't let you just assign arrays into eachother, we have to loop through everything to do this.
  // We use the digit to loop, because the frames are what we're flooping around.
  for (int digit = 0; digit < NUM_DIGITS; digit++) {
    //TODO: make this flexible in case we add more displays
    slideAnimation[0][digit] = slideArray[5][digit];
    slideAnimation[1][digit] = slideArray[4][digit];
    slideAnimation[2][digit] = slideArray[3][digit];
    slideAnimation[3][digit] = slideArray[2][digit];
    slideAnimation[4][digit] = slideArray[1][digit];
    slideAnimation[5][digit] = slideArray[0][digit];
  }
}

// Time for the setup, finally...
void setup() {

  // Setting the pinmode for our IO
  pinMode(MAXDATA, OUTPUT);
  pinMode(MAXCLOCK, OUTPUT);
  pinMode(MAXLATCH, OUTPUT);
  pinMode(ROLL_PIN, INPUT_PULLUP);
  pinMode(SEL_PIN, INPUT_PULLUP);

  // Attaching interrupt handlers to the button presses.
  attachInterrupt(digitalPinToInterrupt(ROLL_PIN), rollPressed, FALLING);
  attachInterrupt(digitalPinToInterrupt(SEL_PIN), selPressed, FALLING);

  // Fetch random seed
  int total = 0;
  for (int i = A0; i <= A5; i++) {  // Add up a reading from all the (floating) analog pins
    total += analogRead(i);
  }
  randomSeed(total);  // Use that as our seed.

  // Set the MAX pins LOW to start.
  digitalWrite(MAXDATA, LOW);
  digitalWrite(MAXCLOCK, LOW);
  digitalWrite(MAXLATCH, LOW);

  // Set up the display driver
  sendMaxCommand(scanLimit, 0x03);    // 4 characters
  sendMaxCommand(brightness, 0x0F);   // Max brightness. Maybe we add a little knob to set this brightness?
  sendMaxCommand(decodeMode, 0x00);   // Turn off decode mode everywhere
  sendMaxCommand(shutdown, 0x01);     // Wake up from sleep
  sendMaxCommand(displayTest, 0x01);  // Turn on the display test
  delay(animDelay);                   // Wait a bit
  sendMaxCommand(displayTest, 0x00);  // Turn off the display test
  clearDisplay();                     // Clear the display
}

void rollPressed() {

  // First, handle the debounce
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > BUTTON_DEBOUNCE_DELAY) {

    switch (displayState) {
      case DISP_MENU:
        {
          displayState = DISP_SLIDEOUT;  // on the menu this should slide out and then transition to roll animation
        }
        break;

      case DISP_RESULT:
        {
          displayState = DISP_ROLLANIM;  // if you press roll again after the result is displayed it should reroll going back to roll animation
        }
        break;
    }
  }

  lastInterruptTime = interruptTime;
}

void selPressed() {

  // First, handle the debounce
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > BUTTON_DEBOUNCE_DELAY) {

    switch (displayState) {
      case DISP_MENU:
        {                         // okay so if it's on the menu
          if (currentDie == 7) {  // if the current selected die in the array is seven
            currentDie = 0;       // then it should go back to the first die in the array because there are only six dice ya dingus
          } else {
            currentDie++;         // otherwise add one to the value to go to the next die in the array
          }
        }
        break;

      case DISP_RESULT:
        {
          displayState = DISP_SLIDEIN;  // go to slide IN, which will direct back to Main Menu
        }
        break;
    }
  }

  lastInterruptTime = interruptTime;
}

void loop() {
  static int rolledNumber = 0;

  switch (displayState) {
    case DISP_MENU:
      {  // 0 Main Menu
        setDigit(NUM_DIGITS - 1, 0x0D);             // Last Digit has a "D"
        displayNumber(diceTypes[currentDie]);       // And show the value of the selected die
      }
      break;

    case DISP_SLIDEOUT:
      {  // 1 Slide Out
        generateSlideAnimation();
        delay(animDelay);
        runAnimation(slideAnimation, slideAnimFrames);
        displayState = DISP_ROLLANIM;
      }
      break;

    case DISP_ROLLANIM:
      {  // 2 Roll Animation
        rolledNumber = rollDice(diceTypes[currentDie]);
        runAnimation(rollAnimation, rollAnimFrames);
        displayState = DISP_RESULT;

        if (currentDie > 0) {               // Only use Crit/Fail anims if it isn't a d2/coin
          if (rolledNumber == diceTypes[currentDie]) {
            displayState = DISP_CRITANIM;
          }

          if (rolledNumber == 1) {
            displayState = DISP_FAILANIM;
          }
        }
      }
      break;

    case DISP_CRITANIM:
      {  // 3 Crit Animation
        runAnimation(critAnimation, critAnimFrames);
        displayState = DISP_RESULT;
      }
      break;

    case DISP_FAILANIM:
      {  // 4 Fail Animation
        runAnimation(failAnimation, failAnimFrames);
        displayState = DISP_RESULT;
      }
      break;

    case DISP_RESULT:
      {  // 5 Display Result
        if (currentDie > 0) {           // If it isn't a "d2"/coin
          displayNumber(rolledNumber);  // Show the number
        } else {                        // Otherwise show...
          if (rolledNumber == 1) {      // TAIL
            setDigit(3, 0x16);
            setDigit(2, 0x0A);
            setDigit(1, 0x01);
            setDigit(0, 0x17);
          } else {                      // HEAD
            setDigit(3, 0x12);
            setDigit(2, 0x0E);
            setDigit(1, 0x0A);
            setDigit(0, 0x0D);
          }
        }
      }
      break;

    case DISP_SLIDEIN:
      {  // 6 Slide In
        flipSlideAnimation();
        runAnimation(slideAnimation, slideAnimFrames);
        displayState = DISP_MENU;
      }
      break;
  }
}
