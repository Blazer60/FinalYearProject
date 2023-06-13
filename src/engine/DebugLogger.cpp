/**
 * @file DebugLogger.cpp
 * @brief Logs info to a file/terminal upon users request. Depending on the severity, the logger will also abort the program.
 * @author Ryan Purse
 * @date 20/07/2021
 */

#include "DebugLogger.h"

#include "Pch.h"
#include <fstream>
#include <sstream>
#include <deque>

namespace debug
{
    static severity throwLevel = severity::Major;
    static unsigned char outputSources = static_cast<unsigned char>(sources::File)
                                       | static_cast<unsigned char>(sources::IoStream);

    static std::unordered_map<severity, std::string_view> severityStringMap {
            { severity::Notification,   "Notification" },
            { severity::Warning,        "Warning" },
            { severity::Minor,          "Minor" },
            { severity::Major,          "Major" },
            { severity::Fatal,          "Fatal" },
            { severity::Unknown,        "Unknown" },
    };

    static std::string_view fileName = "log.txt";

    static std::unordered_map<GLenum, std::string_view> glSourceMap {
            { GL_DEBUG_SOURCE_API,              "API" },
            { GL_DEBUG_SOURCE_WINDOW_SYSTEM ,   "Window System" },
            { GL_DEBUG_SOURCE_SHADER_COMPILER,  "Shader Compiler" },
            { GL_DEBUG_SOURCE_THIRD_PARTY,      "Third Party" },
            { GL_DEBUG_SOURCE_APPLICATION,      "Application" },
            { GL_DEBUG_SOURCE_OTHER,            "Other" },
    };

    static std::unordered_map<GLenum, std::string_view> glTypeMap {
            { GL_DEBUG_TYPE_ERROR,                  "Error" },
            { GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR,    "Deprecated Behavior" },
            { GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR,     "Undefined Behavior" },
            { GL_DEBUG_TYPE_PORTABILITY,            "Portability" },
            { GL_DEBUG_TYPE_MARKER,                 "Marker" },
            { GL_DEBUG_TYPE_PUSH_GROUP,             "Push Group" },
            { GL_DEBUG_TYPE_POP_GROUP,              "Pop Group" },
            { GL_DEBUG_TYPE_OTHER,                  "Other" },
    };

    // GL notification is not aligned with the others.
    static std::unordered_map<GLenum, severity> glSeverityCastMap {
            { GL_DEBUG_SEVERITY_NOTIFICATION,   severity::Notification },
            { GL_DEBUG_SEVERITY_LOW,            severity::Minor },
            { GL_DEBUG_SEVERITY_MEDIUM,         severity::Major },
            { GL_DEBUG_SEVERITY_HIGH,           severity::Fatal },
    };

    static std::deque<std::string> logQueue;
    static uint64_t logQueueSizeMax { 20ull };
    
    void setSources(unsigned char sourcesFlag)
    {
    
    }
    // Throw level getters and setters.
    void setThrowLevel(severity level) { throwLevel = level; }
    
    severity getThrowLevel() { return throwLevel; }
    
    void logFile(std::string_view msg)
    {
        if (!(outputSources & static_cast<unsigned char>(sources::File)))
            return;
        
        std::ofstream file(fileName.data(), std::ios_base::app);
        file << msg;
        file.close();
    }
    
    void logConsole(std::string_view msg)
    {
        if (!(outputSources & static_cast<unsigned char>(sources::IoStream)))
            return;
        
        std::cout << msg;
    }
    
    void logToQueue(std::string_view msg)
    {
        if (!(outputSources & static_cast<unsigned char>(sources::Queue)))
            return;
        
        if (logQueue.size() >= logQueueSizeMax)
            logQueue.pop_front();
        logQueue.emplace_back(msg);
    }
    
    void logToSources(const std::stringstream &ss)
    {
        std::string output = ss.str();

        logFile(output);
        logConsole(output);
        logToQueue(output);
    }
    
    void log(std::string_view message, severity level)
    {
        std::stringstream ss;
        ss << "-- Log Call --\n";
        ss << "Severity: " << severityStringMap.at(level) << "\n";
        ss << " Message: " << message << "\n";
        ss << "-- End Log Call --\n";

        logToSources(ss);
        if (level >= throwLevel) { throw LogException(); }
    }
    
    void log(const unsigned char *message, severity level)
    {
        std::stringstream ss;
        ss << "-- Log Call --\n";
        ss << "Severity: " << severityStringMap.at(level) << "\n";
        ss << " Message: " << message << "\n";
        ss << "-- End Log Call --\n";

        logToSources(ss);
        if (level >= throwLevel) { throw LogException(); }
    }
    
    void openglCallBack(GLenum source, GLenum type, GLuint id,
                        GLenum severity, GLsizei length,
                        const GLchar *message, const void *userParam)
    {
        if (blackList.find(id) != blackList.end())
            return;  // Ignore black listed elements.
        
        debug::severity level = glSeverityCastMap.at(severity);

        std::stringstream ss;
        ss << "-- OpenGL Log ( "<< id <<" ) --\n";

        ss << "  Source: " <<  glSourceMap.at(source) << "\n";
        ss << "    Type: " << glTypeMap.at(type) << "\n";
        ss << "Severity: " << severityStringMap.at(level) << "\n";
        ss << " Message: " << message << "\n";

        logToSources(ss);
        if (level >= throwLevel) { throw LogException(); }
    }
    
    void clearLogs()
    {
        std::ofstream file(fileName.data(), std::ios_base::out | std::ios_base::trunc);
        file.close();
    }
    
    const std::deque<std::string> &getLogs()
    {
        return logQueue;
    }
}