/**
 * @file Logger.h
 * @author Ryan Purse
 * @date 06/08/2023
 */


#pragma once

#include <string_view>
#include <unordered_map>
#include <set>
#include "glew.h"

namespace engine
{
    extern class Logger *logger;
    
    enum Severity_
    {
        Severity_Notification,
        Severity_Warning,
        Severity_Minor,
        Severity_Major,
        Severity_Fatal,
        Severity_Unknown,
    };
    
    enum OutputSourceFlag_
    {
        OutputSourceFlag_File       = 0b001,
        OutputSourceFlag_IoStream   = 0b010,
        OutputSourceFlag_Queue      = 0b100,
    };
    
    /**
     * @author Ryan Purse
     * @date 06/08/2023
     */
    class Logger
    {
    public:
        void log(const char file[], int line, std::string_view message, Severity_ severity=Severity_Notification);
        void log(const char file[], int line, const glm::vec3 &message, Severity_ severity=Severity_Notification);
        
        /** Call back to attach to opengl when in debug mode. */
        void openglCallBack(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
        
    protected:
        void logToConsole(std::string_view message) const;
        void logToFile(std::string_view message) const;
        
    protected:
        Severity_ throwLevel { Severity_Major };
        int sources { OutputSourceFlag_File | OutputSourceFlag_IoStream };
        std::string_view fileName = "log.txt";
        
        std::set<uint64_t> blackList { 131185, 131218 };
        
        const std::unordered_map<Severity_, std::string_view> severityStringMap {
            { Severity_Notification,   "Notification" },
            { Severity_Warning,        "Warning" },
            { Severity_Minor,          "Minor" },
            { Severity_Major,          "Major" },
            { Severity_Fatal,          "Fatal" },
            { Severity_Unknown,        "Unknown" },
        };
        
        const std::unordered_map<GLenum, std::string_view> glSourceMap {
            { GL_DEBUG_SOURCE_API,              "API" },
            { GL_DEBUG_SOURCE_WINDOW_SYSTEM ,   "Window System" },
            { GL_DEBUG_SOURCE_SHADER_COMPILER,  "Shader Compiler" },
            { GL_DEBUG_SOURCE_THIRD_PARTY,      "Third Party" },
            { GL_DEBUG_SOURCE_APPLICATION,      "Application" },
            { GL_DEBUG_SOURCE_OTHER,            "Other" },
        };
        
        const std::unordered_map<GLenum, std::string_view> glTypeMap {
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
        const std::unordered_map<GLenum, Severity_> glSeverityCastMap {
            { GL_DEBUG_SEVERITY_NOTIFICATION,   Severity_Notification },
            { GL_DEBUG_SEVERITY_LOW,            Severity_Minor },
            { GL_DEBUG_SEVERITY_MEDIUM,         Severity_Major },
            { GL_DEBUG_SEVERITY_HIGH,           Severity_Fatal },
        };
    };
    
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

