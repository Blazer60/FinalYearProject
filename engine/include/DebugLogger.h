/**
 * @file DebugLogger.h
 * @brief Logs info to a file/terminal upon users request. Depending on the severity, the logger will also abort the program.
 * @author Ryan Purse
 * @date 20/07/2021
 */


#pragma once

#ifdef NO_PCH
#include "Pch.h"
#endif  // NO_PCH

#include <exception>
#include <deque>
#include <set>
#include <string_view>
#include <string>
#include <glew.h>

namespace debug
{
    static std::set<uint64_t> blackList { 131185, 131218 };
    
    /**
     * Severity types for when you want to log something.
     * @enum Notification - Used for logging miscellaneous information.
     * @enum Warning - Used when the program can still continue but said thing should be avoided.
     * @enum Minor - Used when an error could occur or something has an easy fix. Such as changing an enum to the correct state.
     * @enum Major - Used when ar error will occur when an action is performed. E.g.: Run-time behaviour is ill defined.
     * @enum Fatal - Used when the program cannot continue regardless of whats happened.
     * @enum Unknown - An incoming message has no known type attached to it.
     */
    enum class severity : unsigned char { Notification, Warning, Minor, Major, Fatal, Unknown };
    
    enum class sources : unsigned char { File=0b001, IoStream=0b010, Queue=0b100 };
    
    void setSources(unsigned char sourcesFlag);

    /** Sets the severity level in which the program will crash. */
    void setThrowLevel(severity level);

    /** Gets the severity level in which the program will crash. */
    [[nodiscard]] severity getThrowLevel();

    /**
     * Logs a message, force crashing if the severity level is above a threshold.
     * @param message The message that you want to be outputted.
     * @param level The severe the message is.
     */
    void log(std::string_view message, severity level=severity::Notification);

    /**
     * Logs a message, force crashing if the severity level is above a threshold.
     * @param message The message that you want to be outputted.
     * @param level The severe the message is.
     */
    void log(const unsigned char *message, severity level=severity::Notification);

    /** Call back to attach to opengl when in debug mode. */
    void openglCallBack(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);

    /** Clears the log file. */
    void clearLogs();
    
    /** Gets the logs so that they can be printed elsewhere. */
    const std::deque<std::string> &getLogs();

    /** Used when a log exception occurs. */
    class LogException
            : public std::exception
    {
        [[nodiscard]] const char *what() const noexcept override
        {
            return "Log message exceed the maximum throw level threshold. Check the logs for more information.";
        }
    };
}