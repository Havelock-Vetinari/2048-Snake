#include <Adafruit_GFX.h>   // Graphics Library
#include <RGBmatrixPanel.h> // Hardware Library
#include <EEPROM.h>

#include "Font3x5FixedNum.h"
#include "Font2x5FixedMonoNum.h"
#include "Font5x5Fixed.h"
#include <Fonts/Picopixel.h>

/**
 * 2048 Snake
 * by Julian Szulc 2016-2023
 * @julian.szulc
 * https://github.com/Havelock-Vetinari
 */
 
#define ACTIVATED LOW
#define DEACTIVATED HIGH

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
#define KEY_NOT_PRESSED(key) digitalRead(key)==DEACTIVATED

#define INITIAL_GAME_SPEED 320
#define MAX_GAME_SPEED 60
#define LEVEL_UP_EVERY 10
#define SPEEDUP 20
#define TURBO_SPEED 30

#define DIR_UP -64
#define DIR_RIGHT 1
#define DIR_DOWN 64
#define DIR_LEFT -1

#define NUM_HI_SCORES 10
#define NAME_LEN 6

const uint8_t eeprom_magic[] = { 0x58, 0xCE };
const char eeprom_version = 0x01;

#define HIGH_SCORES_ADDRESS 3

RGBmatrixPanel creoqode(A, B, C, D, CLK, LAT, OE, false, 64);
 
const int button_left = 34;
const int button_up = 35;
const int button_right = 36;
const int button_down = 37;
const int button_turbo = 38;
const int button_pause = 39;

const unsigned int color_logo = creoqode.Color444(1, 2, 1);
const unsigned int color_border = creoqode.Color444(0, 1, 1);
const unsigned int color_title = creoqode.Color444(10, 0, 0);
const unsigned int color_gameover = creoqode.Color444(6, 0, 0);
const unsigned int color_food = creoqode.Color444(0, 6, 0);
const unsigned int color_snake_head = creoqode.Color444(7, 0, 2);
const unsigned int color_snake_even = creoqode.Color444(0, 1, 5);
const unsigned int color_snake_odd = creoqode.Color444(1, 0, 5);
const unsigned int color_score_title = creoqode.Color444(0, 2, 0);
const unsigned int color_score_points = creoqode.Color444(0, 6, 0);
const unsigned int color_level_mark = creoqode.Color444(4, 0, 0);

unsigned int snake_len = 2;
int snake_direction = 1;
int snake_next_dir = 1;
unsigned int snake_old_tail = 0;
unsigned int food;
unsigned long curtime;
unsigned long game_speed = INITIAL_GAME_SPEED;

uint16_t points = 0;
unsigned int points_factor = 1;
unsigned int catches = 0;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

String enter_name();

typedef struct {
  char name[NAME_LEN] = {'\0'};
  uint16_t points = 0;
} highscore_entry;

typedef struct {
  uint8_t version = 1;
  highscore_entry scores[NUM_HI_SCORES];
} highscores;

void show_high_scores(highscores&);

void register_high_score(String name, uint16_t points, highscores &highscores_table);
bool is_high_score_eligable(uint16_t points, highscores &highscores_table);

highscores scores;

void intro();
void draw_logo();

void reset_snake(unsigned int snake[]);
void draw_snake(unsigned int snake[]);
void put_food(int first, int last, unsigned int snake[]);
void move_snake(unsigned int snake[]);
void print_points();
void game_over();
bool detect_colision(unsigned int snake[]);
void play_game();
 
void setup() {
  int a1 = analogRead(5) * analogRead(5);
  creoqode.begin();
  randomSeed(analogRead(5)*millis() + a1);

  if (EEPROM.read(0) != eeprom_magic[0] || EEPROM.read(1) != eeprom_magic[1]) {
    EEPROM.write(0, eeprom_magic[0]);
    EEPROM.write(1, eeprom_magic[1]);
    EEPROM.write(2, eeprom_version);
    EEPROM.put(HIGH_SCORES_ADDRESS, scores);
  } else {
    EEPROM.get(HIGH_SCORES_ADDRESS,scores);
  }

  intro();

  creoqode.drawRect(0, 0, 64, 32, color_border);
  delay(1200);
  pinMode(button_left, INPUT_PULLUP);
  pinMode(button_up, INPUT_PULLUP);
  pinMode(button_right, INPUT_PULLUP);
  pinMode(button_down, INPUT_PULLUP);
  pinMode(button_turbo, INPUT_PULLUP);
  pinMode(button_pause, INPUT_PULLUP);
  // Serial.begin(9600);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void loop() {
  play_game();
  if (is_high_score_eligable(points, scores)) {
     String name = enter_name();
     register_high_score(name, points, scores);
     EEPROM.put(HIGH_SCORES_ADDRESS, scores);
  }
  
  show_high_scores(scores);
  delay(125);
}

void intro() {
  draw_logo();
  delay(1500);
  creoqode.setTextSize(2);
  creoqode.setCursor(3, 5);
  creoqode.setTextColor(color_title);
  creoqode.fillRect(2, 4, 60, 16, 0);
  if (random(0, 10) > 5) {
    creoqode.print("Wonsz");
    delay(2000);
    creoqode.setTextSize(1);
    creoqode.fillRect(2, 20, 62, 12, 0);
    creoqode.setCursor(3, 21);
    creoqode.print("tududu");
    delay(750);
    creoqode.fillRect(2, 20, 62, 12, 0);
    creoqode.setCursor(25, 24);
    creoqode.print("tududu");
    delay(2000);
  } else {
    creoqode.print("Snake");
    delay(2000);
  }
  
  creoqode.drawRect(0, 0, 64, 32, color_border);
  delay(1200);
}

void play_game() {
  unsigned int snake[62*30];

  randomSeed(analogRead(5)*millis());
  reset_snake(snake);
  put_food(GET_POS(31, 15), GET_POS(33, 30), snake);
  unsigned long next_move = 0;
  draw_snake(snake);
  bool paused = false;
  bool turbo = false;
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
    } else if(KEY_PRESSED(button_pause)){
      paused = !paused;
      delay(250);
    } else if(KEY_PRESSED(button_turbo)){
      turbo = true;
    }
    if(paused) {
      delay(100);
      turbo = false;
      next_move = curtime + game_speed;
    }
    if(curtime > next_move) {
      move_snake(snake);
      if(detect_colision(snake)) {
        game_over();
        delay(2000);
        creoqode.fillRect(1, 1, 60, 30, 0);
        print_points();
        while(true){
          if(KEY_PRESSED(button_up) || KEY_PRESSED(button_down) ||
             KEY_PRESSED(button_left) || KEY_PRESSED(button_right) ||
             KEY_PRESSED(button_turbo) || KEY_PRESSED(button_pause)){
            return;
          }
          delay(10);
        }
        break;
      }
      draw_snake(snake);
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
        put_food(GET_POS(1,1), GET_POS(62,14), snake);
      }
      next_move = millis() + (turbo ? TURBO_SPEED : game_speed);
      turbo = false;
    }
  }
}

void reset_snake(unsigned int snake[]) {
  game_speed = INITIAL_GAME_SPEED;
  snake_len = 2;
  points = 0;
  points_factor = 1;
  catches = 0;
  snake_direction = DIR_RIGHT;
  snake_next_dir = snake_direction;
  memset(snake, 0, sizeof(*snake));
  snake_old_tail = 0;
  snake[0] = GET_POS(31,15);
  snake[1] = GET_POS(32,15);
  creoqode.drawRect(0, 0, 64, 32, color_border);
  creoqode.fillRect(1, 1, 62, 30, 0);
}

void draw_snake(unsigned int snake[]) {
  if(snake_old_tail!=0) creoqode.drawPixel(GET_X(snake_old_tail), GET_Y(snake_old_tail), 0);
  creoqode.drawPixel(GET_X(snake[0]), GET_Y(snake[0]), color_snake_head);
  for(unsigned int i = 1; i < snake_len; i++){
    creoqode.drawPixel(GET_X(snake[i]), GET_Y(snake[i]), (i%2==0 ? color_snake_even : color_snake_odd));
  } 
}

void move_snake(unsigned int snake[]) {
  snake_direction = snake_next_dir;
  snake_old_tail = snake[snake_len-1];
  for(int i = snake_len -1; i>0; i--){
    snake[i] = snake[i-1];
  }
  snake[0] = snake[0] + snake_direction;
}

bool detect_colision(unsigned int snake[]) {
  if(GET_X(snake[0]) == 0 || GET_X(snake[0]) == 63) {
    return true;
  }
  if(GET_Y(snake[0])== 0 || GET_Y(snake[0])== 31) {
    return true;
  }
  for(unsigned int i=1; i<snake_len; i++){
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
  creoqode.setTextWrap(true);
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
  creoqode.setCursor(32-(text_width/2), 12);
  creoqode.setTextColor(color_score_points);
  creoqode.print(String(points_string));
  creoqode.setTextColor(color_score_title);
  creoqode.setCursor(points==1?18:14, 22);
  creoqode.print(points==1?"point":"points");

}

void put_food(int first, int last, unsigned int snake[]){
  unsigned int new_food;
  while(true){
    new_food = random(first, last+1);
    bool colision = false;
    for(unsigned int i = 0; i < snake_len; i++){
      if(new_food == snake[i]) {
        colision = true;
        break;
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
  for(unsigned int i = 0; i < sizeof(code); i++){
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

String enter_name() {
  unsigned long entry_time = millis();
  const long action_delay = 500;
  const int x = 8;
  const int y = 6;
  const int buff_width = 1;
  const int char_height = 6;
  const int before_lines = 6;
  const int after_lines = 7;
  const int max_letters = NAME_LEN;
  int letter_indexes[max_letters];
  int current_letter = 0;
  bool allow_commit = true;
  unsigned char name[max_letters+1];
  const char letters[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    ' ', '_', '.', '@', '!', '?', ':'
  };
  memset((int*) letter_indexes, -1, sizeof(letter_indexes));
  memset((unsigned char*) name, '\0', sizeof(name));
  unsigned int canvas_h = char_height*sizeof(letters);
  GFXcanvas1 canvas(8, canvas_h);
  canvas.setFont(&Font5x5Fixed);
  unsigned int additional_color = creoqode.Color444(0, 2, 0);
  unsigned int main_color = creoqode.Color444(4, 0, 2);
  unsigned int bg_color = creoqode.Color444(0, 0, 0);
  for (unsigned int i = 0; i < sizeof(letters); i++) {
    canvas.drawChar(1, 6*(i+1)-1, letters[i], additional_color, 0, 1);
  }

  uint8_t before_buffs[before_lines][buff_width];
  uint8_t after_buffs[after_lines][buff_width];
  uint8_t buff[buff_width * char_height];
  uint8_t* snap = canvas.getBuffer();

  bool left_btn_up = true;
  bool right_btn_up = true;

  int i = 0;
  int selected_index = 0;
  int letters_dir = 1;
  name[0] = letters[selected_index];
  letter_indexes[0] = selected_index;
  if (KEY_PRESSED(button_turbo)) allow_commit = false;
  creoqode.drawRect(x-1, y-1, max_letters*buff_width*8+2, before_lines+char_height+after_lines+2, creoqode.Color444(2, 2, 0));
  creoqode.fillRect(x,y,max_letters*buff_width*8, before_lines+char_height+after_lines, bg_color);
  while(true) {
    while (true) {
      for (unsigned int j = 0; j < sizeof(buff); j++ ) {
        buff[j] = snap[(i*buff_width+j)%(buff_width*canvas_h)];
      }
      for (int b = before_lines; b > 0; b--) {
        int index = i - b;
        if (index < 0) {
        index = index + canvas_h;
        }
        for (unsigned int j = 0; j < sizeof(before_buffs[b]); j++ ) {
          before_buffs[before_lines - b][j] = snap[(index*buff_width+j)%(buff_width*canvas_h)];
        }
      }

      for (int a = 0; a < after_lines; a++) {
        unsigned int index = i + char_height + a;
        if (index >= canvas_h) {
          index = index - canvas_h;
        }
        for (unsigned int j = 0; j < sizeof(after_buffs[a]); j++ ) {
          after_buffs[a][j] = snap[index*buff_width+j];
        }
      }

      for (int i = 0; i < before_lines; i++) {
        creoqode.drawBitmap(x + (current_letter*8), y+i, before_buffs[i], 8, 1, additional_color, bg_color);
      }
      creoqode.drawBitmap(x + (current_letter*8), y+before_lines, buff, 8, char_height, main_color, bg_color);
      for(int i = 0; i < after_lines; i++) {
        creoqode.drawBitmap(x + (current_letter*8), y+i + before_lines + char_height, after_buffs[i], 8, 1, additional_color, bg_color);
      }
      if (i == selected_index * char_height) {
        break;
      }
      i = i + letters_dir;
      if (i < 0) {
        i = canvas_h -1;
      }
      i = (i % canvas_h);
      delay(20);
    }
    while(true) {
      if (millis() - entry_time < action_delay) continue;
      if(!allow_commit && KEY_NOT_PRESSED(button_turbo)) {
        allow_commit = true;
        delay(100);
      }
      if(!left_btn_up && KEY_NOT_PRESSED(button_left)) {
        left_btn_up = true;
        delay(15);
      }
      if(!right_btn_up && KEY_NOT_PRESSED(button_right)) {
        right_btn_up = true;
        delay(15);
      }
      if (KEY_PRESSED(button_up)) {
        selected_index -= 1;
        if (selected_index < 0) selected_index = sizeof(letters) - 1;
        letters_dir = -1;
        letter_indexes[current_letter] = selected_index;
        name[current_letter] = letters[selected_index];
        break;
      } else if (KEY_PRESSED(button_down)) {
        selected_index = (selected_index + 1) % sizeof(letters);
        letters_dir = 1;
        letter_indexes[current_letter] = selected_index;
        name[current_letter] = letters[selected_index];
        break;
      } else if (right_btn_up && KEY_PRESSED(button_right)) {
        if (current_letter < max_letters-1) {
          right_btn_up = false;
          letter_indexes[current_letter] = selected_index;
          name[current_letter] = letters[selected_index];
          creoqode.fillRect(x+(current_letter*buff_width*8),y,buff_width*8, before_lines, bg_color);
          creoqode.fillRect(x+(current_letter*buff_width*8),y+before_lines+char_height,buff_width*8, after_lines, bg_color);
          current_letter += 1;
          selected_index = 0;
          letter_indexes[current_letter] = selected_index;
          name[current_letter] = letters[selected_index];
          i = 0;
          break;
        }
      } else if (left_btn_up && KEY_PRESSED(button_left)) {
        if (current_letter > 0) {
          left_btn_up = false;
          selected_index = letter_indexes[current_letter-1];
          i = selected_index * char_height;
          creoqode.fillRect(x+(current_letter*buff_width*8),y,buff_width*8, before_lines+char_height+after_lines, bg_color);
          letter_indexes[current_letter] = -1;
          name[current_letter] = '\0';
          current_letter -= 1;
          break;
        }
      } else if (allow_commit && KEY_PRESSED(button_turbo)) {
        creoqode.fillRect(x,y,(current_letter+1)*buff_width*8, before_lines+char_height+after_lines, bg_color);
        return String((const char*)name);
      }
    }
  }
  return "ERROR";  
}

int compare_points(const void * a, const void *b) {
  uint16_t pa = ((highscore_entry *) a)->points;
  uint16_t pb = ((highscore_entry *) b)->points;
  return pa > pb ? -1 : (pa < pb ? 1 : 0);
}

void show_high_scores(highscores &scores_table) {
  const unsigned int score_line_height = 7;

  highscore_entry entries[NUM_HI_SCORES];
  unsigned int found_scores = 0;

  for (int i = 0; i < NUM_HI_SCORES; i++) {
    if (scores_table.scores[i].name[0] != '\0') {
      entries[found_scores] = scores_table.scores[i];
      found_scores += 1;
    }
  }
  creoqode.fillRect(0, 0, 64, 32, 0);
  if (found_scores == 0) {
    creoqode.setFont(&Picopixel);
    creoqode.setTextColor(creoqode.Color444(1,3,2));
    creoqode.setTextSize(1);
    creoqode.setCursor(5, 8);
    creoqode.println("No High Scores\n\nPlay some games");
    while ((true)) {
      if (KEY_PRESSED(button_turbo) || KEY_PRESSED(button_pause)) {
          break;
      }
    }
    
  } else {
    qsort(entries, NUM_HI_SCORES, sizeof(entries[0]), compare_points);
    unsigned int offset = 0;
    bool do_score_loop = true;
    unsigned long latch_durarion = 750;
    unsigned long up_latch = 0;
    unsigned long down_latch = 0;
    uint16_t pos_color = creoqode.Color444(2, 2, 0);
    uint16_t name_color = creoqode.Color444(0, 2, 0);
    uint16_t points_color = creoqode.Color444(2, 0, 2);
    while (do_score_loop) {
      unsigned int page_index = 0;
      creoqode.fillRect(0, 0, 64, 32, 0);
      for (unsigned int i = offset; i < found_scores; i++) {
        unsigned int base_line = page_index*score_line_height;
        char name_buff[NAME_LEN+1];
        memset(name_buff, '\0', sizeof(name_buff));
        memcpy(name_buff, entries[i].name, NAME_LEN);
        creoqode.setTextSize(1);
        creoqode.setTextWrap(false);
        // place
        unsigned int place = i+1;
        creoqode.setFont(&Font2x5FixedMonoNum);
        creoqode.setTextColor(pos_color);
        creoqode.setCursor(place < 10 ? 3 : 0, 6+base_line);
        creoqode.print(i+1);

        // name
        creoqode.setFont(&Font5x5Fixed);
        creoqode.setTextColor(name_color);
        creoqode.setCursor(7,5+base_line);
        creoqode.print((const char*) name_buff);

        // points
        unsigned int row_points = entries[i].points;
        unsigned int points_margin = 0;
        if (row_points < 10) {
          points_margin = 4*4;
        } else if (row_points < 100) {
          points_margin = 3*4;
        } else if (row_points < 1000) {
          points_margin = 2*4;
        } else if (row_points < 10000) {
          points_margin = 4;
        }
        creoqode.setFont(&Font3x5FixedNum);
        creoqode.setCursor(45 + points_margin,6+base_line);
        creoqode.setTextColor(points_color);
        creoqode.print(row_points); 
        page_index += 1;
        if (page_index > 4) {
          break;
        }
      }
      while (true) {
        if (down_latch > 0 && KEY_NOT_PRESSED(button_down)) {
          down_latch = 0;
          delay(20);
        }
        if (up_latch > 0 && KEY_NOT_PRESSED(button_up)) {
          up_latch = 0;
          delay(20);
        }
        if ((found_scores > 4 && offset < found_scores - 4) && KEY_PRESSED(button_down)) {
          if (down_latch == 0 || (millis() - down_latch) > latch_durarion) {
            down_latch = millis();  
            offset += 1;
            break;
          }
        }
        if ((offset > 0) && KEY_PRESSED(button_up)) {
          if (up_latch == 0 || (millis() - up_latch) > latch_durarion) {
            up_latch = millis();
            offset -= 1;
            break;
          }
        }
        if (KEY_PRESSED(button_turbo) || KEY_PRESSED(button_pause)) {
          do_score_loop = false;
          break;
        }
      }
    }
  }
  creoqode.setFont();
}

void register_high_score(String name, uint16_t points, highscores &highscores_table) {
  highscore_entry* lowest_entry = nullptr;
  for (unsigned int i = 0; i < NUM_HI_SCORES; i++) {
    uint16_t current_points = highscores_table.scores[i].points;
    if (current_points == 0) {
      lowest_entry = &highscores_table.scores[i];
      break;
    }
    if (current_points < points && (lowest_entry == NULL || lowest_entry->points < current_points)) {
      lowest_entry = &highscores_table.scores[i];
    }
  }
  if (lowest_entry != nullptr) {
    memcpy(lowest_entry->name, name.c_str(), NAME_LEN);
    lowest_entry->points = points;
  }
}

bool is_high_score_eligable(uint16_t current_points, highscores &highscores_table) {
  if (current_points == 0) {
    return false;
  }
  for (unsigned int i = 0; i < NUM_HI_SCORES; i++) {
      if (current_points > highscores_table.scores[i].points) return true;
  }
  return false;
}
