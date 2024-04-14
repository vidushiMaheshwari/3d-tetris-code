#include <SPI.h>

const int CLOCK_PIN = 52;
const int DATA_PIN = 51;

const int CLOCK_XY_SWITCH = 5; // 2560 can only handle 2, 3, 18, 19, 20, 21
const int CLOCK_YZ_SWITCH = 6;
const int CLOCK_XZ_SWITCH = 7;

const int LATCH_PIN = A0; // on port f??

volatile uint8_t red1[10] = {
  255,
  255,
  255,
  255,
  255,
  255,
  255,
  255,
  255,
  255,
};

int red2[10] = {
  255,
  255,
  255,
  255,
  255,
  255,
  255,
  255,
  255,
  255,
};


struct Point {
  int x;
  int y;
  int z;
};

int current_drop_level = 0;
uint8_t drop_value = 254;
const int drop_interval = 2000;
unsigned long previousMillis = 0;  // will store last time LED was updated

const struct Point tetromino[][8] = {
  {{0, 0, 0}, {0, 0, 1}, {0, 0, 2}},
  {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0}, {0, 0, 1}, {1, 0, 1}, {0, 1, 1}, {1, 1, 1}},
  {{0, 0, 0}, {-1, 0, 0}, {0, 0, 1}, {0, 0, 2}}, // L
  {{0, 0, 0}, {-1, 0, 0}, {0, 0, 1}, {1, 0, 1}} // 
};

int game_matrix[10][4][4] = {};
Point current_blocks[8];

int anode_level = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(2000000);

  randomSeed(analogRead(0)); // don't put anything in pin0... Ensures randomness


  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV128);
  pinMode(CLOCK_XY_SWITCH, INPUT);

  pinMode(LATCH_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);

  digitalWrite(LATCH_PIN, LOW);

  for (int i = 0; i < 8; i++) {current_blocks[i] = {-1, -1, -1};}

  SPI.begin();

  generate_new_block();
  Serial.println("Generated new block");
}

bool is_valid_point(Point p) {
  return (p.x >= 0 && p.x < 4 && p.y >= 0 && p.y < 4 && p.z >= 0 && p.z < 10 && game_matrix[p.z][p.y][p.x] != 1);
}

bool generate_new_block() {
    int randNumber = random(4);
    
    Point start_index = {1, 1, 1};
    Point new_blocks[8] = {};
    for (int i = 0; i < sizeof(tetromino[randNumber]) / sizeof(struct Point); i++) {
      Point new_block;
      new_block.x = start_index.x + tetromino[randNumber][i].x;
      new_block.y = start_index.y + tetromino[randNumber][i].y;
      new_block.z = start_index.x + tetromino[randNumber][i].z;
      if (!is_valid_point(new_block)) {
        return false;
      }
      new_blocks[i] = new_block;
    }

    // All blocks are valid so update game matrix and current blocks
    for (int i = 0; i < 8; i++) {
      current_blocks[i] = new_blocks[i];
      game_matrix[new_blocks[i].z][new_blocks[i].y][new_blocks[i].x] = 1;
    }
    print_game_matrix();
    return true;
}

int alpha = 0;

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

void single_color_cathode_transfer() {
  // Okkk, I have game matrix and current anode level
  int y_x_to_on[16];
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      y_x_to_on[4*i + j] = game_matrix[anode_level][i][j];
    }
  }

  if (alpha == 0 && anode_level == 2) {
    for (int i = 0; i < 16; i++) {
      Serial.print(y_x_to_on[i]);
    }
  }

  // Finally we can get two 8 bit integer from this to send SPI.
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

  if (alpha == 0 && anode_level == 2) {
  Serial.println("");
  Serial.println(cathode1, BIN);
    Serial.println(cathode2, BIN);
   alpha =1;
  }

  SPI.transfer(~cathode2);
  SPI.transfer(~cathode1);

}

// red cathode 5 and 6 are swapped.... HMMMM


void anode_spi_transfer() {

  switch (anode_level) {
    case 0:
    SPI.transfer(0);
    SPI.transfer(1);
    break;
    case 1:
    SPI.transfer(0);
    SPI.transfer(2);
    break;
    case 2:
    SPI.transfer(0);
    SPI.transfer(4);
    break;
    case 3:
    SPI.transfer(0);
    SPI.transfer(8);
    break;
    case 4:
    SPI.transfer(0);
    SPI.transfer(16);
    break;
    case 5:
    SPI.transfer(0);
    SPI.transfer(32);
    break;
    case 6:
    SPI.transfer(0);
    SPI.transfer(64);
    break;
    case 7:
    SPI.transfer(0);
    SPI.transfer(128);
    break;
    case 8:
    SPI.transfer(2);
    SPI.transfer(0);
    break;
    case 9:
    SPI.transfer(1);
    SPI.transfer(0);
    break;

  }
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

void loop() {

  // if (current_blocks[0].x == -1) {
  //   while(!generate_new_block()) {
  //     continue;
  //   }
  // }

  anode_spi_transfer();

  // blue
  SPI.transfer(255);
  SPI.transfer(255);
  // green
  SPI.transfer(255);
  SPI.transfer(255);
  //red
  // SPI.transfer(255);
  // SPI.transfer(red1[anode_level]);

  single_color_cathode_transfer();

  anode_level += 1;
  anode_level %= 10;

  PORTF |= B00000001; // Latch pin HIGH
  PORTF &= ~(1 << 0); // Latch pin LOW

}