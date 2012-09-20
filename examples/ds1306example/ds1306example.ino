/*
 * File                 ds1306example.ino
 *
 * Synopsis             Example Sketch for DS1306 library
 *
 * Author               Chris Bearman
 *
 * Version              1.0
 *
 * License              This software is released under the terms of the Mozilla Public License (MPL) version 2.0
 *                      Full details of licensing terms can be found in the "LICENSE" file, distributed with this code
 *
 * Instructions
 *                      Connect DS1306 to SPI bus using SS pin for chip select
 *                      Connect alarm0 interrupt to pin 2 so we can use HW interrupt
 *                      Download and run sketch. Sketch will set time to Aug 28 2008, midnight
 *                      and then report time every second. Alarm will be set to go off on 30 seconds
 *                      past every minute, at which point the interrupt should trigger and display
 *                      an additional message
 */
#include <DS1306.h>

// Create a new object to interact with the RTC
DS1306 rtc;

// Interrupt handler for alarm
void alarmTriggered()
{
  Serial.println("** ALARM WENT OFF! **");
  rtc.clearAlarmState(0);
}

void setup()
{
  // Initialize serial monitor for 9600 baud
  Serial.begin(9600);
  
  // Initialize RTC
  // Assumes that RTC select line is SS (digital 10 on Uno)
  rtc.init(SS);
  
  // Let's set a time on the clock
  ds1306time t;
  t.day = 28;
  t.month = 8;
  t.year = 8;
  t.minutes = 0;
  t.hours = 0;
  t.seconds = 0;
  t.dow = DS1306_THURSDAY;
  
  rtc.setTime(&t);
  
  // Let's set an alarm to go off at 30 seconds past each minute
  ds1306alarm a;
  a.dow = DS1306_ANY;
  a.hours = DS1306_ANY;
  a.minutes = DS1306_ANY;
  a.seconds = 30;
  rtc.setAlarm(0, &a);
  rtc.enableAlarm(0);
  
  // Let's grab the interrupt for the alarm
  // Assumes that pin 2 is connected properly to the alarm0 interrupt line
  pinMode(2, INPUT);
  attachInterrupt(0, alarmTriggered, FALLING); 
}

void loop()
{
  // Get the time
  ds1306time t;
  rtc.getTime(&t);
  
  // Format time
  char buf[128];
  snprintf(buf, 128, "%02d/%02d/20%02d %d:%02d:%02d %cM",
    t.month,
    t.day,
    t.year,
    t.hours12,
    t.minutes,
    t.seconds,
    t.ampm
  );
  
  // Output time
  Serial.println(buf);
  
  // Wait for a second before allowing loop to restart
  delay(1000);
  
}

