#include <dht_nonblocking.h>

#include <bitswap.h>
#include <chipsets.h>
#include <color.h>
#include <colorpalettes.h>
#include <colorutils.h>
#include <controller.h>
#include <cpp_compat.h>
#include <dmx.h>
#include <FastLED.h>
#include <fastled_config.h>
#include <fastled_delay.h>
#include <fastled_progmem.h>
#include <fastpin.h>
#include <fastspi.h>
#include <fastspi_bitbang.h>
#include <fastspi_dma.h>
#include <fastspi_nop.h>
#include <fastspi_ref.h>
#include <fastspi_types.h>
#include <hsv2rgb.h>
#include <led_sysdefs.h>
#include <lib8tion.h>
#include <noise.h>
#include <pixelset.h>
#include <pixeltypes.h>
#include <platforms.h>
#include <power_mgt.h>


/// HUMIDITY AND TEMPERATURE 
#define DHT_SENSOR_TYPE DHT_TYPE_11

static const int DHT_SENSOR_PIN = 2;
DHT_nonblocking dht_sensor( DHT_SENSOR_PIN, DHT_SENSOR_TYPE );
/// HUMIDITY AND TEMPERATURE 

#define BUTTON_PIN 5
#define RPM_PIN 6
#define LED_PIN     7
#define NUM_LEDS    6
#define animationTime 100
#define animationTime2 1

const int potentio_pin = A7; // use analog pin to read input from potentiometer

CRGB leds[NUM_LEDS];

int offset = 0;
int offset2 = 0;

int mode = 0;

unsigned long cTime; // curent time
unsigned long pTime; // previous time
unsigned long dTime; // delta time

#define STEPS 50
CRGB rainbow[NUM_LEDS];

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

  rainbowColors2();

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(RPM_PIN, OUTPUT);
  pinMode(potentio_pin, INPUT);
  
  analogWrite(RPM_PIN, 100);
  
  cTime = millis();
  pTime = cTime;
}


void rainbowColors1(){
  rainbow[0] = CRGB(255,0,0);
  rainbow[1] = CRGB(255,128,0);
  rainbow[2] = CRGB(255,255,0);
  rainbow[3] = CRGB(0,255,0);
  rainbow[4] = CRGB(0,0,255);
  rainbow[5] = CRGB(255,0,128); 
}

void rainbowColors2(){
  rainbow[0] = CRGB(255,0,0);
  rainbow[1] = CRGB(232,00,0);
  rainbow[2] = CRGB(150,0,0);
  rainbow[3] = CRGB(120,0,0);
  rainbow[4] = CRGB(80,0,00);
  rainbow[5] = CRGB(0,0,0); 
}

int lastButtonState = HIGH;

int lastPotValue = 0;

/*
 * Poll for a measurement, keeping the state machine alive.  Returns
 * true if a measurement is available.
 */
static bool measure_environment( float *temperature, float *humidity )
{
  static unsigned long measurement_timestamp = millis( );
  /* Measure once every four seconds. */
  if( millis( ) - measurement_timestamp > 3000ul )
  {
    
    if( dht_sensor.measure( temperature, humidity ) == true )
    {
      measurement_timestamp = millis( );
      return( true );
    }
  }

  return( false );
}


void loop() {
  cTime = millis();
  dTime = cTime - pTime;

  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState != lastButtonState && buttonState == LOW){
    mode = (mode + 1)%2;
      switch(mode){
      case 1:
        rainbowColors1();
        break;
      case 0: 
        rainbowColors2();
        break;
      }

      
  }
  lastButtonState = buttonState;

  rainbowMode2();

  int value = analogRead(potentio_pin);          //Read and save analog value from potentiometer
//  Serial.print("Value: ");
//  Serial.print(value);
  value = map(value, 0, 1023, 0, 255); //Map value 0-1023 to 0-255 (PWM)
//  Serial.print(" - Value mapped: ");
//  Serial.println(value);
  if (value != lastPotValue){
    if (value < 15)
      analogWrite(RPM_PIN, 0);
    else
      analogWrite(RPM_PIN, value);          //Send PWM value to led
  }
  lastPotValue = value;


  float temperature;
  float humidity;

  /* Measure temperature and humidity.  If the functions returns
     true, then a measurement is available. */
  if( measure_environment( &temperature, &humidity ) == true )
  {
    Serial.print( "T = " );
    Serial.print( temperature, 1 );
    Serial.print( " deg. C, H = " );
    Serial.print( humidity, 1 );
    Serial.println( "%" );
  }
  
  pTime = cTime;


  
}

#define RAINBOW_TIME 200
unsigned long runningTime;

void rainbowMode2(){
  runningTime += dTime;

  if (runningTime > RAINBOW_TIME){
    offset++;
    if (offset == NUM_LEDS)
      offset = 0;

     runningTime = runningTime % RAINBOW_TIME;
  }
  float transitionFactor = runningTime *1.0f / RAINBOW_TIME;
  for (int i = 0; i < NUM_LEDS; i++) {
    CRGB cur = rainbow[(i + offset) % NUM_LEDS];
    CRGB next = rainbow[(i+1+offset) % NUM_LEDS];
    CRGB transition = calculateTransition(next, cur, transitionFactor);
    
    leds[i] = transition;
  }
  FastLED.show();
}

CRGB calculateTransition(CRGB next, CRGB cur, float transitionFactor){
  return CRGB(cur.red + (next.red - cur.red)* transitionFactor,cur.green + (next.green - cur.green) * transitionFactor,cur.blue + (next.blue - cur.blue) * transitionFactor);
}


void rainbowMode(){
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[(i + offset) % NUM_LEDS] = rainbow[i];
    //leds[(i)] = rainbow[i];
  }
  FastLED.show();
  delay(animationTime);
  offset++;

  if (offset == NUM_LEDS)
    offset = 0;
}

void mode1(){
  // put your main code here, to run repeatedly:
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB ( 0, 0255, 0);
    FastLED.show();
    delay(animationTime);
  }
    for (int i = 0; i < NUM_LEDS; i++) {
//  for (int i = NUM_LEDS; i >= 0; i--) {
    leds[i] = CRGB ( 255, 0, 0);
    FastLED.show();
    delay(animationTime);
  }
}
