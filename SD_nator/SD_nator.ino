#include <SPI.h>
#include <SD.h>

Sd2Card card;
SdVolume volume; 
File root;
boolean go = true;

void setup(){

  Serial.begin(9600);
  pinMode(10, OUTPUT);
  SD.begin(10);
  root = SD.open("/");
//  removeFiles();
//  readDir();
  readFile("REC1.CSV");
//  readFiles();

}

void loop(){
}

void readDir(){
  go = true;
  while ( go ){
    File entry = root.openNextFile();
    if (! entry){
      Serial.println("No more files!");
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

void removeFiles(){
  while ( go ){
    File entry = root.openNextFile();
    if (! entry){
      Serial.println("End!");
      go = false;
      break;
    }
    else{
      if (entry.isDirectory()){
        Serial.println("/");
        removeFiles();
      }
      else{
        Serial.print("File name: ");
        Serial.print(entry.name());
        Serial.print(" Size: ");
        Serial.print(entry.size(), DEC);
        Serial.println(" bytes");
        Serial.println("Termination...");
        SD.remove(entry.name());
        Serial.println("File terminated");
      }
    }
  }
}

void readFile(String filename){
  File entry = SD.open(filename, FILE_READ);
  if (entry){
    Serial.println("Reading..");
    while (entry.available()){
    Serial.write(entry.read());    
    }
    Serial.println("EOF.");
  }
  else{
    Serial.println(F("Can't open file."));
  }
  entry.close();
}

void readFiles(){
  while (go){
    File entry = root.openNextFile();
    if (! entry){
      Serial.print("No more files to read");
      go = false;
      break;
    }
    else{
      if (entry.isDirectory()){
        readFiles();
      }
      else{
        while (entry.available()){
          Serial.write(entry.read());
        } 
      }        
    }
  }
}
