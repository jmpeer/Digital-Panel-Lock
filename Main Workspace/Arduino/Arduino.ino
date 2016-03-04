////////////////////////////////////////////////////
// HEADERS
////////////////////////////////////////////////////

#include <LiquidCrystal.h>



////////////////////////////////////////////////////
// USER CONFIGURATION
////////////////////////////////////////////////////

// The pin numbers associated with the shift register's clock and input lines. 
// To clarify, input with respect to the register. It's the bit clocked into the register. 
#define SHIFT_REGISTER_CLOCK_PIN 2
#define SHIFT_REGISTER_INPUT_PIN 3

// The pin numbers associated with the switch manager's input lines.  
// To clarify, input with respect to the microcontroller. It's the bit read when sampling switches. The manager samples them. 
#define SWITCH_MANAGER_INPUT_PIN 4

// The switch numbers associated with the numeric and non-numeric switches. 
#define SWITCH_0		0
#define SWITCH_1		1
#define SWITCH_2		2
#define SWITCH_3		3
#define SWITCH_4		4
#define SWITCH_5		5
#define SWITCH_6		6
#define SWITCH_7		7
#define SWITCH_8		8
#define SWITCH_9		9
#define SWITCH_OK		10
#define SWITCH_CANCEL	11

// The switch numbers associated with the output lines of the shift registers. 
#define SHIFT_REGISTER_PIN0_SWITCH	SWITCH_CANCEL
#define SHIFT_REGISTER_PIN1_SWITCH	SWITCH_7
#define SHIFT_REGISTER_PIN2_SWITCH	SWITCH_4
#define SHIFT_REGISTER_PIN3_SWITCH	SWITCH_1
#define SHIFT_REGISTER_PIN4_SWITCH	SWITCH_0
#define SHIFT_REGISTER_PIN5_SWITCH	SWITCH_8
#define SHIFT_REGISTER_PIN6_SWITCH	SWITCH_5
#define SHIFT_REGISTER_PIN7_SWITCH	SWITCH_2
#define SHIFT_REGISTER_PIN8_SWITCH	SWITCH_OK
#define SHIFT_REGISTER_PIN9_SWITCH	SWITCH_9
#define SHIFT_REGISTER_PIN10_SWITCH	SWITCH_6
#define SHIFT_REGISTER_PIN11_SWITCH	SWITCH_3

// The led pin numbers. 
#define LED_Y_PIN 5
#define LED_G_PIN 6
#define LED_R_PIN 7

// The LCD pin numbers. 
#define LCD_RS_PIN	8
#define LCD_EN_PIN	9
#define LCD_DB4_PIN	10
#define LCD_DB5_PIN	11
#define LCD_DB6_PIN	12
#define LCD_DB7_PIN	13



////////////////////////////////////////////////////
// CLASS DECLARATIONS
////////////////////////////////////////////////////

namespace My {
	
	class Pin {
		public:
			int num;
			int mode;
			int value;
		public:
			Pin (void);
			Pin (int pNum, int pMode);
			void setValue (int pValue);
			int getValue (void);
	};
	
};

namespace My {
	
	class LCDManager {
		public:
			LiquidCrystal lcd;
			char line [2][16+1];
			int colIndex [2];
			int rowIndex;
		public:
			LCDManager (void);
			void clearLine (int pLine);
			void clear (void);
			void print (int pDigit);
			void print (char pDigit);
			void print (char *pString);
			void setCursor (int pColIndex, int pRowIndex);
			void deleteChar();
	};
	
};

namespace My {

	class ShiftRegister {
		public:
			struct Pins {
				Pin clock;
				Pin input;
			} pin;
		public:
			ShiftRegister (void);
			void setInputValue (int pValue);
			void clock (void);
	};
	
};

namespace My {
	
	class SwitchState {
		public:
			int pressed;
			int time;
		public:
			SwitchState (void);
			SwitchState (int pPressed, int pTime);
	};
	
};

namespace My {
	
	class SwitchManager {
		public:
			struct Pins {
				Pin input;
			} pin;
			ShiftRegister shiftRegister;
			SwitchState switchState [12];
			int pinToSwitch [12];
		public:
			SwitchManager (void);
			void update (void);
	};
	
};

namespace My {
	
	class LEDManager {
		public:
			struct Pins {
				Pin r;
				Pin g;
				Pin y;
			} pin;
		public:
			LEDManager (void);
	};
	
};

namespace My {
	
	enum State {
		ACTIVE_LOCKED, 
		ACTIVE_UNLOCKED, 
		INACTIVE_LOCKED, 
		INACTIVE_UNLOCKED
	};

	class System {
		public:
			SwitchManager switchManager;
			LEDManager ledManager;
			LCDManager lcdManager;
			int isActive;
			int isLocked;
			char buffer [256];
		public:
			System (void);
			void init (void);
			void update (void);
			void switchEventHandler (int switchNum, SwitchState oldState, SwitchState newState);
			void setActive();
			void setInactive();
			void setLocked();
			void setUnlocked();
	};
	
};



////////////////////////////////////////////////////
// GLOBAL VARIABLES
////////////////////////////////////////////////////

My::System system;



////////////////////////////////////////////////////
// CLASS DEFINITIONS
////////////////////////////////////////////////////

namespace My {
	
	Pin::Pin (void) {
		num = 0;
		mode = 0;
		value = 0;
	}

	Pin::Pin (int pNum, int pMode) {
		num = pNum;
		mode = pMode;
		value = 0;
		pinMode(num, mode);
	}

	void Pin::setValue (int pValue) {
		value = pValue;
		digitalWrite(num, value);
	}

	int Pin::getValue (void) {
		value = digitalRead(num);
		return value;
	}
	
};

namespace My {
	
	LCDManager::LCDManager (void) : lcd(LCD_RS_PIN, LCD_EN_PIN, LCD_DB4_PIN, LCD_DB5_PIN, LCD_DB6_PIN, LCD_DB7_PIN) {
		lcd.begin(16, 2);
		rowIndex = 0;
		colIndex[0] = 0;
		colIndex[1] = 0;
	}
	
	void LCDManager::clearLine (int pLine) {
		lcd.setCursor(0, pLine);
		lcd.print("                ");
		lcd.setCursor(0, pLine);
		rowIndex = pLine;
		colIndex[rowIndex] = 0;
	}
	
	void LCDManager::clear (void) {
		clearLine(1);
		clearLine(0);
	}
	
	void LCDManager::print (int pDigit) {
		print((char) (pDigit + '0'));
	}
	
	void LCDManager::print (char pDigit) {
		int temp;
		if (colIndex[rowIndex] < 16) {
			line[rowIndex][colIndex[rowIndex]] = pDigit;
			lcd.setCursor(colIndex[rowIndex], rowIndex);
			lcd.print(pDigit);
			temp = colIndex[rowIndex] + 1;
			colIndex[rowIndex]++;
			if (rowIndex == 0 && colIndex[rowIndex] == 16) {
				colIndex[rowIndex] = 0;
				rowIndex = 1;
			}
		}
	}
	
	void LCDManager::print (char *pString) {
		int i;
		for (i = 0; pString[i]; i++) print(pString[i]);
	}
	
	void LCDManager::setCursor (int pColIndex, int pRowIndex) {
		rowIndex = pRowIndex;
		colIndex[rowIndex] = pColIndex;
	}
	
	void LCDManager::deleteChar() {
		if (rowIndex == 1 && colIndex[rowIndex] == 0) {
			rowIndex = 0;
			colIndex[rowIndex] = 15;
		} else if (colIndex[rowIndex] > 0) {
			colIndex[rowIndex]--;
		}
		lcd.setCursor(colIndex[rowIndex], rowIndex);
		lcd.print(' ');
		line[rowIndex][colIndex[rowIndex]] = '\0';
	}
	
};

namespace My {
	
	ShiftRegister::ShiftRegister (void) {
		int i;
		pin.clock = Pin(SHIFT_REGISTER_CLOCK_PIN, OUTPUT);
		pin.input = Pin(SHIFT_REGISTER_INPUT_PIN, OUTPUT);
		setInputValue(1);
		for (i = 0; i < 16; i++) clock();
	}
	
	void ShiftRegister::setInputValue (int pValue) {
		pin.input.setValue(pValue);
	}
	
	void ShiftRegister::clock (void) {
		pin.clock.setValue(0);
		pin.clock.setValue(1);
	}
	
};

namespace My {
	
	SwitchState::SwitchState (void) {
		pressed = 0;
		time = 0;
	}
	
	SwitchState::SwitchState (int pPressed, int pTime) {
		pressed = pPressed;
		time = pTime;
	}
	
};

namespace My {
	
	SwitchManager::SwitchManager (void) : shiftRegister() {
		int i;
		pin.input = Pin(SWITCH_MANAGER_INPUT_PIN, INPUT_PULLUP);
		pinToSwitch[0]  = SHIFT_REGISTER_PIN0_SWITCH;
		pinToSwitch[1]  = SHIFT_REGISTER_PIN1_SWITCH;
		pinToSwitch[2]  = SHIFT_REGISTER_PIN2_SWITCH;
		pinToSwitch[3]  = SHIFT_REGISTER_PIN3_SWITCH;
		pinToSwitch[4]  = SHIFT_REGISTER_PIN4_SWITCH;
		pinToSwitch[5]  = SHIFT_REGISTER_PIN5_SWITCH;
		pinToSwitch[6]  = SHIFT_REGISTER_PIN6_SWITCH;
		pinToSwitch[7]  = SHIFT_REGISTER_PIN7_SWITCH;
		pinToSwitch[8]  = SHIFT_REGISTER_PIN8_SWITCH;
		pinToSwitch[9]  = SHIFT_REGISTER_PIN9_SWITCH;
		pinToSwitch[10] = SHIFT_REGISTER_PIN10_SWITCH;
		pinToSwitch[11] = SHIFT_REGISTER_PIN11_SWITCH;
		for(i = 0; i < 12; i++) switchState[i] = SwitchState();
	}
	
	void SwitchManager::update (void) {
		int i, pressed;
		SwitchState newState;
		shiftRegister.setInputValue(0);
		shiftRegister.clock();
		shiftRegister.setInputValue(1);
		shiftRegister.clock();
		for (i = 0; i < 12; i++) {
			pressed = !pin.input.getValue(); // ! is to turn active low input to active high logic
			if (pressed != switchState[i].pressed) {
				if (pressed) {
					newState = SwitchState(pressed, millis());
				} else {
					newState = SwitchState(pressed, millis());
				}
				system.switchEventHandler(pinToSwitch[i], switchState[i], newState);
				switchState[i] = newState;
			}
			shiftRegister.clock();
		}
	}
	
};

namespace My {
	
	LEDManager::LEDManager (void) {
		pin.r = Pin(LED_R_PIN, OUTPUT);
		pin.g = Pin(LED_G_PIN, OUTPUT);
		pin.y = Pin(LED_Y_PIN, OUTPUT);
	}
	
};

namespace My {
	
	System::System (void) : switchManager(), ledManager(), lcdManager() {
		
	}
	
	void System::init() {
		lcdManager.print("Welcome");
		delay(2000);
		lcdManager.clear();
		// update only print this in active mode
		// print locked in inactive mode
		// print unlocked in unlocked mode
		lcdManager.print("Enter code:");
		lcdManager.setCursor(0, 1);
		setActive();
		setLocked();
	}
	
	void System::setActive () {
		isActive = 1;
		ledManager.pin.y.setValue(1);
		lcdManager.clear();
		if (isLocked) {
			lcdManager.print("Enter code:");
			lcdManager.setCursor(0, 1);
		} else {
			lcdManager.print("Unlocked");
		}
	}

	void System::setInactive () {
		isActive = 0;
		ledManager.pin.y.setValue(0);
		lcdManager.clear();
		if (isLocked) {
			lcdManager.print("Locked");
		} else {
			lcdManager.print("Unlocked");
		}
	}

	void System::setLocked () {
		isLocked = 1;
		ledManager.pin.g.setValue(0);
		ledManager.pin.r.setValue(1);
		lcdManager.clear();
		if (isActive) {
			lcdManager.print("Enter code:");
			lcdManager.setCursor(0, 1);
		} else {
			lcdManager.print("Locked");
		}
		
	}

	void System::setUnlocked () {
		isLocked = 0;
		ledManager.pin.g.setValue(1);
		ledManager.pin.r.setValue(0);
		lcdManager.clear();
		lcdManager.print("Unlocked");
		lcdManager.setCursor(0, 1);
	}

	void System::update (void) {
		switchManager.update();
		if (Serial.available()) {
			Serial.readBytes(buffer, 1);
			Serial.readBytes(&buffer[1], buffer[0]);
			if (buffer[1] == 0) {
				// receive: set locked
				setLocked();
			} else if (buffer[1] == 1) {
				// receive: set unlocked
				setUnlocked();
			} else if (buffer[1] == 2) {
				// receive: set active
				setActive();
			} else if (buffer[1] == 3) {
				// receive: set inactive
				setInactive();
			}
		}
	}
	
	void System::switchEventHandler (int switchNum, SwitchState oldState, SwitchState newState) {
		if (!isActive) return;
		if (newState.pressed) {
			if (isLocked) {
				// IS LOCKED
				if (switchNum >= SWITCH_0 && switchNum <= SWITCH_9) {
					if (lcdManager.colIndex[lcdManager.rowIndex] < 10) {
						lcdManager.print(switchNum);
					}
				} else if (switchNum == SWITCH_OK) {
					if (lcdManager.colIndex[lcdManager.rowIndex] == 10) {
						// send: request set unlocked
						buffer[0] = 12;
						buffer[1] = 1;
						buffer[2] = lcdManager.line[1][0];
						buffer[3] = lcdManager.line[1][1];
						buffer[4] = lcdManager.line[1][2];
						buffer[5] = lcdManager.line[1][3];
						buffer[6] = lcdManager.line[1][4];
						buffer[7] = lcdManager.line[1][5];
						buffer[8] = lcdManager.line[1][6];
						buffer[9] = lcdManager.line[1][7];
						buffer[10] = lcdManager.line[1][8];
						buffer[11] = lcdManager.line[1][9];
						buffer[12] = 0;
						Serial.write((uint8_t*) buffer, 13);
						lcdManager.clear();
						Serial.readBytes(buffer, 1);
						Serial.readBytes(&buffer[1], buffer[0]);
						if (buffer[1] == 4) {
							// receieve: accept set unlocked
							setUnlocked();
						} else if (buffer[1] == 5) {
							// receieve: reject set unlocked
							lcdManager.clear();
							lcdManager.print("Invalid code");
							delay(2000);
							setLocked();
						}
					}
				} else if (switchNum == SWITCH_CANCEL) {
					if (lcdManager.colIndex[lcdManager.rowIndex] > 0) {
						lcdManager.deleteChar();
					}
				}
			// IS NOT LOCKED
			} else {
				if (switchNum == SWITCH_CANCEL) {
					// send: set locked (by user, is active and unlocked)
					buffer[0] = 1;
					buffer[1] = 0;
					Serial.write((uint8_t*) buffer, 2);
					setLocked();
				}
			}
		}
	}

};

void setup() {
	Serial.begin(9600);
	system.init();
}

void loop() {
	system.update();
	delay(50);
}

