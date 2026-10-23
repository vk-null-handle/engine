#include "logger.h"
#include <stdarg.h>
#include <stdio.h>

void _log_output(LogSeverity severity, const char* msg, ...) {
	char buff[1024];
	const char* severity_str;

	va_list list;
	va_start(list, msg);
	vsnprintf(buff, sizeof(buff), msg, list);
	va_end(list);

	// Switch on severity
	switch (severity) {
	case LOG_SEVERITY_TRACE:
		severity_str = "TRACE";
		break;
	case LOG_SEVERITY_DEBUG:
		severity_str = "DEBUG";
		break;
	case LOG_SEVERITY_WARN:
		severity_str = "WARN";
		break;
	case LOG_SEVERITY_ERROR:
		severity_str = "ERROR";
		break;
	default:
		break;
	}

	b8 is_error = (severity == LOG_SEVERITY_ERROR);
	FILE* console = is_error ? stderr : stdout;
	fprintf(console, "[%s] %s\n", severity_str, buff);
}

void _report_assertion_failure(const char* expression, const char* message, const char* file, u32 line) {
	_log_output(LOG_SEVERITY_ERROR, "Assertion Failure: %s, message: '%s', file: %s line: %d\n", expression, message, file, line);
}
