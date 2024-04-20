#include <SPI.h>

const int CLOCK_PIN = 52;
const int DATA_PIN = 51;
const int LATCH_PIN = A0; // on port f??
const int OE = 4; // Activw LOW

const int CLOCK_XY_SWITCH = 5;
const int CLOCK_YZ_SWITCH = 6;
const int CLOCK_ZX_SWITCH = 7;

const int ANTI_CLOCK_XY_SWITCH = 8;
const int ANTI_CLOCK_YZ_SWITCH = 9;
const int ANTI_CLOCK_ZX_SWITCH = 10;

const int MOVE_X = A2;
const int MOVE_Y = A3;
// const int MOVE_Z = 4;

unsigned long last_move_x = 0;
unsigned long last_move_y = 0;
unsigned long clock_rotate_x_y = 0;

const int drop_interval = 2000;
unsigned long previousMillis = 0;  // will store last time LED was updated
unsigned long last_show = 0;

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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(2000000);

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

  SPI.begin();

  generate_new_block();

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

void pos_global_blocks() {
  for (int i = 0; i < 8; i++) {
    Serial.print(global_current_blocks[i].x);
    Serial.print(global_current_blocks[i].y);
    Serial.println(global_current_blocks[i].z);
  }
}

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
        breaking_rows();
        generate_new_block();
      }
      Serial.println("Failed");
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
  
}

void breaking_rows() {
  // Serial.println("Time to check for breaking rows");

  // check game matrix
  bool break_rows[10];
  for (int i = 0; i < 10; i++) {
    bool break_row = true;
    for (int j = 0; i < 4; j++) {
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

  // 
}


void generate_new_block() {
    // int randNumber = random(4);


    if (try_rand_generate == 5) {
      while(true) {
        send_to_shift_reg(); // Program end
      }
      return;
    }

    int randNumber = 2;
    
    Point start_index = {1, 1, 1}; // I want this block to be randomly translated

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

    for (int i = 0; i < 8; i++) {
      global_current_blocks[i] = new_blocks[i];
    }

    draw_tetromino();
    print_game_matrix();

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
  // // Okkk, I have game matrix and current anode level
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

/*
  def rotate_x_y(self):
    pivot_pos = self.current_blocks[0] # The pivot would stay at the same position, but everything around it will move
    new_blocks = []
    for (i, j, k) in self.current_blocks:
      translated = (pivot_pos[0] - i, pivot_pos[1] - j, pivot_pos[2] - k)
      # Now if I swap x and y
      translated = (translated[1] + pivot_pos[0], translated[0] + pivot_pos[1], translated[2] + pivot_pos[2])

      ## IF it is equal to its current position all good
      if translated == (i, j, k):
        new_blocks.append(translated)
        continue

      if not self.is_valid_pos(translated[0], translated[1], translated[2]):
        return
      new_blocks.append(translated)

    self.erase_tetromino()
    self.current_blocks = new_blocks
    self.draw_tetromino()
*/
int gamma = 1;

void rotate_x_y() {
  // Point pivot_pos = global_current_blocks[0]; // pivot would remain at the same position, everything around it will move

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

    Point translated = {global_current_blocks[i].x - pivot_pos_x, global_current_blocks[i].y - pivot_pos_y,  global_current_blocks[i].z - pivot_pos_z};

    // Now swap x and y
    translated = {translated.y + pivot_pos_x, translated.x + pivot_pos_y, translated.z + pivot_pos_z};

    // if this is equal to corrent position, all good
    if(translated.x == global_current_blocks[i].x && translated.y == global_current_blocks[i].y && translated.z == global_current_blocks[i].z) {
      new_blocks[i] = translated;
      continue;
    } else {
      rotated = true;
    }

    // else validate
    if (!is_valid_point(translated)) {
      Serial.print("Cannot rotate because ");
      Serial.print(global_current_blocks[i].x);
      Serial.print(global_current_blocks[i].y);
      Serial.print(global_current_blocks[i].z);
      Serial.print(" WHich is converted to ");
      Serial.print(translated.x);
      Serial.print(translated.y);
      Serial.println(translated.z);
      return;
    }

    new_blocks[i] = translated;
  }

  // Serial.println("Before Rotation:");
  // print_game_matrix();

  if (rotated) {
    Serial.println("Rotated finished");
    erase_tetromino();

    for (int i = 0; i < 8; i++) {
      global_current_blocks[i] = new_blocks[i];
    }
   
    draw_tetromino();
  } else {
    Serial.println("Never rotated");
  }
  // Serial.println("After Rotation:");
  // print_game_matrix();

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



void print_game_matrix() {
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 4; j++) {
      for (int k = 0; k < 4; k++) {
        Serial.print(game_matrix[i][j][k]);
      }
    }
    Serial.println("");
    Serial.println("--------");
  }
}

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
  if (anode_level == 10) {
    color += 1;
    color %= 3;
  }
  anode_level %= 10;

  PORTF |= B00000001; // Latch pin HIGH
  PORTF &= ~(1 << 0); // Latch pin LOW  

  // digitalWrite(OE, HIGH);

}

int beta = 0;

void user_input() {

    unsigned long current_millis = millis();

  //   if (analogRead(MOVE_X) == 0 && (current_millis - last_move_x >= 200)) {
  //   move({-1, 0, 0});
  //   last_move_x = current_millis;
  // } else if (analogRead(MOVE_X) == 1023 && (current_millis - last_move_x >= 200)) {
  //   move({1, 0, 0});
  //   last_move_x = current_millis;
  // }

  // if (analogRead(MOVE_Y) < 200 && (current_millis - last_move_y >= 200)) {
  //   move({0, 1, 0});
  //   last_move_y = current_millis;
  // } else if (analogRead(MOVE_Y) > 900 && (current_millis - last_move_y >= 200)) {
  //   move({0, -1, 0});
  //   last_move_y = current_millis;
  // }

  if (digitalRead(CLOCK_XY_SWITCH) == 1 && (current_millis - clock_rotate_x_y >= 2000)) {
    // Serial.println("Rotating");
    // previousMillis = current_millis;
    trigger = true;
    clock_rotate_x_y = current_millis;
    Serial.println("ROtating");
    rotate_x_y();
  }

}

void loop() {

  unsigned long current_millis = millis();
  send_to_shift_reg();

  // if (current_millis - last_show >= 50) {
  //   send_to_shift_reg();
  //   last_show = current_millis;
  // }

  user_input();

  if (current_millis - previousMillis >= drop_interval) {
    move({0, 0, 1});
    pos_global_blocks();
    previousMillis = current_millis;
  }

}
  