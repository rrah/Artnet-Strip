// Artnet control of WS2812b LED strip
//
// Robert Walker 11/12/16

#include <Artnet.h>
#include <Adafruit_NeoPixel.h>

// Definitions for the LED strip
#define PIN 6
#define MAX_LED 300

// Definitions for Art-Net
#define ART_START_UNI 3

// Network settings
byte ip[] = {192, 168, 1, 251};
byte mac[] = {0x90, 0xA2, 0xDA, 0x10, 0x5F, 0x28};

// Artnet variables
bool art_read = false;
Artnet artnet;
// 1 = RGB, 2 = RGB+Dim, 3 = RGB+Dim (3-cell)
int LED_MODE = 2;
int LED_CHNS;
int LED_UNIS;
int LED_CELL;

uint16_t last_uni[] = {0, 0, 0};
int last_uni_time = 0;
int last_uni_2 = 255;

// LED Strip variables
Adafruit_NeoPixel strip = Adafruit_NeoPixel(MAX_LED, PIN, NEO_GRB + NEO_KHZ800);


void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Set up Art-net
  artnet.begin(mac, ip);
  if (LED_MODE == 1) {
    LED_CHNS = 3;
    LED_UNIS = 2;
    LED_CELL = 1;
  } else if (LED_MODE == 2) {
    LED_CHNS = 4;
    LED_UNIS = 3;
    LED_CELL = 1;
  } else if (LED_MODE == 3) {
    LED_CHNS = 4;
    LED_UNIS = 1;
    LED_CELL = 3;
  }

  // Set up the LED Strip
  strip.begin();
  strip.setBrightness(255);
  strip_off();
}

void loop() {
  while (1) {
    if (artnet.read() == ART_DMX) {

      // Get relevant packet data
      uint16_t universe = artnet.getUniverse() - ART_START_UNI;
      // Check universe is one we care about

      Serial.println(universe);
      if (0 > universe || universe >= LED_UNIS) {
        continue;
      }

      for ( int i = 0; i < 3; i++) {
        Serial.print(last_uni[i]);
        Serial.print(" ");
      }
      Serial.println(millis());
      Serial.println();

      uint16_t secs = millis() + 50;

      Serial.println(last_uni[universe] - secs);

      if (last_uni[universe] < (millis() + 50)) {
        continue;
      }
      
      last_uni[universe] = millis();

      Serial.print(millis());
      Serial.print(" - ");
      Serial.print(universe);
      Serial.println();

      uint16_t length = artnet.getLength();
      uint8_t* data = artnet.getDmxFrame();

      // Work out which part of the strip we're dealing with
      int uni_offset = universe * (512 / LED_CHNS);

      // Pull out the data and set the LED's
      int i_limit = length / LED_CHNS;
      for (int i = 0 ; i < i_limit ; i++)
      {
        int j = i * LED_CHNS;
        int k = (i + uni_offset) * LED_CELL;
        if (k >= MAX_LED) {
          continue;
        }

        int r = data[j];
        int g = data[j + 1];
        int b = data[j + 2];

        int d = 255;

        if (LED_MODE == 2 || LED_MODE == 3) {
          d = data[j + 3];
        }
        // Loop for if it's a multi LED cell mode
        for (int c = LED_CELL - 1; c >= 0; c--) {
          strip.setPixelColor(k + c, (r * d) / 255, (g * d) / 255, (b * d) / 255);
        }
      }
      strip.show();
    }
  }
}

void strip_off() {
  // Turn the strip off
  strip_color(0, 0, 0);
}

void strip_white() {
  // Put the strip to white
  strip_color(255, 255, 255);
}

void strip_color(uint8_t r, uint8_t g, uint8_t b) {
  // Set the entire strip to the same colour
  uint32_t c = strip.Color(r, g, b);
  strip_color(c);
}

void strip_color(uint32_t c) {
  // Set the entire strip to the same colour
  for (uint16_t i = 0; i < MAX_LED; i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
}

