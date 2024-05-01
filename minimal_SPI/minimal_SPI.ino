#include <SPI.h>

const int CLOCK_PIN = 52;
const int DATA_PIN = 51;
const int LATCH_PIN = A0; // on port f??
const int OE = 4; // Activw LOW

volatile bool game_end = true;

const int CLOCK_XY_SWITCH = A10;
const int CLOCK_YZ_SWITCH = A7;
const int CLOCK_ZX_SWITCH = A6;

const int ANTI_CLOCK_XY_SWITCH = A9;
const int ANTI_CLOCK_YZ_SWITCH = A8;
const int ANTI_CLOCK_ZX_SWITCH = A6;

const int MOVE_X = A3;
const int MOVE_Y = A2;
// const int MOVE_Z = 4;

unsigned long last_move_x = 0;
unsigned long last_move_y = 0;
unsigned long clock_rotate_x_y = 0;

const int drop_interval = 2000;
unsigned long previousMillis = 0;  // will store last time LED was updated
unsigned long last_show = 0;
unsigned long userInterval = 700; // refresh rate for user input
unsigned long previousUserInput = 0;
int try_rand_generate = 0;
int color = 0;

bool landed = false;

struct Point {
  int x;
  int y;
  int z;
};

int alpha = 1;
int trigger = false;

const struct Point tetromino[][8] = {
  {{0, 0, 0}, {0, 0, 1}, {0, 0, 2}},
  {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0}, {0, 0, 1}, {1, 0, 1}, {0, 1, 1}, {1, 1, 1}},
  {{0, 0, 0}, {-1, 0, 0}, {0, 0, 1}, {0, 0, 2}}, // L
  {{0, 0, 0}, {-1, 0, 0}, {0, 0, 1}, {1, 0, 1}} // 
};

volatile int game_matrix[10][4][4] = {};


const int display_segment[] = { // These are active HIGH
 3,
7,
11,
9,
8,
26,
12
};

// const int display_digit[] = { // These are active LOW
//   28,
// 5,
// 6,
// 13
// };

const int display_digit[] = {
  13,
  6,
  5,
  28,
};

int digit_show = 0;
unsigned long last_time_digit = 0;

// const byte numbers[10] =   // Describe each digit in terms of display segments  0, 1, 2, 3, 4, 5, 6, 7, 8, 9
// {
//   B11111100, //0
//   B01100000, //1
//   B11011010, //2
//   B11110010, //3
//   B01100110, //4
//   B10110110, //5
//   B10111110, //6
//   B11100000, //7
//   B11111111, //8
//   B11110110, //9
// };

volatile Point global_current_blocks[8] = {
  {-1, -1, -1},
  {-1, -1, -1},
  {-1, -1, -1},
  {-1, -1, -1},
  {-1, -1, -1},
  {-1, -1, -1},
  {-1, -1, -1},
  {-1, -1, -1},
};

int anode_level = 0;

// const int DIGIT_ONE = A4;
// const int SEGMENT_A = A5;
// const int SEGMENT_F = A6;
// const int DIGIT_TWO = A7;
// const int DIGIT_THREE = A8;
// const int SEGMENT_B = A9;
// const int DIGIT_FOUR = A10;
// const int SEGMENT_G = A11;
// const int SEGMENT_C = A12;
// // A13 is decimal
// const int SEGMENT_D= A14;
// const int SEGMENT_E = A15;

const int numbers[10][8] = {
  {1, 1, 1, 1, 1, 1, 0}, 
  {0, 1, 1, 0, 0, 0, 0},
  {1, 1, 0, 1, 1, 0, 1},
  {1, 1, 1, 1, 0, 0, 1},
  {0, 1, 1, 0, 0, 1, 1},
  {1, 0, 1, 1, 0, 1, 1},
  {1, 0, 1, 1, 1, 1, 1},
  {1, 1, 1, 0, 0, 0, 0},
  {1, 1, 1, 1, 1, 1, 1},
  {1, 1, 1, 1, 0, 1, 1}
};

// const int segment_out[] = {
//   SEGMENT_A, SEGMENT_B, SEGMENT_C, SEGMENT_D, SEGMENT_E, SEGMENT_F, SEGMENT_G
// };

// const int segment_digit[] = {
//   DIGIT_ONE, DIGIT_TWO, DIGIT_THREE, DIGIT_FOUR
// };

int score = 0;
int segment_digit_counter = 0; 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("started");

//     /** Testing whether the LEDs are properly connected **/
//   for (int i = 0; i < 10; i++) {
//     for (int j = 0; j < 4; j++) {
//       for (int k = 0; k < 4; k++) {
//         game_matrix[i][j][k] = 1;
//       }
//     }
// }

  randomSeed(analogRead(0)); // don't put anything in pin0... Ensures randomness


  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV128);
  pinMode(CLOCK_XY_SWITCH, INPUT);
  pinMode(CLOCK_YZ_SWITCH, INPUT);
  pinMode(CLOCK_ZX_SWITCH, INPUT);
  // pinMode(OE, OUTPUT);
  digitalWrite(OE, HIGH);

  pinMode(ANTI_CLOCK_XY_SWITCH, INPUT);
  pinMode(ANTI_CLOCK_YZ_SWITCH, INPUT);
  pinMode(ANTI_CLOCK_ZX_SWITCH, INPUT);

  pinMode(LATCH_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);

  pinMode(MOVE_X, INPUT);
  pinMode(MOVE_Y, INPUT);
  // pinMode(MOVE_Z, INPUT);

  digitalWrite(LATCH_PIN, LOW);

  for (int i = 0; i < 8; i++) {global_current_blocks[i] = {-1, -1, -1};}

  for (int i = 0; i < 7; i++) {
    pinMode(display_segment[i], OUTPUT);
    digitalWrite(display_segment[i], LOW); // everything is on
  }


  for (int i = 0; i < 4; i++) {
    pinMode(display_digit[i], OUTPUT);
    digitalWrite(display_digit[i], HIGH); // everything is off
  }

  SPI.begin();

  // generate_new_block();

}

bool is_valid_point(Point p) {
  return (p.x >= 0 && p.x < 4 && p.y >= 0 && p.y < 4 && p.z >= 0 && p.z < 10 && game_matrix[p.z][p.y][p.x] != 1);
}

void draw_tetromino(){
  for (int i = 0; i < 8; i++) {
    if (global_current_blocks[i].x != -1) {
      game_matrix[global_current_blocks[i].z][global_current_blocks[i].y][global_current_blocks[i].x] = 1;
    }
  }
}

// void pos_global_blocks() {
//   for (int i = 0; i < 8; i++) {
//     Serial.print(global_current_blocks[i].x);
//     Serial.print(global_current_blocks[i].y);
//     Serial.println(global_current_blocks[i].z);
//   }
// }

void erase_tetromino() {
  for (int i = 0; i < 8; i++) {
    if (global_current_blocks[i].x != -1) {
      game_matrix[global_current_blocks[i].z][global_current_blocks[i].y][global_current_blocks[i].x] = 0;
    }
  }
}

void move(Point direction) {
  Point new_block_location[8];
  for (int i = 0; i < 8; i++) {
    if (global_current_blocks[i].x == -1 && global_current_blocks[i].y == -1 && global_current_blocks[i].z == -1) {
      new_block_location[i] = {-1, -1, -1};
    }

    Point pos_position = {global_current_blocks[i].x + direction.x, global_current_blocks[i].y + direction.y, global_current_blocks[i].z + direction.z};
    // if the pos_position is colliding with itself its not an issue.... 
    bool possibly_invalid = true;
    for (int j = 0; j < 8; j++) {
      if (pos_position.x == global_current_blocks[j].x && pos_position.y == global_current_blocks[j].y && pos_position.z == global_current_blocks[j].z) {
        possibly_invalid = false;
        break;
      }
    }

    if (possibly_invalid && !is_valid_point(pos_position)) {
      if (direction.z == 1) {
        landed = true;
        score += 1;
        breaking_rows();
        generate_new_block();
      }
      // Serial.println("Failed");
      return;
    }
    new_block_location[i] = pos_position;
  }

  erase_tetromino();

  for (int i = 0; i < 8; i++) {
    global_current_blocks[i] = new_block_location[i];
  }

  draw_tetromino();
}

void remove_row(int row_number) {
  // 0 is the top most row, 9 the bottommost
  for (int i = row_number; i > 0; i--) {
    for (int j = 0; j < 4; j++) {
      for (int k = 0; k < 4; k++) {
        game_matrix[i][j][k] = game_matrix[i - 1][j][k]; // copying the row above it
      }
    }
  }

  for (int j = 0; j < 4; j++) {
     for (int k = 0; k < 4; k++) {
      game_matrix[0][j][k] = 0;
    } 
  }

}

void breaking_rows() {
  // Serial.println("Time to check for breaking rows");

  // check game matrix
  bool break_rows[10];
  for (int i = 0; i < 10; i++) {
    bool break_row = true;
    for (int j = 0; j < 4; j++) {
      for (int k = 0; k < 4; k++) {
        if (game_matrix[i][j][k] == 0) {
          break_row = false;
          break;
        }
      }
      if (!break_row) {
        break;
      }
    }
    break_rows[i] = break_row;
  }

  int deleted_yet = 0;
  for (int j = 9; j >= 0; j--) {
    if (break_rows[j]) {
      score += 10;
      remove_row(j + deleted_yet);
      deleted_yet += 1;
    }
  }
}




void generate_new_block() {
    // int randNumber = random(4);
    int randNumber = 3;

    if (try_rand_generate == 1000) {

      for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 4; j++) {
          for (int k = 0; k < 4; k++) {
            game_matrix[i][j][k] = 0;
          }
        }
      }

      for (int i = 0; i < 8; i++) {
        global_current_blocks[i] = {-1, -1, -1};
      }
      
      game_end = true;
      return;
    }

    // int randNumber = 0;
    
    // Point start_index = {1, 1, 1}; // I want this block to be randomly translated
    Point start_index = {random(4), random(4), 0};

    // Point start_index = {1, 1, 1};

    Point new_blocks[8] = {};
    for (int i = 0; i < sizeof(tetromino[randNumber]) / sizeof(struct Point); i++) { // TODO: Check the i less than condition
      Point new_block;
      new_block.x = start_index.x + tetromino[randNumber][i].x;
      new_block.y = start_index.y + tetromino[randNumber][i].y;
      new_block.z = start_index.z + tetromino[randNumber][i].z;
      if (!is_valid_point(new_block)) {
        try_rand_generate += 1;
        generate_new_block(); // keep trying until you finally get something
        return;
      }
      new_blocks[i] = new_block;
    }

    try_rand_generate = 0;

    for (int i = 0; i < 8; i++) {
      global_current_blocks[i] = new_blocks[i];
    }

    draw_tetromino();
    // print_game_matrix();

    return;
}

uint8_t swap_5_and_6(uint8_t number) {
  int mask5 = 0b00010000; 
  int mask6 = 0b00100000; 

  int bit5 = number & mask5;
  int bit6 = number & mask6;

  number &= ~(mask5 | mask6);

  number |= (bit5 << 1);
  number |= (bit6 >> 1);

  return number;
}

// int cathode_pillar = 0;

void single_color_cathode_transfer() {
  // Okkk, I have game matrix and current anode level
  int y_x_to_on[16];
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      y_x_to_on[4*i + j] = game_matrix[anode_level][i][j];
    }
  }

  // // Finally we can get two 8 bit integer from this to send SPI.
  uint8_t cathode2 = 0;
  for (int i = 15; i >= 8; i--) {
    cathode2 = cathode2 << 1;
    cathode2 = cathode2 | y_x_to_on[i];
  }

  uint8_t cathode1 = 0;
  for (int i = 7; i >= 0; i--) {
    cathode1 = cathode1 << 1;
    cathode1 = cathode1 | y_x_to_on[i];
  }

  cathode1 = swap_5_and_6(cathode1);
  
  // turning the Pillars one after one would be much beneficial.. Test of persistence of vision.. Did not work :(

  SPI.transfer(~cathode2);
  SPI.transfer(~cathode1);
  

}

// red cathode 5 and 6 are swapped.... HMMMM

int gamma = 1;

// void clock_rotate_x_y_func() {
//   // Point pivot_pos = global_current_blocks[0]; // pivot would remain at the same position, everything around it will move

//   int pivot_pos_x = global_current_blocks[0].x;
//   int pivot_pos_y = global_current_blocks[0].y;
//   int pivot_pos_z = global_current_blocks[0].z;

//   Point new_blocks[8];

//   bool rotated = false;

//   for (int i = 0; i < 8; i++) {
//     if (global_current_blocks[i].x == -1) {
//       new_blocks[i] = {-1, -1, -1};
//       continue;
//     }

//     Point translated = {global_current_blocks[i].x - pivot_pos_x, global_current_blocks[i].y - pivot_pos_y,  global_current_blocks[i].z - pivot_pos_z};

//     // Now swap x and y
//     translated = {translated.y, -translated.x, translated.z};

//     // translated = (translated[0] + pivot_pos[0], translated[1] + pivot_pos[1], translated[2] + pivot_pos[2])
//     translated = {translated.x + pivot_pos_x, translated.y + pivot_pos_y, translated.z + pivot_pos_z};

//     // if this is equal to corrent position, all good
//     if(translated.x == global_current_blocks[i].x && translated.y == global_current_blocks[i].y && translated.z == global_current_blocks[i].z) {
//       new_blocks[i] = translated;
//       continue;
//     } else {
//       rotated = true;
//     }

//     // else validate
//     if (!is_valid_point(translated)) {
//       return;
//     }

//     new_blocks[i] = translated;
//   }

//   // Serial.println("Before Rotation:");
//   // print_game_matrix();

//   if (rotated) {
//     // Serial.println("Rotated finished");
//     erase_tetromino();

//     for (int i = 0; i < 8; i++) {
//       global_current_blocks[i] = new_blocks[i];
//     }
   
//     draw_tetromino();
//   } 
//   // Serial.println("After Rotation:");
//   // print_game_matrix();

// }


Point rotate_point(int pivot_x, int pivot_y, int pivot_z, int x, int y, int z, int axis, bool clock) {
  Point translated = {x - pivot_x, y - pivot_y, z - pivot_z};

  if (axis == 0) {  // Rotate in x-y plane
    if (clock) {
      translated = {translated.y, -translated.x, translated.z};
    } else {
      translated = {-translated.y, translated.x, translated.z};
    }

  } else if (axis == 1) {  // Rotate in y-z plane
    if (clock) {
      translated = {translated.x, translated.z, -translated.y};
    } else {
      translated = {translated.x, -translated.z, translated.y};
    }
  } else if (axis == 2) {  // Rotate in z-x plane
    if (clock) {
      translated = {translated.z, translated.y, -translated.x};
    } else {
      translated = {translated.z, -translated.y, translated.x};
    }
  }

  translated = {translated.x + pivot_x, translated.y + pivot_y, translated.z + pivot_z};
  return translated;
}

void rotate(int axis, bool clock) {
  int pivot_pos_x = global_current_blocks[0].x;
  int pivot_pos_y = global_current_blocks[0].y;
  int pivot_pos_z = global_current_blocks[0].z;

  Point new_blocks[8];
  bool rotated = false;

  for (int i = 0; i < 8; i++) {
    if (global_current_blocks[i].x == -1) {
      new_blocks[i] = {-1, -1, -1};
      continue;
    }

    Point translated = rotate_point(pivot_pos_x, pivot_pos_y, pivot_pos_z, global_current_blocks[i].x, global_current_blocks[i].y, global_current_blocks[i].z, axis, clock);

    bool trans_copy = false;

    for (int i = 0; i < 8; i++) {
      if (translated.x == global_current_blocks[i].x && translated.y == global_current_blocks[i].y && translated.z == global_current_blocks[i].z) {
        trans_copy = true;
        break;
      }
    }

    if (!trans_copy) {
      // Serial.println("New rotation");
      rotated = true;
    }



    // if (translated.x == global_current_blocks[i].x && translated.y == global_current_blocks[i].y && translated.z == global_current_blocks[i].z) {
    //   new_blocks[i] = translated;
    //   continue;
    // } else {
    //   rotated = true;
    // }

    if (!trans_copy && !is_valid_point(translated)) {
      // Serial.println("Failed rotating");
      return;
    }

    new_blocks[i] = translated;
  }

  if (rotated) {
    erase_tetromino();

    for (int i = 0; i < 8; i++) {
      global_current_blocks[i] = new_blocks[i];
    }

    draw_tetromino();
  }
}



void anode_spi_transfer() {

  switch (anode_level) {
    case 0:
    SPI.transfer(~0);
    SPI.transfer(~1);
    break;
    case 1:
    SPI.transfer(~0);
    SPI.transfer(~2);
    break;
    case 2:
    SPI.transfer(~0);
    SPI.transfer(~4);
    break;
    case 3:
    SPI.transfer(~0);
    SPI.transfer(~8);
    break;
    case 4:
    SPI.transfer(~0);
    SPI.transfer(~16);
    break;
    case 5:
    SPI.transfer(~0);
    SPI.transfer(~32);
    break;
    case 6:
    SPI.transfer(~0);
    SPI.transfer(~64);
    break;
    case 7:
    SPI.transfer(~0);
    SPI.transfer(~128);
    break;
    case 8:
    SPI.transfer(~1);
    SPI.transfer(~0);
    break;
    case 9:
    SPI.transfer(~2);
    SPI.transfer(~0);
    break;

  }

  // if (anode_level <= 8) {
  //   SPI.transfer(~0);
  //   SPI.transfer(~(2 << anode_level));
  // } else {
  //   SPI.transfer(~(2 << (anode_level - 8)));
  //   SPI.transfer(~0);
  // }
}



// void print_game_matrix() {
//   for (int i = 0; i < 10; i++) {
//     for (int j = 0; j < 4; j++) {
//       for (int k = 0; k < 4; k++) {
//         Serial.print(game_matrix[i][j][k]);
//       }
//     }
//     Serial.println("");
//     Serial.println("--------");
//   }
// }

void send_to_shift_reg() {

  // digitalWrite(OE, LOW);
  anode_spi_transfer();

  if (color == 2) {
    // blue
    SPI.transfer(255);
    SPI.transfer(255);
  } else {
    SPI.transfer(255);
    SPI.transfer(255);
  }

  if (color == 1) {
      // green
    SPI.transfer(8);
    SPI.transfer(255);
  } else {
    SPI.transfer(255);
    SPI.transfer(255);
  }

  if (color == 0) {
    // red
    single_color_cathode_transfer();
  } else {
    SPI.transfer(255);
    SPI.transfer(255);   
  }

  anode_level += 1;
  // if (anode_level == 10) {
  //   color += 1;
  //   color %= 3;
  // }
  anode_level %= 10;

  PORTF |= B00000001; // Latch pin HIGH
  PORTF &= ~(1 << 0); // Latch pin LOW  

  // digitalWrite(OE, HIGH);

}

int beta = 0;

void user_input() {

  if (!game_end) {

    if (analogRead(MOVE_X) < 200) {
        move({1, 0, 0});
      previousUserInput = millis();
    } else if (analogRead(MOVE_X) > 900) {
      move({-1, 0, 0});
      previousUserInput = millis();
    }

    if (analogRead(MOVE_Y) < 200) {
      move({0, 1, 0});
      previousUserInput = millis();
    } else if (analogRead(MOVE_Y) > 900) {
      move({0, -1, 0});
      previousUserInput = millis();
    }

    // if (digitalRead(CLOCK_XY_SWITCH) == 1 && (current_millis - clock_rotate_x_y >= 2000)) {
    //   // Serial.println("Rotating");
    //   // previousMillis = current_millis;
    //   trigger = true;
    //   clock_rotate_x_y = current_millis;
    //   // Serial.println("ROtating");
    //   rotate(0, true);
    // }

    if (digitalRead(CLOCK_XY_SWITCH)) {
      // Serial.println("Detected green button");
      rotate(0, true);
      previousUserInput = millis();
    } else if (digitalRead(CLOCK_YZ_SWITCH)) {
      // Serial.println("Detected red button");
      rotate(1, true);
      previousUserInput = millis();
    } else if (digitalRead(CLOCK_ZX_SWITCH)) {
      // Serial.println("Tried rotating xz");
      // Serial.println("Detected white button");
      rotate(2, true);
      previousUserInput = millis();
    } else if (digitalRead(ANTI_CLOCK_XY_SWITCH)) {
      rotate(0, false);
      previousUserInput = millis();
    } else if (digitalRead(ANTI_CLOCK_YZ_SWITCH)) {
      rotate(1, false);
      previousUserInput = millis();
    } else if (digitalRead(ANTI_CLOCK_ZX_SWITCH)) {
      rotate(2, false);
      previousUserInput = millis();
    }
  } else {
      if (digitalRead(CLOCK_XY_SWITCH)) {
      game_end = false;
      score = 0;
      Serial.println("Switch pressed");
      generate_new_block();
    }
  }

}


void show_number(int num, int digit) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(display_digit[i], HIGH); // everything is deactivated
  }
  digitalWrite(display_digit[digit], LOW);
  for (int i = 0; i < 7; i++) {
    digitalWrite(display_segment[i], numbers[num][i]);
  } 
}

// void show_score(int number) {
//   // digitalWrite(segment_digit[(segment_digit_counter+1)%4], HIGH); // So remove the previous digit
//   // ((score / (10 ** segment_digit_counter)) % 10)
//   int ten_pow = 1;
//   // unsigned long int prev_score = 0;
//   for (int i = 0; i < 4; i++) {
//     show_number((number / ten_pow) % 10, i);
//     // delay(2); // 2 milliseconds :(
//     ten_pow *= 10;
//   }
// }

int ten_pows[4] = {1, 10, 100, 1000};

void loop() {

  unsigned long current_millis = millis();
  
  send_to_shift_reg();
  
  show_number((score / ten_pows[digit_show]) % 10, digit_show);
  digit_show += 1;
  digit_show %= 4;

  if (current_millis - previousUserInput >= userInterval) {
    user_input();    
  }

  if (!game_end) {
    if (current_millis - previousMillis >= drop_interval) {
      move({0, 0, 1});
      // pos_global_blocks();
      previousMillis = current_millis;
    }
  }

}
  