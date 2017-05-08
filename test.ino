/*
  This File tests to make sure everything
  is wired properly
*/

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

int pinBuzzer = 4;
int pinButton = 15;

int pinCS = 5; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 1;
int numberOfVerticalDisplays = 1;

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

void setup() {
  matrix.setIntensity(2);
  matrix.setRotation(0, 2);    // The first display is position upside down
  matrix.invertDisplay(true);
  pinMode(pinBuzzer, OUTPUT);
  pinMode(pinButton, INPUT);
}



void loop() {

  bool back = LOW;
  bool front = HIGH;
  bool buzz = false;

  int prev = 0;
  int prevButton = 0;
  int prevBuz = 0;

  int x = 0;
  int y = 0;
  int a = 0;

  String disp = "Cyber";

  while (true) {
    int cur = millis();
    int button = digitalRead(pinButton);
    int wait = 250;

    if (cur >= prev) {
      matrix.fillScreen(back);
      if (x  < matrix.width()) {
        matrix.drawLine(x, 0, x, matrix.height(), front);
        x++;
      } else if (y < matrix.height()) {
        matrix.drawLine(0, y, matrix.width(), y, front);
        y++;
      } else if (a < disp.length()) {
        matrix.drawChar(1, 0, disp[a], front, back, 1);
        a++;
      } else {
        x = 0;
        y = 0;
        a = 0;
        matrix.drawLine(x, 0, x, matrix.height(), front);
        x++;
      }
      matrix.write();
      prev = cur + wait;
    }


    if (button && cur >= prevButton) {
      prevButton = cur + wait;
      prevBuz = cur + wait;
      back = front;
      if(front){
        front = LOW;
      }else{
        front = HIGH;
      }
      digitalWrite(pinBuzzer, HIGH);
      buzz = true;
    }



    if (buzz && cur >= prevBuz) {
      digitalWrite(pinBuzzer, LOW);
      buzz = false;
    }

    delay(1);
  }
}
