/**
 * @file LoggerMacros.h
 * @author Ryan Purse
 * @date 06/08/2023
 */


#pragma once

#include "Pch.h"

/**
 * @brief Logs a message at a specific severity.
 */
#define LOG(message, severity)  (debug::logger->log(__FILE__, __LINE__, severity, message))

/**
 * @brief Logs a notification message.
 */
#define MESSAGE(message, ...)   (debug::logger->log(__FILE__, __LINE__, debug::Severity_Notification,     message, __VA_ARGS__))

/**
 * @brief Logs a warning message.
 */
#define WARN(message, ...)      (debug::logger->log(__FILE__, __LINE__, debug::Severity_Warning,          message, __VA_ARGS__))

/**
 * @brief Logs a crash message.
 */
#define CRASH(message, ...)     (debug::logger->log(__FILE__, __LINE__, debug::Severity_Fatal,            message, __VA_ARGS__))

#define LOG_MINOR(message)      LOG(message, debug::Severity_Minor)
#define LOG_MAJOR(message)      LOG(message, debug::Severity_Major)
#define LOG_WARNING(message)    LOG(message, debug::Severity_Warning)
