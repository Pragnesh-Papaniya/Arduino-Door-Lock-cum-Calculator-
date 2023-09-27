#include <LiquidCrystal.h>
#include <Keypad.h>
#include <EEPROM.h>

int mode;//mode determines whether to use calc or door lock
int buzzPin=10;//to alert by buzzer for 3 wrong attempts 
int modePin=11;//will determine calc or door lock

//Calculator variables
String operand1 = "";//where num1 keys will accumulate 
String operand2 = "";//where num2 keys will accumulate 
String operatorChar = "";//where operation keys will accumulate 
int num1 = 0;//actual number1 to be used in operation 
int num2 = 0;//actual number2 to be used in operation
float result = 0.0;
int secondline = 0;//increment it to display each character of result
int firstline = 0;//increment it to display each character of num1,operator and num2
bool calculate = false;//condition 

// Door Lock variables
String password = ""; // the user defined password
String input = ""; // the user's input
bool unlocked = false; // whether the door is unlocked or not
byte count = 0;//represents no of elements entered for displaying each as '*' on LCD
byte false_counter=0;//represents no of false attempts to alert by buzzer

// Define the address in EEPROM to store the password
int address = 0;

// Define the pins for the LCD and keypad
const byte ROWS = 4, COLS = 4; //4*4 keypad
char keys[ROWS][COLS] = {
{'1', '2', '3', 'A'},
{'4', '5', '6', 'B'},
{'7', '8', '9', 'C'},
{'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {2,3,4,5}; //Digital pins
byte colPins[COLS] = {6,7,8,9}; //Digital pins

// Initialize the LCD and keypad
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);
Keypad keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  pinMode(buzzPin,OUTPUT);//will go to buzzer
  pinMode(modePin,INPUT);//will determine mode(pin D11)
  lcd.begin(16, 2);// Set up the LCD
  lcd.print("D11==0 Calc");
  lcd.setCursor(0,1);
  lcd.print("D11==1 Door Lock");
  delay(2000);
  lcd.clear();
  keypad.setDebounceTime(50);// Set up the keypad
}

void loop() {
  mode=digitalRead(modePin);//set it low or high
  if(mode==LOW){
    calculator();
  }
  else if(mode==HIGH){
    doorLock();
  }
}

void calculator(){
  char key = keypad.getKey();//take key
  if(key == '*' && key != NO_KEY){//make sure key is actually pressed
    //clear door lock variables and display when changing mode
    input="";
    password="";
    count=0;
    lcd.setCursor(0,0);
    clearDisplayAndVars(); //Clear display and variables
  } 
  if(key == '#' && key != NO_KEY){
    doCalculation();               //Do the calculation
    lcd.setCursor(0, 1);           //Result will be written on the second row,1st column onwards
    lcd.print(result);             //Calculation result will be shown
    delay(1000);
  } 
  else if(key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8' || key == '9' || key == '0' && key != NO_KEY){ //A number is entered
    lcd.setCursor(firstline, 0);  //Inputs will be shown on the first line
    lcd.print(key);
    makeNumbers(key);
    firstline++;//increment to show each entered character
  } 
  else if(key != NO_KEY && key!='*'){  //An operator is entered
    lcd.setCursor(firstline, 0);   //Operators will be shown on the first line
    if(key == 'A') {
      operatorChar = "+";
      lcd.print("+");
    } 
    else if(key == 'B') {
      operatorChar = "-";
      lcd.print("-");
    } 
    else if(key == 'C') {
      operatorChar = "*";
      lcd.print("*");
    } 
    else if(key == 'D') {
      operatorChar = "/";
      lcd.print("/");
    } 
    secondline++;
    firstline++;
  }
}

void clearDisplayAndVars(){//clears everything to do next operation
  lcd.clear();
  operand1 = "";
  operand2 = "";
  operatorChar = "";
  secondline = 0;
  firstline = 0;
  result = 0.0;
  calculate = false;  
}

void makeNumbers(char key){
  if(operatorChar == ""){//entering first num till operator is not entered
    operand1 += key;//accumulate all keys in op1
  } 
  else if(operatorChar.length() == 1 and calculate == false) { 
    //First number completed,checks operator entered and '#' is not yet entered to do calculation 
    operand2 += key;//accumulate all keys in op2
  }
  num1 = operand1.toInt();//converts all accumulated keys to operatable values
  num2 = operand2.toInt();//has a limit of how much number calculation it can handle
}

void doCalculation(){
  calculate = true;
  if(operatorChar == "+") {
    result = num1 + num2;
  }
  else if(operatorChar == "-") {
    result = num1 - num2;
  }
  else if(operatorChar == "*") {
    result = num1 * num2;
  } 
  else if(operatorChar == "/") {
    result = float(num1) / float(num2);
  }
}

void doorLock(){
  char key = keypad.getKey();
  // If a key is pressed, then only add it to the input
  if (key) {
    if (password == "") {
      lcd.setCursor(0,0);
      lcd.print("Enter new passW");
    }
    input += key;//accumulate keys to input
    lcd.setCursor(count, 1); // move cursor to show each new char as secret ‘*’
    lcd.print("*");
    count++; // increment data array by 1 to store new char, also keep track of the number of chars entered
    if(key=='*'){//to clear display, calc variables and to set new user defined password again
      clearDisplayAndVars();
      input="";
      password="";
      count=0;
      lcd.clear();
      lcd.print("Enter new passW");
      lcd.setCursor(0,1);//password on the 2nd row
      char key = keypad.getKey();
    }
    if(key=='#'){//to clear input
      input="";
      count=0;
      lcd.clear();
      lcd.print("Enter password");
      lcd.setCursor(0,1);
      char key = keypad.getKey();
    }
    }
    // Check if the input is complete
    if (input.length() == 4) {//change this 4 to different values
      count=0;
    // If the password has not been set yet, store the input as the new password
    if (password == "") {
      password = input;
      storePassword(password);//in EEPROM
      lcd.clear();
      lcd.print("Password set");
      delay(1000);
      lcd.clear();
      lcd.print("Enter password:");
      input = "";
      }
      // Otherwise, directly check if the input matches the password
      else {
      // If the input matches, unlock the door
        if (input == password) {
          unlocked = true;
          false_counter=0;
          lcd.clear();
          lcd.print("Door unlocked");
          delay(2000);
          lcd.clear();
          lcd.print("Enter password:");
          input = ""; //make input null string again to retake input
        }
      // Otherwise, display an error message and reset the input
      else {
        false_counter++;
        if(false_counter==3){
          lcd.clear();
          lcd.print("3 penalties");
          lcd.setCursor(0,1);
          lcd.print("Beware");
          digitalWrite(buzzPin,HIGH);
          delay(3000); //to punish the user 5 seconds for 3 wrong attempts
          false_counter=0;//reset false attempts
          digitalWrite(buzzPin,LOW);
          }
        lcd.clear();
        lcd.print("Incorrect");
        delay(1000);
        lcd.clear();
        lcd.print("Enter password:");
        input = ""; //make input null string again to retake input
      }
      count=0;
    }
    }
    // If the door is unlocked, display a message
    if (unlocked) {
      lcd.clear();
      lcd.print("Welcome!");
      delay(2000);
      lcd.clear();
      lcd.print("Enter password:");
      unlocked = false;
    }
}

// Function to store the password in EEPROM
void storePassword(String password) {
  for (int i = 0; i < 4; i++) {//password length is 4
  EEPROM.write(address + i, password[i]);
  }
}
