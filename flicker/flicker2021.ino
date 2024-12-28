#include "chainprinter.h"
/**
 * flicker some pins.
 * todo: import unsignedrecognizer and finish live editing via serial debug. EEprom should be included else there is little value in this.
*/

#if 1 //8684/606 with debug 7616/483 without, so around 1k of debug statments 
ChainPrinter dbg(Serial, true); //true adds linefeeds to each invocation.
#else
#define dbg(...)
#endif

//eventually these will be made editable by Serial
bool dbgpin = 1;
bool dbgi2c = 0;
#include "scani2c.h"

#include "millievent.h"
#include "flickery.h"

#include "pcf8574.h"  //for remote LEDs, numbers >32

/**
  flicker a pin
*/
struct FlickeryPin: public Flickery {
  int Led_Pin;
  FlickeryPin(int apin, unsigned mintime, unsigned maxtime): Flickery(mintime,  maxtime), Led_Pin(apin) {
    //nada.
  }

  void setup() {
    //set pin as output
    pinMode(Led_Pin, OUTPUT);
    Flickery::setup();
  }

  void be(bool on) {
    Flickery::be(on);
    digitalWrite(Led_Pin, on ? HIGH : LOW);
    if (dbgpin) dbg("Pin ", Led_Pin, '=', on ? "HIGH " : "LOW ", " for ", MilliTick(duration));
  }
};

//not const so that we can manipulate from debugger
FlickeryPin led[] = {
  //station 4 will use one of these, scanner hallway two, station 9 all four
  { 10, 759, 100},
  { 9, 1250, 150},
  {LED_BUILTIN,500,500},
  { 8, 1431, 175},
  { 7, 330, 25},
};

static const int numleds = sizeof(led) / sizeof(*led);

void freezeAll() {
  for (unsigned count = numleds; count-- > 0;) {
    led[count].freeze();
  }
}

//we burned out the PCF8574 chip and instead ran another cable out of the display so that we could use the mcu gpio pins.


void setup() {
  Serial.begin(115200);//used by dbg()
  dbg("setup flickerers");
  for (unsigned count = numleds; count-- > 0;) {
    led[count].setup();
  }
  randomSeed(202142);
  dbg("setup complete");
}


unsigned testitem = 0; //index into led[]

void loop() {
  if (MilliTicked) { //only run the loop once a millisecond to lower power consumption, might be able to run on batteries if we do instead of needing another P/S.
    for (unsigned count = numleds; count-- > 0;) {
      led[count].onTick();
    }
  }
  if (Serial) {
    int key = Serial.read();
    if (key > 0) {
      switch (tolower(key)) {
        case '-':
          led[testitem].be(false);
          break;
        case '=':
          led[testitem].be(true);
          break;
        case '!': //stop activity so that we can test individual events
          freezeAll();
          break;
        case '@': //soft reset
          setup();
          break;
        case 'p':
          dbgpin = isupper(key);
          break;
        case 'i':
          dbgi2c = isupper(key);
          break;
        case '?':
          if(dbgi2c){
            scanI2C(Serial);
          }
          break;
        default:
          if (key >= '0' && key < '0' + numleds) {
            testitem = key - '0';
            dbg("selected:", testitem);
          } else {
            dbg("Key:", key);
          }
          break;
      }
    }

  }
}
