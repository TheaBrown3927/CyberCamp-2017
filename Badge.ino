/*
    HTTP over TLS (HTTPS)
    This connects to wifi and then fetches
    mikej.tech and then displays text file.
    This also boots into an easter egg simple
    game.
    Created by Mike Julander and Weston Shakespear 2017.
    This is in public domain.
    https://github.com/NaH012/CyberCamp-2017
    https://mikej.tech
    https://westonshakespear.tech
*/
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <EEPROM.h>
#include "Badge.h"
#include "pitches.h"

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char* fingerprint = "D3 1B F4 69 70 D3 75 6E 01 EF C7 C5 E4 96 A5 DC C3 91 2E FA";


//to add ssid and passwords add them like this {"ssid1", "ssid2", "ssid3"}
String ssid[] = {"Cybercamp2017"};
String password[] = {"samurai7"};

const char* ssid2;
const char* password2;
int conn;
String results;

int movCounter = 0;

int lives = 4;
int board[8][8] = {0};
int obstaclePlace = 0;
long gameTick = 0;
int difficulty = 0;

//player variables
int playerX = 0;
int playerHome = 0;
int score = 0;
bool left = true;
int moveTime = 0;
int coin = -1;

int melodyPacman[] = {
  NOTE_B3, NOTE_B4, NOTE_FS4, NOTE_DS4, NOTE_B4, NOTE_FS4, NOTE_DS4, NOTE_C4, NOTE_C5, NOTE_G4, NOTE_E4, NOTE_C5, NOTE_G4, NOTE_E4, NOTE_B3, NOTE_B4, NOTE_FS4, NOTE_DS4, NOTE_B4, NOTE_FS4, NOTE_DS4, NOTE_DS4, NOTE_E4, NOTE_F4, NOTE_F4, NOTE_FS4, NOTE_G4, NOTE_G4, NOTE_GS4, NOTE_A4, NOTE_B4
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurationsPacman[] = {
  8, 8, 8, 8, 16, 8, 4, 8, 8, 8, 8, 16, 8, 4, 8, 8, 8, 8, 16, 8, 4, 16, 16, 8, 16, 16, 8, 16, 16, 8, 4
};

const char* host = "mikej.tech";
const int httpsPort = 443;

int pinButton = 15;
int pinBuzzer = 4;
int pinCS = 5; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 1;
int numberOfVerticalDisplays = 1;
int spacer = 2;
int width = 5 + spacer; // The font width is 5 pixels

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

int option = 0; //This tells the program what to load, the text scrolling (0) or the game (1)

void setup() {
  matrix.setIntensity(0); // Use a value between 0 and 15 for brightness
  matrix.setRotation(0, 2);
  EEPROM.begin(512);
  Serial.begin(115200);
  pinMode(pinButton, INPUT);
  pinMode(pinBuzzer, OUTPUT);
  matrix.fillScreen(HIGH);
  matrix.write(); // Send bitmap to display
  wifisetup();

}

void loop() {
  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    String text = readText(0);
    displayLine(text, 0);
  } else {
    if (client.verify(fingerprint, host)) {
      Serial.println("certificate matches");
    } else {
      Serial.println("certificate doesn't match");
    }
  }


  if(option == 0){
    option = getMessage(client, option);
  }else{
    option = game(client, option);
  }

}

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case WIFI_EVENT_STAMODE_GOT_IP:
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      break;
    case WIFI_EVENT_STAMODE_DISCONNECTED:
      break;
  }
}

int getMessage(WiFiClientSecure client, int opt) {
  String url = "/content/hidden.txt";

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    Serial.println(line);
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }


  String line = client.readStringUntil('\n');
  save(line, 0);
  Serial.print("Raw data = ");
  Serial.println(line);
  opt = displayLine(line, opt);
  return opt;
}

int displayLine(String line, int option) {
  unsigned long buttonTime = 0;
  bool pressed = false;
  int pos = line.indexOf(":");
  String tape = line.substring(0, pos) + " ";
  String brightness = line.substring(pos + 1);
  pos = brightness.indexOf(":");
  String count = brightness.substring(pos + 1);
  brightness = brightness.substring(0, pos);
  int wait = count.toInt();
  Serial.print("Scrolling text = ");
  Serial.println(tape);
  Serial.print("Brightness = ");
  Serial.println(brightness);
  Serial.print("Delay between screen updates = ");
  Serial.println(wait);
  matrix.setIntensity(brightness.toInt());
  unsigned long startTime = millis();
  unsigned long endTime = startTime + 120000;
  while (startTime < endTime && option == 0) {
    for ( int i = 0 ; i < width * tape.length() + matrix.width() - 1 - spacer && option == 0; i++ ) {
      matrix.fillScreen(LOW);
      int letter = i / width;
      int x = (matrix.width() - 1) - i % width;
      int y = (matrix.height() - 8) / 2; // center the text vertically
      while ( x + width - spacer >= 0 && letter >= 0 ) {
        if (digitalRead(pinButton) && buttonTime <= startTime && pressed) {
          option = 1;
          pressed = false;
        }else if(digitalRead(pinButton) && buttonTime <= startTime && !pressed){
          pressed = true;
          buttonTime = startTime + 1000;
        }else if(!digitalRead(pinButton) && buttonTime <= startTime && pressed){
          pressed = false;
        }

        if ( letter < tape.length() ) {
          matrix.drawChar(x, y, tape[letter], HIGH, LOW, 1);
        }

        letter--;
        x -= width;
      }
      matrix.write(); // Send bitmap to display
      startTime = millis();
      delay(wait);
    }
    Serial.print(endTime - startTime);
    Serial.println(" milliseconds until update is checked for");
    if (line == "") {
      save("Cyber Camp 2017:5:80", 0);

      startTime = endTime;
    }
  }
  return option;
}
void save(String text, int addr) {
  for (int a = addr; a < (addr + 50); a++) {
    EEPROM.write(a, 0);
  }
  for (int i = 0; i < text.length(); i++) {
    addr++;
    char val = text[i];
    EEPROM.write(addr, val);
  }
  EEPROM.commit();
}


String readText(int addr) {
  String text = "";
  while (addr < 200) {
    int value = EEPROM.read(addr);
    char data = EEPROM.read(addr);
    if (value != 0 ) {
      text += data;
    }
    addr++;
  }
  return text;
}

int game(WiFiClientSecure client, int option) {
  //save("WRS", 100);
  //This is where the programming for the game will go
  matrix.fillScreen(HIGH);
  matrix.write();
  delay(1000);

  //iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 32; thisNote++) {

    // to calculate the note duration, take one second
    // divided by the note type.
    //e.g. quarter note = 1000 / buzzerPin, eighth note = 1000/8, etc.
    int noteDuration = 1000/noteDurationsPacman[thisNote];
    tone(pinBuzzer, melodyPacman[thisNote],noteDuration);
    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(pinBuzzer);
  }

  while (lives > 0) {
  //  Serial.println("Currently in game loop");
    noTone(pinBuzzer);
    render();
    //draw everything to screen
    matrix.write();
    gameTick++;

    if (score > 300) {
      difficulty = 3;
    }
    else if (score > 150) {
      difficulty = 2;
    }
    else if (score > 50) {
      difficulty = 1;
    }

    matrix.setRotation(difficulty);

    delay(100 - (difficulty * 15));
    Serial.println(score);
    Serial.println(difficulty);
  }
  Serial.println("Exited game loop");
  Serial.print("SCORE: ");
  Serial.print(score);
  Serial.println("\n");
  uploadScores(client, "WRS", score);
  //clean up
  noTone(pinBuzzer);
  lives = 4;
  score = 0;
  difficulty = 0;
  matrix.setRotation(0);

  return 0; //used to switch back to scrolling the text by sending the value of 0
}

//game functions
void render() {
//  Serial.println("Render Board");

  //render obstacles
  if ((gameTick % (4 - (difficulty / 2))) == 0) {
    randomSeed(analogRead(0) * millis());
    obstaclePlace = random(0, 8);
    Serial.println(obstaclePlace);
    board[obstaclePlace][0] = 1;
  }

  //shift all pixels down
  for(int x = 7;x >= 0;x--) {
    for(int y = 7;y > 0;y--) {
      board[x][y] = board[x][y - 1];
    }
  }

  //clear top row
  for(int c = 0;c < 8;c++) {
    board[c][0] = 0;
  }

  //draw board
  for(int x = 0;x < 8;x++) {
    for(int y = 0;y < 8;y++) {
      if (board[x][y] == 1) {
      //  Serial.print("1 ");
        matrix.drawPixel(x, y, HIGH);
      } else {
      //  Serial.print("0 ");
        matrix.drawPixel(x, y, LOW);
      }
    }
  //  Serial.print("\n");
  }

  //update lives
  //erase life area
  for(int a = 0;a < 4;a++) {
    matrix.drawPixel((a + 2), 0, LOW);
  }
  for(int b = 0;b < lives;b++) {
    matrix.drawPixel((b + 2), 0, HIGH);
  }

  //render player
  matrix.drawPixel(playerX, 7, HIGH);

  //move player
  if (digitalRead(pinButton) == HIGH) {
    movCounter++;
    if (movCounter < 8) {
      if (playerHome == 0) {
        playerX++;
      } else {
        playerX--;
      }
    }
  } else {
    movCounter = 0;
  }

  moveTime++;

  if (playerX == 0 && left == true) {
    playerHome = 0;
    if (moveTime < 20) {
      score += difficulty + (20 - moveTime);
    } else {
      score += difficulty;
    }
    tone(pinBuzzer, NOTE_B5, (1000/8));
    moveTime = 0;
    left = false;
  }
  if (playerX == 7 && left == false) {
    playerHome = 1;
    if (moveTime < 20) {
      score += difficulty + (20 - moveTime);
    } else {
      score += difficulty;
    }
    tone(pinBuzzer, NOTE_B5, (1000/8));
    moveTime = 0;
    left = true;
  }

  if(board[playerX][6] == 1) {
    lives--;
    tone(pinBuzzer, NOTE_B2, (1000/8));
  }
}

void uploadScores(WiFiClientSecure client, String user, int score) {
  String url = "/cybergame/scores.php?name=" + user + "&score=" + score;

  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

}

//add to top
int counter = 0;

void resetEverything() {
  //reset code
  for (int i = 0 ; i < 512 ; i++) {
    EEPROM.write(i, 0);
  }

  EEPROM.commit();

  while(true) {
    matrix.fillScreen(HIGH);
    matrix.write();
    delay(100);
    matrix.fillScreen(LOW);
    matrix.write();
    delay(100);
  }
}

void wifisetup() {
  WiFi.onEvent(WiFiEvent);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  bool known;
  int logginNum = sizeof(ssid);
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) {
    for (int a = 0; a < logginNum; a++) {
      if (ssid[a] == WiFi.SSID(i)) {
        Serial.println("Connecting to");
        Serial.println(ssid[a]);
        known = true;
        ssid2 = ssid[a].c_str();
        password2 = password[a].c_str();
        WiFi.begin(ssid2, password2);
        int timeout = 0;
        Serial.println(" ");
        while (WiFi.status() != WL_CONNECTED && timeout <= 17) {
          delay(500);
          timeout += 1;
          if(digitalRead(pinButton) == HIGH) {
            counter++;
          }
          if (counter == 4) {
            resetEverything();
          }
          Serial.print(".");
        }
        if (timeout <= 17 && timeout != 0) {
          conn = i;
          i = n;
        }
        a = logginNum;
      } else {
        known = false;
      }
    }
  }
}
