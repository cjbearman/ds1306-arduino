/*
 * File                 ds1306test.ino
 *
 * Synopsis             Test Sketch for DS1306 library
 *
 * Author               Chris Bearman
 *
 * Version              1.0
 *
 * License              This software is released under the terms of the Mozilla Public License (MPL) version 2.0
 *                      Full details of licensing terms can be found in the "LICENSE" file, distributed with this code
 *
 * Instructions
 *                      This example sketch tests the function of the DS1035 library and is intended for
 *                      code validation purposes.
 *
 *                      Download sketch, open serial monitor and set 9600 baud rate.
 *                      Tests will run after reset and all should pass, results are written to the monitor.
 */
#include <DS1306.h>

/* You should ONLY define __CHARGING_SUPPORTED is you have a chargable battery or super-cap
   properly connected, per DS1306 spec sheet */
   
/* #define __CHARGING_SUPPORTED */

// This test will test both the 24 and 12 hour write/read modes for the clock
// using these two objects
DS1306 clk24, clk12(false);

// Allow us to embed some PROGMEM strings inline so we don't run out of memory
class __FlashStringHelper;
#define F(str) reinterpret_cast<__FlashStringHelper *>(PSTR(str))

// Convert a 12 hour + ampm to a 24 hour  
unsigned char convert_to_24(unsigned char hours12, char ampm)
{
  if (ampm == 'A') {
    if (hours12 == 12) return 0;
    return hours12;
  } else if (ampm == 'P') {
    if (hours12 == 12) return 12;
    return hours12+12;
  } else {
    return 255;  // Error signal because of missing ampm
  }
}

// Convert a 24 hour to a 12 hour format
void convert_to_12(unsigned char hours24, unsigned char *hours12, char *ampm)
{
  if (hours24 == 0) {
    *hours12 = 12;
    *ampm = 'A';
  } else if (hours24 == 12) {
    *hours12 = 12;
    *ampm = 'P';
  } else if (hours24 < 12) {
    *hours12 = hours24;
    *ampm = 'A';
  } else {
    *hours12 = hours24 - 12;
    *ampm = 'P';
  }
} 

// Compare two bytes, return true if the same, false otherwise.
// Output error to serial on false
bool compare(unsigned char in, unsigned char out, const char *desc, bool *passflag, int aschar = false)
{
  if (in != out) {
    if (*passflag) {
      *passflag = false;
      Serial.println(F("Fail"));
    }
    Serial.print(F("..Failure comparing "));
    Serial.print(desc);
    Serial.print(F(" - in="));
    if (aschar) {
      Serial.print((char) in);
    } else {
      Serial.print(in);
    }
    Serial.print(F(", out="));
    if (aschar) {
      Serial.println((char) out);
    } else {
      Serial.println(out);
    }
  }
}

// Routine to write and read the clock using 24 hour format
// and check the two values against each other
bool read_write_compare_clock24(__FlashStringHelper *tc, unsigned char year, unsigned char month, unsigned char day, unsigned char hours, unsigned char mins, unsigned char secs, unsigned char dow)
{
  Serial.print(tc);
  Serial.print(F(" Read/Write/Compare Main Clock 24hr - "));

  ds1306time ts_in, ts_out;
  ts_in.year = year;
  ts_in.month = month;
  ts_in.day = day;
  ts_in.dow=dow;
  ts_in.hours=hours;
  ts_in.minutes=mins;
  ts_in.seconds=secs;
  
  clk24.setTime(&ts_in);
  clk24.getTime(&ts_out);

  bool pass = true;
  
  unsigned char hours12;
  char ampm;
  
  convert_to_12(ts_in.hours, &hours12, &ampm);

  compare(ts_in.year, ts_out.year, "Year", &pass);
  compare(ts_in.month, ts_out.month, "Month", &pass);
  compare(ts_in.day, ts_out.day, "Day", &pass);
  compare(ts_in.hours, ts_out.hours, "Hours", &pass);
  compare(hours12, ts_out.hours12, "Hours (12)", &pass);
  compare(ampm, ts_out.ampm, "AMPM Flag", &pass, true);
  compare(ts_in.minutes, ts_out.minutes, "Minutes", &pass);
  compare(ts_in.seconds, ts_out.seconds, "Seconds", &pass);
  compare(ts_in.dow, ts_out.dow, "DOW", &pass);
  
  if (pass) Serial.println(F("Pass"));
  
  return pass; 
}

// Routine to write and read the clock using 12 hour format
// and check the two values against each other
bool read_write_compare_clock12(__FlashStringHelper *tc, unsigned char year, unsigned char month, unsigned char day, unsigned char hours, char ampm, unsigned char mins, unsigned char secs, unsigned char dow)
{
  Serial.print(tc);
  Serial.print(F(" Read/Write/Compare Main Clock 12hr - "));

  ds1306time ts_in, ts_out;
  ts_in.year = year;
  ts_in.month = month;
  ts_in.day = day;
  ts_in.dow=dow;
  ts_in.hours12=hours;
  ts_in.ampm=ampm;
  ts_in.minutes=mins;
  ts_in.seconds=secs;
  
  clk12.setTime(&ts_in);
  clk12.getTime(&ts_out);

  bool pass = true;
  
  unsigned char hours24;
  
  hours24 = convert_to_24(ts_in.hours12, ampm);

  compare(ts_in.year, ts_out.year, "Year", &pass);
  compare(ts_in.month, ts_out.month, "Month", &pass);
  compare(ts_in.day, ts_out.day, "Day", &pass);
  compare(ts_in.hours12, ts_out.hours12, "Hours", &pass);
  compare(ts_in.ampm, ts_out.ampm, "AMPM Flag", &pass, true);
  compare(hours24, ts_out.hours, "Hours (24)", &pass);
  compare(ts_in.minutes, ts_out.minutes, "Minutes", &pass);
  compare(ts_in.seconds, ts_out.seconds, "Seconds", &pass);
  compare(ts_in.dow, ts_out.dow, "DOW", &pass);
  
  if (pass) Serial.println(F("Pass"));
  
  return pass; 
}

// Routine to write and read and alarm using 24 hour format
// and check the two values against each other
bool read_write_compare_alarm24(__FlashStringHelper *tc, int alarm, unsigned char hours, unsigned char mins, unsigned char secs, unsigned char dow)
{
  Serial.print(tc);
  Serial.print(F(" Read/Write/Compare Alarm 24hr - "));

  ds1306alarm ts_in, ts_out;
  ts_in.dow=dow;
  ts_in.hours=hours;
  ts_in.minutes=mins;
  ts_in.seconds=secs;
  
  clk24.setAlarm(alarm, &ts_in);
  clk24.getAlarm(alarm, &ts_out);

  bool pass = true;
  
  unsigned char hours12;
  char ampm;
  
  convert_to_12(ts_in.hours, &hours12, &ampm);

  compare(ts_in.hours, ts_out.hours, "Hours", &pass);
  if (ts_in.hours != DS1306_ANY) compare(hours12, ts_out.hours12, "Hours (12)", &pass);
  if (ts_in.hours != DS1306_ANY) compare(ampm, ts_out.ampm, "AMPM Flag", &pass, true);
  compare(ts_in.minutes, ts_out.minutes, "Minutes", &pass);
  compare(ts_in.seconds, ts_out.seconds, "Seconds", &pass);
  compare(ts_in.dow, ts_out.dow, "DOW", &pass);
  
  if (pass) Serial.println(F("Pass"));
  
  return pass; 
}

// Routine to write and read and alarm using 12 hour format
// and check the two values against each other
bool read_write_compare_alarm12(__FlashStringHelper *tc, int alarm, unsigned char hours, char ampm, unsigned char mins, unsigned char secs, unsigned char dow)
{
  Serial.print(tc);
  Serial.print(F(" Read/Write/Compare Alarm 12hr - "));

  ds1306alarm ts_in, ts_out;
  ts_in.dow=dow;
  ts_in.hours12=hours;
  ts_in.ampm=ampm;
  ts_in.minutes=mins;
  ts_in.seconds=secs;
  
  clk12.setAlarm(alarm, &ts_in);
  clk12.getAlarm(alarm, &ts_out);

  bool pass = true;
  
  unsigned char hours24;
  
  hours24 = convert_to_24(ts_in.hours12, ampm);
  compare(ts_in.hours12, ts_out.hours12, "Hours", &pass);
  if (ts_in.hours12 != DS1306_ANY) compare(ts_in.ampm, ts_out.ampm, "AM PM Flag", &pass);
  if (ts_in.hours12 != DS1306_ANY) compare(hours24, ts_out.hours, "Hours (24)", &pass);
  compare(ts_in.minutes, ts_out.minutes, "Minutes", &pass);
  compare(ts_in.seconds, ts_out.seconds, "Seconds", &pass);
  compare(ts_in.dow, ts_out.dow, "DOW", &pass);
  
  if (pass) Serial.println(F("Pass"));
  
  return pass; 
}

// Validate that the two alarms do not match each other
bool check_alarm_diff(__FlashStringHelper *tc)
{
  ds1306alarm a0, a1;

  Serial.print(tc);  
  Serial.print(F(" Compare alarms are different - "));
  clk24.getAlarm(0, &a0);
  clk24.getAlarm(1, &a1);
  
  if (a0.hours == a1.hours &&
      a0.minutes == a1.minutes &&
      a0.seconds == a1.seconds &&
      a0.dow == a1.dow) {
    Serial.println(F("Fail"));
    return false;
  } else {    
    Serial.println(F("Pass"));
    return true;
  }
}  

// Run tests related to the primary clock
int clocktests()
{
  int failures = 0;
  // RWC24 series tests
  // Write clock (24 hour mode), Read clock, compare written timestamp with read timestamp
  
  // RWC24.01 - Lower bound (lowest supportable values)
  if (!read_write_compare_clock24(F("RWC24.01"), 0, 1, 1, 0, 0, 0, DS1306_SUNDAY)) failures++;
  
  // RWC24.02 - Upper bound (maximum supportable values)
  if (!read_write_compare_clock24(F("RWC24.02"), 99, 12, 31, 23, 59, 59, DS1306_SATURDAY)) failures++;
  
  // RWC24.03 - Check operation of 12th hour for PM decode, unique values for ALL fields
  if (!read_write_compare_clock24(F("RWC24.03"), 1, 2, 3, 12, 4, 5, DS1306_SATURDAY)) failures++;
  
  // RWC12 series tests
  // Write clock (12 hour mode), Read clock, compare written timestamp with read timestamp
  
  // RWC12.01 - Lower bound (lowest supportable values)
  if (!read_write_compare_clock12(F("RWC12.01"), 0, 1, 1, 0, 'A', 0, 0, DS1306_SUNDAY)) failures++;
  
  // RWC12.02 - Upper bound (maximum supportable values)
  if (!read_write_compare_clock12(F("RWC12.02"), 99, 12, 31, 11, 'P', 59, 59, DS1306_SATURDAY)) failures++;
  
  // RWC12.03 - Check operation of 12th hour for PM decode, unique values for ALL fields
  if (!read_write_compare_clock12(F("RWC12.03"), 1, 2, 3, 9, 'P', 4, 5, DS1306_SATURDAY)) failures++;

  return failures;
}

// Run tests related to primary alarm setting / retrieval
int alarmtests1()
{  
  int failures = 0;
  // ALM24 series tests
  // Write alarm (1 or 2), Read back, compare written and read values
  
  // ALM24.01 - Lower bound (lowest supportable values)
  if (!read_write_compare_alarm24(F("ALM24.01"), 0, 0, 0, 0, DS1306_SUNDAY)) failures++;
  
  // ALM24.02 - Upper bound (highest supportable values)
  if (!read_write_compare_alarm24(F("ALM24.02"), 0, 23, 59, 59, DS1306_SATURDAY)) failures++;
  
  // ALM24.03 - Check operation of 12th hour for PM decode, unique values for ALL fields
  if (!read_write_compare_alarm24(F("ALM24.03"), 0, 12, 2, 3, DS1306_FRIDAY)) failures++;

  // Repeat previous 3 tests for second alarm (alarm 1)
  if (!read_write_compare_alarm24(F("ALM24.11"), 1, 0, 0, 0, DS1306_SUNDAY)) failures++;
  if (!read_write_compare_alarm24(F("ALM24.12"), 1, 23, 59, 59, DS1306_SATURDAY)) failures++;
  if (!read_write_compare_alarm24(F("ALM24.13"), 1, 12, 2, 3, DS1306_FRIDAY)) failures++;

  // Do some testing of the ANY indicator in all fields, alternating alarm 1 and alarm 2
  // to ensure that both alarms end up on different values for a further test
  if (!read_write_compare_alarm24(F("ALM24.21"), 0, DS1306_ANY, 1, 2, DS1306_SUNDAY)) failures++;
  if (!read_write_compare_alarm24(F("ALM24.22"), 1, 1, DS1306_ANY, 2, DS1306_SUNDAY)) failures++;
  if (!read_write_compare_alarm24(F("ALM24.24"), 0, 1, 2, DS1306_ANY, DS1306_SUNDAY)) failures++;
  if (!read_write_compare_alarm24(F("ALM24.23"), 1, 1, 2, 3, DS1306_ANY)) failures++;

  // ALM12 series tests
  // Write alarm (1 or 2), Read back, compare written and read values
  
  // ALM12.01 - Lower bound (lowest supportable values)
  if (!read_write_compare_alarm12(F("ALM12.01"), 0, 12, 'A', 0, 0, DS1306_SUNDAY)) failures++;
  
  // ALM12.02 - Upper bound (highest supportable values)
  if (!read_write_compare_alarm12(F("ALM12.02"), 0, 11, 'P', 59, 59, DS1306_SATURDAY)) failures++;
  
  // ALM12.03 - Check operation of 12th hour for PM decode, unique values for ALL fields
  if (!read_write_compare_alarm12(F("ALM12.03"), 0, 12, 'P', 2, 3, DS1306_FRIDAY)) failures++;

  // Repeat previous 3 tests for second alarm (alarm 1)
  if (!read_write_compare_alarm12(F("ALM12.11"), 1, 0, 'A', 0, 0, DS1306_SUNDAY)) failures++;
  if (!read_write_compare_alarm12(F("ALM12.12"), 1, 11, 'P', 59, 59, DS1306_SATURDAY)) failures++;
  if (!read_write_compare_alarm12(F("ALM12.13"), 1, 12, 'A', 2, 3, DS1306_FRIDAY)) failures++;

  // Do some testing of the ANY indicator in all fields, alternating alarm 1 and alarm 2
  // to ensure that both alarms end up on different values for a further test
  if (!read_write_compare_alarm12(F("ALM12.21"), 0, DS1306_ANY, '\0', 1, 2, DS1306_SUNDAY)) failures++;
  if (!read_write_compare_alarm12(F("ALM12.22"), 1, 1, 'P', DS1306_ANY, 2, DS1306_SUNDAY)) failures++;
  if (!read_write_compare_alarm12(F("ALM12.24"), 0, 1, 'A', 2, DS1306_ANY, DS1306_SUNDAY)) failures++;
  if (!read_write_compare_alarm12(F("ALM12.23"), 1, 1, 'P', 2, 3, DS1306_ANY)) failures++;


  // Make sure that the alarms are set differently
  if (!check_alarm_diff(F("ALMXX.01"))) failures++;

  return failures;
}

// Run tests related to alarm enable / disable / state and triggering
int runalarmtests2(int alarmnumber, __FlashStringHelper *prefix)
{
  int failures = 0;
  
  // Start off by setting alarm as on
  Serial.print(prefix);
  Serial.print(F(".01 Set alarm to disabled - " ));
  clk24.enableAlarm(alarmnumber);
  if (clk24.getAlarmEnabled(alarmnumber)) {
    Serial.println(F("Pass"));
  } else {
    Serial.println(F("Fail"));
    failures++;
  }
  
  // Now set alarm as off
  Serial.print(prefix);
  Serial.print(F(".02 Set alarm to enabled - "));
  clk24.disableAlarm(alarmnumber);
  if (!clk24.getAlarmEnabled(alarmnumber)) {
    Serial.println(F("Pass"));
  } else {
    Serial.println(F("Fail"));
    failures++;
  }
  
  Serial.print(prefix);
  Serial.print(F(".03 Trigger alarm - "));
  ds1306alarm a;
  a.dow = DS1306_ANY;
  a.hours = DS1306_ANY;
  a.minutes = DS1306_ANY;
  a.seconds = DS1306_ANY;
  
  clk24.clearAlarmState(alarmnumber);
  if (clk24.getAlarmState(alarmnumber)) {
    Serial.println(F("Fail"));
    Serial.println(F("..Alarm did not clear"));
    failures++;
  } else {
    clk24.setAlarm(alarmnumber, &a);
    delay(2000);
    if (!clk24.getAlarmState(alarmnumber)) {
      Serial.println(F("Fail"));
      Serial.println(F("..Alarm did not trigger"));
      failures++;
    } else {
      Serial.println("Pass");
    }
  }
  clk24.disableAlarm(alarmnumber);
  
  return failures;
}
  
  
// Run functional alarm tests
int alarmtests2()
{
  int failures = 0;

  failures += runalarmtests2(0, F("AL0FN"));  
  //Cannot run alarm test 2 for 1306 like we can for the 1305 because
  //it only works when the clock is running on backup
  //failures += runalarmtests2(1, F("AL1FN"));
  return failures;
}

// Run user memory tests
int usermemtests()
{
  int failures = 0;
  char outbuf[128];
  char inbuf[128];
  int i;
  bool fail;
  
  Serial.print(F("USMEM.01 Read/Write Sequential Pattern - "));
  
  memset(inbuf, 0, 128);
  memset(outbuf, 0, 128);
  
  // Write an obvious test pattern into buf
  for (i = 0 ; i < 96 ; i ++) {
    outbuf[i] = i;
  }
  
  clk24.writeUser(DS1306_USER_START, outbuf, 96);
  clk24.readUser(DS1306_USER_START, inbuf, 96);
  
  fail = false;
  for (i = 0 ; i < 96 ; i++) {
    if (inbuf[i] != outbuf[i]) {
      fail = true;
      break;
    }
  }
  
  if (fail) {
    Serial.println(F("Fail"));
    Serial.print(F("..Offset = 0x"));
    Serial.println(i, HEX);
    failures++;
  } else {
    Serial.println(F("Pass"));
  }
  
    Serial.print(F("USMEM.02 Read/Write Random Pattern - "));
  
  memset(inbuf, 0, 128);
  memset(outbuf, 0, 128);
  
  // Write a (pseudo) random value test pattern into buf
  for (i = 0 ; i < 96 ; i ++) {
    outbuf[i] = random() % 0x100;
  }
  
  clk24.writeUser(DS1306_USER_START, outbuf, 96);
  clk24.readUser(DS1306_USER_START, inbuf, 96);
  
  fail = false;
  for (i = 0 ; i < 96 ; i++) {
    if (inbuf[i] != outbuf[i]) {
      fail = true;
      break;
    }
  }
  
  if (fail) {
    Serial.println(F("Fail"));
    Serial.print(F("..Offset = 0x"));
    Serial.println(i, HEX);
    failures++;
  } else {
    Serial.println(F("Pass"));
  }
  
  return failures;
}

// Run control register tests
int crtests()
{
  int failures = 0;
  
  Serial.print(F("CRRWP.01 Write Protect Clock - "));
  clk24.setWriteProtection(true);
  if (clk24.isWriteProtected()) {
    Serial.println(F("Pass"));
  } else {
    failures++;
    Serial.println(F("Fail"));
  }
  
  Serial.print(F("CRRWP.02 Write Unprotect Clock - "));
  clk24.setWriteProtection(false);
  if (!clk24.isWriteProtected()) {
    Serial.println(F("Pass"));
  } else {
    failures++;
    Serial.println(F("Fail"));
  }
  
#ifdef __CHARGING_SUPPORTED
  Serial.print(F("TCREN.01 Enable Trickle Charge - "));
  clk24.enableTrickleCharge(2, 8);
  unsigned char diodes;
  unsigned char resistance;
  bool en = clk24.getTrickleChargeState(&diodes, &resistance);
  if (diodes != 2 || resistance != 8 || en == false) {
    Serial.println(F("Fail"));
    if (diodes != 2) {
      Serial.print(F("..Expected 2 diodes, got "));
      Serial.println(diodes, DEC);
    }
    if (resistance != 8) {
      Serial.print(F("..Expected resistance of 8, got "));
      Serial.println(resistance, DEC);
    }
    if (en != true) {
      Serial.println(F("..Got disabled indication"));
    }
    failures++;
    
  } else {
    Serial.println("Pass");
  }
  
  Serial.print(F("TCREN.02 Disable Trickle Charge - "));
  clk24.disableTrickleCharge();
  en = clk24.getTrickleChargeState(&diodes, &resistance);
  if (diodes != 0 || resistance != 0 || en == true) {
    Serial.println(F("Fail"));
    if (diodes != 0) {
      Serial.print(F("..Expected 0 diodes, got "));
      Serial.println(diodes, DEC);
    }
    if (resistance != 0) {
      Serial.print(F("..Expected resistance of 0, got "));
      Serial.println(resistance, DEC);
    }
    if (en == true) {
      Serial.println(F("..Got disabled indication"));
    }
    failures++;
    
  } else {
    Serial.println("Pass");
  }
#else
  Serial.println(F("TCREN.01 Enable Trickle Charge - Skipped"));
  Serial.println(F("TCREN.02 Disable Trickle Charge - Skipped"));
#endif

  Serial.print(F("TCREN.03 Enable 1KHZ Signal - "));
  clk24.set1HzState(true);
  if (!clk24.get1HzState()) {
    Serial.println(F("Fail"));
    failures++;
  } else {
    Serial.println(F("Pass"));
  }

  Serial.print(F("TCREN.04 Disable 1KHZ Signal - "));
  clk24.set1HzState(false);
  if (clk24.get1HzState()) {
    Serial.println(F("Fail"));
    failures++;
  } else {
    Serial.println(F("Pass"));
  }

  return failures;
}
  
// Setup routine
void setup()
{
  Serial.begin(9600);
  
  clk12.init(SS);
  clk24.init(SS);
}

// Main run loop
void loop()
{
  int failures = 0; 
  
  // Provide a countdown to start
  for (int i = 5 ; i > 0; i--) {
    Serial.print(F("Test will start in "));
    Serial.print(i, DEC);
    Serial.println(F("..."));
    delay(500);
  }
  
  // Run all tests
  Serial.println(F("Test starting"));
  failures+=clocktests();
  failures+=alarmtests1();
  failures+=alarmtests2();
  failures+=usermemtests();
  failures+=crtests();

  // Report on outcome
  Serial.println("");
  Serial.println("");
  Serial.print(F("Final result - "));
  Serial.println(failures == 0? F("Pass") : F("Fail"));
  if (failures) {
    Serial.print(failures, DEC);
    Serial.println(" total failures");
  }
    
  while(true) { };
}


