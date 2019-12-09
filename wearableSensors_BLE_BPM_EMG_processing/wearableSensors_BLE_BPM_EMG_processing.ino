/*********************************************************************
 This is an example for our nRF51822 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/
////////////BPM///////////////
#define USE_ARDUINO_INTERRUPTS true    // Set-up low-level interrupts for most acurate BPM math.
#include <PulseSensorPlayground.h>     // Includes the PulseSensorPlayground Library.  

//////////////////////////////

#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

/*=========================================================================
    APPLICATION SETTINGS

    FACTORYRESET_ENABLE       Perform a factory reset when running this sketch
   
                              Enabling this will put your Bluefruit LE module
                              in a 'known good' state and clear any config
                              data set in previous sketches or projects, so
                              running this at least once is a good idea.
   
                              When deploying your project, however, you will
                              want to disable factory reset by setting this
                              value to 0.  If you are making changes to your
                              Bluefruit LE device via AT commands, and those
                              changes aren't persisting across resets, this
                              is the reason why.  Factory reset will erase
                              the non-volatile memory where config data is
                              stored, setting it back to factory default
                              values.
       
                              Some sketches that require you to bond to a
                              central device (HID mouse, keyboard, etc.)
                              won't work at all with this feature enabled
                              since the factory reset will clear all of the
                              bonding data stored on the chip, meaning the
                              central device won't be able to reconnect.
    MINIMUM_FIRMWARE_VERSION  Minimum firmware version to have some new features
    MODE_LED_BEHAVIOUR        LED activity, valid options are
                              "DISABLE" or "MODE" or "BLEUART" or
                              "HWUART"  or "SPI"  or "MANUAL"
    -----------------------------------------------------------------------*/
    #define FACTORYRESET_ENABLE         1
    #define MINIMUM_FIRMWARE_VERSION    "0.6.6"
    #define MODE_LED_BEHAVIOUR          "MODE"
/*=========================================================================*/
//  Variables
const int PulseWire = A7;       // PulseSensor PURPLE WIRE connected to ANALOG PIN A7
//const int LED13 = 13;          // The on-board Arduino LED, close to PIN 13.
int Threshold = 550;           // Determine which Signal to "count as a beat" and which to ignore.
                               // Use the "Gettting Started Project" to fine-tune Threshold Value beyond default setting.
                               // Otherwise leave the default "550" value. 
const int analogInPin = A11;
int sensorValue = 0;
int outputValue = 0;
                               
PulseSensorPlayground pulseSensor;  // Creates an instance of the PulseSensorPlayground object called "pulseSensor" 
// Create the bluefruit object, either software serial...uncomment these lines
/*
SoftwareSerial bluefruitSS = SoftwareSerial(BLUEFRUIT_SWUART_TXD_PIN, BLUEFRUIT_SWUART_RXD_PIN);

Adafruit_BluefruitLE_UART ble(bluefruitSS, BLUEFRUIT_UART_MODE_PIN,
                      BLUEFRUIT_UART_CTS_PIN, BLUEFRUIT_UART_RTS_PIN);
*/

/* ...or hardware serial, which does not need the RTS/CTS pins. Uncomment this line */
Adafruit_BluefruitLE_UART ble(Serial1, BLUEFRUIT_UART_MODE_PIN);

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
//Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/* ...software SPI, using SCK/MOSI/MISO user-defined SPI pins and then user selected CS/IRQ/RST */
//Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_SCK, BLUEFRUIT_SPI_MISO,
//                             BLUEFRUIT_SPI_MOSI, BLUEFRUIT_SPI_CS,
//                             BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);


// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module (this function is called
            automatically on startup)
*/
/**************************************************************************/

int bpmWin [20] = { };
int emgWin [20] = { };
int count = 20;
int bpmTemp = 0;
int emgTemp = 0;
int i = 0;
int j = 0;
int k = 0;
int bpmAvg = 0;
int emgAvg = 0;
int bpmRange = 0;
int emgRange = 0;

void setup(void)
{
  while (!Serial);  // required for Flora & Micro
  delay(500);

  Serial.begin(115200);

  //////////////BPM//////////////////
  // Configure the PulseSensor object, by assigning our variables to it. 
  pulseSensor.analogInput(PulseWire);   
  //pulseSensor.blinkOnPulse(LED13);       //auto-magically blink Arduino's LED with heartbeat.
  pulseSensor.setThreshold(Threshold);   

  // Double-check the "pulseSensor" object was created and "began" seeing a signal. 
   if (pulseSensor.begin()) {
    Serial.println("We created a pulseSensor Object !");  //This prints one time at Arduino power-up,  or on Arduino reset.  
  }
  ///////////////////////////////////
  
  Serial.println(F("Adafruit Bluefruit Command Mode Example"));
  Serial.println(F("---------------------------------------"));

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in UART mode"));
  Serial.println(F("Then Enter characters to send to Bluefruit"));
  Serial.println();

  ble.verbose(false);  // debug info is a little annoying after this point!

  /* Wait for connection */
  while (! ble.isConnected()) {
      delay(500);
  }

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    Serial.println(F("******************************"));
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
    Serial.println(F("******************************"));
  }

  // HERE, SET UP CALIBRATION WINDOW
  // Window should last for 40 seconds (20 data collections, .5 Hz)
  Serial.println(("BEGINNING CALIBRATION"));
  Serial.println(("Please wait 40 seconds..."));
  
  for(i = 0; i < count; i++)
  {
    bpmTemp = pulseSensor.getBeatsPerMinute();
    emgTemp = analogRead(analogInPin);
    bpmWin[i] = bpmTemp;
    emgWin[i] = emgTemp;
    delay(2000);  //wait 2 sec
  }
  Serial.println(("Calibration Complete!"));
  //print BPM array for validation
  Serial.print(("BMP Window: "));
  for(j = 0; j < count; j++);
  {
    Serial.println(bpmWin[j]);
  }
  //print EMG array for validation
  Serial.print(("EMG Window: "));
  for(k = 0; k < count; k++);
  {
    Serial.println(emgWin[k]);
  }

  //calculate average value for each window
  //in implementation, if there are more than n data values past a certain threshold greater than the calibrated avg, send a warning (in a window of 20, if 10 points are greater than the avg by 50%, send warning)
  for(i = 0; i < count; i++)
  {
    bpmAvg = bpmAvg + bpmWin[i];
    emgAvg = emgAvg + emgWin[i];
  }
  bpmAvg = bpmAvg / count;
  emgAvg = emgAvg / count;
  Serial.print(("Avg BPM: "));
  Serial.println(bpmAvg);
  Serial.print(("Avg EMG: "));
  Serial.println(emgAvg);

  //calculate tolerance by finding the range between the max and min values for both BPM and EMG windows
  int bpmMax = bpmWin[0];
  int bpmMin = bpmWin[0];
  int emgMax = emgWin[0];
  int emgMin = emgWin[0];
  for(i = 0; i < count; i++)
  {
    if(bpmWin[i] > bpmMax)
    {
      bpmMax = bpmWin[i];
    }
    if(bpmWin[i] < bpmMin)
    {
      bpmMin = bpmWin[i];
    }
    if(emgWin[i] > emgMax)
    {
      emgMax = emgWin[i];
    }
    if(emgWin[i] < emgMin)
    {
      emgMin = emgWin[i];
    }
  }
  bpmRange = bpmMax - bpmMin;
  emgRange = emgMax - emgMin;
  
}

/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data
*/
/**************************************************************************/
void loop(void)
{
  //NEW PROCESSING WITH WINDOW (still window of 40 sec / 20 points) (uses the calibration array)
  //Single value will be sent over bluetooth (derived from EMG and BPM)
  int myBPM = pulseSensor.getBeatsPerMinute();
  int myEMG = analogRead(analogInPin);
  //shift all values in the window down by one, then add the new value to the end
  i = 0;
  for(i = 0; i < count-1; i++)
  {
    bpmWin[i] = bpmWin[i+1];
    emgWin[i] = emgWin[i+1];
  }
  bpmWin[count-1] = myBPM;
  emgWin[count-1] = myEMG;
  //check how many values are outside of the acceptable bounds for each array
  int bpmCount = 0;
  int emgCount = 0;
  for(i = 0; i < count; i++)
  {
    if(bpmWin[i] > bpmAvg + bpmRange)
    {
      bpmCount = bpmCount + 1;
    }
    if(emgWin[i] > emgAvg + emgRange)
    {
      emgCount = emgCount + 1;
    }
  }
  Serial.println(bpmCount);
  Serial.println(emgCount);
  //evaluate number of unacceptable values in current window and set message to user
  char result[BUFSIZE +  1];
  bool trigger = false;
  if((bpmCount >= 2) && (emgCount >= 2))
  {
    strcpy(result, "HALP");
    trigger = true;
  }
  
  // Check for user input
//  int myBPM = pulseSensor.getBeatsPerMinute();
//  int myEMG = analogRead(analogInPin);
//  char inputs[BUFSIZE+1];
//  char inputs2[BUFSIZE+1];
//  char sBPM[4];
//  char sEMG[4];
  //bool trigger = false;
  
//  if (pulseSensor.sawStartOfBeat()) {          
//    trigger = true;
//    strcpy(inputs, " ");
//    strcpy(inputs2,",");
//    strcat(inputs, itoa(myBPM, sBPM, 10));
//    strcat(inputs2, itoa(myEMG, sEMG, 10));
//    strcat(inputs,inputs2);
//    //Serial.println(inputs);
//  }

  

  //if ( getUserInput(inputs, BUFSIZE, trigger) )
  if(trigger == true)
  {
    // Send characters to Bluefruit
    Serial.print("[Send] ");
    delay(2000);
    Serial.println(result);
    //Serial.println(inputs);
    //Serial.println(inputs2);

    ble.print("AT+BLEUARTTX=");
    ble.println(result);
    //ble.println(inputs);
    //ble.println(inputs2);

    // check response stastus
    if (! ble.waitForOK() ) {
      Serial.println(F("Failed to send?"));
    }
  }
  

  // Check for incoming characters from Bluefruit
  ble.println("AT+BLEUARTRX");
  ble.readline();
  if (strcmp(ble.buffer, "OK") == 0) {
    // no data
    return;
  }
  // Some data was found, its in the buffer
  Serial.print(F("[Recv] ")); Serial.println(ble.buffer);
  ble.waitForOK();
}

/**************************************************************************/
/*!
    @brief  Checks for user input (via the Serial Monitor)
*/
/**************************************************************************/
bool getUserInput(char buffer[], uint8_t maxSize, bool trigger)
{
  // timeout in 100 milliseconds
  TimeoutTimer timeout(100);

  memset(buffer, 0, maxSize);
  while( (!trigger == true) && !timeout.expired() ) { delay(1); }

  if ( timeout.expired() ) return false;

  delay(2);
  uint8_t count=0;
  do
  {
    count += Serial.readBytes(buffer+count, maxSize);
    delay(2);
  } while( (count < maxSize) && (trigger == true) );

  return true;
}
