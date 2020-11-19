#include <Arduino.h>
#include <TimeUtil.h>
#include <NTPClient.h>
#include <Config.h>
#include <Common.h>

volatile time_t t = 0;

void InitTime()
{
#ifdef DEBUG_TIME
  Serial.print("Time Server: ");
  Serial.println(Config::timeServer);
#endif
  t = NTPClient::getUTC();
  if (t != 0)
    t += Config::timeZone * 60;

#ifdef DEBUG_TIME
  char buff[128];
  tm tr;
  time_t now = t;

  gmtime_r(&now, &tr);
  strftime(buff, sizeof(buff), "DateTime: %a %d/%m/%Y %T%n", &tr);
  Serial.print(buff);
#endif

  cli();//stop interrupts

  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();//allow interrupts
}

//boolean toggle1 = false;

ISR(TIMER1_COMPA_vect){//timer1 interrupt 1Hz toggles pin 13 (LED)
  // Generates pulse wave of frequency 1Hz/2 = 0.5Hz
  if (t == 0)
    return;

  // digitalWrite(13, toggle1 ? HIGH : LOW);
  // toggle1 = !toggle1;
  t++;
}

void getFatDateTime(uint16_t *date, uint16_t *time)
{
  if (t == 0)
  {
    *date = FAT_DEFAULT_DATE;
    *time = FAT_DEFAULT_TIME;
  }
  else
  {
    tm tr;
    time_t now = t;
    gmtime_r(&now, &tr);
    *date = FAT_DATE(tr.tm_year + 1900, tr.tm_mon + 1, tr.tm_mday);
    *time = FAT_TIME(tr.tm_hour, tr.tm_min, tr.tm_sec);
  }
}

