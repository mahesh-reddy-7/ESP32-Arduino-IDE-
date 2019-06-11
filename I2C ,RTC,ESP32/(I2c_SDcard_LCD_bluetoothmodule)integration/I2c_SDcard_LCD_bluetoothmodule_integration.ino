//SOC,RM,FCC,VOLT,TEMP,CURRENT,AVAILABLeENERGY,SERIAL NUMBER,STATE OF HEALTH,DESIGN CAPACITY,TRUE RC ,TRUE FCC,
// Time elapsed(sec),voltage(mV),current(mA),cell temperature(C),max charge voltage(4.2V / current<=30mA),max discharge voltage(2.8V)
#include <Wire.h>
#include <LiquidCrystal.h>
#define address 0x55  
#define DS3231_I2C_ADDRESS 0x68
#define n 16
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

SoftwareSerial BTserial(2,3); //(rx,tx)
File LogFile;
int a[8];

String data_str ;
//PackConfiguration(): 0x3A/0x3B
//soc 58%
//V 383mV
// T 25.5c
// I - 370ma
// cyclecount 7
// ae 6036mvh
// soh 98

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
  // convert the byte variable to a decimal number when displayed
  Serial.print(":");
  if (minute < 10)
  {
    Serial.print("0");
  }
  Serial.print(minute, DEC);
  Serial.print(":");
  if (second < 10)
  {
    Serial.print("0");
  }
  Serial.print(second, DEC);
  Serial.print(" ");
  Serial.print(dayOfMonth, DEC);
  Serial.print("/");
  Serial.print(month, DEC);
  Serial.print("/");
  Serial.print(Year, DEC);
  Serial.print(" Day of week: ");
  switch (dayOfWeek) {
    case 1:
      Serial.println("Sunday");
      break;
    case 2:
      Serial.println("Monday");
      break;
    case 3:
      Serial.println("Tuesday");
      break;
    case 4:
      Serial.println("Wednesday");
      break;
    case 5:
      Serial.println("Thursday");
      break;
    case 6:
      Serial.println("Friday");
      break;
    case 7:
      Serial.println("Saturday");
      break;
  }
}



/********************************************************** DEFINING VARIABLES *****************************************************************************/
float  fcc,rm,temperature ;
signed current ;
unsigned int voltage,timef,timee ;
 float  Flags,  AvailableEnergy, DesignCapacity, TrueRc, TrueFcc,StateOfHealth;
float SerialNumber;
int   CycleCount, soc ;
int voltagescalefactor, currentscalefactor ;
const int rs = 5, en = 6, d4 = 7, d5 = 8, d6 = 9, d7 = 10;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
void setup() {
  Wire.begin();
  Serial.begin(9600);
  BTserial.begin(9600);
  lcd.begin(20, 4);
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
  voltage = voltage*2;
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

void ReadPackConfiguration() {
  Wire.beginTransmission(address);
  Wire.write(0x3A);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int pc_low_byte = Wire.read();

  Wire.beginTransmission(address);
  Wire.write(0x3B);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int  pc_high_byte = Wire.read();
  int   nb = pc_high_byte;
  int   i = 7;

  while (i >= 0) {
    if ((nb >> i) & 1)
       a[i] = 1;
    else
      a[i] = 0 ;
    --i;
  }
 int   scaled = a[5] ;

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
  current =  current_high_byte + current_low_byte;
   current = current/10;
    data_str += ","  + String(current);
    current = current*2;
}

/**************************Function to read state of charge*****************************/
void readsoc() {
  Wire.beginTransmission(address); //transmit to device
  Wire.write(0x02);
  Wire.endTransmission();
  delay(5);
  Wire.requestFrom(address, 1);
  soc = Wire.read(); // receive high byte
  soc = 100;
  data_str += "," +  String(soc);
  
  int range = soc/4 ;
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
  temperature = temperature/10 - 273.15 ;
 data_str += "," + String(int(temperature));
}

/*************************Function to read  fcc******************************************/
void readfcc() {
  Wire.beginTransmission(address);
  Wire.write(0x06);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int fcc_low_byte = Wire.read();

  Wire.beginTransmission(address);
  Wire.write(0x07);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int  fcc_high_byte = Wire.read();
  fcc_high_byte = fcc_high_byte << 8;
  fcc =  fcc_high_byte + fcc_low_byte;
}

/***************************Function to read rm*******************************************/
void readrm() {
  Wire.beginTransmission(address);
  Wire.write(0x04);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int rm_low_byte = Wire.read();

  Wire.beginTransmission(address);
  Wire.write(0x05);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int  rm_high_byte = Wire.read();
  rm_high_byte = rm_high_byte << 8;
  rm = rm_high_byte + rm_low_byte;
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
  AvailableEnergy =  AvailableEnergy * pow(10,-3); 
  data_str += "," +  String(AvailableEnergy);
  AvailableEnergy =AvailableEnergy *4;
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
   data_str +=  String(int(SerialNumber));
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

/************************* Function to read True RC **********************************/
void readTrueRc() {
  Wire.beginTransmission(address);
  Wire.write(0x68);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int trc_low_byte = Wire.read();

  Wire.beginTransmission(address);
  Wire.write(0x69);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int  trc_high_byte = Wire.read();
  trc_high_byte = trc_high_byte << 8;
  TrueRc = trc_high_byte + trc_low_byte;
}

/************************* Function to read TrueFcc **********************************/
void readTrueFcc() {
  Wire.beginTransmission(address);
  Wire.write(0x6A);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int tfcc_low_byte = Wire.read();
  Wire.beginTransmission(address);
  Wire.write(0x6B);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int  tfcc_high_byte = Wire.read();
  tfcc_high_byte = tfcc_high_byte << 8;
  TrueFcc = tfcc_high_byte + tfcc_low_byte;
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

/************************* Function to read Flags **********************************/
void readFlags() {
  Wire.beginTransmission(address);
  Wire.write(0x0E);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int flags_low_byte = Wire.read();

  Wire.beginTransmission(address);
  Wire.write(0x0F);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  unsigned int  flags_high_byte = Wire.read();
  flags_high_byte = flags_high_byte << 8;
  Flags = flags_high_byte + flags_low_byte;
}
/************************* Function to get bit at a particular position **********************************/
int get_bit_status(unsigned int bitdata , int pos) {
  bitdata = bitdata / pow(10, n - pos - 1);
  return bitdata % 10;
}


int scalefactor = 2 ;
void loop() {
  // put your main code here, to run repeatedly:
  //lcd.print-----batteryid, soc, a energy , temperature,       "   "  , cycle count,soh
  readSerialnumber();
  readTimeToFull();
  readcurrent();
  readAvailableEnergy();
  readsoc();
  readtemp(); 
  readCycleCount();
//  readvoltage();
 // readStateofHealth();  
//  ReadPackConfiguration();
  readTimeToEmpty();
  
  Serial.print(a[5]);
  Serial.print("voltage:");
  Serial.print(voltage);
   Serial.print(" Timetofull-");
  Serial.print(timef);
   Serial.print(" TimetoEmpty");
  Serial.print(timee);
   Serial.print(" current:");
  Serial.print(current);
  Serial.print(" s.no:");
  Serial.print(SerialNumber,0);
  Serial.print("  SoC:");
  Serial.print(soc);
  Serial.print(" SoH:");
  Serial.print(StateOfHealth,0);
  Serial.print(" Temp-");
  Serial.print(temperature,1);
  Serial.print(" cycles:");
  Serial.print(CycleCount);
  Serial.print(" Energy:");
  Serial.println(AvailableEnergy);
  lcd.setCursor(4, 0);
  lcd.print("#HYD20190001");
  lcd.setCursor(0, 1);
  lcd.print("SoC:");
  lcd.print(soc);
  lcd.print("%");
  lcd.setCursor(11, 1);  
  lcd.print("E:");
  lcd.print(AvailableEnergy,1);
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
  lcd.print(StateOfHealth,0);
  lcd.println("%");
 
 Serial.println(data_str);
 BTserial.println(data_str);
//log 3c cell data arduino- 1c,2c,3c    
  /************************************************************** Writing Data To SD Card *************************************************************/
#if 0
  while (!Serial) {
    ;
  }
 // Serial.print("Initializing SD card...");

  if (!SD.begin(4)) {
   // Serial.println("initialization failed!");
    while (1);
  }
  //Serial.println("initialization done.");

  LogFile = SD.open("test.txt", FILE_WRITE);
  if (LogFile) {
    Serial.print("Writing to test.txt...");
    LogFile.print("Voltage :");
    LogFile.print(voltage);
    LogFile.print(",  ");
    LogFile.print("Temperature :") ;
    LogFile.print(temperature);
    LogFile.print(",  ");
    LogFile.print("current :");
    LogFile.print(current);
    byte second, minute, Hour, dayOfWeek, dayOfMonth, month, Year;
    // retrieve data from DS3231
    readDS3231time(&second, &minute, &Hour, &dayOfWeek, &dayOfMonth, &month,
                   &Year);
    // send it to the serial monitor
    LogFile.print(Hour, DEC);
    // convert the byte variable to a decimal number when displayed
    LogFile.print(":");
    if (minute < 10)
    {
      LogFile.print("0");
    }
    LogFile.print(minute, DEC);
    LogFile.print(":");
    if (second < 10)
    {
      LogFile.print("0");
    }
    LogFile.print(second, DEC);
    LogFile.print(" ");
    LogFile.print(dayOfMonth, DEC);
    LogFile.print("/");
    LogFile.print(month, DEC);
    LogFile.print("/");
    LogFile.print(Year, DEC);
    LogFile.print(" Day of week: ");
    switch (dayOfWeek) {
      case 1:
        LogFile.println("Sunday");
        break;
      case 2:
        LogFile.println("Monday");
        break;
      case 3:
        LogFile.println("Tuesday");
        break;
      case 4:
        LogFile.println("Wednesday");
        break;
      case 5:
        LogFile.println("Thursday");
        break;
      case 6:
        LogFile.println("Friday");
        break;
      case 7:
        LogFile.println("Saturday");
        break;
    }
    // close the file:
    LogFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
#endif
  
#if 0
  LogFile = SD.open("test.txt");
  if (LogFile) {
    Serial.println("test.txt:");
    while (LogFile.available()) {
      Serial.write(LogFile.read());
    }
    LogFile.close() ;
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
#endif
  delay(4000);
  data_str = "" ;
}
