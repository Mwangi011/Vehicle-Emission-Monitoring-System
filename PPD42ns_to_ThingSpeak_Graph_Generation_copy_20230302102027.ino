#include <SoftwareSerial.h>
#define RX 2
#define TX 3
String AP = "OPPO A1k";
String PASS = "happyhealthy";
String API = "C74MTTRQVRO16B13";
String HOST = "api.thingspeak.com";
String PORT = "80";
String field = "field1";
boolean found = false;
int countTrueCommand;
int countTimeCommand;
int pin = 8;       //declaring a pin for this sensor, which is D8 
unsigned long duration; //declare extended size variables for number storage only
unsigned long starttime;  //declare extended size variables for number storage only
unsigned long sampletime_ms = 2000; 
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;
SoftwareSerial esp8266(RX,TX);


void setup(){
  Serial.begin(115200);
  esp8266.begin(115200);
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1", 5,"OK");
  sendCommand("AT+CWJAP=\""+AP +"\",\""+ PASS +"\"",20,"OK");
  }

void loop(){
  duration = pulseIn(pin, LOW);   //reads the pulse, whether it is HIGH or LOW 
  lowpulseoccupancy = lowpulseoccupancy+duration;
  String getData = "GET /update?api_key="+ API +"&"+ field +"="+String(concentration);
  sendCommand("AT+CIPMUX=1",5,"OK");
  sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
  sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
  esp8266.println(getData);delay(1500);countTrueCommand++;
  sendCommand("AT+CIPCLOSE=0",5,"OK");
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
      Serial.println("Normal air concentration");
    }
    else{
      Serial.print("Low dust concentration");
    }
    Serial.println("\n");
    lowpulseoccupancy = 0;   //reset to 0 for a new reading to be gained
    starttime = millis();
  }
  
}
int getSensorData(){
  return random(1000); // Replace with your own sensor code
}

void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime*1))
  {
    esp8266.println(command);//at+cipsend
    if(esp8266.find(readReplay))//ok
    {
      found = true;
      break;
    }
  
    countTimeCommand++;
  }
  
  if(found == true)
  {
    Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  
  if(found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  
  found = false;
 }

