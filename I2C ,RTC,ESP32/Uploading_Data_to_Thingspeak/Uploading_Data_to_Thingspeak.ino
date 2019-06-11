#include <Wire.h>
#include <WiFi.h>
String apiKey = "YOUr api key";              
const char *ssid =  "your wifi's ssid";
const char *pass =  "Password";
const char* server =   "184.106.153.149";   //this is the thingspeak serverand is a default
WiFiClient client;
#define address 0x55
String data_str ;
/********************************************************** DEFINING VARIABLES *****************************************************************************/
float  fcc, rm, temperature ;
int temperature_i, voltage_i, current_i, AvailableEnergy_i, timef_i, timee_i, cc_i, soc_i ;
signed current ;
unsigned int voltage, timef, timee ;
float  Flags,  AvailableEnergy, DesignCapacity, TrueRc, TrueFcc, StateOfHealth;
float SerialNumber;
int   CycleCount, soc ;
int voltagescalefactor, currentscalefactor ;
const int rs = 5, en = 6, d4 = 7, d5 = 8, d6 = 9, d7 = 10;

void setup() {
  Wire.begin();
  Serial.begin(115200);
  delay(10);
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
  current = current / 10;
  data_str += ","  + String(current);
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
void readsoc() {
  Wire.beginTransmission(address); //transmit to device
  Wire.write(0x02);
  Wire.endTransmission();
  delay(5);
  Wire.requestFrom(address, 1);
  soc = Wire.read(); // receive high byte
  soc = 100;
  data_str += "," +  String(soc);

  int range = soc / 4 ;
  data_str += "," + String(range);
}
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
}
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




void loop() {
  // put your main code here, to run repeatedly:
  readSerialnumber();
  readTimeToFull();
  readcurrent();
  readAvailableEnergy();
  readsoc();
  readtemp();
  readCycleCount();
  readTimeToEmpty();
  readvoltage();
  if (client.connect("184.106.153.149", 80))                                //   "184.106.153.149" or api.thingspeak.com
  {
    String postStr = "/update?key=";
    postStr += apiKey;
    if (temperature != temperature_i) {
      postStr += "&field1=";
      postStr += String(temperature);
    }
    if (voltage != voltage_i) {
      postStr += "&field2=";
      postStr += String(voltage);
    }
    if (current != current_i) {
      postStr += "&field3=";
      postStr += String(current);
    }
    if (timee != timee_i) {
      postStr += "&field4=";
      postStr += String(timee);
    }
    if (timef != timef_i) {
      postStr += "&field5=";
      postStr += String(timef);
    }
    if (AvailableEnergy != AvailableEnergy_i) {
      postStr += "&field6=";
      postStr += String(AvailableEnergy);
    }
    if (CycleCount != cc_i) {
      postStr += "&field7=";
      postStr += String(CycleCount);
    }
    postStr += "\r\n\r\n";
    client.print(String("GET ") + postStr + " HTTP/1.1\r\n" +
                 "Host: " + server + "\r\n" +
                 "Connection: close\r\n\r\n");

    Serial.println("%. Send to Thingspeak.");
  }
  temperature_i = temperature, voltage_i = voltage, current_i = current, AvailableEnergy_i = AvailableEnergy , timef_i = timef, timee_i = timee, cc_i = CycleCount ;
  client.stop();
  Serial.println("Waiting...");
  delay(4000);


}
