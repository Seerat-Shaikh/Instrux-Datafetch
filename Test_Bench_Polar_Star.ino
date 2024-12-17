//Instrux 2.0 - for ADVICE
//For Wemos D1 Mini Board
//Author: M. Wasiq Pervez
#include <SoftwareSerial.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ModbusMaster.h>
#include <HardwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <string.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiManager.h>
#include "RTClib.h"
#include <stdint.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <TimeLib.h>

const char *host = "instrux.live/";

SoftwareSerial Meter_Port(2, 0); //RX, TX
//Adafruit_MPU6050 mpu;
//
File dataFile;
RTC_DS3231 rtc;

//WiFiClient client;
ESP8266WiFiMulti WiFiMulti;
typedef int                 INT32;
typedef short int           INT16;
typedef unsigned short int  UINT16;
typedef unsigned long int   UINT32;
#include "MeterConfig.h";
#include "Parameters_var.h"

#define RXPin        D5  // Serial Receive pin (not used)
#define TXPin        D6  // Serial Transmit pin (not used)

char server[] = "instrux.live";

#if ___PM2120 == 1
#include "Meters_Registers_Library/PM2120.h";
#endif

#if ___Ci3_01 == 1
#include "Meters_Registers_Library/Ci3_01.h";
#endif

#if ___EM_3490_SS == 1
#include "Meters_Registers_Library/EM_3490_SS.h";
#endif

#if ___MFM_376 == 1
#include "Meters_Registers_Library/MFM_376.h";
#endif

#if ___LG5310 == 1
#include "Meters_Registers_Library/LG5310.h";
#endif

#if ___POLARSTAR == 1
#include "POLARSTAR.h";
#endif

#define Serial_Monitor Serial
#define MAX485_DE 9   //(not used)
#define MAX485_RE 10  //(not used)
#define DataSending_Interval 5000   //ms

#define SerialMonitor 1
#define DebugMonitor 0

#if SerialMonitor == 1
#define Sprintln(a) (Serial_Monitor.println(a))
#define Sprint(a) (Serial_Monitor.print(a))
#else
#define Sprintln(a)
#define Sprint(a)
#endif


String DataToSend = " ";

boolean newData = false;
boolean newDataComplete = false;

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

String strArr[5];

//-----------------------------------------------
//-----------------------------------------------*/

String ReceivedCommand = "";

bool Values_Display = 0;

//{_PM2120, 5, 9600, SERIAL_8N1}
//
//{_PM2120, 3, 19200, SERIAL_8N1} at Rainbow
//{_Ci3_01, 1, 9600, SERIAL_8E1}
//{_LG5310, 1, 9600, SERIAL_8E1}
//{_MFM_376, 2, 9600, SERIAL_8N1}
//{_EM_3490_SS, 3, 19200, SERIAL_8N1}
//int ConnectedMeters[][4]    = {{_MFM_376, 2, 9600, SWSERIAL_8N1}};  // { {Meter Model, Meter ID, Baud Rate, Serial Config (databits, parity, stop bit)} , .... }
//int ConnectedMeters[][4]    = {{_PM2120, 4, 9600, SWSERIAL_8E1}};  // { {Meter Model, Meter ID, Baud Rate, Serial Config (databits, parity, stop bit)} , .... }
//int ConnectedMeters[][4]    = {{_PM2120, 4, 9600, SWSERIAL_8E1}};  // { {Meter Model, Meter ID, Baud Rate, Serial Config (databits, parity, stop bit)} , .... }
int ConnectedMeters[][4]    = {{_POLARSTAR, 1, 9600, SWSERIAL_8E1}};  // { {Meter Model, Meter ID, Baud Rate, Serial Config (databits, parity, stop bit)} , .... }


int NumberOfMeters = 0;
int NumberOfParameters = 0;
int MeterNo = 0;

String URLToSend = "";

uint8_t j, result;
int16_t datas[2];

#define NumberOfMeters (sizeof(ConnectedMeters) / sizeof(int)) / 4

float Acquired_Data[NumberOfMeters][140];

#define ParameterFailureValue 77.7

byte i = 0; //counter
char dataStr[100] = "";
char buffer[7];
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const int chipSelect = D8;
String dataString = "";
char fileName[16];
char VibFileName[16];
int counts = 0;

//Changes----------------------
const char* ssid = "OPPO A15";            // Replace with your Wi-Fi SSID
const char* password = "Seerat123";     // Replace with your Wi-Fi password

// Initialize NTP client to get time from server
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 5 * 3600, 60000); // NTP server, UTC offset (in seconds), update interval (1 minute)
//////////////////////////


ModbusMaster node;

UINT32 BCDToDecimal(UINT32 nDecimalValue) {
  UINT32 nResult = 0;
  INT32  nPartialRemainder, ncnt, anHexValueStored[8];
  UINT16 unLengthOfHexString = 0, unflag = 0;
  for (ncnt = 7 ; ncnt >= 0 ; ncnt--) {
    anHexValueStored[ncnt] = nDecimalValue & (0x0000000f << 4 * (7 - ncnt));
    anHexValueStored[ncnt] = anHexValueStored[ncnt] >> 4 * (7 - ncnt);
    if (anHexValueStored[ncnt] > 9)
      unflag = 1;
  }
  if (unflag == 1) {
    return 0;
  }
  else {
    for (ncnt = 0 ; ncnt < 8 ; ncnt++) {

      nResult = nResult + anHexValueStored[ncnt] * round(pow(10, (7 - ncnt)));

    }
    return nResult;
  }
}

int binaryToDecimal(int n)
{
  int num = n;
  int dec_value = 0;

  // Initializing base value to 1, i.e 2^0
  int base = 1;

  int temp = num;
  while (temp) {
    int last_digit = temp % 10;
    temp = temp / 10;

    dec_value += last_digit * base;

    base = base * 2;
  }

  return dec_value;
}

void RTCtime() {
//
  DateTime now = rtc.now();

  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.print(':');
  Serial.print(millis());
  Serial.println();
//
 dataFile = SD.open(fileName, FILE_WRITE);
 if (dataFile) {
  dataFile.print(now.year(), DEC);
  dataFile.print('/');
  dataFile.print(now.month(), DEC);
  dataFile.print('/');
  dataFile.print(now.day(), DEC);
  dataFile.print(',');
  dataFile.print(now.hour(), DEC);
  dataFile.print(':');
  dataFile.print(now.minute(), DEC);
  dataFile.print(':');
  dataFile.print(now.second(), DEC);
  dataFile.print(",");
  dataFile.close();
 }
  dataFile = SD.open(VibFileName, FILE_WRITE);
  if (dataFile) {
    dataFile.print(now.year(), DEC);
    dataFile.print('/');
    dataFile.print(now.month(), DEC);
    dataFile.print('/');
    dataFile.print(now.day(), DEC);
    dataFile.print(',');
    dataFile.print(now.hour(), DEC);
    dataFile.print(':');
    dataFile.print(now.minute(), DEC);
    dataFile.print(':');
    dataFile.print(now.second(), DEC);
    dataFile.print(":");
    dataFile.print(millis());
    dataFile.print(",");
    dataFile.close();
  }
}



//void LocalData() {
//
//  dataFile = SD.open(fileName, FILE_WRITE);
//
//  if (dataFile) {
//    dataFile.println(dataString);
//    dataFile.close();
//    // print to the serial port too:
//    Serial.println("dataString saved in SD Card");
//    Serial.println(dataString);
//    dataString = "";
//  }
//  // if the file isn't open, pop up an error:
//  else {
//    Serial.println("error opening datalog.txt");
//  }
//}

//void(* resetFunc) (void) = 0;

//Changes comment kr rhey isko wesye yeh uncomment tha
/*void SendData_WiFi() {

  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;

    Serial.println("//-------------------------");
    Serial.println("[HTTP] begin...");
    Serial.println(URLToSend);

    if (http.begin(client, URLToSend)) {  // HTTP

      //http.addHeader("Content-Type", "text/plain");
      //http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      Serial.print("[HTTP] GET...\n");
      int httpCode = http.GET();
      // int httpCode=http.POST(URLToSend);

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.print("Payload= ");
          Serial.println(payload);

        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        Serial.println("\n-------------> Rebooting ESP <-------------\n");
        delay(500);
        //ESP.reset();
      }

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }

    Serial.println("//-------------------------");
    newData = false;
    DataToSend = "";
  }
  else {
    Serial.println("\n------------> WiFi not Connected <------------\n");
    Serial.print("Reset_MCU#");
    delay(500);
    Serial.print("Reset_MCU#");
    Serial.println();
    WiFi.mode(WIFI_STA);
    delay(2000);
    if (counts == 4) {
      counts = 0;
      ReconnectWiFi();
      delay(1000);
      if ((WiFiMulti.run() != WL_CONNECTED)) {
        ESP.reset();
      }
    }
    counts = counts + 1;
    Serial.println(counts);

  }
}

void ReconnectWiFi() {
  if ((WiFiMulti.run() != WL_CONNECTED)) {
    WiFi.begin(WiFi_SSID, WiFi_PSWD);
    Serial.print("\nReconnecting to SSID= ");
    Serial.print(WiFi_SSID);
    Serial.print(", Pass= ");
    Serial.print(WiFi_PSWD);
  }
}

void WriteEEPROM() {
  if (ssid1 != WiFi.SSID() || password1 != WiFi.psk() || ssid1 == "") {
    Serial.println("Writing to EEPROM");

    for (int w = 0; w < WiFi_SSID.length(); ++w)
    {
      EEPROM.write(w, WiFi_SSID[w]);
      Serial.println("Wrote ");
      Serial.print(WiFi_SSID[w]);
      //  id+=WiFi_SSID[w];
    }

    for (int p = 0; p < WiFi_PSWD.length(); p++)
    {
      EEPROM.write(32 + p, WiFi_PSWD[p]);
      Serial.println("Wrote ");
      Serial.print(WiFi_PSWD[p]);
      // pass+=WiFi_PSWD[p];
    }
    EEPROM.commit();
  }
}

void ReadEEPROM() {
  Serial.println("Reading SSID and Password");

  ssid1 = "";
  password1 = "";
  //Reading EEPROM SSID
  for (int k = 0; k < WiFi_SSID.length(); ++k)
  {
    ssid1 += char(EEPROM.read(k));
  }
  Serial.print("SSID: ");
  Serial.println(ssid1);

  //Reading EEPROM PASSWORD
  for (int l = 32; l < WiFi_PSWD.length() + 32; ++l)
  {
    password1 += char(EEPROM.read(l));
  }
  Serial.print("PASSWORD: ");
  Serial.println(password1);
}
void preTransmission()
{
  digitalWrite(MAX485_DE, 1);
  digitalWrite(MAX485_RE, 1);
}

void postTransmission()
{
  digitalWrite(MAX485_DE, 0);
  digitalWrite(MAX485_RE, 0);
}*/

//iskey upar tk ka comment kiya


//changes
// Last NTP sync attempt time
unsigned long lastSyncAttempt = 0;
const unsigned long syncInterval = 60000; // Sync interval 1 minutes in milliseconds

void syncTimeWithNTP();
///////////////////////////


void setup() {

  //Changes---------------------------------
   // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected to WiFi!");

  // Initialize NTP client
  timeClient.begin();

  // SD card initialization
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized.");


  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1); // Loop until RTC is initialized
  }

  // Sync RTC with NTP time on startup
  syncTimeWithNTP();

/////////////////////////////////////////////////////////////

  //  pinMode(MAX485_DE, OUTPUT);
  //  pinMode(MAX485_RE, OUTPUT);

  //ReconnectWiFi();

  if (SerialMonitor == 1) {
    Serial_Monitor.begin(115200);
  }
  //ReadEEPROM();

  WiFiManager wifiManager;
  wifiManager.setTimeout(30);

  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("Instrux Configuration Portal")) {
    if (!wifiManager.startConfigPortal("Instrux Configuration Portal")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      WiFi.mode(WIFI_STA);
      // ESP.reset();
      // Serial.println("In Station Mode");
      delay(5000);
    }
  }

  if (! rtc.begin()) {
      Serial.println("Couldn't find RTC");
      Serial.flush();
      //abort();
  }

  
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
      // When time needs to be set on a new device, or after a power loss, the
      // following line sets the RTC to the date & time this sketch was compiled
      //rtc.adjust(DateTime(F(DATE), F(TIME)));
      // This line sets the RTC with an explicit date & time, for example to set
      // January 21, 2014 at 3am you would call:
      //rtc.adjust(DateTime(2021, 8, 20, 16, 0, 30));
  }

  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  rtc.adjust(DateTime(F(_DATE), F(TIME_)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
   rtc.adjust(DateTime(2024, 11, 1, 12, 52, 25));

  
  ////  Serial.print("Initializing SD card...");
  //  see if the card is present and can be initialized:
    if (!SD.begin(chipSelect)) {
      Serial.println("Card failed, or not present");
  //    // don't do anything more:
  //    return;
    }
    Serial.println("card initialized.");
  //
    DateTime now = rtc.now();
    sprintf(fileName, "%02d_%02d_%4d.CSV", (now.day()), (now.month()), (now.year()));
    Serial.println(fileName);
  //
    dataFile = SD.open(fileName, FILE_WRITE);
    if (dataFile) {
  //
      Serial.println("Writing headers to csv.txt");
  //    dataFile.println("Date,Time,VR_phase,VY_phase,VB_phase,IR_phase,IY_phase,IB_phase,Watts_R_phase,Watts_Y_phase,Watts_B_phase,VA_R,VA_Y,VA_B,VAR_R,VAR_Y,VAR_B,pfR,pfY,pfB,TC,TWP,TVA,TVAR,f,MWD,MVAD,vRY,vYB,vRB,NC,THD_vR,THD_vY,THD_vB,THD_cR,THD_cY,THD_cB,THD_aLN,tcLN,mcR,mcY,mcB,pfA,cHA1,cHA2,cHA3,cHA4,cHA5,cHA6,cHA7,cHA8,cHA9,cHA10,cHA11,cHA12,cHA13,cHA14,cHA15,cHA16,cHA17,cHA18,cHA19,cHA20,cHA21,cHA22,cHA23,cHA24,cHA25,cHA26,cHA27,cHA28,cHA29,cHA30,cHA31,cHB1,cHB2,cHB3,cHB4,cHB5,cHB6,cHB7,cHB8,cHB9,cHB10,cHB11,cHB12,cHB13,cHB14,cHB15,cHB16,cHB17,cHB18,cHB19,cHB20,cHB21,cHB22,cHB23,cHB24,cHB25,cHB26,cHB27,cHB28,cHB29,cHB30,cHB31,cHC1,cHC2,cHC3,cHC4,cHC5,cHC6,cHC7,cHC8,cHC9,cHC10,cHC11,cHC12,cHC13,cHC14,cHC15,cHC16,cHC17,cHC18,cHC19,cHC20,cHC21,cHC22,cHC23,cHC24,cHC25,cHC26,cHC27,cHC28,cHC29,cHC30,cHC31,DIPS,Swells");
      dataFile.close();
      Serial.println("Headers written");
    }
    // if the file didn't open, print an error:
    else {
      Serial.println("error opening for the first time");
    }
    delay(50);

  EEPROM.begin(512);  //Initialize EEPROM

  //read here
  //ReadEEPROM();

  //WriteEEPROM();

  Sprintln(F("\nInitializing Instrux..."));

  Sprintln();

  Sprint(F("Number of Meters added: "));
  Sprintln(NumberOfMeters);
  delay(20);
  //ReconnectWiFi();
}

void createLogFile(char *filename, String headers) {
  dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile) {
    dataFile.println(headers);  // Write headers
    dataFile.close();
    Serial.println("Headers written to: " + String(filename));
  } else {
    Serial.println("Error opening file: " + String(filename));
  }
}




void loop() {

    //Changes-----------------------

  unsigned long currentMillis = millis();

  // Check Wi-Fi connection and sync time every 5 minutes
  if (WiFi.status() == WL_CONNECTED) {
    if (currentMillis - lastSyncAttempt >= syncInterval) {
      lastSyncAttempt = currentMillis; // Update the last sync attempt time
      syncTimeWithNTP();
      Serial.println("WiFi syncing time with NTP.");

    }
  } else {
    Serial.println("WiFi disconnected. Displaying RTC time only.Log data to SD card");
    logDataToSD();  // Log data to SD card when Wi-Fi is disconnected
  }


  // Check if Wi-Fi is still connected
  /*if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected! Attempting to reconnect...");
    WiFi.reconnect();
    unsigned long startAttemptTime = millis();
    
    // Attempt to reconnect for 10 seconds
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      delay(500);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println(" Reconnected to Wi-Fi!");
      // Synchronize time again after reconnecting
      syncTimeWithNTP();
    } else {
      Serial.println(" Unable to reconnect to Wi-Fi. RTC will continue running.");
    }
  } else {
    // Synchronize time periodically if Wi-Fi is connected
    if (timeClient.update()) {
      syncTimeWithNTP();
    }
  }*/


  // Display the time from RTC
  DateTime now = rtc.now();
  Serial.print("RTC Time: ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  // Display WiFi connection status
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Status: Disconnected");
  }

  delay(1000); // Short delay before the next display


////////////////////////////////////////////////////////////////

  currentMillis = millis();

  if (currentMillis - previousMillis >= DataSending_Interval) {

    previousMillis = currentMillis;

    logDataToSD(); 

    for (MeterNo = 0; MeterNo < NumberOfMeters; MeterNo++) {

      FetchAndSendData();

//            SendData_WiFi();

    }
    //  delay(100);
  }

  if (URLToSend.substring(0, 25) == "/instrux/crud/create.php?")
  {
    newData = true;
    Serial.println("\n------------> Valid Data Received <------------\n");
  }

  if (newData == true) {

    if (URLToSend.length() > 150) {

      URLToSend = "http://instrux.live" + URLToSend;
      //      Serial.println("\nData concatenated= " + DataToSend);
          //SendData_WiFi();
      newData = false;
    }
  }

  timeClient.update(); // Update time from NTP server

  // Extract date information from epoch time
  unsigned long epochTime = timeClient.getEpochTime();
  int currentDay = day(epochTime);
  int currentMonth = month(epochTime);
  int currentYear = year(epochTime);

  // Generate filename using date
  String fileName = String(currentDay) + "-" + String(currentMonth) + "-" + String(currentYear) + ".txt"; // Format: DD-MM-YYYY.txt


  if (Serial.available()) {
    String data = Serial.readStringUntil('\n'); // Enter dabane tak ka data read karega

    // Data ko file mei likhna
    File dataFile = SD.open(fileName.c_str(), FILE_WRITE);

    if (dataFile) {
      dataFile.println(data);
      dataFile.close();
      Serial.println("Data successfully written to SD card!");
    } else {
      Serial.println("Error opening the file!");
    }
  }

}

void logDataToSD() {
  DateTime now = rtc.now();
  dataFile = SD.open(fileName, FILE_WRITE);
  if (dataFile) {
    // Format the current time and data to be saved to the SD card
    dataFile.print(now.year(), DEC);
    dataFile.print('/');
    dataFile.print(now.month(), DEC);
    dataFile.print('/');
    dataFile.print(now.day(), DEC);
    dataFile.print(" ");
    dataFile.print(now.hour(), DEC);
    dataFile.print(':');
    dataFile.print(now.minute(), DEC);
    dataFile.print(':');
    dataFile.print(now.second(), DEC);

    // Example data (replace with actual data readings)
    dataFile.print(",");  
    dataFile.print(random(1, 100));  // Replace with actual data variable
    dataFile.print(",");
    dataFile.print(random(1, 100));  // Replace with actual data variable
    dataFile.print(",");
    dataFile.println(random(1, 100));  // Replace with actual data variable

    dataFile.close();
    Serial.println("Data written to SD card.");
  } else {
    Serial.println("Error opening data file for writing.");
  }
}



void syncTimeWithNTP() {
  Serial.println("Attempting NTP synchronization...");

  if (timeClient.update()) { // Check if time sync is successful
    unsigned long epochTime = timeClient.getEpochTime(); // Get epoch time
    DateTime ntpTime = DateTime(epochTime);
    rtc.adjust(ntpTime); // Set RTC time to NTP time

    Serial.println("RTC synchronized with NTP time:");
    Serial.print("NTP Time: ");
    Serial.print(ntpTime.year(), DEC);
    Serial.print('/');
    Serial.print(ntpTime.month(), DEC);
    Serial.print('/');
    Serial.print(ntpTime.day(), DEC);
    Serial.print(" ");
    Serial.print(ntpTime.hour(), DEC);
    Serial.print(':');
    Serial.print(ntpTime.minute(), DEC);
    Serial.print(':');
    Serial.print(ntpTime.second(), DEC);
    Serial.println();
  } else {
    Serial.println("NTP sync failed. Using RTC time.");
  }
}
//////////////////////////////////////////////////////////////

void FetchAndSendData() {

  if (ConnectedMeters[MeterNo][0] == 3)   // MFM_376
  {
#if ___MFM_376 == 1
    __MFM_376();
    RTCtime();
    LocalData();
    FormData();
    SendData();
#else
    Sprintln("MFM_376 Meter is disabled.  Please enable it from MeterConfig.h file.");
    Sprintln();
#endif
  }
  if (ConnectedMeters[MeterNo][0] == 0)   // PM2120
  {
#if ___PM2120 == 1
    __PM2120();
    RTCtime();
    LocalData();
    FormData();
    SendData();
#else
    Sprintln("PM2120 Meter is disabled. Please enable it from MeterConfig.h file.");
    Sprintln();
#endif
  }

  if (ConnectedMeters[MeterNo][0] == 5)   // POLARSTAR
  {
#if ___POLARSTAR == 1
    __POLARSTAR();
        RTCtime();
    //    LocalData();
    //    VibrationData();
    FormData();
    SendData();
#else
    Sprintln("POLAR STAR Meter is disabled.  Please enable it from MeterConfig.h file.");
    Sprintln();
#endif
  }
}

void FormData() {

  URLToSend  = "/instrux/crud/create.php?";
  URLToSend += "&localId=0";
  //URLToSend += String(ConnectedMeters[MeterNo][0]);
  //  URLToSend += "1";
  URLToSend += "&password=123456";

  for (int param = 0; param < NumberOfParameters; param++) {

    if (Acquired_Data[MeterNo][param] != ParameterFailureValue) {
      URLToSend += "&";
      URLToSend += parameter_names[param];
      URLToSend += "=";
      URLToSend += String(Acquired_Data[MeterNo][param]);
    }
  }

  URLToSend += "&mac=";

  //  URLToSend += data.ESPMACAddress;
  URLToSend += String(WiFi.macAddress());
  //URLToSend += "#";

}



void SendData() {

  Sprintln("URL To Send= " + URLToSend);
  Sprintln("");

  if (result == node.ku8MBSuccess) {

    //Serial.print(URLToSend);
    Serial.print("Data Sent");
    //SendData_WiFi();
    //WiFi_Serial.flush();
  }

  if (result == node.ku8MBResponseTimedOut) {
    Sprintln(F("---------------> Failed to Fetch Data <---------------"));
  }
  Sprintln("");
}


#if ___MFM_376 == 1

void __MFM_376() {

  //Meter_Port.flush();
  Meter_Port.begin(9600, SWSERIAL_8N1, 2, 0);

  NumberOfParameters = sizeof(MFM_376) / sizeof(int);

  node.begin(ConnectedMeters[MeterNo][1], Meter_Port);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  dataString = "";

  Sprint(F("MFM_376="));

  for (int param = 0; param < NumberOfParameters ; param++) {

    if (MFM_376[param] == 0xFFFF) {
      Acquired_Data[MeterNo][param] = ParameterFailureValue;
    }
    else {
      result = node.readInputRegisters(MFM_376[param], 2);

      if (result == node.ku8MBSuccess) {


        for (j = 0; j < 2; j++) {
          datas[j] = node.getResponseBuffer(j);
        }

        unsigned long temp = (unsigned long)datas[1] << 16 | datas[0];
        Acquired_Data[MeterNo][param] = (float)&temp;

        if (((param >= 6) && (param <= 14)) || ((param >= 21) && (param <= 23)) || (param == 47)) {
          Acquired_Data[MeterNo][param] = Acquired_Data[MeterNo][param] * 1000;
        }

      }

      if (isnan(Acquired_Data[MeterNo][param])) {
        Acquired_Data[MeterNo][param] = ParameterFailureValue;
      }
    }

    if (result == node.ku8MBResponseTimedOut) {
      Sprint(F(" Response Timeout..."));
      param = NumberOfParameters;
    }
    else {
      Sprint(F(" "));
      Sprint(Acquired_Data[MeterNo][param]);
      dataString  += String(Acquired_Data[MeterNo][param]);
      if (param < NumberOfParameters - 1) {
        dataString += ",";
      }
    }
    delay(20);
  }
  Sprintln();
  Serial.println(dataString);
}

#endif

#if ___PM2120 == 1

void __PM2120() {

  Meter_Port.flush();
  Meter_Port.begin(9600, SWSERIAL_8E1, 2, 0);

  NumberOfParameters = sizeof(PM2120) / sizeof(int);

  node.begin(ConnectedMeters[MeterNo][1], Meter_Port);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  dataString = "";

  Sprint(F("PM2120="));

  for (int param = 0; param < NumberOfParameters ; param++) {

    if (PM2120[param] == 0xFFFF) {
      Acquired_Data[MeterNo][param] = ParameterFailureValue;
    }
    else {
      result = node.readHoldingRegisters(PM2120[param], 2);

      if (result == node.ku8MBSuccess) {

        for (j = 0; j < 2; j++) {
          datas[j] = node.getResponseBuffer(j);
        }

        unsigned long temp = (unsigned long)datas[0] << 16 | datas[1];
        Acquired_Data[MeterNo][param] = (float)&temp;

        if ((param >= 6 && param < 15) || (param >= 21 && param < 24) || (param == 47)) {
          Acquired_Data[MeterNo][param] = Acquired_Data[MeterNo][param] * 1000;
        }
      }

      if (isnan(Acquired_Data[MeterNo][param])) {
        Acquired_Data[MeterNo][param] = ParameterFailureValue;
      }
    }

    if (result == node.ku8MBResponseTimedOut) {
      Sprint(F(" Response Timeout..."));
      param = NumberOfParameters;
    }
    else {
      Sprint(F(" "));
      Sprint(Acquired_Data[MeterNo][param]);
      dataString  += String(Acquired_Data[MeterNo][param]);
      if (param < NumberOfParameters - 1) {
        dataString += ",";
      }
    }
    delay(20);
  }
  Sprintln();
  Serial.println(dataString);
}

#endif

#if ___POLARSTAR == 1

void __POLARSTAR() {

  Meter_Port.flush();
  Meter_Port.begin(9600, SWSERIAL_8N1, 2, 0);

  NumberOfParameters = sizeof(POLARSTAR) / sizeof(int);

  node.begin(ConnectedMeters[MeterNo][1], Meter_Port);
  //node.preTransmission(preTransmission);                          //for previous RS485 module
  //node.postTransmission(postTransmission);                        //for previous RS485 module

  Sprint(F("POLARSTAR="));

  for (int param = 0; param < NumberOfParameters ; param++) {

    if (POLARSTAR[param] == 0xFFFF) {
      Acquired_Data[MeterNo][param] = ParameterFailureValue;
    }
    else {
      result = node.readInputRegisters(POLARSTAR[param], 2);

      if (result == node.ku8MBSuccess) {
        //        Serial.print("param ");
        //        Serial.println(param);
        if (param != 47) {
          for (j = 0; j < 2 ; j++) {
            datas[j] = node.getResponseBuffer(j);

            //          Serial.println(datas[j]);
          }
        }
        else {
          for (j = 0; j < 4 ; j++) {
            datas[j] = node.getResponseBuffer(j);
          }
        }
        if (param == 47) {
//          Serial.print("1st ");
//          Serial.println((BCDToDecimal(datas[0])));
//          Serial.println((BCDToDecimal(datas[1])));
//          Serial.println((BCDToDecimal(datas[2])));
//          Serial.println((BCDToDecimal(datas[3])));
          Acquired_Data[MeterNo][param] = (BCDToDecimal(datas[0])) + ((BCDToDecimal(datas[1]))/10000); // + ((float(BCDToDecimal(datas[2]))) / 1000); //((float(BCDToDecimal(datas[1])))/10000)
        }
        else {
          Acquired_Data[MeterNo][param] = BCDToDecimal(datas[0]) * pow(10, binaryToDecimal(datas[1]));
        }
      }

      if (isnan(Acquired_Data[MeterNo][param])) {
        Acquired_Data[MeterNo][param] = ParameterFailureValue;
      }
    }

    if (result == node.ku8MBResponseTimedOut) {
      Sprint(F(" Response Timeout..."));
      param = NumberOfParameters;
    }
    else {

      Sprint(F(" "));
      Sprint(Acquired_Data[MeterNo][param]);
      dataString  += String(Acquired_Data[MeterNo][param]);
      if (param < NumberOfParameters - 1) {
        dataString += ",";
      }
    }

    delay(10);

  }

  Sprintln();
}

#endif
