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
boolean go = true; 
long lastMillis = 0;
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

void readDir(){
  while ( go ){
    File entry = root.openNextFile();
    if (! entry){
      Serial.println("No files left.");
      go = false;
      break;
    }
    else{
      if (entry.isDirectory()){
        Serial.println("/");
        readDir();
      }
      else{
        Serial.print("File name: ");
        Serial.print(entry.name());
        Serial.print(" Size: ");
        Serial.print(entry.size(), DEC);
        Serial.println(" bytes");
      }
    }
  }
}

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
    Serial.println(dataTxt);
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
  DataString.reserve(256);
  //DataString += "DATE, TIME, LAT, LON, SAT, [ELEVATION/AZ] ";
  //DataString += "\r\n";

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

  DataString += fix.latitude();
  DataString += ", ";
  DataString += fix.longitude();
  DataString += ", ";

  DataString += fix.satellites;
  DataString += ", ";
  DataString += "[";

  for (uint8_t i = 0; i < fix.satellites; i++){
    DataString += gps.satellites[i].id;
    DataString += ", ";
    DataString += gps.satellites[i].elevation;
    DataString += "/";
    DataString += gps.satellites[i].azimuth;
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
      DataToString();
      writeData(filename);
      fixCp = 0;
    }
  }
}

void displaySerialPos(){
  
  if (fix.valid.location) {
    Serial.print("Position: ");
    Serial.print(fix.latitudeL() );
    Serial.print( ',' );
    Serial.println(fix.longitudeL() );
  }
  else{
    Serial.println(F("fix not valid"));
  }
}

void displaySerialSatInfo(){
    
    for (uint8_t i = 0; i < fix.satellites; i++){
      Serial.print(F("ID number: "));
      Serial.print(gps.satellites[i].id);
      Serial.print(F(" Elevation: "));
      Serial.print(gps.satellites[i].elevation);
      Serial.print(F(" Azimuth: "));
      Serial.println(gps.satellites[i].azimuth);
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

void displaySerialDateAndTime(){

  Serial.print(F("Date : "));
  if (fix.valid.date) {
    Serial.print(localTime.date);
    Serial.print('/');
    Serial.print(localTime.month);
    Serial.print('/');
    Serial.println(localTime.year);
  } else {
    Serial.println(F("INVALID"));
  }

  Serial.print(F("Time : "));
  if (fix.valid.time) {
    Serial.print(localTime.hours);
    Serial.print(':');
    if (localTime.minutes < 10) Serial.print('0');
    Serial.print(localTime.minutes);
    Serial.print(':');
    if (localTime.seconds < 10) Serial.print(F("0"));
    Serial.println(localTime.seconds);
    
  } else {
    Serial.println(F("INVALID"));
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
    lcd.print("FIX...");
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
    int n = 1;
    printf(filename, "REC%d.csv", n);
    while (SD.exists(filename)){
      n++;
      sprintf(filename, "REC%d.csv", n); 
     
    }
    Serial.println(filename);
    File data = SD.open(filename, FILE_WRITE);
    CurrentFile = filename;
    data.println("DATE, TIME, LAT, LON, NBSAT, [ID ELEV/AZ]");
    data.close();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(CurrentFile);
    flag = 0;
  }
  
 
  if (bouncer17.rose() && value16 == HIGH && value15 == HIGH && flag != 1){
    Serial.println("Bouton 1 appuyé");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Serial");
    lcd.setCursor(0,1);
    lcd.print("Dump");
    flag = 1;
  }
  if (bouncer17.rose() && bouncer16.fell() && value15 == HIGH && flag != 2){
    Serial.println("Bouton 2 appuyé");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Record");
    lcd.setCursor(0,1);
    lcd.print("Started");
    flag = 2;
  }
  if (bouncer17.rose() && bouncer15.fell() && value16 == HIGH && flag != 3){
    lcd.clear();
    Serial.println("Bouton 3 appuyé");
    lcd.setCursor(0,0);
    lcd.print("File");
    lcd.setCursor(0,1);
    lcd.print("Menu");
    flag = 3;
  }
  if (bouncer15.fell() && bouncer16.fell() && value17 == HIGH && flag != 4){
    lcd.clear();
    Serial.println("Bouton 4 appuyé");
    lcd.setCursor(0,0);
    lcd.print("Create");
    lcd.setCursor(0,1);
    lcd.print("File");
    flag = 4;
  }
}
