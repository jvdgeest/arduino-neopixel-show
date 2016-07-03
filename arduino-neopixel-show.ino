#include <Adafruit_NeoPixel.h>

enum pattern { NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE };
enum direction { FORWARD, REVERSE };
 
class NeoPatterns : public Adafruit_NeoPixel {
  public:
  pattern ActivePattern; // which pattern is running
  direction Direction; // direction to run the pattern
  
  unsigned long Interval; // milliseconds between updates
  unsigned long lastUpdate; // last update of position
  
  uint32_t Color1, Color2; // What colors are in use
  uint16_t TotalSteps; // total number of steps in the pattern
  uint16_t Index; // current step within the pattern
  
  void (*OnComplete)(); // Callback on completion of pattern
  
  NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
  :Adafruit_NeoPixel(pixels, pin, type) {
    OnComplete = callback;
  }
  
  void Update() {
    if ((millis() - lastUpdate) > Interval) {
      lastUpdate = millis();
      switch (ActivePattern) {
        case RAINBOW_CYCLE:
          RainbowCycleUpdate();
          break;
        case THEATER_CHASE:
          TheaterChaseUpdate();
          break;
        case COLOR_WIPE:
          ColorWipeUpdate();
          break;
        case SCANNER:
          ScannerUpdate();
          break;
        case FADE:
          FadeUpdate();
          break;
        default:
          break;
      }
    }
  }

  void Increment() {
    if (Direction == FORWARD) {
      Index++;
      if (Index >= TotalSteps) {
        Index = 0;
        if (OnComplete != NULL) {
          OnComplete();
        }
      }
    } else {
      --Index;
      if (Index <= 0) {
        Index = TotalSteps-1;
        if (OnComplete != NULL) {
          OnComplete(); // call the comlpetion callback
        }
      }
    }
  }
  
  void Reverse() {
    if (Direction == FORWARD) {
      Direction = REVERSE;
      Index = TotalSteps-1;
    } else {
      Direction = FORWARD;
      Index = 0;
    }
  }
  
  void RainbowCycle(uint8_t interval, direction dir = FORWARD) {
    ActivePattern = RAINBOW_CYCLE;
    Interval = interval;
    TotalSteps = 255;
    Index = 0;
    Direction = dir;
  }
  
  void RainbowCycleUpdate() {
    for (int i=0; i< numPixels(); i++) {
      setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
    }
    show();
    Increment();
  }

  void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, direction dir = FORWARD) {
    ActivePattern = THEATER_CHASE;
    Interval = interval;
    TotalSteps = numPixels();
    Color1 = color1;
    Color2 = color2;
    Index = 0;
    Direction = dir;
  }

  void TheaterChaseUpdate() {
    for (int i = 0; i < numPixels(); i++) {
      if ((i + Index) % 3 == 0) {
        setPixelColor(i, Color1);
      } else {
        setPixelColor(i, Color2);
      }
    }
    show();
    Increment();
  }

  void ColorWipe(uint32_t color, uint8_t interval, direction dir = FORWARD) {
    ActivePattern = COLOR_WIPE;
    Interval = interval;
    TotalSteps = numPixels();
    Color1 = color;
    Index = 0;
    Direction = dir;
  }
  
  void ColorWipeUpdate() {
    setPixelColor(Index, Color1);
    show();
    Increment();
  }
  
  void Scanner(uint32_t color1, uint8_t interval) {
    ActivePattern = SCANNER;
    Interval = interval;
    TotalSteps = (numPixels() - 1) * 2;
    Color1 = color1;
    Index = 0;
  }

  void ScannerUpdate() { 
    for (int i = 0; i < numPixels(); i++) {
      if (i == Index) {
        setPixelColor(i, Color1); // Scan Pixel to the right
      } else if (i == TotalSteps - Index) {
        setPixelColor(i, Color1); // Scan Pixel to the left
      } else {
        setPixelColor(i, DimColor(getPixelColor(i))); // Fading tail
      }
    }
    show();
    Increment();
  }
  
  void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD) {
    ActivePattern = FADE;
    Interval = interval;
    TotalSteps = steps;
    Color1 = color1;
    Color2 = color2;
    Index = 0;
    Direction = dir;
  }

  void FadeUpdate() {
    // Calculate linear interpolation between Color1 and Color2
    // Optimise order of operations to minimize truncation error
    uint16_t FadeSteps = TotalSteps / 2;
    uint16_t FadeIndex = Index > FadeSteps ? TotalSteps - Index : Index;
    
    uint8_t red = ((Red(Color1) * (FadeSteps - FadeIndex)) + (Red(Color2) * FadeIndex)) / FadeSteps;
    uint8_t green = ((Green(Color1) * (FadeSteps - FadeIndex)) + (Green(Color2) * FadeIndex)) / FadeSteps;
    uint8_t blue = ((Blue(Color1) * (FadeSteps - FadeIndex)) + (Blue(Color2) * FadeIndex)) / FadeSteps;
    
    ColorSet(Color(red, green, blue));
    show();
    Increment();
  }
 
  // Calculate 50% dimmed version of a color (used by ScannerUpdate)
  uint32_t DimColor(uint32_t color) {
    // Shift R, G and B components one bit to the right
    uint32_t dimColor = Color(Red(color) * 0.85, Green(color) * 0.85, Blue(color) * 0.85);
    return dimColor;
  }

  // Set all pixels to a color (synchronously)
  void ColorSet(uint32_t color) {
    for (int i = 0; i < numPixels(); i++) {
      setPixelColor(i, color);
    }
    show();
  }

  // Returns the Red component of a 32-bit color
  uint8_t Red(uint32_t color) {
    return (color >> 16) & 0xFF;
  }

  // Returns the Green component of a 32-bit color
  uint8_t Green(uint32_t color) {
    return (color >> 8) & 0xFF;
  }

  // Returns the Blue component of a 32-bit color
  uint8_t Blue(uint32_t color) {
    return color & 0xFF;
  }
  
  // Input a value 0 to 255 to get a color value.
  // The colours are a transition r - g - b - back to r.
  uint32_t Wheel(byte WheelPos) {
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85) {
      return Color(255 - WheelPos * 3, 0, WheelPos * 3);
    } else if(WheelPos < 170) {
      WheelPos -= 85;
      return Color(0, WheelPos * 3, 255 - WheelPos * 3);
    } else {
      WheelPos -= 170;
      return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
  }
};

#define BUTTON_PIN    3
#define PIXEL_PIN     2 
#define PIXEL_COUNT   55
#define SHOW_COUNT    11
#define RANDOM_COUNT  3

int randomRounds = 0;       // rounds that have completed for random show
bool randomMode = false;    // randomly go through shows
int currentShow = 0;        // the current show
int buttonState;            // the current reading from the input pin
int lastButtonState = LOW;  // the previous reading from the input pin
long lastPressTime = 0;     // the last time the button was pressed
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers

void LedComplete();
NeoPatterns Led(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800, &LedComplete);
 
void setup() {
  pinMode(BUTTON_PIN, INPUT);
  Led.begin();
  startShow(currentShow);
}

void loop() {
  Led.Update(); 
  
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) {
        lastPressTime = millis();
        randomMode = false;
        nextShow();
      }
    } else if (reading == HIGH && (millis() - lastPressTime) > 2500) {
      if ((millis() - lastPressTime) > 5000) {
        startShow(0);
        randomMode = false;
        lastPressTime = millis();
      } else if (!randomMode) {
        randomRounds = 0;
        randomMode = true;
        nextShow();
      }
    }
  }
  
  lastButtonState = reading;
}

void nextShow() {
  currentShow++;
  if (currentShow > SHOW_COUNT - 1) {
    currentShow = 1;
  }
  startShow(currentShow);
}

void startShow(int index) {
  currentShow = index;
  switch (index) {
    case 0:
      Led.ColorWipe(Led.Color(0, 0, 0), 10); // Black (off)
      break;
    case 1:
      Led.TheaterChase(Led.Color(0, 0, 0), Led.Color(0, 255, 255), 100);
      break;
    case 2:
      Led.RainbowCycle(30);
      break;
    case 3:
      Led.Fade(Led.Color(255, 255, 255), Led.Color(0, 255, 255), 100, 100);
      break;
    case 4:
      Led.Scanner(Led.Color(255, 0, 0), 55);
      break;
    case 5:
      Led.ColorWipe(Led.Color(0, 255, 255), 50); // Light blue
      break;
    case 6:
      Led.ColorWipe(Led.Color(255, 0, 0), 50); // Red
      break;
    case 7:
      Led.ColorWipe(Led.Color(0, 255, 0), 50); // Green
      break;
    case 8:
      Led.ColorWipe(Led.Color(0, 0, 255), 50); // Blue
      break;
    case 9:
      Led.ColorWipe(Led.Color(255, 0, 255), 50); // Pink
      break;
    case 10:
      Led.ColorWipe(Led.Color(255, 255, 255), 50); // White
      break;
  }
}

void LedComplete() {
  if (randomMode) {
    randomRounds++;
    if (randomRounds > RANDOM_COUNT) {
      randomRounds = 0;
      nextShow();
    }
  }
}
