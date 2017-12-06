/*
   HTTP - Get BART station times and display on OLED screen.
*/
#include <ESP8266WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
Adafruit_SSD1306 display = Adafruit_SSD1306();
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

// Button pin
#define buttonPin 13
#define neoPin 14

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      8

// SSID + Passwords
//const char* ssid     = "Mills-Guest";
//const char* password = "Breaking_Barrier$!";
// Home wifi network
// Insert Wifi Network Here!
const char* ssid = "NS-B338-2.4";
const char* password = "Wasnotwas1";

// the BART API host
const char* BARThost = "api.bart.gov";

boolean stationsSetUp = false;
boolean stationsListed = false;

// Customizable API URL
const char* beginningAPICall = "/api/etd.aspx?cmd=etd&orig=";
// insert bart station abbrev here example "ROCK"
const char* middleAPICall = "&key=MW9S-E7SL-26DU-VV8V&dir=";
// insert n or s here - 'n'
const char* endAPICall = "&json=y";

String nextTrains[8];
String nextTrainTimes[8];
String nextTrainColors[8];
int selectedStation[2];
char inputBuffer[20];
char numBuffer[3];
int numSetUp = 0;
boolean stationsSelected = false;
char chosenDirection[2];
boolean firstTrain = true;

const int pauseDuration = 2000;
char scrolling[20];
int nChars = 0;
boolean printed = false;

unsigned int nextAPICall = 0;

boolean hasTrainInfo = false;
unsigned int nextScroll = 0;
unsigned int nextDisplay = 0;

int numResponses = 0;
int trainIndex = 0;

// Button variables
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
const unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
const unsigned long longPressDelay = 2000; // time for a 'long press'

const int blue[] = {0, 76, 102};
const int yellow[] = {127, 127, 25};
const int red[] = {127, 0, 0};
const int orange[] = {127, 76, 25};
const int green[] = {51, 153, 51};

const char* stationabbrev[] = {"12TH", "16TH", "19TH", "24TH", "ASHB", "BALB", "BAYF", "CAST", "CIVC", "COLS", "COLM", "CONC", "DALY",
                               "DBRK", "DUBL", "DELN", "PLZA", "EMBR", "FRMT", "FTVL", "GLEN", "HAYW", "LAFY", "LAKE", "MCAR", "MLBR", "MONT", "NBRK", "NCON", "OAKL", "ORIN",
                               "PITT", "PHIL", "POWL", "RICH", "ROCK", "SBRN", "SFIA", "SANL", "SHAY", "SSAN", "UCTY", "WCRK", "WARM", "WDUB", "WOAK"
                              };

const char* stations[] = {"12th St. Oakland City Center", "16th St. Mission", "19th St. Oakland", "24th St. Mission", "Ashby", "Balboa Park",
                          "Bay Fair", "Castro Valley", "Civic Center/UN Plaza", "Coliseum", "Colma",  "Concord", "Daly City", "Downtown Berkeley", "Dublin/Pleasanton",
                          "El Cerrito del Norte", "El Cerrito Plaza", "Embarcadero", "Fremont", "Fruitvale", "Glen Park", "Hayward", "Lafayette", "Lake Merritt",
                          "MacArthur", "Millbrae",  "Montgomery St.", "North Berkeley", "North Concord/Martinez", "Oakland International Airport", "Orinda",
                          "Pittsburg/Bay Point",  "Pleasant Hill/Contra Costa Centre",  "Powell St.", "Richmond", "Rockridge", "San Bruno", "San Francisco International Airport",
                          "San Leandro", "South Hayward", "South San Francisco", "Union City",   "Walnut Creek", "Warm Springs/South Fremont",
                          "West Dublin/Pleasanton", "West Oakland"
                         };

const uint8_t PROGMEM gamma8[] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
  2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
  10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
  17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
  25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
  37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
  51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
  69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
  90, 92, 93, 95, 96, 98, 99, 101, 102, 104, 105, 107, 109, 110, 112, 114,
  115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142,
  144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175,
  177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
  215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255
};

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, neoPin, NEO_GRB + NEO_KHZ800);


void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT_PULLUP);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print(F("Connecting to "));
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }

  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());

  // Set up display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(2.5);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(F("Booted up!"));
  display.display();

  pixels.begin(); // This initializes the NeoPixel library.
}

void loop() {

  // read the state of the switch into a local variable:
  int reading = digitalRead(buttonPin);

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:
    // If longer than the long press delay then reset
    if ((millis() - lastDebounceTime) > longPressDelay) {
      if (buttonState == LOW) {
        Serial.println(F("Switching button "));
        firstTrain = !firstTrain;
        // Get new data now!
        nextAPICall = millis();
        hasTrainInfo = false;
        printed = false;
        // Adjusted so that the old train will not scroll
        nextScroll = millis() + 30000;
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.print(F("Fetching\nNew Data!"));
        display.display();
      }
    }
    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // If button state is pressed then deal with the presses
      if (buttonState == LOW) {
        Serial.println(F("Next reading"));
        printed = false;
        trainIndex++;
      }
    }
  }

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;

  if (!stationsSetUp) {
    if (!stationsListed) {
      for (int i = 0; i < 45; i++) {
        Serial.print(i + 1);
        Serial.print(F(": "));
        Serial.print(stations[i]);
        Serial.print(F(" ("));
        Serial.print(stationabbrev[i]);
        Serial.println(F(")"));
      }
      Serial.println(F("Please type in the number for your first station, then second station"));
      stationsListed = true;
    } else if (!stationsSelected) {
      if (numSetUp < 2) {
        if (Serial.available()) {
          memset(numBuffer, 0, sizeof(numBuffer));
          nChars = Serial.readBytesUntil('\n', numBuffer, 3);
          boolean isNum = true;
          for (int i = 0; i < nChars; i++) {
            if (!isDigit(numBuffer[i])) {
              isNum = false;
            }
          }
          if (isNum) {
            int number = atoi(numBuffer);
            Serial.println(number);
            selectedStation[numSetUp] = number - 1;
            Serial.print(F("Station "));
            Serial.print(numSetUp + 1);
            Serial.print(F(" selected "));
            Serial.println(stationabbrev[selectedStation[numSetUp]]);
            numSetUp++;

          }
          if (numSetUp == 2) {
            numSetUp = 0;
            stationsSelected = true;
            Serial.println(F("Stations selected!"));
            Serial.print(F("Please select N or S direction for your first station "));
            Serial.println(stationabbrev[selectedStation[0]]);
          } else if (numSetUp == 1) {
            Serial.println(F("Please select your second station"));
          }
        }
      }
    } else {
      if (numSetUp < 2) {
        if (Serial.available()) {
          memset(numBuffer, 0, sizeof(numBuffer));
          Serial.readBytesUntil('\n', numBuffer, 2);
          if (numBuffer[0] == 'n' || numBuffer[0] == 'N') {
            chosenDirection[numSetUp] = 'n';
            numSetUp++;
            if (numSetUp == 1) {
              Serial.print("Please select N or S direction for your second station ");
              Serial.println(stationabbrev[selectedStation[1]]);
            }
          } else if (numBuffer[0] == 's' || numBuffer[0] == 'S') {
            chosenDirection[numSetUp] = 's';
            numSetUp++;
            if (numSetUp == 1) {
              Serial.print("Please select N or S direction for your second station ");
              Serial.println(stationabbrev[selectedStation[1]]);
            }
          } else {
            Serial.println(F("Please choose a direction N or S"));
          }
        }
        if (numSetUp == 2) {
          stationsSetUp = true;
        }
      }
    }
  } else {
    if (millis() > nextAPICall) {
      Serial.print(F("connecting to "));
      Serial.println(BARThost);

      // Use WiFiClient class to create TCP connections
      WiFiClient client;
      const int httpPort = 80;
      if (!client.connect(BARThost, httpPort)) {
        Serial.println(F("connection failed"));
        return;
      }
      String url = beginningAPICall;
      if (firstTrain) {
        Serial.println(stationabbrev[selectedStation[0]]);
        url += stationabbrev[selectedStation[0]];
        url += middleAPICall;
        url += chosenDirection[0];
        url += endAPICall;
      } else {
        Serial.println(stationabbrev[selectedStation[1]]);
        url += stationabbrev[selectedStation[1]];
        url += middleAPICall;
        url += chosenDirection[1];
        url += endAPICall;
      }

      Serial.print(F("Requesting URL: "));
      Serial.println(url);

      // This will send the request to the server
      client.print(String("GET ") + String(url) + " HTTP/1.0\r\n" +
                   "Host: " + BARThost + "\r\n" +
                   "Connection: close\r\n" +
                   "\r\n"
                  );
      client.println();

      // Read all the lines of the reply from server and print them to Serial
      // Get HTML page
      String line = "";
      while (client.available()) {
        line = client.readStringUntil('\r');
        Serial.print(line);
      }
      Serial.println();
      Serial.println(F("Last line should be JSON"));
      Serial.println(line);
      Serial.println(F("closing connection"));

      parseJSON(line);
      trainIndex = 0;

      // Adjust for how often you will call API
      nextAPICall = millis() + 60000;
    }

    if (hasTrainInfo) {
      if (trainIndex < numResponses) {
        if (!printed) {
          printChars();
          trainIndex++;
        }
        if (millis() > nextScroll) {
          scroll();
        }
      } else {
        trainIndex = 0;
      }
    }
  }
}

void printChars() {
  display.clearDisplay();
  Serial.println(F("Printing Chars!"));
  Serial.println(nextTrains[trainIndex]);
  nextTrains[trainIndex].toCharArray(inputBuffer, (nextTrains[trainIndex].length() + 1));
  for (int j = 4; j < 10; j++) {
    inputBuffer[j] = ' ';
  }
  char secondLine[11];
  nextTrainTimes[trainIndex].toCharArray(secondLine, 10);
  for (int k = 0; k < 11; k++) {
    inputBuffer[k + 10] = secondLine[k];
  }
  turnOnLEDS(nextTrainColors[trainIndex]);
  Serial.print(F("Next train time: "));
  Serial.println(secondLine);
  Serial.println(inputBuffer);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(inputBuffer);
  display.display();
  printed = true;

  // Adjust for how long station lingers before scrolling
  nextScroll = millis() + 9000;
}

void scroll() {
  Serial.println(F("Scrolling!"));
  // Iterate 10 times through array. 'Moving' chars through array.
  for (int k = 0; k < 10; k++) {
    for (int i = 0; i < 10; i++) {
      if (i == 0) {
        scrolling[i] = ' ';
        scrolling[i + 10] = ' ';
      }
      else {
        scrolling[i] = inputBuffer[i - 1];
        inputBuffer[i - 1] = scrolling[i - 1];
        scrolling[i + 10] = inputBuffer[i + 10 - 1];
        inputBuffer[i + 10 - 1] = scrolling[i + 10 - 1];
      }
    }

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print(scrolling);
    display.display();
  }

  display.clearDisplay();
  // Done with scrolling
  printed = false;
}

void parseJSON(String line) {
  numResponses = 0;
  Serial.println(F("Parsing JSON"));
  char json[line.length() + 1];
  line.toCharArray(json, line.length() + 1);

  const size_t bufferSize = JSON_ARRAY_SIZE(1) + 4 * JSON_ARRAY_SIZE(3) + JSON_ARRAY_SIZE(4) + JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 4 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(6) + 12 * JSON_OBJECT_SIZE(8) + 1790;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  JsonObject& root1 = jsonBuffer.parseObject(json);

  // Test if parsing succeeds.
  if (!root1.success()) {
    Serial.println(F("parseObject() failed"));
    hasTrainInfo = false;
    stationsSetUp = false;
    stationsListed = false;
    return;
  }

  JsonObject& root = root1[F("root")];

  JsonObject& root_station0 = root[F("station")][0];
  const char* root_station0_name = root_station0[F("name")]; // Departing station name ie - "Embarcadero"
  const char* root_station0_abbr = root_station0[F("abbr")]; // Departing station abbrevation - "EMBR"

  JsonArray& root_station0_etd = root_station0[F("etd")];
  Serial.print(F("Num of different destinations is: " ));
  int numDestinations = root_station0_etd.size();
  Serial.println(root_station0_etd.size());
  for (int i = 0; i < numDestinations; i++) {
    JsonObject& root_station0_etd0 = root_station0_etd[i];
    // Destination station 0
    const char* root_station0_etd0_destination = root_station0_etd0[F("destination")]; // Destination station - example "Dublin/Pleasanton"
    const char* root_station0_etd0_abbreviation = root_station0_etd0[F("abbreviation")]; // Destination station abbreviation - example "DUBL"
    Serial.println(root_station0_etd0_abbreviation);

    JsonArray& root_station0_etd0_estimate = root_station0_etd0[F("estimate")];
    Serial.print("Num of times for this station is: " );
    int numTimesForDestination = root_station0_etd0_estimate.size();
    Serial.println(root_station0_etd0_estimate.size());
    nextTrainTimes[i] = "";
    for (int j = 0; j < numTimesForDestination; j++) {
      JsonObject& root_station0_etd0_estimate0 = root_station0_etd0_estimate[j];
      const char* root_station0_etd0_estimate0_minutes = root_station0_etd0_estimate0[F("minutes")]; // min to next train - example "12"
      const char* root_station0_etd0_estimate0_color = root_station0_etd0_estimate0[F("color")]; // color of next train - example "RED"
      if (strcmp(root_station0_etd0_estimate0_minutes, "Leaving") == 0) {
        root_station0_etd0_estimate0_minutes = "0";
      }
      if (j == 0) {
        nextTrains[i] = root_station0_etd0_abbreviation;
        nextTrainColors[i] = root_station0_etd0_estimate0_color;
        numResponses++;
      }
      if (j == numTimesForDestination - 1) {
        nextTrainTimes[i] += root_station0_etd0_estimate0_minutes;
      } else {
        nextTrainTimes[i] += root_station0_etd0_estimate0_minutes + String(F(","));
      }
      Serial.print(root_station0_etd0_estimate0_minutes);
      Serial.println(F(" minutes until the next train"));
      Serial.println(nextTrainTimes[i]);
    }
  }
  hasTrainInfo = true;
}

void turnOffLEDS() {
  for (int i = 0; i < NUMPIXELS; i++) {
    // pixels.Color takes RGB values, from 0,0,0
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));

    // going to start from bottom
    pixels.show(); // This sends the updated pixel color to the hardware.
  }
}

void turnOnLEDS(String color) {
  for (int i = 0; i < NUMPIXELS; i++) {
    if (color == "RED") {
      pixels.setPixelColor(i, pixels.Color(pgm_read_byte(&gamma8[red[0]]), pgm_read_byte(&gamma8[red[1]]), pgm_read_byte(&gamma8[red[2]])));
    } else if (color == "BLUE") {
      pixels.setPixelColor(i, pixels.Color(pgm_read_byte(&gamma8[blue[0]]), pgm_read_byte(&gamma8[blue[1]]), pgm_read_byte(&gamma8[blue[2]])));
    } else if (color == "YELLOW") {
      pixels.setPixelColor(i, pixels.Color(pgm_read_byte(&gamma8[yellow[0]]), pgm_read_byte(&gamma8[yellow[1]]), pgm_read_byte(&gamma8[yellow[2]])));
    } else if (color == "ORANGE") {
      pixels.setPixelColor(i, pixels.Color(pgm_read_byte(&gamma8[orange[0]]), pgm_read_byte(&gamma8[orange[1]]), pgm_read_byte(&gamma8[orange[2]])));
    } else if  (color == "GREEN") {
      pixels.setPixelColor(i, pixels.Color(pgm_read_byte(&gamma8[green[0]]), pgm_read_byte(&gamma8[green[1]]), pgm_read_byte(&gamma8[green[2]])));
    } else {
      turnOffLEDS();
    }
    // going to start from bottom
    pixels.show(); // This sends the updated pixel color to the hardware.
  }
}


