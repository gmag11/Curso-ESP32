#include "timeSync.h"
#include <sntp.h> 
#include <M5StickCPlus.h>
#include <QuickDebug.h>

#if __has_include("wificonfig.h")
#include "wificonfig.h"
#else
constexpr auto NTP_SERVER = "time.cloudflare.com";
#endif

bool timeSyncd = false;

constexpr auto TAG_TIME = "TIME";

void initTimeSync (){
    sntp_setoperatingmode (SNTP_OPMODE_POLL);
    sntp_set_sync_mode (SNTP_SYNC_MODE_SMOOTH);
    sntp_setservername (0, NTP_SERVER);
    setenv ("TZ", PSTR ("CET-1CEST,M3.5.0,M10.5.0/3"), 1);
    tzset ();
    sntp_set_time_sync_notification_cb (time_sync_cb);
    sntp_init ();
    DEBUG_INFO (TAG_TIME, "Time sync initialized");
}

void time_sync_cb (timeval* ntptime) {
    time_t hora_actual = time (NULL);

    tm* timeinfo = localtime (&hora_actual);
    RTC_DateTypeDef sDate;
    RTC_TimeTypeDef sTime;

    sTime.Hours = timeinfo->tm_hour;
    sTime.Minutes = timeinfo->tm_min;
    sTime.Seconds = timeinfo->tm_sec;

    M5.Rtc.SetTime (&sTime);

    sDate.WeekDay = timeinfo->tm_wday;
    sDate.Month = timeinfo->tm_mon + 1;
    sDate.Date = timeinfo->tm_mday;
    sDate.Year = timeinfo->tm_year + 1900;

    M5.Rtc.SetData (&sDate);

    DEBUG_INFO (TAG_TIME, "Time sync done");

    timeSyncd = true;
}