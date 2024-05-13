#include <Arduino.h>

#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <TinyGPSPlus.h>
#include <EasyButton.h>

#define buttonPin 0 // D3 - Flash button
#define longPressDuration 3000
#define gpsTX 14 // D5
#define gpsRX 12 // D6

WiFiUDP udp;
SoftwareSerial gpsSerial(gpsRX, gpsTX);
TinyGPSPlus gps;
EasyButton button(buttonPin);

unsigned int localPort = 1111;
char incomingPacket[1];

char replyPacket[120];

void fetchGPS()
{
	replyPacket[0] = 0;
	char temp[12];
	if (gps.location.isValid())
	{
		strcat(replyPacket, "At \n https://www.google.com/maps/search/?api=1&query=");
		dtostrf(gps.location.lat(), 11, 8, temp);
		strcat(replyPacket, temp);
		strcat(replyPacket, ",");
		dtostrf(gps.location.lng(), 11, 8, temp);
		strcat(replyPacket, temp);
	}
	if (gps.date.isValid() && gps.time.isValid())
	{
		strcat(replyPacket, "\nOn ");
		itoa(gps.date.day(), temp, 10);
		strcat(replyPacket, temp);
		strcat(replyPacket, "/");
		itoa(gps.date.month(), temp, 10);
		strcat(replyPacket, temp);
		strcat(replyPacket, "/");
		itoa(gps.date.year(), temp, 10);
		strcat(replyPacket, temp);
		int h = gps.time.hour(), m = gps.time.minute(), s = gps.time.second();
		if (m >= 30)
		{
			m -= 30;
			h += 1;
		}
		else
			m += 30;
		h += 5;
		strcat(replyPacket, "\nAt ");
		if (h < 10)
			strcat(replyPacket, "0");
		itoa(h, temp, 10);
		strcat(replyPacket, temp);
		strcat(replyPacket, m < 10 ? ":0" : ":");
		itoa(m, temp, 10);
		strcat(replyPacket, temp);
		strcat(replyPacket, s < 10 ? ":0" : ":");
		itoa(s, temp, 10);
		strcat(replyPacket, temp);
	}
	else
		strcat(replyPacket, "error");
}

void sendToPhone()
{
	Serial.println("button pressed");
	fetchGPS();
	udp.beginPacket({192, 168, 4, 2}, 2222);
	udp.write(replyPacket);
	udp.endPacket();
	Serial.println("sent to phone");
}

void setup()
{
	Serial.begin(9600);
	WiFi.softAP("eButton", "", 1, 0, 1);
	button.begin();
	gpsSerial.begin(9600);
	button.onPressedFor(longPressDuration, sendToPhone);
	udp.begin(localPort);
	delay(2000);
}

void loop()
{
	while (gpsSerial.available() > 0)
	{
		gps.encode(gpsSerial.read());
		button.read();

		if (udp.parsePacket())
		{
			Serial.printf("Received bytes from %s, port %d\n", udp.remoteIP().toString().c_str(), udp.remotePort());
			udp.read(incomingPacket, 1);
			if (incomingPacket[0] == 69)
				sendToPhone();

			Serial.printf("UDP packet contents: %s\n", incomingPacket);
		}
	}
}
