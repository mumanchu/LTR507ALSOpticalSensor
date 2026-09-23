#pragma once

//////////////////////////////////////////////////////////////////////////////////////////
// LiteOn LTR-507ALS Optical Sensor with I2C interface
// Copyright (C) muman.ch, 2025.09.05
// info@muman.ch
// https://github.com/mumanchu/LTR507ALSOpticalSensor
/*
See the muman blog for details
https://muman.ch/muman/index.htm?muman-light-sensors.htm

LTR-507 Data Sheet
https://optoelectronics.liteon.com/upload/download/DS86-2013-0014/LTR-507ALS-01_FINAL%20DS.pdf

The code was tested with the SolderedElectronics LTR507 board
https://soldered.com/documentation/ltr-507/overview/
*/

#include <Wire.h>

/* for example...
#ifdef DEBUG
#define LOGERROR(s) Serial.println(s); Serial.flush()
#define ASSERT(b) if (!(b)) LOGERROR("ASSERT failed")
#else
#define LOGERROR(s)
#define ASSERT(b)
#endif
*/


class LTR507ALSOpticalSensor
{
protected:
	TwoWire* wire;
	int i2cAdds;

public:
	uint alsGain, alsResolution;

	// 8-bit registers, see data sheet "6. Register Set"
	enum REG 
	{
		ALS_CONTR =				0x80,		// ALS operation mode control SW reset
		PS_CONTR =				0x81,		// PS operation mode control
		PS_LED = 				0x82,		// PS LED setting
		PS_N_PULSES = 			0x83,		// PS number of pulses
		PS_MEAS_RATE = 			0x84,		// PS measurement rate in active mode
		ALS_MEAS_RATE =			0x85,		// ALS measurement rate in active mode
		PART_ID =				0x86,		// Part Number ID and Revision ID
		MANUFAC_ID =			0x87,		// Manufacturer ID

		ALS_DATA_0 =			0x88,		// Direct ALS measurement, lower byte
		ALS_DATA_1 = 			0x89,		// Direct ALS measurement, upper byte
		ALS_PS_STATUS = 		0x8A,		// ALS and PS new data status
		PS_DATA_0 = 			0x8B,		// PS measurement data, lower byte
		PS_DATA_1 = 			0x8C,		// PS measurement data, upper byte

		ALS_DATA_CH1_0 =		0x8D,		// ALS measurement CH1 data, lower byte
		ALS_DATA_CH1_1 =		0x8E,		// ALS measurement CH1 data, mid byte
		ALS_DATA_CH1_2 =		0x8F,		// ALS measurement CH1 data, upper byte
		ALS_DATA_CH2_0 =		0x90,		// ALS measurement CH2 data, lower byte
		ALS_DATA_CH2_1 = 		0x91,		// ALS measurement CH2 data, mid byte
		ALS_DATA_CH2_2 = 		0x92,		// ALS measurement CH2 data, upper byte

		ALS_COEFF1_DATA_0 =		0x93,		// Coefficient for Clear diode, lower byte
		ALS_COEFF1_DATA_1 =		0x94,		// Coefficient for Clear diode, upper byte
		ALS_COEFF2_DATA_0 =		0x95,		// Coefficient for IR diode, lower byte
		ALS_COEFF2_DATA_1 =		0x96,		// Coefficient for IR diode, upper byte

		ALS_IRF_CUT_OFF =		0x97,		// ALS cut-off limit of IR factor
		INTERRUPT =				0x98,		// Interrupt settings
		PS_THRES_UP_0 =			0x99,		// PS interrupt upper threshold, lower byte
		PS_THRES_UP_1 =			0x9A,		// PS interrupt upper threshold, upper byte
		PS_THRES_LOW_0 =		0x9B,		// PS interrupt lower threshold, lower byte
		PS_THRES_LOW_1 =		0x9C,		// PS interrupt lower threshold, upper byte
		ALS_THRES_UP_0 =		0x9E,		// ALS interrupt upper threshold, lower byte
		ALS_THRES_UP_1 =		0x9F,		// ALS interrupt upper threshold, upper byte
		ALS_THRES_LOW_0 =		0xA0,		// ALS interrupt lower threshold, lower byte
		ALS_THRES_LOW_1 =		0xA1,		// ALS interrupt lower threshold, upper byte
		INTERRUPT_PERSIST =		0xA4		// ALS / PS Interrupt persist setting
	};

public:
	bool begin(TwoWire* wire, int i2cAddress);
	bool softwareReset();
	bool readIDs(uint* partNumberID, uint* revisionID, uint* manufacturerID);
	bool setMode(bool alsActive, bool psActive);
	
	bool readLux(long* lux, bool* newData);
	bool readLux(float* lux, bool* newData);
	bool readRawLux(long* lux, bool* newData);
	bool readProximity(uint* proximity, bool* overflow);
	bool readStatus(uint* interruptSource, bool* alsInterruptStatus, 
		bool* alsDataStatus, bool* psInterruptStatus, bool* psDataStatus);
	bool readRawAdcValues(ulong* adcCh1, ulong* adcCh2);
	bool autoSelectGain(long lux);
	
	bool configureALS(uint gain, uint resolution, uint rate);
	bool configurePS(uint pulseFreq, uint peakCurrent, uint pulseCount, uint rate);
	bool configureInterrupt(bool outputMode, bool polarity, uint interruptMode);
	bool configureALSCoeff(uint coeffCh1, uint coeffCh2);
	bool readALSCoeff(uint* coeffCh1, uint* coeffCh2);
	bool configureALSInterruptThreshold(uint upper, uint lower);
	bool readALSInterruptThreshold(uint* upper, uint* lower);
	bool configurePSInterruptThreshold(uint upper, uint lower);
	bool readPSInterruptThreshold(uint* upper, uint* lower);
	bool configureInterruptPersist(uint alsPersist, uint psPersist);
	bool readInterruptPersist(uint* alsPersist, uint* psPersist);
	bool configureALSIRFCutoff(uint cutoff);
	bool readALSIRFCutoff(uint* cutoff);

protected:
	bool writeRegisters(REG reg, const byte* values, uint length);
	bool readRegisters(REG reg, byte* values, uint length);
	bool readRegister(REG reg, byte* value);
	bool writeRegister(REG reg, byte value);
};


// Initialize Wire before calling this
//    Wire.begin();
//    Wire.setClock(1000000);
//    Wire.setTimeout(100);
//    ltr.begin(&Wire, 0x3a);
bool LTR507ALSOpticalSensor::begin(TwoWire* twoWire, int i2cAddress)
{
	wire = twoWire;
	i2cAdds = i2cAddress;
	return softwareReset();
}

// Set all registers to default values
// ALS and PS set to standby
bool LTR507ALSOpticalSensor::softwareReset()
{
	alsGain = 0;		// 0 = x1, 0..65355 lux (default)
	alsResolution = 4;	// 16-bit resolution (default)

	bool ok = writeRegister(ALS_CONTR, 0b00000100);
	delay(100);			// startup time, assume 100ms
	return ok;
}

// Read the hard-wired chip IDs
bool LTR507ALSOpticalSensor::readIDs(uint* partNumberID, uint* revisionID, uint* manufacturerID)
{
	byte data[2];
	if (!readRegisters(PART_ID, data, 2))
		return false;
	*partNumberID = data[0] >> 4;
	*revisionID = data[0] & 0x0f;
	*manufacturerID = data[1];
	return true;
}

// Sets active or standby mode for ALS and PS
// NOTE: Configure interrupts BEFORE setting Active mode
// alsActive : true = ambient light sensor ALS active; false = ALS standby
// psActive :  true = position sensor PS active;  false = PS standby
bool LTR507ALSOpticalSensor::setMode(bool alsActive, bool psActive)
{
	byte contr[2];
	if (!readRegisters(ALS_CONTR, contr, 2))
		return false;

	if (alsActive)
		contr[0] |= 0b00000010;
	else
		contr[0] &= ~0b00000010;
	if (psActive) 
		contr[1] |= 0b00000010;
	else
		contr[1] &= ~0b00000010;

	return writeRegisters(ALS_CONTR, contr, 2);
}

// Reads the ambient light reading as a long integer
// 0..65535|32767|655|327 lux according to the gain
// the lux value is adjusted for the configured 'alsGain'
// 'newData' is true if this is a new value, false if it's not been updated
// a lux value of -1 (0xFFFFFFFF) means overflow or saturation
bool LTR507ALSOpticalSensor::readLux(long* lux, bool* newData)
{
	long rawLux;
	if (!readRawLux(&rawLux, newData))
		return false;
	if (rawLux <= 0) {
		*lux = rawLux;
		return true;
	}

	// lux value depends on 'gain'
	long lux1 = rawLux;
	switch (alsGain) {
	case 0:			// x1, 1 lux/count
		//lux1 = rawLux;
		break;
	case 1:			// x2, 0.5 lux/count
		lux1 = rawLux >> 1;
		break;
	case 2:			// x100, 0.01 lux/count
		lux1 = (rawLux + 49) / 100;
		break;
	case 3:			// x200, 0.005 lux/count
		lux1 = (rawLux + 99) / 200;
		break;
	}
	*lux = lux1;

	return true;
}

// Reads the ambient light reading as a floating point value
// use this for very low lux levels
// 0..65535|32767.5|655.35|327.675 lux according to the gain
// the lux value is adjusted for the configured 'alsGain'
// 'newData' is true if this is a new value, false if it's not been updated
// a lux value of -1 means overflow or saturation
bool LTR507ALSOpticalSensor::readLux(float* lux, bool* newData)
{
	long rawLux;
	if (!readRawLux(&rawLux, newData))
		return false;
	float flux = (float)rawLux;

	// lux value depends on 'gain'
	if (rawLux > 0) {
		switch (alsGain) {
		case 0:			// x1, 1 lux/count
			break;
		case 1:			// x2, 0.5 lux/count
			flux /= 2.0f;
			break;
		case 2:			// x100, 0.01 lux/count
			flux /= 100.0f;
			break;
		case 3:			// x200, 0.005 lux/count
			flux /= 200.0f;
			break;
		}
	}
	*lux = flux;
	return true;
}

// Reads the lux value, unadjusted for the gain setting
// this returns 0..65535 regardless of the gain setting
// the actual lux value must be adjusted for the gain
// if saturation or overflow occurs it returns lux = -1
bool LTR507ALSOpticalSensor::readRawLux(long* lux, bool* newData)
{
	byte data[3];
	if (!readRegisters(ALS_DATA_0, data, 3))
		return false;
	long rawLux = (data[1] << 8) + data[0];
	*newData = (data[2] & 0x04) != 0;
	
	if (*newData) {
		if (rawLux == 0 || rawLux == 65535) {
			ulong adcCh1, adcCh2;
			if (!readRawAdcValues(&adcCh1, &adcCh2))
				return false;

			// saturation or overflow
			if ((adcCh1 & 0x000f0000) == 0x000f0000)
				rawLux = -1;
		}
	}
	*lux = rawLux;
	return true;
}

// Automatically sets the optimum alsGain according to the lux value
// this affects the next call to readLux()
// lex = the last lux value from readLux(), even if -1
// if lux == -1 (saturation or overflow) then alsGain is set to 0 (x1)
// of lux == 0, it means absolute darkness or ALS_IRF_CUT_OFF (cannot tell)
// returns true if alsGain was changed, false if not
bool LTR507ALSOpticalSensor::autoSelectGain(long lux)
{
	uint newGain = 0;
	if (lux >= 32767 || lux == -1 || lux == 0)
		newGain = 0;
	else if (lux >= 655)
		newGain = 1;
	else if (lux >= 327)
		newGain = 2;
	else
		newGain = 3;
	if (newGain == alsGain)
		return false;
	alsGain = newGain;

	//Serial.printf("\n\ralsGain changed to %u\n\r", newGain);

	// note: ALS Mode is assumed to be Active!
	return writeRegister(ALS_CONTR, 0b00000010 | (alsGain << 3));
}


// Returns proximity detection distance value
// this requires external IR LED connected between VLED (K) and VCC (A)
// 'proximity' is the ADC value (not cm or mm), it increases as the distance decreases
// the range ~5..20cm depending on reflectivity
// 'overflow' is set when the distance is too close, value = 2047
bool LTR507ALSOpticalSensor::readProximity(uint* proximity, bool* overflow)
{
	byte data[2];
	if (!readRegisters(PS_DATA_0, data, 2))
		return false;
	*overflow = (bool)(data[1] & 0b00010000);
	*proximity = ((data[1] & 7) << 8) + data[0];	// 11-bit
	return true;
}

// Return interrupt and data status from ALS_PS_STATUS register
// interruptSource : 0 = no interrupt; 1 = PS interrupt; 2 = ALS interrupt
bool LTR507ALSOpticalSensor::readStatus(uint* interruptSource, bool* alsInterruptStatus,
	bool* alsDataStatus, bool* psInterruptStatus, bool* psDataStatus)
{
	byte b;
	if (!readRegister(ALS_PS_STATUS, &b))
		return false;
	*interruptSource = (b >> 4) & 0x03;
	*alsInterruptStatus = (bool)(b & 0b00001000);
	*alsDataStatus = (bool)(b & 0b00000100);
	*psInterruptStatus = (bool)(b & 0b00000010);
	*psDataStatus = (bool)(b & 0b00000001);
	return true;
}

// Returns the raw ADC values for the ALS sensor 'clear diode' (CH1) 
// and the PS sensor 'IR diode' (CH2)
// values are 4..20 significant bits according to 'resolution'
// >>> the MS bit is always bit 19 <<<
bool LTR507ALSOpticalSensor::readRawAdcValues(ulong* adcCh1, ulong* adcCh2)
{
	byte data[6];
	if (!readRegisters(ALS_DATA_CH1_0, data, 6))
		return false;
	*adcCh1 = (data[2] << 12) + (data[1] << 4) + (data[0] >> 4);
	*adcCh2 = (data[5] << 12) + (data[4] << 4) + (data[3] >> 4);
	return true;
}

// Configure the Ambient Light Sensor (ALS)
// gain :       0 = 1x, 1..65535 lux (default) (1 lux/count)
//              1 = 2x, 0.5..32767 lux (0.5 lux/count)
//              2 = 100x, 0.02..655 lux (0.01 lux/count)
//              3 = 200x, 0.01..327 lux (0.005 lux/count)
// resolution : 0 = 20 bits; 1 = 19; 2 = 18; 3 = 17; 4 = 16 (default); 5 = 12; 
//              6 = 8; 7 = 4 bits
//              NOTE! 8 and 4-bit resolutions don't really make any sense???
// rate :       0 = 100ms; 1 = 200ms; 3 = 500ms (default); 3 = 1000ms; 4..7 = 2000ms
bool LTR507ALSOpticalSensor::configureALS(uint gain, uint resolution, uint rate)
{
	ASSERT(gain < 4 && resolution < 8 && rate < 8);

	// alsGain is used to get the actual LUX reading, see readLightIntensity()
	alsGain = gain;
	alsResolution = resolution;

	byte b;
	if (!readRegister(ALS_CONTR, &b))
		return false;
	// keep ALS mode bit 1
	b = (b & 0b00000010) | (gain << 3);
	if (!writeRegister(ALS_CONTR, b))
		return false;
	b = (resolution << 5) | rate;
	return writeRegister(ALS_MEAS_RATE, b);
}

// Configure the Proximity Sensor (PS) and the external IR LED
// pulseFreq :   0 = 30kHz; 1 = 40kHz; 2 = 50kHz; 3 = 60kHz (default); 4 = 70kHz; 
//               5 = 80kHz; 6 = 90kHz; 7 = 100kHz
// peakCurrent : 0 = 5mA; 1 = 10mA; 2 = 20mA; 3 = 50mA (default); 4..7 = 100mA 
// pulseCount :  number of pulses, 0..255, default = 127
// rate :        0 = 12.5ms; 1 = 50ms; 2 = 70ms; 3 = 100ms (default); 4 = 200ms; 
//               5 = 500ms; 6 = 1000ms; 7 = 2000ms
bool LTR507ALSOpticalSensor::configurePS(uint pulseFreq, uint peakCurrent, uint pulseCount, uint rate)
{
	ASSERT(pulseFreq < 8 && peakCurrent < 8 && pulseCount < 256 && rate < 8);

	byte b = (pulseFreq << 5) | 0b00001000 | peakCurrent;
	if (!writeRegister(PS_LED, b))
		return false;
	if (!writeRegister(PS_N_PULSES, (byte)pulseCount))
		return false;
	return writeRegister(PS_MEAS_RATE, (byte)rate);
}

// Interrupt Configuration
// NOTE: Configure interrupts BEFORE setting Active mode
// Connect INT output to an input which generates an interrupt, then use attachInterrupt()

// outputMode :    0 = latched until ALS_PS_STATUS is read; 0 = updated after every measurement (default)
// polarity :      0 = active low (default); 1 = active high
// interruptMode : 0 = inactive; 1 = PS interrupt only; 2 = ALS interrupt only; 3 = PS and ALS interrupts
bool LTR507ALSOpticalSensor::configureInterrupt(bool outputMode, bool polarity, uint interruptMode)
{
	ASSERT((unsigned)interruptMode < 4);

	byte b = interruptMode & 3;
	if (outputMode)
		b |= 0b00001000;
	if (polarity)
		b |= 0b00000100;
	return writeRegister(INTERRUPT, b);
}

bool LTR507ALSOpticalSensor::configureALSInterruptThreshold(uint upper, uint lower)
{
	ASSERT(upper <= 0xffff && lower <= 0xffff);	// 16-bit

	return writeRegisters(ALS_THRES_UP_0, (byte*)&upper, 2) &&
		writeRegisters(ALS_THRES_LOW_0, (byte*)&lower, 2);
}

bool LTR507ALSOpticalSensor::readALSInterruptThreshold(uint* upper, uint* lower)
{
	byte data[4];
	if (!readRegisters(ALS_THRES_UP_0, data, 4))
		return false;
	*upper = (data[1] << 8) + data[0];
	*lower = (data[3] << 8) + data[2];
	return true;
}

bool LTR507ALSOpticalSensor::configurePSInterruptThreshold(uint upper, uint lower)
{
	ASSERT(upper < 0x800 && lower < 0x800);	// 11-bit

	if (!writeRegisters(PS_THRES_UP_0, (byte*)&upper, 2))
		return false;
	return writeRegisters(PS_THRES_LOW_0, (byte*)&lower, 2);
}

bool LTR507ALSOpticalSensor::readPSInterruptThreshold(uint* upper, uint* lower)
{
	byte data[4];
	if (!readRegisters(PS_THRES_UP_0, data, 4))
		return false;
	*upper = ((data[1] & 7) << 8) + data[0];
	*lower = ((data[3] & 7) << 8) + data[2];
	return true;
}

bool LTR507ALSOpticalSensor::configureInterruptPersist(uint alsPersist, uint psPersist)
{
	ASSERT(alsPersist < 16 && psPersist < 16);	// 4-bit

	byte b = (psPersist << 4) + alsPersist;
	return writeRegister(INTERRUPT_PERSIST, b);
}

bool LTR507ALSOpticalSensor::readInterruptPersist(uint* alsPersist, uint* psPersist)
{
	byte b;
	if (!readRegister(INTERRUPT_PERSIST, &b))
		return false;
	*alsPersist = b & 0x0f;
	*psPersist = b >> 4;
	return true;
}

// Coefficients for calculating luminance in LUX
// >>> it's probably best not to change these <<<
// defaults : coeffCh1 = 0x0380, coeffCh2 = 0xfbc8
bool LTR507ALSOpticalSensor::configureALSCoeff(uint coeffCh1, uint coeffCh2)
{
	ASSERT(coeffCh1 <= 0xffff && coeffCh2 <= 0xffff);	// 16-bit

	return writeRegisters(ALS_COEFF1_DATA_0, (byte*)(&coeffCh1), 2) &&
		writeRegisters(ALS_COEFF2_DATA_0, (byte*)(&coeffCh2), 2);
}

bool LTR507ALSOpticalSensor::readALSCoeff(uint* coeffCh1, uint* coeffCh2)
{
	byte data[4];
	if (!readRegisters(ALS_COEFF1_DATA_0, data, 4))
		return false;
	*coeffCh1 = (data[1] << 8) + data[0];
	*coeffCh2 = (data[3] << 8) + data[2];
	return true;
}


// ALS cutoff limit of IR factor, default = 0xD0 (208)
// too much infrared light?
// when the IR factor exceeds the cut-off limit, the output value is set to '0'.
// if ((ADCIR * 256) / ADCCLEAR) > ALS_IRF_CUT_OFF, then ALS_DATA := 0
bool LTR507ALSOpticalSensor::configureALSIRFCutoff(uint cutoff)
{
	ASSERT(cutoff <= 0xff);
	return writeRegister(ALS_IRF_CUT_OFF, (byte)cutoff);
}

// if (ADCIR / ADCCLEAR) > ALS_IRF_CUT_OFF, then ALS_DATA := 0
bool LTR507ALSOpticalSensor::readALSIRFCutoff(uint* cutoff)
{
	byte b;
	if (!readRegister(ALS_IRF_CUT_OFF, &b))
		return false;
	*cutoff = b;
	return true;
}


// Read/write multiple 8-bit registers

bool LTR507ALSOpticalSensor::readRegisters(REG reg, byte* values, uint length)
{
	wire->beginTransmission(i2cAdds);
	if (wire->write((byte)reg) != 1) {
		LOGERROR("write failed");
		return false;
	}
	if (wire->endTransmission() != 0) {
		LOGERROR("endtx failed");
		return false;
	}
	if (wire->requestFrom(i2cAdds, length) != length) {
		//avoid the bug in Arduino's Wire.cpp
		//LOGERROR("requestFrom failed");
		//return false;
	}
	if (wire->readBytes(values, length) != length) {
		LOGERROR("readBytes failed");
		return false;
	}
	return true;
}

bool LTR507ALSOpticalSensor::writeRegisters(REG reg, const byte* values, uint length)
{
	wire->beginTransmission(i2cAdds);
	if (wire->write((byte)reg) != 1) {
		LOGERROR("write failed");
		return false;
	}
	if (wire->write(values, length) != length) {
		LOGERROR("write failed");
		return false;
	}
	if (wire->endTransmission() != 0) {
		LOGERROR("endtx failed");
		return false;
	}
	return true;
}

// Read/write a single 8-bit register

bool LTR507ALSOpticalSensor::readRegister(REG reg, byte* value)
{
	return readRegisters(reg, value, 1);
}

bool LTR507ALSOpticalSensor::writeRegister(REG reg, byte value)
{
	return writeRegisters(reg, &value, 1);
}
