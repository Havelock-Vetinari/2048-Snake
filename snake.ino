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

#define GET_X(p) p%64
#define GET_Y(p) p/64
#define GET_POS(x,y) (64*y+x)

#define KEY_PRESSED(key) digitalRead(key)==ACTIVATED

#define INITIAL_GAME_SPEED 320
#define MAX_GAME_SPEED 60
#define LEVEL_UP_EVERY 10
#define SPEEDUP 20

#define DIR_UP -64
#define DIR_RIGHT 1
#define DIR_DOWN 64
#define DIR_LEFT -1

RGBmatrixPanel creoqode(A, B, C, D, CLK, LAT, OE, false, 64);
 
const int button_left = 34;
const int button_up = 35;
const int button_right = 36;
const int button_down = 37;
const int button_5 = 38;
const int button_6 = 39;

const unsigned int color_logo = creoqode.Color333(1, 2, 1);
const unsigned int color_border = creoqode.Color333(0, 1, 1);
const unsigned int color_title = creoqode.Color333(5, 0, 0);
const unsigned int color_gameover = creoqode.Color333(3, 0, 0);
const unsigned int color_food = creoqode.Color333(0, 3, 0);
const unsigned int color_snake = creoqode.Color333(1, 2, 0);
const unsigned int color_score_title = creoqode.Color333(0, 1, 0);
const unsigned int color_score_points = creoqode.Color333(0, 3, 0);
const unsigned int color_level_mark = creoqode.Color333(2, 0, 0);

unsigned int snake[62*30];
unsigned int snake_len = 2;
int snake_direction = 1;
int snake_next_dir = 1;
unsigned int snake_old_tail = 0;
unsigned int food;
unsigned long curtime;
unsigned long game_speed = INITIAL_GAME_SPEED;

unsigned long points = 0;
unsigned int points_factor = 1;
unsigned int catches = 0;


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
void setup() { 
  
  creoqode.begin();
  draw_logo();
  delay(1500);
  creoqode.setTextSize(2);
  creoqode.setCursor(3, 5);
  creoqode.setTextColor(color_title);
  creoqode.fillRect(2, 4, 60, 16, 0);
  creoqode.print("Snake");
  delay(3000);
  
  creoqode.drawRect(0, 0, 64, 32, color_border);
  delay(1250);
  pinMode(button_left, INPUT_PULLUP);
  pinMode(button_up, INPUT_PULLUP);
  pinMode(button_right, INPUT_PULLUP);
  pinMode(button_down, INPUT_PULLUP);
  pinMode(button_5, INPUT_PULLUP);
  pinMode(button_6, INPUT_PULLUP);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void loop() {
  randomSeed(analogRead(5)*millis());
  reset_snake();
  put_food(GET_POS(31, 15), GET_POS(33, 30));
  unsigned long next_move = 0;
  draw_snake();
  bool paused = false;
  while(true){
    curtime = millis();
    if(KEY_PRESSED(button_left)){
      if(snake_direction != DIR_RIGHT) snake_next_dir = DIR_LEFT;
    } else if(KEY_PRESSED(button_right)){
      if(snake_direction != DIR_LEFT) snake_next_dir = DIR_RIGHT;
    } else if(KEY_PRESSED(button_up)){
      if(snake_direction != DIR_DOWN) snake_next_dir = DIR_UP;
    } else if(KEY_PRESSED(button_down)){
      if(snake_direction != DIR_UP) snake_next_dir = DIR_DOWN;
    } else if(KEY_PRESSED(button_6)){
      paused = !paused;
      delay(1000);
    }
    if(paused) {
      delay(100);
      next_move = curtime + game_speed;
    }
    if(curtime > next_move) {
      move_snake();
      if(detect_colision()) {
        game_over();
        delay(2000);
        creoqode.fillRect(1, 1, 60, 30, 0);
        print_points();
        while(true){
          if(KEY_PRESSED(button_up) || KEY_PRESSED(button_down) ||
             KEY_PRESSED(button_left) || KEY_PRESSED(button_right) ||
             KEY_PRESSED(button_5) || KEY_PRESSED(button_6)){
            break;
          }
          delay(10);
        }
        break;
      }
      if(snake[0] == food){
        snake[snake_len]=snake[snake_len-1];
        snake_len++;
        catches++;
        points+=points_factor;
        if((catches%LEVEL_UP_EVERY)==0 && game_speed > MAX_GAME_SPEED) {
          points_factor++;
          game_speed-=SPEEDUP;
          creoqode.drawPixel(catches/10-1, 0, color_level_mark);
        }
        put_food(GET_POS(1,1), GET_POS(62,14));
      }
      draw_snake();
      next_move = curtime + game_speed;
    }
  }
}

void reset_snake() {
  game_speed = INITIAL_GAME_SPEED;
  snake_len = 2;
  points = 0;
  points_factor = 1;
  catches = 0;
  snake_direction = DIR_RIGHT;
  snake_next_dir = snake_direction;
  memset(snake, 0,sizeof(snake));
  snake_old_tail = 0;
  snake[0] = GET_POS(31,15);
  snake[1] = GET_POS(32,15);
  creoqode.drawRect(0, 0, 64, 32, color_border);
  creoqode.fillRect(1, 1, 62, 30, 0);

}

void draw_snake() {
  if(snake_old_tail!=0) creoqode.drawPixel(GET_X(snake_old_tail), GET_Y(snake_old_tail), 0);
  for(int i = 0; i < snake_len; i++){
    creoqode.drawPixel(GET_X(snake[i]), GET_Y(snake[i]), color_snake);
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
  creoqode.setTextColor(color_gameover);
  creoqode.print("GAME OVER");
}

void print_points(){
  creoqode.setTextSize(1);
  creoqode.setCursor(2, 2);
  creoqode.setTextColor(color_score_title);
  creoqode.print("You've got");
  String points_string = String(points);
  uint16_t text_width = 0;
  creoqode.getTextBounds((char*)points_string.c_str(), 0, 0, NULL, NULL, &text_width, NULL);
  creoqode.setCursor(32-(text_width/2), 13);
  creoqode.setTextColor(color_score_points);
  creoqode.print(String(points_string));
  creoqode.setTextColor(color_score_title);
  creoqode.setCursor(points==1?18:14, 23);
  creoqode.print(points==1?"point":"points");

}

void put_food(int first, int last){
  unsigned int new_food;
  while(true){
    new_food = random(first, last+1);
    bool colision = false;
    for(int i = 0; i < snake_len; i++){
      if(new_food == snake[i]) {
        colision = true;
        continue;
      }
    }
    if(colision == true) continue;
    if(GET_X(new_food) == 0 || GET_X(new_food) == 63) continue;
    if(GET_Y(new_food) == 0 || GET_Y(new_food) == 31) continue;
    break;
  }
  food = new_food;
  creoqode.drawPixel(GET_X(food), GET_Y(food), color_food);
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
        creoqode.drawPixel(i+2, j, color_logo);
      }
    }
  }
  for(int j=20; j < 23; j++){
    creoqode.drawPixel(30+2, j, color_logo);
  }
  for(int i = 0; i < 123; i++){
    creoqode.drawPixel(((code_and_text[i]-1)%LOGO_WIDTH)+2, ((code_and_text[i]-1)/LOGO_WIDTH)+23, color_logo);
  }
}

