#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

RH_ASK driver;
int LED = 3;
int RLED = 9;
int GLED = 10;
int button = 5;
int voltagePin = A4;
int speakerPin = 13;
int numTones = 10;
const char *msg = "r-set";
const char *msg2 = "s-set";


int tones[] = {261, 277, 294, 311, 330, 349, 370, 392, 415, 440};
//            mid C  C#   D    D#   E    F    F#   G    G#   A


#define LEDON() analogWrite(LED, 255)
#define LEDOFF() analogWrite(LED, 0)
#define RLEDON() analogWrite(RLED, 255)
#define RLEDOFF() analogWrite(RLED, 0)
#define GLEDON() analogWrite(GLED, 255)
#define GLEDOFF() analogWrite(GLED, 0)

void setup()
{
  Serial.begin(9600);
  if (!driver.init())
    Serial.println("init failed");
  pinMode(LED, OUTPUT);
  LEDOFF();
  pinMode(button, INPUT_PULLUP);
}

void loop()
{
  Serial.println(analogRead(voltagePin) * 5.0/1023.0);
  Serial.println(analogRead(voltagePin));
  if(analogRead(voltagePin) * 5/1023.0 > 4.75){
    GLEDON();
    RLEDOFF();
  }
  else if(analogRead(voltagePin) * 5.0/1023.0 > 3.5){
    RLEDON();
    GLEDON();
  }
  else {
    RLEDON();
    GLEDOFF();
  }
  if (digitalRead(button) == HIGH ) {
    sendM(msg2);
    if (digitalRead(button) == HIGH ) {
      sendM(msg);
  }
}

void sendM(const char *m) {
  tone(speakerPin, tones[0]);
  LEDON();
  long starttime = millis();
  long endtime = starttime;
  while ((endtime - starttime) <= 3000) // do this loop for up to 1000mS
  {
    driver.send((uint8_t *)m, strlen(m));
    driver.waitPacketSent();
    if((endtime - starttime) >= 500){
      noTone(speakerPin);
    }
    endtime = millis();
  }
  LEDOFF();

}
