#include <SPI.h>

const int CLOCK_PIN = 52;
const int DATA_PIN = 51;

const int CLOCK_XY_TACTILE_SWITCH = 5; // 2560 can only handle 2, 3, 18, 19, 20, 21
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

int current_drop_level = 0;
uint8_t drop_value = 254;
const int drop_interval = 2000;
unsigned long previousMillis = 0;  // will store last time LED was updated


volatile int anode_level = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(2000000);

  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV128);
  pinMode(TACTILE_SWITCH, INPUT);

  // noInterrupts();

  // TCCR1A = B00000000; // Normal Mode
  // TCCR1B = B00001011; // Prescale by 64 (so actually SLOWING the interrupt frequency) and CTC Mode.. Why do we need both? Can we not just change the counting to something much "bigger"  
  // TIMSK1 = B00000010;
  // OCR1A=30; // So the clock now counts till 30 (i.e. interrupt happen in 31x4us)

  pinMode(LATCH_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);

  digitalWrite(LATCH_PIN, LOW);

  SPI.begin();
  // attachInterrupt(digitalPinToInterrupt(TACTILE_SWITCH), tact_switch, CHANGE); // User input
  // interrupts();

  // check_portf();

}

// void tact_switch() {
//   noInterrupts();
//   Serial.println("TACT PRESSED WOWOWO");
//   interrupts();
// }


void loop() {
  // put your main code here, to run repeatedly:


  if (digitalRead(TACTILE_SWITCH) == 1) {
    // Serial.println("Tactile switch");
    // drop_value = ~((drop_value + 1) << 2);
    // Serial.println(drop_value);
  }

  // Serial.println(digitalRead(TACTILE_SWITCH));

  // all the data
  
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

  // blue
  SPI.transfer(255);
  SPI.transfer(255);
  // green
  SPI.transfer(255);
  SPI.transfer(255);
  //red
  SPI.transfer(255);
  SPI.transfer(red1[anode_level]);

  anode_level += 1;
  anode_level %= 10;

  PORTF |= B00000001; // Latch pin HIGH
  PORTF &= ~(1 << 0); // Latch pin LOW

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= drop_interval) {
    
    previousMillis = currentMillis;
    switch (current_drop_level) {
      case 0:
      red1[0] = drop_value;
      break;
      case 1:
      red1[1] = drop_value;
      break;
      case 2:
      red1[2] = drop_value;
      break;
      case 3:
      red1[3] = drop_value;
      break;
      case 4:
      red1[4] = drop_value;
      break;
      case 5:
      red1[5] = drop_value;
      break;
      case 6:
      red1[6] = drop_value;
      break;
      case 7:
      red1[7] = drop_value;
      break;
      case 8:
      red1[8] = drop_value;
      break;
      case 9:
      red1[9] = drop_value;
      break;
    }
    current_drop_level += 1;
    current_drop_level %= 10;
  }

}

// ISR(TIMER1_COMPA_vect) {

//   // Serial.println("Interrupted");

//   // // Extra 0s for the last 6 unused pins

//   // if (anode_level >= 8) {
//   //   // THIS IS BECAUSE OF WIRING ERROE
//   //   if (anode_level == 9) {
//   //     SPI.transfer(2);
//   //   } else {
//   //     SPI.transfer(1);
//   //   }
//   //   SPI.transfer(0);
//   // } else {
//   //   SPI.transfer(0);

//   //   if (anode_level == 0) {
//   //     SPI.transfer(1);
//   //   } else {
//   //     if (anode_level == 1) {
//   //       SPI.transfer(2);
//   //     } else {
//   //       SPI.transfer(2 << anode_level);
//   //     }
//   //   }
//   // }

//   switch (anode_level) {
//     case 0:
//     SPI.transfer(0);
//     SPI.transfer(1);
//     break;
//     case 1:
//     SPI.transfer(0);
//     SPI.transfer(2);
//     break;
//     case 2:
//     SPI.transfer(0);
//     SPI.transfer(4);
//     break;
//     case 3:
//     SPI.transfer(0);
//     SPI.transfer(8);
//     break;
//     case 4:
//     SPI.transfer(0);
//     SPI.transfer(16);
//     break;
//     case 5:
//     SPI.transfer(0);
//     SPI.transfer(32);
//     break;
//     case 6:
//     SPI.transfer(0);
//     SPI.transfer(64);
//     break;
//     case 7:
//     SPI.transfer(0);
//     SPI.transfer(128);
//     break;
//     case 8:
//     SPI.transfer(2);
//     SPI.transfer(0);
//     break;
//     case 9:
//     SPI.transfer(1);
//     SPI.transfer(0);
//     break;

//   }

//   // // Blue cathode.. 1 means OFF
//   SPI.transfer(255); // last 8
//   SPI.transfer(255); // first 8

//   // // Green
//   SPI.transfer(255);
//   SPI.transfer(255);

//   // // Red
//   SPI.transfer(red2[anode_level]);
//   SPI.transfer(red1[anode_level]); // Red cathode... 1 means OFF

//   anode_level += 1;
//   anode_level %= 10; // It cannot be 10.. 0-indexed

//   PORTF |= B00000001; // Latch pin HIGH
//   PORTF &= ~(1 << 0); // Latch pin LOW
// }
