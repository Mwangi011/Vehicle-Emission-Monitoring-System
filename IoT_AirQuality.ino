#include <SoftwareSerial.h>
#include <String.h>
#include <Wire.h> 
#include "s300i2c.h"

SoftwareSerial gprsSerial(7,8);


int pin = 8;       //declaring a pin for this sensor, which is D8 
unsigned long duration; //declare extended size variables for number storage only
unsigned long starttime;  //declare extended size variables for number storage only
unsigned long sampletime_ms = 2000; 
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;
String field2 = "co2";
String field1 = "concentration";
String API = "F6O1KABJHLY38DHA";


S300I2C s3(Wire);
unsigned int co2;

void setup()
{
  gprsSerial.begin(9600); // the GPRS baud rate   
  Serial.begin(9600);    // the GPRS baud rate 
  delay(1000);

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
 
void loop()
{
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

    Serial.println("\n");
    lowpulseoccupancy = 0;   //reset to 0 for a new reading to be gained
    starttime = millis();    
  }  
   
if (gprsSerial.available())
  Serial.write(gprsSerial.read());
 
  gprsSerial.println("AT");
  delay(1000);
 
  gprsSerial.println("AT+CPIN?");
  delay(1000);
 
  gprsSerial.println("AT+CREG?");
  delay(1000);
 
  gprsSerial.println("AT+CGATT?");
  delay(1000);
 
  gprsSerial.println("AT+CIPSHUT");
  delay(1000);
 
  gprsSerial.println("AT+CIPSTATUS");
  delay(2000);
 
  gprsSerial.println("AT+CIPMUX=0");
  delay(2000);
 
  ShowSerialData();
 
  gprsSerial.println("AT+CSTT=\"airtelgprs.com\"");//start task and setting the APN,
  delay(1000);
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIICR");//bring up wireless connection
  delay(3000);
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIFSR");//get local IP adress
  delay(2000);
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIPSPRT=0");
  delay(3000);
 
  ShowSerialData();
  
  gprsSerial.println("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"");//start up the connection
  delay(6000);
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIPSEND");//begin send data to remote server
  delay(4000);
  ShowSerialData();
  
  String str="GET https://api.thingspeak.com/update?api_key=F6O1KABJHLY38DHA&field1=" + String(concentration) +"&field2="+String(co2);
  Serial.println(str);
  gprsSerial.println(str);//begin send data to remote server
  
  delay(4000);
  ShowSerialData();
 
  gprsSerial.println((char)26);//sending
  delay(5000);//waitting for reply, important! the time is base on the condition of internet 
  gprsSerial.println();
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIPSHUT");//close the connection
  delay(100);
  ShowSerialData();
} 
void ShowSerialData()
{
  while(gprsSerial.available()!=0)
  Serial.write(gprsSerial.read());
  delay(5000); 
  
}
