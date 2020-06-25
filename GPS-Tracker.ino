#include <Bounce2.h>

#include <SPI.h>

#include <SoftwareSerial.h>

#include <Bounce2.h>

#include <SD.h>

#include <NMEAGPS.h>


Bounce bouncer17 = Bounce(); 
Bounce bouncer16 = Bounce();
Bounce bouncer15 = Bounce();

NMEAGPS gps;
gps_fix fix;
#define RX_PIN 3
#define TX_PIN 2
#include <GPSport.h>
SoftwareSerial GPSSerial(3,2);

#include <LiquidCrystal.h>
LiquidCrystal lcd(4,5,6,7,8,9);

//-------------Global values and objects---------------------//
//#define TIMEOUT 1000

uint8_t fixCp = 0;
uint8_t GPS_Sat = 0;

NeoGPS::clock_t localSeconds;
NeoGPS::time_t  localTime;

Sd2Card card;
SdVolume volume;
File root;

int flag = 0;
String CurrentFile;
//-------------------SETUP AND LOOP---------------------//
void setup(){
  
  lcd.begin(8,2);
  Serial.begin(9600);
  GPSSerial.begin(9600);
  pinMode(10, OUTPUT);
  SD.begin(10);
  root = SD.open("/");  
  gpsPort.begin(9600);

  pinMode(17, INPUT_PULLUP);
  bouncer17.attach(17);
  bouncer17.interval(25);
  pinMode(16, INPUT_PULLUP);
  bouncer16.attach(16);
  bouncer16.interval(25);
  pinMode(15, INPUT_PULLUP);
  bouncer15.attach(15);
  bouncer15.interval(25);
}

void loop(){
  lcdMenuInterface();
}


//----------------------SD FILE METHODS---------------------------//

//void transfer_file(String filename){
//  unsigned long wait_start;
//  byte buff[64];
//
//  File file = SD.open(filename, FILE_READ);
//  unsigned long file_size = file.size();
//  //Sending the file size through serial port in Big Endian (Most Significant Byte First)
//  Serial.write((byte)(file_size >> 24));
//  Serial.write((byte)(file_size >> 16));
//  Serial.write((byte)(file_size >> 8));
//  Serial.write((byte)file_size);
//
//  //Wait for ACK from pyscript
//  if (! wait_for_ACK('2')){
//    return;
//  }
//
//  //Send file content in blocks of 64 bytes (Arduino Serial Buffer size is 64 max)
//  unsigned int n;
//  while(file_size){
//    //n is the number of bytes read
//    n = file.read(buff, 64);
//    Serial.write(buff, n);
//    file_size -= n;
//
//    if (! wait_for_ACK('3')) {
//      return;
//    }
//  }
//}
//
////Wait method. Return false if it gets the wrong char or go past the timeout 
//bool wait_for_ACK(char c) {
//  unsigned long wait = millis();
//  while (! Serial.available() || millis() - wait < TIMEOUT);
//  if(! Serial.available() || Serial.read() != c){
//    return false;
//  }
//  return true;
//}

void readFile(String filename){
  File data = SD.open(filename, FILE_READ);
  if (data) {
    while (data.available()){
      Serial.write(data.read());
    }
    
    Serial.println(F("EOF"));
  }
  else{
    Serial.print(F("The file doesn't exist"));
  }
  data.close();
}

//Write the string into the SD card, under the file test.txt
void writeData(String filename){
  File data = SD.open(filename, FILE_WRITE);
  if (data){
    String dataTxt = DataToString();
    Serial.println(F("Writing to SD card"));
    data.println(dataTxt);
    data.close();
    Serial.println(F("Done"));
  }
  else{
    Serial.println(F("Failed to open file"));
  }
}

//Convert all the struct members into one long string, each field separated by a comma
String DataToString(){
  String DataString;
  DataString.reserve(512);

  DataString += localTime.date;
  DataString += '/';
  DataString += localTime.month;
  DataString += '/';
  DataString += localTime.year;
  DataString += ", ";

  DataString += localTime.hours;
  DataString += ':';
  DataString += localTime.minutes;
  DataString += ':';
  DataString += localTime.seconds;
  DataString += ", ";

  DataString += fix.latitudeL();
  DataString += ", ";
  DataString += fix.longitudeL();
  DataString += ", ";

  DataString += fix.altitude();
  DataString += ", ";

  DataString += fix.speed_kph();
  DataString += ", ";
  
  DataString += fix.satellites;
  DataString += ", ";
  DataString += "[";

  
  for (uint8_t i = 0; i < fix.satellites; i++){
    DataString += gps.satellites[i].elevation;
    DataString += "/";
    DataString += gps.satellites[i].azimuth;
    DataString += "@";
    DataString += gps.satellites[i].snr;
      if (i < fix.satellites -1){
        DataString += ", ";
      }
    
  }
  DataString += "]";
  return DataString;
}
//-------------------GPS METHODS----------------------//
void gpsReception(String filename){
  while (gps.available( gpsPort )){
    fix = gps.read();
    if (++fixCp >= 10){ 
      convertLocalTime();
      writeData(filename);
      fixCp = 0;
    }
  }
}


void convertLocalTime(){
  if (fix.valid.date && fix.valid.time) {
    using namespace NeoGPS;

    //Timezone shift, passing dateTime into seconds, adds the corresponding amount of seconds j
    localSeconds = (clock_t) fix.dateTime; 
    localSeconds += 2 * SECONDS_PER_HOUR;
    localTime = localSeconds;              
  }
}

//-------------------LCD DISPLAY METHODS----------------------//
void lcdDisplayPos(){
  if (fix.valid.location) {
    lcd.setCursor(0,0);
    lcd.print("Lat:");
    lcd.setCursor(4,0);
    lcd.print(fix.latitude());
    lcd.setCursor(0,1);
    lcd.print("Lon:");
    lcd.setCursor(4,1);
    lcd.print(fix.longitude());
  }
  else{
    lcd.setCursor(0,0);
    lcd.print("Search");
    lcd.setCursor(0,1);
    lcd.print("FIX....");
  }
}

void lcdMenuInterface(){
  bouncer17.update();
  bouncer16.update();
  bouncer15.update();

  int value17 = bouncer17.read();
  int value16 = bouncer16.read();
  int value15 = bouncer15.read();

  if (flag == 1){
    Serial.println(CurrentFile);
    readFile(CurrentFile);
    flag = 0;
  }
  if (flag == 2){
    gpsReception(CurrentFile);
    lcdDisplayPos();
  }

  if (bouncer17.rose() && bouncer15.fell() && value16 == HIGH && flag == 3){
    File data = root.openNextFile();
    if (! data){
      root.rewindDirectory();
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("END");
      flag = 0;
    }
    else{
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(data.name());
      lcd.setCursor(0,1);
      lcd.print(data.size());
      CurrentFile = data.name();
    }
    data.close();
  } 


  if (flag == 4){
    char filename[8];
    int n = 0;
    printf(filename, "REC%d.csv", n);
    while (SD.exists(filename)){
      n++;
      sprintf(filename, "REC%d.csv", n); 
     
    }
    Serial.println(filename);
    File data = SD.open(filename, FILE_WRITE);
    CurrentFile = filename;
    data.println(F("Date, Time, latitude, longitude, altitude, speed, NbSat, [elev/az@SNR] "));
    data.close();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(CurrentFile);
    flag = 0;
  }
  
 
  if (bouncer17.rose() && value16 == HIGH && value15 == HIGH && flag != 1){
    Serial.println(F("Bouton 1 appuyé"));
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Serial");
    lcd.setCursor(0,1);
    lcd.print("Dump");
    flag = 1;
  }
  if (bouncer17.rose() && bouncer16.fell() && value15 == HIGH && flag != 2){
    Serial.println(F("Bouton 2 appuyé"));
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Record");
    lcd.setCursor(0,1);
    lcd.print("Started");
    flag = 2;
  }
  if (bouncer17.rose() && bouncer15.fell() && value16 == HIGH && flag != 3){
    lcd.clear();
    Serial.println(F("Bouton 3 appuyé"));
    lcd.setCursor(0,0);
    lcd.print("File");
    lcd.setCursor(0,1);
    lcd.print("Menu");
    flag = 3;
  }
  if (bouncer15.fell() && bouncer16.fell() && value17 == HIGH && flag != 4){
    lcd.clear();
    Serial.println(F("Bouton 4 appuyé"));
    lcd.setCursor(0,0);
    lcd.print("Create");
    lcd.setCursor(0,1);
    lcd.print("File");
    flag = 4;
  }
}
