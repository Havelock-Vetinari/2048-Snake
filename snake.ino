#include <Adafruit_GFX.h>   // Graphics Library
#include <RGBmatrixPanel.h> // Hardware Library
 
#define ACTIVATED LOW
 
#define CLK 11
#define LAT 10
#define OE  9
#define A   12
#define B   13
#define C   14
#define D   15
 
RGBmatrixPanel creoqode(A, B, C, D, CLK, LAT, OE, false, 64);

#define DIR_UP -64
#define DIR_RIGHT 1
#define DIR_DOWN 64
#define DIR_LEFT -1
 
const int button_left = 34;
const int button_up = 35;
const int button_right = 36;
const int button_down = 37;
const int button5 = 38;
const int button6 = 39;


unsigned int snake[62*30];
unsigned int snake_len = 2;
int snake_direction = 1;
unsigned int snake_old_tail = 0;
unsigned long curtime;


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
void setup() { 
  
  creoqode.begin();

  creoqode.drawRect(0, 0, 64, 32, creoqode.Color333(0, 1, 1));
 
  pinMode(button_left, INPUT_PULLUP);
  pinMode(button_up, INPUT_PULLUP);
  pinMode(button_right, INPUT_PULLUP);
  pinMode(button_down, INPUT_PULLUP);
  pinMode(button5, INPUT_PULLUP);
  pinMode(button6, INPUT_PULLUP);


  
 
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void loop() {
  reset_snake();
  unsigned long next_move = 0;
  draw_snake();
  while(true){
    curtime = millis();
    if(digitalRead(button_left)==ACTIVATED){
      if(snake_direction != DIR_RIGHT) snake_direction = DIR_LEFT;
    } else if(digitalRead(button_right)==ACTIVATED){
      if(snake_direction != DIR_LEFT) snake_direction = DIR_RIGHT;
    } else if(digitalRead(button_up)==ACTIVATED){
      if(snake_direction != DIR_DOWN) snake_direction = DIR_UP;
    } else if(digitalRead(button_down)==ACTIVATED){
      if(snake_direction != DIR_UP) snake_direction = DIR_DOWN;
    }
    if(curtime > next_move) {
      move_snake();
      if(detect_colision()) {
        game_over();
        delay(2000);
        break;
      }
      draw_snake();
      next_move = curtime + 400;
    }
  }
}

void reset_snake() {
  snake_len = 2;
  snake_direction = DIR_RIGHT;
  memset(snake, 0,sizeof(snake));
  snake_old_tail = 1023+31;
  snake[0] = 1025+31;
  snake[1] = 1024+31;
  creoqode.fillRect(1, 1, 62, 30, 0);

}

void draw_snake() {
  creoqode.drawPixel(snake_old_tail%64, snake_old_tail/64, 0);
  for(int i = 0; i < snake_len; i++){
    creoqode.drawPixel(snake[i]%64, snake[i]/64, creoqode.Color333(1, 2, 0));
  } 
}

void move_snake() {
  snake_old_tail = snake[snake_len-1];
  for(int i = snake_len -1; i>0; i--){
    snake[i] = snake[i-1];
  }
  snake[0] = snake[0] + snake_direction;
}

bool detect_colision() {
  if(snake[0] % 64 == 0 || snake[0] % 64 == 63) {
    return true;
  }
  if(snake[0] / 64 == 1 || snake[0] / 64 == 31) {
    return true;
  }

  return false;
}

void game_over(){
  creoqode.setTextSize(2);
  creoqode.setCursor(8, 1);
  creoqode.setTextColor(creoqode.Color333(3, 0, 0));
  creoqode.print("GAME\n OVER");
  
}

void put_food(){
  
}


