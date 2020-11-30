/*
 * core 0: used for wifi, BT, ...
 * core 1: where a *normal* program runs
*/

// CoreMark Benchmark for Arduino compatible boards
//   original CoreMark code: https://github.com/eembc/coremark
#include "Arduino.h"
#include <stdarg.h>
#include <esp_task_wdt.h>

#define TWDT_TIMEOUT_S 20

TaskHandle_t Task0;
TaskHandle_t Task1;

// A way to call the C-only coremark function from Arduino's C++ environment
extern "C" int coremark_main(void);

void setup()
{
  // Set the watchdog to 20s (enough to finish the benchmark, and make sure it won't panic)
  esp_task_wdt_init(TWDT_TIMEOUT_S, false); // (Timeout period of TWDT in seconds, Panic aka reboot)

  // Disable the watchdogs
  //disableCore0WDT(); disableCore1WDT(); disableLoopWDT();
  
	Serial.begin(115200); 
	while (!Serial) ; // wait for Arduino Serial Monitor
	delay(500);
	Serial.println("CoreMark Performance Benchmark");
	Serial.println();
	Serial.println("CoreMark measures how quickly your processor can manage linked");
	Serial.println("lists, compute matrix multiply, and execute state machine code.");
	Serial.println();
	Serial.println("Iterations/Sec is the main benchmark result, higher numbers are better");
//	Serial.println("Running.... (usually requires 12 to 20 seconds)");
//	Serial.println();

  // Dual core code
  xTaskCreatePinnedToCore(
    loop0, /* Function to implement the task */
    "Task0", /* Name of the task */
    10000, /* Stack size in words */
    NULL, /* Task input parameter */
    20, /* Priority of the task */
    &Task0, /* Task handle */
    0); /* Core where the task should run */
  delay(500); // Wait 500ms to make sure the output is displayed nicely. Because the cores are running at the same speed, the output would be scambled.
  xTaskCreatePinnedToCore(
    loop1, /* Function to implement the task */
    "Task1", /* Name of the task */
    10000, /* Stack size in words */
    NULL, /* Task input parameter */
    20, /* Priority of the task */
    &Task1, /* Task handle */
    1); /* Core where the task should run */

/*
  // Original, single core code
  Serial.println("Running.... (usually requires 12 to 20 seconds)");
  Serial.println();
  delay(250);
	coremark_main(); // Run the benchmark  :-)
*/
}

void loop0(void * parameter)
{
  Serial.println("Running Task0.... (usually requires 12 to 20 seconds)");
  coremark_main(); // Run the benchmark  :-)
  Serial.println("Task0 finished.");
  vTaskDelete(Task0);
}

void loop1(void * parameter)
{
  Serial.println("Running Task1.... (usually requires 12 to 20 seconds)");
  coremark_main(); // Run the benchmark  :-)
  Serial.println("Task1 finished.");
  vTaskDelete(Task1);
}

void loop()
{
  // Delete the task which was used for this loop
  vTaskDelete(NULL);
}

// CoreMark calls this function to print results.
extern "C" int ee_printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	for (; *format; format++) {
		if (*format == '%') {
			bool islong = false;
			format++;
			if (*format == '%') { Serial.print(*format); continue; }
			if (*format == '-') format++; // ignore size
			while (*format >= '0' && *format <= '9') format++; // ignore size
			if (*format == 'l') { islong = true; format++; }
			if (*format == '\0') break;
			if (*format == 's') {
				Serial.print((char *)va_arg(args, int));
			} else if (*format == 'f') {
				Serial.print(va_arg(args, double));
			} else if (*format == 'd') {
				if (islong) Serial.print(va_arg(args, long));
				else Serial.print(va_arg(args, int));
			} else if (*format == 'u') {
				if (islong) Serial.print(va_arg(args, unsigned long));
				else Serial.print(va_arg(args, unsigned int));
			} else if (*format == 'x') {
				if (islong) Serial.print(va_arg(args, unsigned long), HEX);
				else Serial.print(va_arg(args, unsigned int), HEX);
			} else if (*format == 'c' ) {
				Serial.print(va_arg(args, int));
			}
		} else {
			if (*format == '\n') Serial.print('\r');
			Serial.print(*format);
		}
	}
	va_end(args);
	return 1;
}

// CoreMark calls this function to measure elapsed time
extern "C" uint32_t Arduino_millis(void)
{
	return millis();
} 