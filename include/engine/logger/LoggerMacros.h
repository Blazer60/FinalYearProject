/**
 * @file LoggerMacros.h
 * @author Ryan Purse
 * @date 06/08/2023
 */


#pragma once

#include "Pch.h"
#include "EngineState.h"

#define LOG(message, severity)  (engine::logger->log(__FILE__, __LINE__, message, severity))
#define MESSAGE(message)        (engine::logger->log(__FILE__, __LINE__, message, engine::Severity_Notification))
#define WARN(message)           (engine::logger->log(__FILE__, __LINE__, message, engine::Severity_Warning))
#define CRASH(message)          (engine::logger->log(__FILE__, __LINE__, message, engine::Severity_Fatal))

#define LOG_MINOR(message)      LOG(message, engine::Severity_Minor)
#define LOG_MAJOR(message)      LOG(message, engine::Severity_Major)
#define LOG_WARNING(message)    LOG(message, engine::Severity_Warning)
