#include <LiquidCrystal.h>

struct IRReader{
  /* This is a Finite State Machine that recognizes
   * the Samsung IR protocol.
   * Other protocols may be handled by changing a few numbers.
   */ 
  
  boolean setState(boolean state){
    boolean newValueReceived = false;
    
    if(state != this -> state){ //On state changed
      long now = micros(), delta = now - when; //Duration of the last pulse

      if(bitCnt == 32){ //32 bits received successfully
        newValueReceived = true;
        bitCnt = -1;
      }else if(bitCnt >= 0){ //Get the next bit
        if(state){ //On rising edge, determines the next bit based on the pulse distance
          if(delta >= 1200 && delta <= 2000){ // 1690us -> bit 1
            value |= ((long) 1) << bitCnt;
            ++bitCnt;
          }else if(delta >= 300 && delta <= 900){ // 590us -> bit 0
            ++bitCnt;
          }else{
            bitCnt = -1;
          }
        }
      }else{ //Nothing received yet, check start bit
        if(state && delta >= 3000 && delta <= 6000) //Pulse distance should be about 4500us
          bitCnt = 0;
          value = 0;
      }
      
      this -> state = state;
      when = now;
    }

    return newValueReceived;
  }

  unsigned long getValue(){
    return value;
  }

  private:
  unsigned long when = 0, value = 0;
  int bitCnt = -1;
  boolean state = false; //Current state of the IR signal
} reader;

const int IR_PIN = A5;
const byte FOOD_ZERO = 4, ZERO_FOOD = 5, FOOD_ONE = 6, ONE_FOOD = 7;
LiquidCrystal lcd(12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2);

const byte UP = 0x9F, DOWN = 0x9E, LEFT = 0x9A, RIGHT = 0x9D, ENTER = 0x97; //Remote control key codes

void setup() {
  pinMode(IR_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println();

  //Each character of the LCD will be used as two 4x4 sub-characters
  //Let us create custom charactes
  {
    char data[8];

    for(int value = 0; value < 4; ++value){
      for(int row = 0; row < 8; ++row){
        int bit_pos = row / 4, mask = 1 << bit_pos;
        data[row] = (value & mask) ? 0xF: 0;
      }
      
      lcd.createChar(value, data); //This creates the four combinations of
                                   //'snake-piece' and 'blank' sub-chars

      if(value == 0){ //blank
        data[1] = data[2] = 6;
        lcd.createChar(FOOD_ZERO, data); //upper sub-char: food; lower sub-char: blank
        data[1] = data[2] = 0;
        data[5] = data[6] = 6;
        lcd.createChar(ZERO_FOOD, data); //upper sub-char: blank; lower sub-char: food
      }else if(value == 1){
        data[5] = data[6] = 6;
        lcd.createChar(ONE_FOOD, data); //upper sub-char: snake-piece; lower sub-char: food
      }else if(value == 2){
        data[1] = data[2] = 6;
        lcd.createChar(FOOD_ONE, data); //upper sub-char: food; lower sub-char: snake-piece
      }
    }
  }
  
  lcd.begin(16, 2);
  lcd.clear();

  //Build the snake
  for(int i = 0; i < 6; ++i){
    addTail(1, i);
  }

  putFood();
}

char s[12];

void loop() {
  static byte direction = LEFT;
  static long nextTime = millis() + 250;
  static boolean game = true;
  boolean isOn = digitalRead(IR_PIN) == LOW;
  digitalWrite(LED_BUILTIN, isOn ? HIGH : LOW);

  if(reader.setState(isOn)){ //On key pressed, change direction of movement
    byte keyCode = reader.getValue() >> 24;
    boolean change = false;

    if(keyCode == UP)
      change = direction != DOWN;
    else if(keyCode == DOWN)
      change = direction != UP;
    else if(keyCode == LEFT)
      change = direction != RIGHT;
    else if(keyCode == RIGHT)
      change = direction != LEFT;

    if(change)
      direction = keyCode;
    
    if(!game && keyCode == ENTER){ //Restart the game
      clearSnake();
      game = true;
      direction = LEFT;
      for(int i = 0; i < 6; ++i){
        addTail(1, i);
      }
      putFood();
      nextTime = millis() + 250;
    }
  }

  if(game && (long) millis() - nextTime >= 0){ //Time to take the next step
    nextTime += 250;

    if(direction == UP)
      game = moveSnake(-1, 0);
    else if(direction == DOWN)
      game = moveSnake(1, 0);
    else if(direction == LEFT)
      game = moveSnake(0, -1);
    else if(direction == RIGHT)
      game = moveSnake(0, 1);
  }
}

int pixel_data[4] = {0, 0, 0, 0};
int food_row = -1, food_col = -1; //Current position of the snake's food
int snake_row[64], snake_col[64]; //Position of snake's pieces
int snakeLength = 0, snakeHead = 0;

void clearSnake(){
  snakeLength = 0;
  food_row = food_col = -1;
  lcd.clear();
  
  for(int i = 0; i < 4; ++i)
    pixel_data[i] = 0;
}

void addTail(int row, int col){
  int snakeTail = (snakeHead + snakeLength) & 63;
  snake_row[snakeTail] = row;
  snake_col[snakeTail] = col;
  ++snakeLength;
  writePixel(row, col, true);
}

void putFood(){ //Puts a new food into a random position
  const int freeSpace = 64 - snakeLength;

  if(freeSpace > 0){
    int foodIndex = rand() % freeSpace, index = -1;

    for(int row = 0; row < 4; ++row){
      for(int col = 0; col < 16; ++col){
        if(!getPixel(row, col)){
          ++index;

          if(index == foodIndex){
            setFoodPosition(row, col);
            return;
          }
        }
      }
    }
  }
}

boolean moveSnake(int delta_row, int delta_col){
  int head_row = snake_row[snakeHead], head_col = snake_col[snakeHead];
  int new_head_row = (head_row + delta_row) & 0x03, new_head_col = (head_col + delta_col) & 0x0F;
  boolean success = !getPixel(new_head_row, new_head_col);

  if(success){
    int snakeTail = (snakeHead + snakeLength - 1) & 63;
    writePixel(new_head_row, new_head_col, true);
    snakeHead = (snakeHead - 1) & 63;
    snake_row[snakeHead] = new_head_row;
    snake_col[snakeHead] = new_head_col;

    if(new_head_row != food_row || new_head_col != food_col)
      writePixel(snake_row[snakeTail], snake_col[snakeTail], false);
    else{ //Snake eats food
      if(++snakeLength < 64)
        putFood();
      else
        success = false;
    }
  }
  
  return success;
}

void setFoodPosition(int row, int col){
  int old_row = food_row, old_col = food_col;
  food_row = row;
  food_col = col;
  updatePixel(old_row, old_col);
  updatePixel(row, col);
}

boolean getPixel(int row, int col){
  return (pixel_data[row] & (1 << col)) != 0;
}

void updatePixel(int row, int col){
  if(row < 0 || row > 3 || col < 0 || col > 15)
    return;
  
  byte char_code = 0;
  int col_mask = 1 << col, lcd_row = row >> 1;

  if(pixel_data[lcd_row * 2] & col_mask)
    char_code |= 1;

  if(pixel_data[lcd_row * 2 + 1] & col_mask)
    char_code |= 2;

  if(food_col == col){
    if(food_row == lcd_row * 2){
      if(char_code == 0)
        char_code = FOOD_ZERO;
      else if(char_code == 2)
        char_code = FOOD_ONE;
    }else if(food_row == lcd_row * 2 + 1){
      if(char_code == 0)
        char_code = ZERO_FOOD;
      else if(char_code == 1)
        char_code = ONE_FOOD;
    }
  }

  lcd.setCursor(col, lcd_row);
  lcd.write(char_code);
}

void writePixel(int row, int col, boolean value){
  const int col_mask = 1 << col;
  boolean pixel_value = (pixel_data[row] & col_mask) != 0;

  if(value != pixel_value){
    pixel_data[row] ^= col_mask;
    updatePixel(row, col);
  }
}

