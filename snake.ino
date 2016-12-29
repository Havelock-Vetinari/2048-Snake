#include <Adafruit_GFX.h>   // Graphics Library
#include <RGBmatrixPanel.h> // Hardware Library

/**
 * 2048 Snake
 * by Julian Szulc 2016
 * @julian.szulc
 * https://github.com/Havelock-Vetinari
 */
 
#define ACTIVATED LOW
 
#define CLK 11
#define LAT 10
#define OE  9
#define A   12
#define B   13
#define C   14
#define D   15
 
RGBmatrixPanel creoqode(A, B, C, D, CLK, LAT, OE, false, 64);

#define GET_X(p) p%64
#define GET_Y(p) p/64

#define GAME_SPEED 500

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
int snake_next_dir = 1;
unsigned int snake_old_tail = 0;
unsigned int food;
unsigned long curtime;


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
void setup() { 
  
  creoqode.begin();
  draw_logo();
  delay(1500);
  creoqode.setTextSize(2);
  creoqode.setCursor(3, 5);
  creoqode.setTextColor(creoqode.Color333(5, 0, 0));
  creoqode.fillRect(2, 4, 60, 16, 0);
  creoqode.print("Snake");
  delay(3000);
  
  creoqode.drawRect(0, 0, 64, 32, creoqode.Color333(0, 1, 1));
  delay(1250);
  pinMode(button_left, INPUT_PULLUP);
  pinMode(button_up, INPUT_PULLUP);
  pinMode(button_right, INPUT_PULLUP);
  pinMode(button_down, INPUT_PULLUP);
  pinMode(button5, INPUT_PULLUP);
  pinMode(button6, INPUT_PULLUP);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void loop() {
  randomSeed(analogRead(5)*millis());
  reset_snake();
  put_food();
  unsigned long next_move = 0;
  draw_snake();
  while(true){
    curtime = millis();
    if(digitalRead(button_left)==ACTIVATED){
      if(snake_direction != DIR_RIGHT) snake_next_dir = DIR_LEFT;
    } else if(digitalRead(button_right)==ACTIVATED){
      if(snake_direction != DIR_LEFT) snake_next_dir = DIR_RIGHT;
    } else if(digitalRead(button_up)==ACTIVATED){
      if(snake_direction != DIR_DOWN) snake_next_dir = DIR_UP;
    } else if(digitalRead(button_down)==ACTIVATED){
      if(snake_direction != DIR_UP) snake_next_dir = DIR_DOWN;
    }
    if(curtime > next_move) {
      move_snake();
      if(detect_colision()) {
        game_over();
        delay(2000);
        break;
      }
      if(snake[0] == food){
        snake[snake_len]=snake[snake_len-1];
        snake_len++;
        put_food();
      }
      draw_snake();
      next_move = curtime + GAME_SPEED;
    }
  }
}

void reset_snake() {
  snake_len = 2;
  snake_direction = DIR_RIGHT;
  snake_next_dir = snake_direction;
  memset(snake, 0,sizeof(snake));
  snake_old_tail = 1023+31;
  snake[0] = 1025+31;
  snake[1] = 1024+31;
  creoqode.fillRect(1, 1, 62, 30, 0);

}

void draw_snake() {
  creoqode.drawPixel(snake_old_tail%64, snake_old_tail/64, 0);
  for(int i = 0; i < snake_len; i++){
    creoqode.drawPixel(GET_X(snake[i]), GET_Y(snake[i]), creoqode.Color333(1, 2, 0));
  } 
}

void move_snake() {
  snake_direction = snake_next_dir;
  snake_old_tail = snake[snake_len-1];
  for(int i = snake_len -1; i>0; i--){
    snake[i] = snake[i-1];
  }
  snake[0] = snake[0] + snake_direction;
}

bool detect_colision() {
  if(GET_X(snake[0]) == 0 || GET_X(snake[0]) == 63) {
    return true;
  }
  if(GET_Y(snake[0])== 0 || GET_Y(snake[0])== 31) {
    return true;
  }
  for(int i=1; i<snake_len; i++){
    if(snake[0] == snake[i]){
      return true;
    }
  }

  return false;
}

void game_over(){
  creoqode.setTextSize(2);
  creoqode.setCursor(8, 1);
  creoqode.setTextColor(creoqode.Color333(3, 0, 0));
  creoqode.print("GAME OVER");
  
}

void put_food(){
  unsigned int new_food;
  while(true){
    new_food = random(65, (2048-64));
    for(int i = 0; i < snake_len; i++){
      if(new_food == snake[i]) continue;
    }
    if(GET_X(new_food) == 0 || GET_X(new_food) == 63) continue;
    break;
  }
  food = new_food;
  creoqode.drawPixel(GET_X(food), GET_Y(food), creoqode.Color333(0, 3, 0));
}

void draw_logo() {
  #define LOGO_WIDTH 60
  const bool code[] = {
    true,true,false,false,true,true,false,true,false,false,true,true,true,false,false,true,
    false,false,true,false,true,true,false,false,true,true,false,true,false,false,true,false,
    false,true,true,false,true,false,false,true,true,false,true,false,false,true,true,false,
    true,false,false,true,true,true,false,false,true,false,false,true
  };
  const unsigned int code_and_text[] = {
    2,11,16,31,52,71,91,112,122,125,128,131,136,139,140,141,142,145,146,147,148,151,154,155,
    156,157,160,161,162,163,166,169,172,177,178,179,180,182,185,188,191,196,202,205,208,214,
    222,226,229,232,237,242,245,248,251,256,259,260,261,262,265,268,274,275,276,277,281,286,
    289,292,297,302,305,308,311,316,319,322,325,328,337,340,346,349,352,357,362,365,366,367,
    368,371,372,373,376,379,380,381,382,385,388,391,394,395,396,397,400,401,402,403,406,407,
    408,409,412,413,414,417,418,419,420,421,422
  };
  for(int i = 0; i < sizeof(code); i++){
    if(code[i]==true) {
      for(int j=1; j < 20; j++){
        creoqode.drawPixel(i+2, j, creoqode.Color333(2, 2, 2));
      }
    }
  }
  for(int j=20; j < 23; j++){
    creoqode.drawPixel(30+2, j, creoqode.Color333(2, 2, 2));
  }
  for(int i = 0; i < 123; i++){
    creoqode.drawPixel(((code_and_text[i]-1)%LOGO_WIDTH)+2, ((code_and_text[i]-1)/LOGO_WIDTH)+23, creoqode.Color333(2, 2, 2));
  }
}

