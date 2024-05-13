#include <Arduino.h>
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>
#include <EasyButton.h>

#define buttonPin 0 // D3 - Flash button
#define longPressDuration 1000
#define gpsTX 14 // D5
#define gpsRX 12 // D6
#define gsmRX 13 // D7
#define gsmTX 15 // D8

SoftwareSerial gpsSerial(gpsRX, gpsTX);
SoftwareSerial gsmSerial(gsmRX, gsmTX);
TinyGPSPlus gps;
EasyButton button(buttonPin);


void fetchGPS()
{
	if (gps.location.isValid())
	{
		gsmSerial.print("At ");
		gsmSerial.print("https://www.google.com/maps/search/?api=1&query=");
		gsmSerial.print(gps.location.lat(), 8);
		gsmSerial.print(",");
		gsmSerial.println(gps.location.lng(), 8);
	}
	if (gps.date.isValid() && gps.time.isValid())
	{
		gsmSerial.print("On ");
		gsmSerial.print(gps.date.day());
		gsmSerial.print("/");
		gsmSerial.print(gps.date.month());
		gsmSerial.print("/");
		gsmSerial.println(gps.date.year());
		int h = gps.time.hour(), m = gps.time.minute(), s = gps.time.second();
		if (m >= 30)
		{
			m -= 30;
			h += 1;
		} else m += 30;
		h += 5;
		gsmSerial.print("At ");
		if (h < 10) gsmSerial.print("0");
		gsmSerial.print(h);
		gsmSerial.print(m < 10 ? ":0" : ":");
		gsmSerial.print(m);
		gsmSerial.print(s < 10 ? ":0" : ":");
		gsmSerial.print(s);
	} else gsmSerial.println("error");
}

void sendSMS()
{
	delay(1000);
	gsmSerial.println("AT+CMGF=1");
	delay(1000);
	gsmSerial.println("AT+CMGS=\"+918004237112\"");
	delay(1000);
	fetchGPS();
	gsmSerial.write(26);
	delay(1000);
	gsmSerial.println("AT+CMGF=1");
	delay(1000);
	gsmSerial.println("AT+CMGS=\"+919909222351\"");
	delay(1000);
	fetchGPS();
	gsmSerial.write(26);
	delay(10000);
}

void setup()
{
	button.begin();
	gpsSerial.begin(9600);
	gsmSerial.begin(9600);
	button.onPressedFor(longPressDuration, sendSMS);
	delay(2000);
}

void loop()
{
	while (gpsSerial.available() > 0)
	{
		gps.encode(gpsSerial.read());
		button.read();
	}
	while (gsmSerial.available() > 0)
	{
		if (gsmSerial.readStringUntil('\n').indexOf("RING") >= 0)
		{
			gsmSerial.println("ATH");
			sendSMS();
		}
	}
}
