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
 
const int button1 = 34;
const int button2 = 35;
const int button3 = 36;
const int button4 = 37;
const int button5 = 38;
const int button6 = 39;


unsigned int snake[62*30];
unsigned int snake_len = 2;
int snake_direction = 1;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
void setup() { 
  
  creoqode.begin();

  creoqode.drawRect(0, 0, 64, 32, creoqode.Color333(0, 1, 1));
 
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(button4, INPUT_PULLUP);
  pinMode(button5, INPUT_PULLUP);
  pinMode(button6, INPUT_PULLUP);


  
 
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void loop() {
  reset_snake();
  while(true){
    draw_snake();
    delay(200);
    move_snake();
    if(detect_colision()) {
      game_over();
      delay(2000);
      break;
    }
  }
}

void reset_snake() {
  snake_len = 2;
  memset(snake, 0,sizeof(snake));
  snake[0] = 1025+31;
  snake[1] = 1024+31;
}

void draw_snake() {
  creoqode.fillRect(1, 1, 62, 30, 0);
  for(int i = 0; i < snake_len; i++){
    creoqode.drawPixel(snake[i]%64, snake[i]/64, creoqode.Color333(1, 2, 0));
  } 
}

void move_snake() {
  for(int i = 0; i < snake_len; i++){
    snake[i] = snake[i] + snake_direction;
  }
}

bool detect_colision() {
  unsigned int snake_head = snake[snake_len - 1];
  if(snake_head % 64 == 1 || snake_head % 64 == 63) {
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


