/*
    HTTP over TLS (HTTPS)

    This connects to wifi and then fetches
    mikej.tech and then displays it.

    Created by Mike Julander 2017.
    This is in public domain.
*/
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <EEPROM.h>


String ssid[1] = {"ssid"};
String password[1] = {"password"};

const char* ssid2;
const char* password2;
int conn;
String results;

const char* host = "mikej.tech";
const int httpsPort = 443;

int pinCS = 5; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 1;
int numberOfVerticalDisplays = 1;
int spacer = 2;
int width = 5 + spacer; // The font width is 5 pixels

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char* fingerprint = "D3 1B F4 69 70 D3 75 6E 01 EF C7 C5 E4 96 A5 DC C3 91 2E FA";
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

void setup() {
  matrix.setIntensity(0); // Use a value between 0 and 15 for brightness
  matrix.setRotation(0, 2);
  EEPROM.begin(512);
  Serial.begin(115200);
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
    readText();
    return;
  }

  if (client.verify(fingerprint, host)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }

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
  save(line);
  Serial.println(line);
  displayLine(line);

}
void displayLine(String line) {
  int pos = line.indexOf(":");
  String tape = line.substring(0, pos) + " ";
  String brightness = line.substring(pos + 1);
  pos = brightness.indexOf(":");
  String count = brightness.substring(pos + 1);
  brightness = brightness.substring(0, pos);
  int wait = count.toInt();
  Serial.println(tape);
  Serial.println(brightness);
  Serial.println(wait);
  matrix.setIntensity(brightness.toInt());
  unsigned long startTime = millis();
  unsigned long endTime = startTime + 60000;
  while (startTime < endTime) {
    for ( int i = 0 ; i < width * tape.length() + matrix.width() - 1 - spacer; i++ ) {
      matrix.fillScreen(LOW);
      int letter = i / width;
      //    int letter = i;
      int x = (matrix.width() - 1) - i % width;
      int y = (matrix.height() - 8) / 2; // center the text vertically
      while ( x + width - spacer >= 0 && letter >= 0 ) {
        if ( letter < tape.length() ) {
          matrix.drawChar(x, y, tape[letter], HIGH, LOW, 1);
        }

        letter--;
        x -= width;
      }
      matrix.write(); // Send bitmap to display

      delay(wait);
    }
    startTime = millis();
    Serial.println(endTime - startTime);
    if (line == "") {
      save("Cyber Camp 2017:5:80");

      startTime = endTime;
    }
  }
}
void save(String text) {
  for (int a = 0; a < 512; a++) {
    EEPROM.write(a, 0);
  }
  int addr = 0;
  for (int i = 0; i < text.length(); i++) {
    addr++;
    char val = text[i];
    EEPROM.write(addr, val);
  }
  //  for (int a = 0; a < 512 - text.length(); a++) {
  //    addr++;
  //    EEPROM.write(addr, 0);
  //  }
  EEPROM.commit();
}
void readText() {
  int addr = 0;
  String text = "";
  while (addr < 200) {
    int value = EEPROM.read(addr);
    char data = EEPROM.read(addr);
    if (value != 0 ) {
      text += data;
    }
    addr++;
  }
  displayLine(text);
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

