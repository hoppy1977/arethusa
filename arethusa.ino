#include <Wire.h>

#include "src/NewliquidCrystal/LiquidCrystal_I2C.h"

// Digital pins
const byte pumpButtonPin = 3;
const byte sensorPowerPin = 5;
const byte pumpRelayPin = 7;
const byte backlightPin = 13;

// Analog pins
const byte moistureSensorPin = A0;
const byte intervalPotPin = A2;

// Moisture sensor constants
const int dryValue = 450;
const int wetValue = 650;
const int friendlyDryValue = 0;
const int friendlyWetValue = 100;

// Interval potentiometer constants
const int intervalPotMinimumValue = 1023;
const int intervalPotMaximumValue = 0;
const int intervalMinimumValue = 1;
const int intervalMaximumValue = 72;

// 3600000 = 1 hour
// 1800000 = 30 minutes
const long intervalLength = 3600000;
const long moistureReadingDelay = 5000;

int mappedMoistureLevel = 0;

long lastPumpTime = 0;
long lastMoistureReadingTime = 0;

// LCD
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

void setup()
{
	Serial.begin(9600);

	// Digital pins
	pinMode(pumpButtonPin, INPUT);

	pinMode(sensorPowerPin, OUTPUT);
	digitalWrite(sensorPowerPin, LOW);

	pinMode(pumpRelayPin, OUTPUT);
	digitalWrite(pumpRelayPin, LOW);

	pinMode(backlightPin, OUTPUT);
	digitalWrite(backlightPin, HIGH);

	// Analog pins
	pinMode(moistureSensorPin, INPUT);
	pinMode(intervalPotPin, INPUT);

	Serial.println("Setting up the LCD...");
	lcd.begin(16, 2);

	// Splash screen
	printStartup();
	delay(500);

	lcd.clear();
}

void loop()
{
	Serial.println("Beginning loop...");

	int mappedInterval = getInterval();

	Serial.println("Interval: " + String(mappedInterval));
	Serial.println("Moisture: " + String(mappedMoistureLevel));

	lcd.setCursor(0, 0);
	lcd.print(padForLcd("H:" + String(mappedInterval) + "  M:" + String(mappedMoistureLevel)));

	if (digitalRead(pumpButtonPin) == HIGH)
	{
		// The user has pushed the pump button
		pump();
	}

	long timeToWait = (lastPumpTime + (mappedInterval * intervalLength)) - millis();
	if (timeToWait <= 0)
	{
		pump();
	}
	else
	{
		float percentageOfInterval = ((double)timeToWait / (double)(mappedInterval * intervalLength));
		int numberOfDots = (percentageOfInterval * 16) + 1;
		for (int i = 0; i < 16; i++)
		{
			lcd.setCursor(i, 1);

			if (i < numberOfDots)
			{
				lcd.print(".");
			}
			else
			{
				lcd.print(" ");
			}
		}
	}

	// Wait between readings
	// This ensures the moisture sensor is not powered up all the time to prevent it from corroding
	long timeToWaitBeforeNextMoistureReading = (lastMoistureReadingTime + moistureReadingDelay) - millis();
	if (timeToWaitBeforeNextMoistureReading <= 0)
	{
		lcd.setCursor(15, 0);
		lcd.print("*"); // A visual cue that we are now updating the moisture value

		readMoisture();
	}
}

int getInterval()
{
	int rawValue = analogRead(intervalPotPin);
	return map(rawValue, intervalPotMinimumValue, intervalPotMaximumValue, intervalMinimumValue, intervalMaximumValue);
}

void readMoisture()
{
	digitalWrite(sensorPowerPin, HIGH);
	delay(250);
	lastMoistureReadingTime = millis();
	int rawValue = analogRead(moistureSensorPin);
	digitalWrite(sensorPowerPin, LOW);

	int mappedValue = map(rawValue, dryValue, wetValue, friendlyDryValue, friendlyWetValue);
	if(mappedValue < 0)
	{
		mappedMoistureLevel = 0;
	}
	else
	{
		mappedMoistureLevel = mappedValue;
	}
}

void pump()
{
	lcd.setCursor(0, 0);
	lcd.print(padForLcd("*** PUMPING ***"));

	digitalWrite(pumpRelayPin, HIGH);
	waitWithProgress(5000);
	digitalWrite(pumpRelayPin, LOW);

	lcd.setCursor(0, 0);
	lcd.print(padForLcd(""));

	lastPumpTime = millis();
}

void waitWithProgress(int waitTime)
{
	lcd.setCursor(0, 1);
	lcd.print(padForLcd(""));

	int secondsPerDot = waitTime / 16;

	int dotCounter = 0;
	while (dotCounter < 16)
	{
		delay(secondsPerDot);
		lcd.setCursor(dotCounter, 1);
		lcd.print(".");

		dotCounter++;
	}
}

String padForLcd(String text)
{
	while (text.length() < 16)
	{
		text.concat(' ');
	}

	return text;
}

void printStartup()
{
	lcd.clear();

	lcd.setCursor(0, 0);
	lcd.print(padForLcd("Hopatech Systems"));
}
