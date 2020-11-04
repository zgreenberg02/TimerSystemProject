
#include <math.h>
#include <StopWatch.h>
#include <RH_ASK.h>
#include <SPI.h>

StopWatch s;

byte segmentClock = 6;
byte segmentLatch = 5;
byte segmentData = 7;

RH_ASK driver;

int speakerPin = 12;
int stopButton = 3;
int startButton = 4;
int resetButton = 2;
int numTones = 10;
long m = 0;

long inputTime = 0;
double sec;

bool start = false;
bool started = false;
bool stopped = false;

int tones[] = {261, 277, 294, 311, 330, 349, 370, 392, 415, 440};
//            mid C  C#   D    D#   E    F    F#   G    G#   A


void setup() {

  Serial.begin(9600);
  Serial.println("Large Digit Driver Example");
  if (!driver.init())
    Serial.println("init failed");
  pinMode(segmentClock, OUTPUT);
  pinMode(segmentData, OUTPUT);
  pinMode(segmentLatch, OUTPUT);

  digitalWrite(segmentClock, LOW);
  digitalWrite(segmentData, LOW);
  digitalWrite(segmentLatch, LOW);


}

String receiver() {
  uint8_t buf[5];
  uint8_t buflen = sizeof(buf);
  if (driver.recv(buf, &buflen))
  {
    String str;
    str.reserve(buflen + 1);
    for (int i = 0; i < buflen; ++i) {
      str += (char) buf[i];
    }
    return str;

  }
  return "";

}




void loop() {
  String temp = receiver();
  if (temp.equals("r-set") || digitalRead(resetButton) == HIGH) {
    if (started) {
      tone(speakerPin, tones[0]);
      started = false;
      stopped = false;
      inputTime = 0;
      m = 0;
      showNumber(inputTime);
      delay(500);
      noTone(speakerPin);
    }
  }
  if (temp.equals("p-set") || digitalRead(stopButton) == HIGH) {
    if (!stopped && started) {
      tone(speakerPin, tones[0]);
      stopped = true;
      s.stop();
      showNumber(inputTime);
      delay(500);
      noTone(speakerPin);
    }
  }
  if (temp.equals("s-set") || digitalRead(startButton) == HIGH) {

    if (!started) {
      tone(speakerPin, tones[0]);
      s.start();
      started = true;
      stopped = false;
    }

  }
  if (!started) {
    inputTime = 0;
    m = 0;
    showNumber(inputTime);

  } else {
    if (!stopped) {
      sec = s.sec();
      if (sec >= 60) {
        s.restart();
        m ++;
        if (m > 99) {
          stopped = true;
        }
      }
      if (sec > .5) {
        noTone(speakerPin);
      }

      inputTime = m * 10000L + (sec * 100);
      showNumber(inputTime);

      delay(70);

    } else {
      s.stop();
      showNumber(inputTime);
    }

  }

}

//Takes a number and displays 2 numbers. Displays absolute value (no negatives)
void showNumber(long number)
{

  String num = String(number);
  while (num.length() < 6) {
    num = "0" + num;
  }
  Serial.print("number: ");
  Serial.println(number);

  for (int x = 0 ; x < 6 ; x++)
  {
    int numb = num.charAt(5 - x) - '0';
    bool decimal = false;
    if (x % 2 == 0 && x != 0) {
      decimal = true;
    }
    postNumber(numb, decimal);
  }

  //Latch the current segment data
  digitalWrite(segmentLatch, LOW);
  digitalWrite(segmentLatch, HIGH); //Register moves storage register on the rising edge of RCK
}



//Given a number, or '-', shifts it out to the display
void postNumber(byte number, boolean decimal)
{
  // Below, the -, |, and . represent the parts of one of the digits of the display. The letter/letters to the side of each line show which letter represents each part of the number.
  //    -    A
  //   | |   F/B
  //    -    G
  //   | |   E/C
  //    - .  D/DP

#define a  1<<0
#define b  1<<6
#define c  1<<5
#define d  1<<4
#define e  1<<3
#define f  1<<1
#define g  1<<2
#define dp 1<<7

  byte segments;

  switch (number)
  {
    case 1: segments = b | c; break;
    case 2: segments = a | b | d | e | g; break;
    case 3: segments = a | b | c | d | g; break;
    case 4: segments = f | g | b | c; break;
    case 5: segments = a | f | g | c | d; break;
    case 6: segments = a | f | g | e | c | d; break;
    case 7: segments = a | b | c; break;
    case 8: segments = a | b | c | d | e | f | g; break;
    case 9: segments = a | b | c | d | f | g; break;
    case 0: segments = a | b | c | d | e | f; break;
    case ' ': segments = 0; break;
    case 'c': segments = g | e | d; break;
    case '-': segments = g; break;
  }

  if (decimal) segments |= dp;

  //Clock these bits out to the drivers
  for (byte x = 0 ; x < 8 ; x++)
  {
    digitalWrite(segmentClock, LOW);
    digitalWrite(segmentData, segments & 1 << (7 - x));
    digitalWrite(segmentClock, HIGH); //Data transfers to the register on the rising edge of SRCK
  }
}
