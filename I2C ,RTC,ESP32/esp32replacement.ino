//SOC,RM,FCC,VOLT,TEMP,CURRENT,AVAILABLeENERGY,SERIAL NUMBER,STATE OF HEALTH,DESIGN CAPACITY,TRUE RC ,TRUE FCC,
// Time elapsed(sec),voltage(mV),current(mA),cell temperature(C),max charge voltage(4.2V / current<=30mA),max discharge voltage(2.8V)
#include <Wire.h>
#include <WiFi.h>
#include <LiquidCrystal.h>
#define address 0x55
#define DS3231_I2C_ADDRESS 0x68
#include "FS.h"
#include <SPI.h>
#include <SD.h>
#include <string.h>
 int del = 0 ;
//#include <bits/stdc++.h>
#include "BluetoothSerial.h"
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
String apiKey = "ZZSX53LHRRB35JEV";                  //  Enter your Write API key from ThingSpeak
const char *ssid =  "RACEnergy (2G)";
const char *pass =  "rndlab123";
const char* server =   "184.106.153.149";
char k;
int count = 5 ;
String filei_str;
WiFiClient client;

File LogFile;

BluetoothSerial SerialBT;

String data_str ;
String clock_str ;
String Date_str ;
String file_str;

int temperature_i, voltage_i, current_i, AvailableEnergy_i, timef_i, timee_i, cc_i, soc_i ;
//
//void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
//  Serial.printf("Listing directory: %s\n", dirname);
//
//  File root = fs.open(dirname);
//  if (!root) {
//    Serial.println("Failed to open directory");
//    return;
//  }
//  if (!root.isDirectory()) {
//    Serial.println("Not a directory");
//    return;
//  }
//
//  File file = root.openNextFile();
//  while (file) {
//    if (file.isDirectory()) {
//      Serial.print("  DIR : ");
//      Serial.println(file.name());
//      if (levels) {
//        listDir(fs, file.name(), levels - 1);
//      }
//    } else {
//      Serial.print("  FILE: ");
//      Serial.print(file.name());
//      Serial.print("  SIZE: ");
//      Serial.println(file.size());
//    }
//    file = root.openNextFile();
//  }
//}
int listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  //  Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
      //  Serial.println("Failed to open directory");
       
    }
    if(!root.isDirectory()){
       // Serial.println("Not a directory");
        
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
         //   Serial.print("  DIR : ");
           // Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
           // Serial.print("  FILE: ");
          //  Serial.print(file.name());
             const char* fil = file_str.c_str();
            if(strcmp(fil,file.name())==0){
           // Serial.print(file.name());
            return 200 ;
            }
           // Serial.print("  SIZE: ");
           // Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char * path) {
  Serial.printf("Creating Dir: %s\n", path);
  if (fs.mkdir(path)) {
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

void removeDir(fs::FS &fs, const char * path) {
  Serial.printf("Removing Dir: %s\n", path);
  if (fs.rmdir(path)) {
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir failed");
  }
}

void readFile(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2) {
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void deleteFile(fs::FS &fs, const char * path) {
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void testFileIO(fs::FS &fs, const char * path) {
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if (file) {
    len = file.size();
    size_t flen = len;
    start = millis();
    while (len) {
      size_t toRead = len;
      if (toRead > 512) {
        toRead = 512;
      }
      file.read(buf, toRead);
      len -= toRead;
    }
    end = millis() - start;
    Serial.printf("%u bytes read for %u ms\n", flen, end);
    file.close();
  } else {
    Serial.println("Failed to open file for reading");
  }


  file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  size_t i;
  start = millis();
  for (i = 0; i < 2048; i++) {
    file.write(buf, 512);
  }
  end = millis() - start;
  Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
  file.close();
}

/********************************************** CODE FOR TIME AND DATE DISPLAY*************************************************************************/
byte decToBcd(byte val)
{
  return ( (val / 10 * 16) + (val % 10) );
}
byte bcdToDec(byte val)
{
  return ( (val / 16 * 10) + (val % 16) );
}
void setDS3231time(byte second, byte minute, byte Hour, byte dayOfWeek, byte
                   dayOfMonth, byte month, byte Year)
{
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(Hour)); // set Hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(Year)); // set Year (0 to 99)
  Wire.endTransmission();
}
void readDS3231time(byte *second,
                    byte *minute,
                    byte *Hour,
                    byte *dayOfWeek,
                    byte *dayOfMonth,
                    byte *month,
                    byte *Year)
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *Hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *Year = bcdToDec(Wire.read());
}
void displayTime()
{
  byte second, minute, Hour, dayOfWeek, dayOfMonth, month, Year;
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &Hour, &dayOfWeek, &dayOfMonth, &month,
                 &Year);
  // send it to the serial monitor
  Serial.print(Hour, DEC);
  clock_str += String(Hour);
  // convert the byte variable to a decimal number when displayed
  Serial.print(":");
  clock_str +=  ":";
  if (minute < 10)
  {
    Serial.print("0");
    clock_str +=  "0";
  }
  Serial.print(minute, DEC);
  clock_str += String(minute);
  Serial.print(":");
  clock_str +=  ":";
  if (second < 10)
  {
    Serial.print("0");
    clock_str += "0";
  }
  Serial.print(second, DEC);
  clock_str += String(second);
  Serial.print(" ");
  clock_str += ",";
  clock_str += " "; 
  Serial.print(dayOfMonth, DEC);
  clock_str += String(dayOfMonth);
  Date_str += String(dayOfMonth);
  Serial.print("/");
  clock_str += "/";
  Date_str += "_";
  Serial.print(month, DEC);
  clock_str += String(month);
  Date_str += String(month);
  Serial.print("/");
  clock_str += "/";
  Date_str += "_";
  Serial.print(Year, DEC);
  clock_str += String(Year);
  Date_str += String(Year);
  clock_str += ",";
  Serial.print(" Day of week: ");
  clock_str += " Day of week:" ;
  switch (dayOfWeek) {
    case 1:
      Serial.println("Sunday");
      clock_str += "Sunday";
      clock_str += ",";
      break;
    case 2:
      Serial.println("Monday");
      clock_str += "Monday";
      clock_str += ",";
      break;
    case 3:
      Serial.println("Tuesday");
      clock_str += "Tuesday";
      clock_str += ",";
      break;
    case 4:
      Serial.println("Wednesday");
      clock_str += "Wednesday";
      clock_str += ",";
      break;
    case 5:
      Serial.println("Thursday");
      clock_str += "Thursday";
      clock_str += ",";
      break;
    case 6:
      Serial.println("Friday");
      clock_str += "Friday";
      clock_str += ",";
      break;
    case 7:
      Serial.println("Saturday");
      clock_str += "Saturday";
      clock_str += ",";
      break;
  }
}



/********************************************************** DEFINING VARIABLES *****************************************************************************/
float   temperature ;
signed current ;
unsigned int voltage, timef, timee ;
float    AvailableEnergy, DesignCapacity, StateOfHealth;
float SerialNumber;
int   CycleCount, soc ;
const int rs = 13, en = 12, d4 = 14, d5 = 27, d6 = 26, d7 = 25;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
void setup() {
  Wire.begin();
  Serial.begin(115200);
  SerialBT.begin("ESP32");
  lcd.begin(20, 4);
  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

/************************* Function to read voltage **********************************/
void readvoltage() {
  Wire.beginTransmission(address);
  Wire.write(0x08);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int volt_low_byte = Wire.read();

  Wire.beginTransmission(address);
  Wire.write(0x09);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int  volt_high_byte = Wire.read();
  volt_high_byte = volt_high_byte << 8;
  voltage =  volt_high_byte + volt_low_byte;
  data_str  +=  "," + String(voltage);
  voltage = voltage * 2;
}
void readTimeToEmpty() {
  Wire.beginTransmission(address);
  Wire.write(0x18);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int timee_low_byte = Wire.read();

  Wire.beginTransmission(address);
  Wire.write(0x19);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int  timee_high_byte = Wire.read();
  timee_high_byte = timee_high_byte << 8;
  timee =  timee_high_byte + timee_low_byte;
  data_str  +=  "," + String(timee) + ",";

}
void readTimeToFull() {
  Wire.beginTransmission(address);
  Wire.write(0x1A);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int timef_low_byte = Wire.read();

  Wire.beginTransmission(address);
  Wire.write(0x1B);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int  timef_high_byte = Wire.read();
  timef_high_byte = timef_high_byte << 8;
  timef =  timef_high_byte + timef_low_byte;
  data_str  +=   "," + String(timef);

}
/*************************** Function to read current**********************************/
void readcurrent() {
  Wire.beginTransmission(address);
  Wire.write(0x10);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int current_low_byte = Wire.read();

  Wire.beginTransmission(address);
  Wire.write(0x11);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int  current_high_byte = Wire.read();
  current_high_byte = current_high_byte << 8;
  current =  current_high_byte | current_low_byte;
  if(current >32767.5){
    current = current-(65535);
  }
  current = current * 2 ;
  if(-50<current && current <50){
    current = 0 ;
  }
  data_str += ","  + String(current); 
}

/**************************Function to read state of charge*****************************/
void readsoc() {
  Wire.beginTransmission(address); //transmit to device
  Wire.write(0x02);
  Wire.endTransmission();
  delay(5);
  Wire.requestFrom(address, 1);
  soc = Wire.read(); // receive high byte
  data_str += "," +  String(soc);

  int range = soc / 4 ;
  data_str += "," + String(range);
}

/*************************Function to read temperature**********************************/
void readtemp() {
  Wire.beginTransmission(address);
  Wire.write(0x0C);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int temp_low_byte = Wire.read();

  Wire.beginTransmission(address);
  Wire.write(0x0D);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int  temp_high_byte = Wire.read();
  temp_high_byte = temp_high_byte << 8;
  temperature = temp_high_byte + temp_low_byte;
  temperature = temperature / 10 - 273.15 ;
  data_str += "," + String(int(temperature));
}

/************************* Function to read Available Energy **********************************/
void readAvailableEnergy() {
  Wire.beginTransmission(address);
  Wire.write(0x24);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int ae_low_byte = Wire.read();

  Wire.beginTransmission(address);
  Wire.write(0x25);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int  ae_high_byte = Wire.read();
  ae_high_byte = ae_high_byte << 8;
  AvailableEnergy = ae_high_byte + ae_low_byte;
  AvailableEnergy =  AvailableEnergy * pow(10, -3);
  data_str += "," +  String(AvailableEnergy);
  AvailableEnergy = AvailableEnergy * 2*13*3.6;
}

/************************* Function to read Serial Number **********************************/
void readSerialnumber() {
  Wire.beginTransmission(address);
  Wire.write(0x28);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int sn_low_byte = Wire.read();

  Wire.beginTransmission(address);
  Wire.write(0x29);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int  sn_high_byte = Wire.read();
  sn_high_byte = sn_high_byte << 8;
  SerialNumber = sn_high_byte + sn_low_byte;
 // data_str +=  String(int(SerialNumber));
 data_str += "M2";
}

/************************* Function to read State of Health **********************************/
void readStateofHealth() {
  Wire.beginTransmission(address);
  Wire.write(0x2E);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int soh_low_byte = Wire.read();

  Wire.beginTransmission(address);
  Wire.write(0x2F);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int  soh_high_byte = Wire.read();
  soh_high_byte = soh_high_byte << 8;
  StateOfHealth = soh_high_byte + soh_low_byte;
  data_str += "," +  String(int (StateOfHealth));

}

/************************* Function to read Design Capacity **********************************/
void readDesignCapacity() {
  Wire.beginTransmission(address);
  Wire.write(0x3C);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int dc_low_byte = Wire.read();

  Wire.beginTransmission(address);
  Wire.write(0x3D);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int  dc_high_byte = Wire.read();
  dc_high_byte = dc_high_byte << 8;
  DesignCapacity = dc_high_byte + dc_low_byte;
}
/************************* Function to get CYCLE COUNT**********************************/
void readCycleCount() {
  Wire.beginTransmission(address);
  Wire.write(0x2C);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int cc_low_byte = Wire.read();
  Wire.beginTransmission(address);
  Wire.write(0x2D);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int  cc_high_byte = Wire.read();
  cc_high_byte = cc_high_byte << 8;
  CycleCount = cc_high_byte + cc_low_byte;
  data_str  +=  "," + String(CycleCount);
}

/************************* Function to get bit at a particular position **********************************/
//int get_bit_status(unsigned int bitdata , int pos) {
//  bitdata = bitdata / pow(10, n - pos - 1);
//  return bitdata % 10;
//}

void loop() {
  //    TIME    ,         DATE     ,Battery_id,Timeto full(s),Current(mAmp),Available Energy(mWh),Soc(%),Range(km),Temp(c),Cyclecount,Voltage(mV),Soh(%),TimetoEmpty
  //16:43:26 26/6/19 Day of week: Tuesday,  M2 ,     63443,    0           ,     33.35           ,35    , 8       , 26    ,       0    , 50505     ,98    ,65535, format stored in sd card 
  readSerialnumber();
  readTimeToFull();
  readcurrent();
  readAvailableEnergy();
  readsoc();
  readtemp();
  readCycleCount();
  readvoltage();
  readStateofHealth();
  readTimeToEmpty();
  Serial.print("voltage:");
  Serial.print(voltage);
  Serial.print(" Timetofull-");
  Serial.print(timef);
  Serial.print(" TimetoEmpty");
  Serial.print(timee);
  Serial.print(" current:");
  Serial.print(current);
  Serial.print(" s.no:");
  Serial.print(SerialNumber, 0);
  Serial.print("  SoC:");
  Serial.print(soc);
  Serial.print(" SoH:");
  Serial.print(StateOfHealth, 0);
  Serial.print(" Temp-");
  Serial.print(temperature, 1);
  Serial.print(" cycles:");
  Serial.print(CycleCount);
  Serial.print(" Energy:");
  Serial.println(AvailableEnergy);
  lcd.setCursor(4, 0);
  lcd.print("M2");
  lcd.setCursor(0, 1);
  lcd.print("SoC:");
  lcd.print(soc);
  lcd.print("%");
  lcd.setCursor(11, 1);
  lcd.print("E:") ;
  lcd.print(AvailableEnergy, 1);
  lcd.print(" kWh");
  lcd.setCursor(0, 2);
  lcd.print("T");
  lcd.setCursor(3, 2);
  lcd.print(":") ;
  lcd.print(temperature, 1);
  lcd.print((char)223);
  lcd.print("C");
  lcd.setCursor(11, 2);
  lcd.print(" ");
  lcd.setCursor(0, 3);
  lcd.print("CC");
  lcd.setCursor(3, 3);
  lcd.print(":") ;
  lcd.print(CycleCount);
  lcd.setCursor(11, 3);
  lcd.print("SoH:");
  lcd.print(StateOfHealth, 0);
  lcd.print("%");
  if(del==4){
    lcd.clear();
  }
  displayTime();
  Serial.println(clock_str);
  Serial.println(Date_str);
  if(del ==4){
  SerialBT.println(data_str);
  del = 0 ; 
  }
  /************************************************************** Writing Data To SD Card *************************************************************/
  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
  }
  String bt="/";
  bt +=  Date_str ;
  bt += "_log";
  file_str = bt ;
  const char* filename = file_str.c_str();
   
  data_str = clock_str  + " " + data_str ;
  const char* datastr = data_str.c_str();
   int a =    listDir(SD, "/", 0);
  if(a != 200){
  writeFile(SD,filename," ");
  }
  appendFile(SD,filename, datastr);
  appendFile(SD,filename, "\n");
//  if (SerialBT.available() > 0) {
//    char k = SerialBT.read();
//    if (k == 's') {
//      k = 's';
//      count = 0 ;
//      client.stop();
//    }
//    else if (k == 'c') {
//      count = 1 ;
//    }
//  }
    if (client.connect("184.106.153.149", 80))                                //   "184.106.153.149" or api.thingspeak.com
    {
      String postStr = "/update?key=";
      postStr += apiKey;
      if (temperature != temperature_i) {
        postStr += "&field1=";
        postStr += String(temperature);
      }
      if (soc != soc_i) {
        postStr += "&field2=";
        postStr += String(soc);
      }
      if (current != current_i) {
        postStr += "&field3=";
        postStr += String(current/1000);
      }
      postStr += "\r\n\r\n";
      client.print(String("GET ") + postStr + " HTTP/1.1\r\n" +
                   "Host: " + server + "\r\n" +
                   "Connection: close\r\n\r\n");

      Serial.println("%. Send to Thingspeak.");
    }
    temperature_i = temperature - 1, current_i = current - 1, soc_i = soc - 1 ;
  data_str = "" ;
  clock_str = "";
  Date_str = "" ;
  del++;
  delay(500);
}
