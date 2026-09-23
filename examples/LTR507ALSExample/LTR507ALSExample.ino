// Example sketch for the LTR-507 Ambient Light Sensor
// Copyright (C) muman.ch, 2025.09.08
// email: info@muman.ch
// See the blog post for details
// https://muman.ch/muman/index.htm?muman-light-sensors.htm

#include <Wire.h>

// for uint and ulong 
typedef unsigned int uint;
typedef unsigned long ulong;

// This is useful for debug output
#ifdef DEBUG
void LogError(char* msg, char* file, unsigned int line)
{
	char buf[100];
	char* fname = strrchr(file, '\\');
	fname = fname ? fname + 1 : file;
	sprintf(buf, "%lu %s %s %u", millis(), msg, fname, line);
	Serial.println(buf);
	Serial.flush();
}

//extern void LogError(char* msg, char* file, unsigned int line)
#define LOGERROR(msg) LogError(msg, __FILE__, __LINE__)
#define ASSERT(b) if(!(b)) LOGERROR("Assert failed")
#else
#define LOGERROR(s)
#define ASSERT(b)
#endif

#include "LTR507ALSOpticalSensor.h"
LTR507ALSOpticalSensor ltr;

void setup() 
{
	Serial.begin(115200);
	delay(1000);

	Serial.println("\n\rSTM32 Started...\n\n\r");
	Serial.flush();

	Wire.begin();
	Wire.setClock(400000);
	Wire.setTimeout(100);

	ltr.begin(&Wire, 0x3A);

	// configure the ambient light sensor (default settings)
	ltr.configureALS(0, 4, 3);

	// turn on the ambient light sensor
	ltr.setMode(true, false);

	//TODO Serial.print() these if you fancy it
	uint i1, i2, i3;
	ltr.readIDs(&i1, &i2, &i3);
}

void loop() 
{
	char buf[200];

	// get new lux reading
	bool newData;
	long ilux;
	ltr.readLux(&ilux, &newData);
	if (!newData)
		return;

	ulong adc1, adc2;
	ltr.readRawAdcValues(&adc1, &adc2);

	float ratio = (float)adc2 / (float)adc1;
	sprintf(buf, "\nratio=%.04f", ratio);
	Serial.println(buf);

	if (adc1 != 0 && adc2 != 0) {
		int cutoff = (int)(ratio * 256);
		sprintf(buf, "cutoff=%u", cutoff);
		Serial.println(buf);
	}
	else
		Serial.println("cutoff=0");
	
	sprintf(buf, "adc1=%08lx adc2=%08lx", adc1, adc2);
	Serial.println(buf);

	sprintf(buf, "lux=%ld  alsGain=%u\n", ilux, ltr.alsGain);
	Serial.println(buf);

	// automatic gain selection
	ltr.autoSelectGain(ilux);

	delay(500);
}
