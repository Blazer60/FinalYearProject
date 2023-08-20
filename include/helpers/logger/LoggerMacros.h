/**
 * @file LoggerMacros.h
 * @author Ryan Purse
 * @date 06/08/2023
 */


#pragma once

#include "Pch.h"
#include "EngineState.h"

/**
 * @brief Logs a message at a specific severity.
 */
#define LOG(message, severity)  (engine::logger->log(__FILE__, __LINE__, severity, message))

/**
 * @brief Logs a notification message.
 */
#define MESSAGE(message, ...)   (engine::logger->log(__FILE__, __LINE__, engine::Severity_Notification,     message, __VA_ARGS__))

/**
 * @brief Logs a warning message.
 */
#define WARN(message, ...)      (engine::logger->log(__FILE__, __LINE__, engine::Severity_Warning,          message, __VA_ARGS__))

/**
 * @brief Logs a crash message.
 */
#define CRASH(message, ...)     (engine::logger->log(__FILE__, __LINE__, engine::Severity_Fatal,            message, __VA_ARGS__))

#define LOG_MINOR(message)      LOG(message, engine::Severity_Minor)
#define LOG_MAJOR(message)      LOG(message, engine::Severity_Major)
#define LOG_WARNING(message)    LOG(message, engine::Severity_Warning)
