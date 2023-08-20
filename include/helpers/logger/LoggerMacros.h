/**
 * @file LoggerMacros.h
 * @author Ryan Purse
 * @date 06/08/2023
 */


#pragma once

#include "Pch.h"
#include "EngineState.h"

#define LOG(message, severity)  (engine::logger->log(__FILE__, __LINE__, severity, message))
#define MESSAGE(message, ...)   (engine::logger->log(__FILE__, __LINE__, engine::Severity_Notification,     message, __VA_ARGS__))
#define WARN(message, ...)      (engine::logger->log(__FILE__, __LINE__, engine::Severity_Warning,          message, __VA_ARGS__))
#define CRASH(message, ...)     (engine::logger->log(__FILE__, __LINE__, engine::Severity_Fatal,            message, __VA_ARGS__))

#define LOG_MINOR(message)      LOG(message, engine::Severity_Minor)
#define LOG_MAJOR(message)      LOG(message, engine::Severity_Major)
#define LOG_WARNING(message)    LOG(message, engine::Severity_Warning)
