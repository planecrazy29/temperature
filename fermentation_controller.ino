#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>

volatile long lastDebounceTimeU = 0;
volatile long lastDebounceTimeD = 0;
#define ONE_WIRE_BUS 4
#define debounceDelay 300

byte buttonInterruptUp = 0;
byte buttonInterruptDown = 1;


LiquidCrystal lcd(12, 11, 10, 9, 8, 6);
double currentTemp = 99.0;
double setTemp = 70.00;


OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


const int COOLING = 1;
const int RESTING = 0;
const double precision = 0.3; //.5 degrees precision

int minOn = 1; //2 mins
int maxOn = 3600; //1h
int minOff = 1; //10m

int rest_count;
int cool_count;
int fridge_status;
int startup_time = 3000;


const int fridge_relay = 5;

boolean tooWarm(){
return (currentTemp >= (setTemp+precision));
}

boolean restLongEnough(){
return (rest_count >= minOff);
}

boolean tooCold(){
return (currentTemp <= setTemp); //temp keeps dropping after temp is reached
}

boolean onLongEnough(){
return(cool_count >= minOn);
}

boolean onTooLong(){
return (cool_count >= maxOn);
}

void setup() {
digitalWrite(fridge_relay, LOW); //start with fridge off, relay is active low

lcd.begin(16, 2);
//Serial.begin(9600);
lcd.print("Beer Cooler");

attachInterrupt(buttonInterruptUp, tempUp, RISING);
attachInterrupt(buttonInterruptDown, tempDown, RISING);
sensors.begin();
pinMode(fridge_relay, OUTPUT);
fridge_status = RESTING;
rest_count = minOff;
cool_count = 0;
} 

void loop() {
delay(startup_time); //allow lcd to be initialized first
startup_time = 0;

getTemp();
lcdPrint();

if(fridge_status == RESTING){
if(tooWarm() && restLongEnough()){
//turn fridge on after temp has risen and waiting for compressor to cool down
digitalWrite(fridge_relay, HIGH);
delay(2000); 
cool_count = 0;
fridge_status = COOLING;
}
rest_count++;
}else{//fridge status is COOLING

if(onTooLong() || (onLongEnough() && tooCold()))
{
//turn the fridge off if its cool enough or compressor on too long
digitalWrite(fridge_relay, LOW);
delay(2000); 
rest_count = 0;
fridge_status = RESTING;
}
cool_count++;
} 
delay(1000);
}


void tempUp(){//dont feel like adding a debouncing circuit.. this is good enough
if((millis()-lastDebounceTimeU) > debounceDelay){
lastDebounceTimeU = millis();
if(setTemp < 100.00){
setTemp += 0.5;
lcdPrint();
}
}
}

void tempDown(){
if((millis()-lastDebounceTimeD) > debounceDelay){
lastDebounceTimeD = millis();
if(setTemp > 30.00){
setTemp -= 0.5;
lcdPrint();
}
}
}


void getTemp(){
sensors.requestTemperatures();
double t = sensors.getTempFByIndex(0);
if(t > 0 && t < 120){ //faulty wiring causes weird temperatures..
currentTemp = t;
} 
}


void lcdPrint(){
lcd.begin(16,2); //sometimes lcd goes blank, i guess this fixes it....
char tbuff[6];
tbuff[0] = '0'+((int)currentTemp/10%10);
tbuff[1] = '0'+((int)currentTemp%10);
tbuff[2] = '.';
tbuff[3] = '0'+((int)(currentTemp*10)%10);
tbuff[4] = '0'+((int)(currentTemp*100)%10);
tbuff[5] = '\0';

char sbuff[6];
sbuff[0] = '0'+((int)setTemp/10%10); 
sbuff[1] = '0'+((int)setTemp%10);
sbuff[2] = '.';
sbuff[3] = '0'+((int)(setTemp*10)%10);
sbuff[4] = '\0';
lcd.clear();
lcd.setCursor(0,0);
lcd.print("TEMP:");
lcd.setCursor(6,0);
lcd.print(tbuff);

lcd.setCursor(1,1);
lcd.print("SET:");
lcd.setCursor(6,1);
lcd.print(sbuff);
lcd.print(" ");

//print a "*" on the display if the fridge is on
lcd.setCursor(12,0);
if(fridge_status == COOLING){
lcd.print("Cool");
}else lcd.print(" ");
lcd.display();

} 


