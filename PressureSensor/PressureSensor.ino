/*
Water motor control code


*/

// include the library code:
#include <EEPROM.h>
#include <LiquidCrystal.h>


#define ON_INTERVL					5	//On Interval in minutes
#define OFF_INTERVAL				60	//Off interval in minutes
#define ON_INTERVAL_FIRST_RUN		6	//On interval in minuuts for first run after switch


#define MOTOR_PIN A0
unsigned short sOnInterval = ON_INTERVL;
unsigned short sOffInterval = OFF_INTERVAL;
unsigned long int pmillis=0, nowmillis=0, on_intervel_val = sOnInterval * 60000 , off_intervel = (sOffInterval * 60000) - on_intervel_val, on_intervel=0;
unsigned long int total_runtime = 0;
long int diffMills = 0;

bool bFirstRun = true;

typedef enum state {TO_BE_ON, TO_BE_OFF};
state Current_state = TO_BE_OFF;

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 6, en = 5, d4 = 8, d5 = 9, d6 = 10, d7 = 11;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

class CTimeDelay
{
private:
	bool isInDelay;
public:
	CTimeDelay()
	{
		isInDelay = false;
	}
	void TimeDelay(void (*DelayFunc)(), uint16_t delay)
	{
		long mills = millis();
		if(((mills%delay) < 20 ) && !isInDelay)
		{
			DelayFunc();
			isInDelay=true;
		}
		else
		{
			if(((mills%delay) > 20) && isInDelay)
			{
				isInDelay=false;
			}
		}
	}
};

CTimeDelay* ptimeDelayForPressurreReads;

//bool isInDelay = false;
//void TimeDelay(void (*DelayFunc)(), uint16_t delay)
//{
//	long mills = millis();
//	if(((mills%delay) < 20 ) && !isInDelay)
//	{
//		DelayFunc();
//		isInDelay=true;
//	}
//	else
//	{
//		if(((mills%delay) > 20) && isInDelay)
//		{
//			isInDelay=false;
//		}
//	}
//}

int total=0;
int index=0;
int average=0;
const byte numReadings = 10;
const int sensorUpperLimit = 725;
const int sensorLowerLimit = 720;
int readings[numReadings];
bool isPumpStartable = true;

void PrintPressure()
{
	static bool bPressureSmoothingCompleted = false;
  // subtract the last reading:
  total = total - readings[index];
  // read from the sensor:
  readings[index] = analogRead(A2);
  // add the reading to the total:
  total = total + readings[index];
  // advance to the next position in the array:
  index = index + 1;



  // calculate the average:
  average = total / numReadings;

	lcd.setCursor(8,0);
	lcd.print(" ");
	lcd.print(average);
	lcd.print("/");
	lcd.print(readings[index -1]);



	// if we're at the end of the array...
	if (index >= numReadings) {
    // ...wrap around to the beginning:
	  index = 0;
	  bPressureSmoothingCompleted = true;
	};

	if(bPressureSmoothingCompleted)
	{
		static int count = 0;
		if(average >= sensorUpperLimit)
		{
			count++;
			if(count >= 10)
			{
				//isPumpStartable = false; // ddb - commented as a workaround for the sensor always detect as tank full
			}
		} 
		else if(average <= sensorLowerLimit)
		{
			count++;
			if(count >= 10)
			{
				isPumpStartable = true;
			}
		}
		else
		{
			count = 0;
		}
	}
}



void setup() {
	//Serial.begin(115200);
	analogReference(INTERNAL);
	
	// set up the LCD's number of columns and rows:
	lcd.begin(16, 2);

	on_intervel = bFirstRun? (ON_INTERVAL_FIRST_RUN * 60000) : on_intervel_val;
	
	pinMode(MOTOR_PIN,OUTPUT);
	//pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);
	digitalWrite(MOTOR_PIN, LOW);
	lcd.setCursor(0, 0);
	lcd.print("ON for ");
	lcd.print(on_intervel/60000);
	lcd.print("m");

	//int address = 0;
	//EEPROM.get(address, total_runtime);
	//Serial.println(total_runtime);

	//print_total_run_time();

	for (int i = 0; i < numReadings; i++) {
		readings[i] = 0;
	}
	ptimeDelayForPressurreReads = new CTimeDelay();
}

void loop() {
	ptimeDelayForPressurreReads->TimeDelay(PrintPressure, 1000);
	//TimeDelay(PrintPressure, 1000);

	nowmillis = millis();
	diffMills = nowmillis - pmillis;

	on_intervel = bFirstRun? (ON_INTERVAL_FIRST_RUN * 60000) : on_intervel_val;

	switch(Current_state){
	case TO_BE_ON: //(motor is currently off)
		{  
			lcd.setCursor(0, 1);
			lcd.print("On in  ");
			lcd_print_time(off_intervel-diffMills);
			if(diffMills > off_intervel && isPumpStartable)
			{
				pmillis = nowmillis;
				lcd.clear();
				lcd.setCursor(0, 0);
				lcd.print("ON for ");
				lcd.print(on_intervel/60000);
				lcd.print("m");
				Current_state = TO_BE_OFF;
				if(isPumpStartable == true)
				{
					digitalWrite(MOTOR_PIN,LOW);
				}
				//print_total_run_time();
			}
		} 
		break;
	case TO_BE_OFF: //(motor is currently on)
		{ 
			lcd.setCursor(0, 1);
			lcd.print("Off in ");
			lcd_print_time(on_intervel-diffMills);
			if(diffMills> on_intervel || !isPumpStartable)
			{
				bFirstRun = false;
				pmillis = nowmillis;
				//Serial.println("Off in ");
				lcd.clear();
				lcd.setCursor(0,0);
				lcd.print("OFF for ");
				lcd.print(off_intervel/60000);
				lcd.print("m");
				Current_state = TO_BE_ON;
				digitalWrite(MOTOR_PIN,HIGH);
				total_runtime += (on_intervel/60000);
				EEPROM.put(0,total_runtime);
				//print_total_run_time();
			}
		} 
		break;
	}
}

void led_blink(short frequency, short interval)
{
	if((diffMills % frequency) < interval)
	{
		digitalWrite(LED_BUILTIN, HIGH);
	}
	else
	{
		digitalWrite(LED_BUILTIN, LOW);
	}
}

//void print_total_run_time()
//{
//	EEPROM.get(0,total_runtime);
//	lcd.print(total_runtime);
//	lcd.print("M");
//}



#pragma region PrintTimeInMillsToLCD
unsigned short get_seconds_in_a_miniute(long int mills)
{
	return (mills / 1000) % 60;
}

unsigned short get_minutes_in_a_hour(long int mills)
{
	return (mills / 60000) % 60;  
}

unsigned short get_hours(long int mills)
{
	return mills / (3600000);
}

void lcd_print_time(long int mills)
{
	lcd.setCursor(7,1);
	short hours = get_hours(mills);
	if(hours < 10)
	{
		lcd.print("0");
		lcd.print(hours);
	}
	else
	{
		lcd.print(hours);
	}
	lcd.print(":");
	
	//lcd.setCursor(11,1);
	short mins = get_minutes_in_a_hour(mills);
	if(mins < 10)
	{
		lcd.print("0");
		lcd.print(mins);
	}
	else
	{
		lcd.print(mins);
	}
	lcd.print(":");

	//lcd.setCursor(14,1);
	short secs = get_seconds_in_a_miniute(mills);
	if(secs<10)
	{
		lcd.print("0");
		lcd.print(secs);
	}
	else
	{
		lcd.print(secs);
	}
}
#pragma endregion


