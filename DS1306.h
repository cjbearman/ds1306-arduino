/*
 * File			DS1306.h
 *
 * Synopsis		Support for Dallas Semiconductor DS1306 Real Time Clock, using SPI
 *
 * Author		Chris Bearman
 *
 * Version		1.0
 *
 * License		This software is released under the terms of the Mozilla Public License (MPL) version 2.0
 * 			Full details of licensing terms can be found in the "LICENSE" file, distributed with this code
 *
 * Instructions
 * 			Create an instance of the DS1306 class using either constructor.
 * 			By default all writes will use 24 hour form, if you wish to write clock using 12 hour form
 * 			then use parameterized constructor and select writeHours24 = false
 *
 *			Current date/time is passed as ds1306time type
 * 			Alarm specifications are passed as ds1306alarm type
 * 			For both types, hours is hours in 24 hour form, hours12 is 12 hour form and ampm is AM/PM
 * 			indicator where AM is represented by (char) 'A' and PM is represented by (char) 'P'
 * 			If ampm indicator is neither 'A' nor 'B' then 12 hour time is considered undefined
 * 			Both 12 hour and 24 hour times are always provided by calls that read the time
 * 			You MUST provide 24 hour time to write time/alarm unless you select writeHours24 = false in which
 * 			case you MUST provide 12 hour time to write time/alarm.
 *
 *			Supports DS1306 RTC chip connected to SPI bus (MOSI/MISO/SCLK) with chip select on any line
 *			as designated by the init call.
 *
 *			On init call, the SPI bus is initialized, the clock oscillator is started and the clock
 *			write protect is deactivated.
 *
 *			For optimization, values are NOT checked for correctness when being written to the DS1306.
 *			Per specification, writing illogical values will result in undefined behavior.
 *
 *			Full details on the operation and use of each method can be found in DS1306.cpp
 */
#ifndef __DS1306_RTC_
#define __DS1306_RTC_

/* Memory Locations */
#define DS1306_DATETIME			0x00
#define DS1306_ALARM0			0x07
#define DS1306_ALARM1			0x0B
#define DS1306_CR 				0x0F
#define DS1306_SR				0x10
#define DS1306_TCR				0x11
#define DS1306_USER_START		0x20
#define DS1306_USER_END			0x7F

/* Buffer sizes required for reading / writing time/date and alarms to the RTC */
#define DS1306_SIZE_DATETIME	7
#define DS1306_SIZE_ALARM		4

/* Bit Position of key register parameters (CR) */
#define DS1306_CR_WP			6
#define DS1306_CR_1HZ			2
#define DS1306_CR_AIE1			1
#define DS1306_CR_AIE0			0

/* Bit Position of key register parameters (SR) */
#define DS1306_SR_IRQF1			1
#define DS1306_SR_IRQF0			0

/* Bit Positions of key register parameters (TCR) */
#define DS1306_TCR_TCS			7
#define DS1306_TCR_DS			3
#define DS1306_TCR_RS			1

/* Days of week (suggested, see spec) */
#define DS1306_SUNDAY			1
#define DS1306_MONDAY			2
#define DS1306_TUESDAY			3
#define DS1306_WEDNESDAY		4
#define DS1306_THURSDAY			5
#define DS1306_FRIDAY			6
#define DS1306_SATURDAY			7

/* "Any" designator, for alarms */
#define DS1306_ANY				0x80

/* Write offset used when writing DS1306 registers */
#define DS1306_WRITE_OFFSET		0x80

/* Representation of the current time/date */
typedef struct {
	unsigned char seconds;
	unsigned char minutes;
	unsigned char hours;
	unsigned char hours12;
	char ampm;					// 'A' = AM, 'P' = PM, anything else is hours12 undefined
	unsigned char dow;
	unsigned char day;
	unsigned char month;
	unsigned char year;
} ds1306time;

/* Representation of an alarm */
typedef struct {
	unsigned char seconds;
	unsigned char minutes;
	unsigned char hours;
	unsigned char hours12;
	char ampm;					// 'A' = AM, 'P' = PM, anything else is hours12 undefined
	unsigned char dow;
} ds1306alarm;

class DS1306
{
	public:

	// Constructors
	DS1306();
	DS1306(bool writeHours24);

	// Initialize DS1306 using ce as chip enable line, turn on osc
	void init(unsigned char ce);

	// Primary clock (time/date) operations
	void setTime(const ds1306time *time);
	void getTime(ds1306time *time);

	// Alarm management operations
	void setAlarm(int alarm, const ds1306alarm *time);
	void getAlarm(int alarm, ds1306alarm *time);
	bool getAlarmState(unsigned int alarm);
	void getAlarmBothState(bool *state1, bool *state2);
	void clearAlarmState(unsigned int alarm);
	void clearAlarmBothState();
	bool getAlarmEnabled(unsigned int alarm);
	void getAlarmBothEnabled(bool *enabled1, bool *enabled2);
	void enableAlarm(unsigned int alarm);
	void disableAlarm(unsigned int alarm);
	void enableBothAlarms();
	void disableBothAlarms();

	// 1Hz state
	bool get1HzState();
	void set1HzState(bool enabled);

	// Trickle charge management
	bool enableTrickleCharge(unsigned char numDiodes, unsigned char kRes);
	void disableTrickleCharge();
	bool getTrickleChargeState(unsigned char *numDiodes, unsigned char *kRes);

	// User memory management
	bool writeUser(unsigned char addr, const char *buf, int num);
	bool readUser(unsigned char addr, char *buf, int num);

	// Write Protection management
	bool isWriteProtected();
	void setWriteProtection(bool on);

	// Direct Register access (use for direct access to registers, if needed)
	void read(unsigned char address, unsigned char *data, int len);
	unsigned char read(unsigned char address);
	void write(unsigned char address, const unsigned char *data, int len);
	void write(unsigned char address, const unsigned char value);

	private:

	// Class Properties
	unsigned char ce;			// Chip enable line
	bool writeHours24;			// True (default) means time/alarm writes use 24 hour form

	// Encode a time / alarm packet
	void encodeTimePacket(unsigned char *buf, const ds1306time *time);
	void encodeAlarmPacket(unsigned char *buf, const ds1306alarm *alarm);

	// Decode a time / alarm packet
	void decodeTimePacket(const unsigned char *buf, ds1306time *time);
	void decodeAlarmPacket(const unsigned char *buf, ds1306alarm *alarm);

	// Hour parameter management
	void decodeHourByte(unsigned char hourByte, unsigned char *hour24, unsigned char *hour12, char *ampm);
	unsigned char encodeHourByte(unsigned char hour24, unsigned char hour12, char ampm);

	// Parameter encode / decode
	unsigned char encodeBCD7(unsigned char value);
	unsigned char encodeBCD7(unsigned char value, unsigned char mask);
	unsigned char encodeBCD8(unsigned char value);
	unsigned char decodeBCD7(unsigned char value);
	unsigned char decodeBCD7(unsigned char value, unsigned char mask);
	unsigned char decodeBCD8(unsigned char value);

	// Wait for SPI operation to finish
	void waitSPI();
};

#endif /* __DS1306_RTC_ */
