#ifndef F_REBOOT_H
#define F_REBOOT_H

// Reboot telemetry: every software restart goes through f_reboot_restart() with a
// short cause, and the next boot reports it (plus the hardware reset reason,
// the previous boot's uptime and lowest free heap, and a crash summary from the
// core dump) to UPDATE_SERVER_BASE/reboot_report.

// Records cause/detail in RTC memory and restarts. detail may be NULL.
void f_reboot_restart(const char *cause, const char *detail) __attribute__((noreturn));

// Early in app_main: read the reset reason, the RTC record and any core dump.
void f_reboot_capture(void);

// Periodically: remember uptime and lowest free heap for the next boot's report.
void f_reboot_tick(void);

// Send the report once per boot (needs WiFi) if the reset can help troubleshooting
// (crash, watchdog, brownout, firmware recovery); retries on later calls until it succeeds.
void f_reboot_report(void);

#endif
