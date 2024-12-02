#include <Adafruit_FONA.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include "s300i2c.h"

#define RX 7
#define TX 8
#define RST 7

int pin = 8;       //declaring a pin for this sensor, which is D8 
unsigned long duration; //declare extended size variables for number storage only
unsigned long starttime;  //declare extended size variables for number storage only
unsigned long sampletime_ms = 2000; 
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;
String field1 = "concentration";
String field2 = "co2";
String API = "F6O1KABJHLY38DHA";
S300I2C s3(Wire);
unsigned int co2;

SoftwareSerial SIM900ss = SoftwareSerial(TX, RX);
Adafruit_FONA SIM900 = Adafruit_FONA(RST);

char http_cmd[80];
String get_data  = "api.thingspeak.com/update?api_key=" + API + "&" + field1 + "=" + float(concentration) + API + "&" + field2 + "=" +int(co2);

int net_status;

uint16_t statuscode;
int16_t length;
String response = "";
char buffer[512];

boolean gprs_on = false;
boolean tcp_on = false;

void setup() {
  while (!Serial);//
  
  Serial.begin(9600);
  SIM900ss.begin(9600); // if you're using software serial

  Serial.println("Dust concentration and Carbon dioxide levels to ThingSpeak");
  Serial.println("Initializing SIM900L....");

  
  if (!SIM900.begin(SIM900ss)) {            
    Serial.println("Couldn't find SIM900L");
    while (1);
  }
  else{
  Serial.println("SIM900L is OK"); 
  delay(1000);
  }
  
  Serial.println("Waiting to be registered to network...");
  net_status = SIM900.getNetworkStatus();
  while(net_status != 1){
     net_status = SIM900.getNetworkStatus();
     delay(2000);
  }
  Serial.println("Registered to home network!");
  Serial.print("Turning on GPRS... ");
  delay(2000); 
  while(!gprs_on){
    if (!SIM900.enableGPRS(true)){  
        Serial.println("Failed to turn on GPRS");
        Serial.println("Trying again...");
        delay(2000);
        gprs_on = false;
    }else{
        Serial.println("GPRS now turned on");
        delay(2000);
        gprs_on = true;   
    } 
  }

  Wire.begin();  
  s3.begin(S300I2C_ADDR);
  delay(10000); // 10sec wait.
  s3.wakeup();
  s3.end_mcdl();
  s3.end_acdl();
  Serial.println("START S300I2C");
  pinMode(8,INPUT);     //set D8 (digital pin 8) as an input pin
  starttime = millis();    //starttime, to read the current time in milliseconds
}

void loop() {    
  co2 = s3.getCO2ppm();
  Serial.print("CO2=");
  Serial.println(co2);
  delay(30000); // 30sec wait

  duration = pulseIn(pin, LOW);   //reads the pulse, whether it is HIGH or LOW 
  lowpulseoccupancy = lowpulseoccupancy+duration;
  if ((millis()-starttime) >= sampletime_ms) //if the sampel time = = 30s
  {
    //Calculation to determine the concentration of particles in the air
    ratio = lowpulseoccupancy/(sampletime_ms*10.0);  
    concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62;
    //The pow() function, in the sketch, is used to raise an integer, value or a variable to the power of something else 
    Serial.print("Concentration = ");
    Serial.print(concentration);
    Serial.println(" pcs/0.01cf");  //Particles per 1/100 cubic foot
    if (concentration > 5000){
        Serial.println("High dust concentration");
    }
    else if(concentration == 0.62){
      Serial.println("Normal dust concentration");
    }
    else{
      Serial.print("Low dust concentration");
    }
    Serial.println("\n");
    lowpulseoccupancy = 0;   //reset to 0 for a new reading to be gained
    starttime = millis();
  }
    while(!tcp_on){
      if (!SIM900.HTTP_GET_start(http_cmd, &statuscode, (uint16_t *)&length)) {
           Serial.println("Failed!");
           Serial.println("Trying again...");
           tcp_on = false;
      }else{
        tcp_on = true;
          while (length > 0) {
           while (SIM900.available()) {
             char c = SIM900.read();
             response += c;
             length--;
           }
        }
        Serial.println(response);
        if(statuscode == 200){
          Serial.println("Success!");
          tcp_on = false;
        }
      }
      delay(2000);
    }
    delay(2000);
}