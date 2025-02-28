#include <PCF8563.h>

PCF8563 pcf;

void setup() {
    Serial.begin(9600);
    pcf.init(); // Initialize the clock
    pcf.stopClock(); // Stop the clock

    // Extract compile-time date and time
    int year, month, day, hour, minute, second;
    char monthStr[4]; // Buffer for storing month abbreviation

    sscanf(__DATE__, "%s %d %d", monthStr, &day, &year);
    sscanf(__TIME__, "%d:%d:%d", &hour, &minute, &second);

    // Convert month abbreviation to number
    const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    for (month = 0; month < 12; month++) {
        if (strncmp(monthStr, months[month], 3) == 0) {
            month++; // Convert 0-based index to 1-based month number
            break;
        }
    }

    year -= 2000; // Convert full year (e.g., 2025) to RTC format (e.g., 25)

    // Set the RTC time
    pcf.setYear(year);
    pcf.setMonth(month);
    pcf.setDay(day);
    pcf.setHour(hour);
    pcf.setMinut(minute);
    pcf.setSecond(second);

    pcf.startClock(); // Start the clock
}

void loop() {
    Time nowTime = pcf.getTime(); // Get current time

    // Print current time
    Serial.print(nowTime.day);
    Serial.print("/");
    Serial.print(nowTime.month);
    Serial.print("/");
    Serial.print(nowTime.year + 2000); // Convert back to full year
    Serial.print(" ");
    Serial.print(nowTime.hour);
    Serial.print(":");
    Serial.print(nowTime.minute);
    Serial.print(":");
    Serial.println(nowTime.second);

    delay(1000);
}
