// ========== LIBRARIES ==========
#include <Keypad.h>
#include <LiquidCrystal.h>

// ========== Group Login ==========
#define NUM_GROUPS 5 // change based on the max number of groups in the session

int groupCodes[NUM_GROUPS] = {738, 291, 465, 823, 679};
int currentGroupId = -1;
bool groupLoggedIn = false;

// LCD Pin Definitions for ESP32
#define RS 23
#define  E 22
#define D4 18
#define D5  5
#define D6 17
#define D7 16

// ===== LCD initialization =====
LiquidCrystal lcd(RS, E, D4, D5, D6, D7);
String currentAttempt = ""; // stores current passcode attempt
int correctCount = 0;       // count of correctly inputted passcodes
const int LCD_COLS = 16;
const int LCD_ROWS = 2;

String inputBuffer = "";

// ====== 3 CODE VERSION ======
const int NUM_PASSCODES = 3;
const String passcodes[NUM_PASSCODES] = {"2458", "8542", "4528"};

// ====== 5 CODE VERSION ======
// const int NUM_PASSCODES = 5;
// const String passcodes[NUM_PASSCODES] = {"2458", "2584", "8542", "4528", "2548"};

// ====== 10 CODE VERSION ======
// const int NUM_PASSCODES = 10;
// const String passcodes[NUM_PASSCODES] = {"2458", "8524", "2584", "8425", "8542", "4852", "2854", "4528", "5248", "4285"};

// ===== Keypad initialization =====
const byte ROWS = 4; // Four rows
const byte COLS = 3; // Three columns
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {32, 33, 25, 26};
byte colPins[COLS] = {27, 14, 12};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(38400);

  lcd.begin(LCD_COLS, LCD_ROWS);
  lcd.print("Initializing...");
  delay(1000);

  displayPrompt();
}

void loop() {
  // wait for a group to log in before the game can be played
  if (!groupLoggedIn) {
    handleLogin();
    displayPrompt();
    return;
  }

  // check if all passcodes have been entered correctly
  if (correctCount == NUM_PASSCODES) {
    puzzleSolved();
    handleLogout();
    return;
  }

  char key = keypad.getKey();

  if (key) {
    if (key == '#') {
      lcd.clear();
      if (currentAttempt == passcodes[correctCount]) {
        correctCount++;
        lcd.print("Correct passcode");
        lcd.setCursor(0, 1);
        lcd.print("Correct: ");
        lcd.print(correctCount);
        lcd.print("/");
        lcd.print(NUM_PASSCODES);
      } else {
        lcd.print("Incorrect please");
        lcd.setCursor(0, 1);
        lcd.print("try again");
      }
      delay(2000); // give players a chance to read feedback

      /**
      // Uncomment this code when using the 5-code or 10-code version
      if (correctCount > 0) {
        lcd.setCursor(0,1);
        lcd.print("Prev. Code: ");
        lcd.print(passcodes[correctCount-1]);
        delay(3000); // Display previous code for 4 seconds
      }
      */

      currentAttempt = "";
    } else if (key == '*' && !currentAttempt.isEmpty()) {
      currentAttempt.remove(currentAttempt.length() - 1); // delete one digit from input
    } else {
      currentAttempt += key; // append keypress to current attempt
    }
    displayPrompt();
    lcd.setCursor(0, 1);
    lcd.print(currentAttempt);
  }
}

// ========== GAME FUNCTIONS ==========

/**
 * Displays a congratulations message when the puzzle is solved.
 * Resets the game for the next group of players.
 */
void puzzleSolved() {
  correctCount = 0; // reset the game for the next group

  lcd.clear();
  lcd.print("Puzzle Solved!");
  delay(5000);
}

/**
 * Prompt players to either enter their group ID or a new passcode attempt
 * depending on whether a group is currently logged in.
 */
void displayPrompt() {
  lcd.clear();
  if (groupLoggedIn) {
    lcd.print("Enter Passcode:");
  } else {
    lcd.print("Enter Group ID:");
  }
}

// ========== LCD FUNCTIONS ==========

// display helper
void showMessage(const String &msg, int row, bool clearFirst, unsigned long waitMs, bool infinite = false) {
  if (clearFirst) lcd.clear();
  lcd.setCursor(0, row);
  if (msg.length() <= LCD_COLS) {
    lcd.print(msg);
  } else {
    scrollMessage(msg, row, 300, infinite);
  }
  if (waitMs) delay(waitMs);
}

// scroll long messages
void scrollMessage(const String &msg, int row, unsigned long delayMs, bool infinite) {
  int len = msg.length();
  int total = len + 16;

  do {
    for (int offset = 0; offset <= total; offset++) {
      lcd.setCursor(0, row);
      for (int i = 0; i < 16; i++) {
        int idx = offset + i - 16;
        if (idx >= 0 && idx < len)
          lcd.print(msg[idx]);
        else
          lcd.print(' ');
      }
      delay(delayMs);
    }
  } while (infinite);
}

// ========== LOGIN / LOGOUT FUNCTIONS ==========

/**
 * Blocks until a valid group ID is entered via the keypad.
 */
void handleLogin() {
  while (!groupLoggedIn) {
    char key = keypad.getKey();
    if (key) {
      if (key == '#') {
        int inputCode = inputBuffer.toInt();
        bool valid = false;
        for (int i = 0; i < NUM_GROUPS; i++) {
          if (groupCodes[i] == inputCode) {
            currentGroupId = i + 1;
            valid = true;
            break;
          }
        }

        if (valid) {
          groupLoggedIn = true;
          return;
        } else {
          showMessage("Invalid ID!", 0, true, 1000);
          showMessage("Enter Group ID:", 0, true, 0);
        }
        inputBuffer = "";
      } else if (key == '*') {
        inputBuffer = "";
        showMessage("Input cleared", 0, true, 1000);
        showMessage("Enter Group ID:", 0, true, 0);
      } else {
        inputBuffer += key;
        lcd.setCursor(inputBuffer.length() - 1, 1);
        lcd.print(key);
      }
    }
  }
}

/**
 * Logs the current group out and resets state for the next group.
 */
void handleLogout() {
  currentGroupId = -1;
  groupLoggedIn = false;
}
