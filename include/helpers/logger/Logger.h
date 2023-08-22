/**
 * @file Logger.h
 * @author Ryan Purse
 * @date 06/08/2023
 */


#pragma once

#include <string_view>
#include <unordered_map>
#include <set>
#include <filesystem>
#include "glew.h"
#include "gtc/matrix_access.hpp"

namespace engine
{
    extern class Logger *logger;
    
    typedef int Severity;
    enum Severity_
    {
        Severity_Unknown        = 0b000001,
        Severity_Notification   = 0b000010,
        Severity_Warning        = 0b000100,
        Severity_Minor          = 0b001000,
        Severity_Major          = 0b010000,
        Severity_Fatal          = 0b100000,
    };
    
    enum OutputSourceFlag_
    {
        OutputSourceFlag_File       = 0b001,
        OutputSourceFlag_IoStream   = 0b010,
        OutputSourceFlag_Queue      = 0b100,
    };
    typedef OutputSourceFlag_ OutputSourceFlag;
    inline OutputSourceFlag operator|(OutputSourceFlag a, OutputSourceFlag b)
    {
        return static_cast<OutputSourceFlag>(static_cast<int>(a) | static_cast<int>(b));
    }
    
    inline OutputSourceFlag operator&(OutputSourceFlag a, OutputSourceFlag b)
    {
        return static_cast<OutputSourceFlag>(static_cast<int>(a) | static_cast<int>(b));
    }
    
    struct Message
    {
        int         line;
        Severity   severity;
        std::string file;
        std::string message;
    };
    
    /**
     * @author Ryan Purse
     * @date 06/08/2023
     */
    class Logger
    {
    public:
        /**
         * @brief Logs a message. Use the macro MESSAGE(X, ...), WARN(X, ...) or CRASH(X, ...) to fill in most of the
         * details.
         * @see LoggerMacros.h
         */
        void log(const char file[], int line, Severity severity, std::string_view message);
        
        /**
         * @brief Logs a vector. Use the macro MESSAGE(X, ...), WARN(X, ...) or CRASH(X, ...) to fill in most of the
         * details.
         * @see LoggerMacros.h
         */
        template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
        void log(const char file[], int line, Severity severity, const glm::vec<L, T, Q> &message);
        
        /**
         * @brief Logs a quaternion. Use the macro MESSAGE(X, ...), WARN(X, ...) or CRASH(X, ...) to fill in most of the
         * details.
         * @see LoggerMacros.h
         */
        template<typename T, glm::qualifier Q = glm::defaultp>
        void log(const char file[], int line, Severity severity, const glm::qua<T, Q> &message);
        
        /**
         * @brief Logs a matrix. Use the macro MESSAGE(X, ...), WARN(X, ...) or CRASH(X, ...) to fill in most of the
         * details.
         * @see LoggerMacros.h
         */
        template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q = glm::defaultp>
        void log(const char file[], int line, Severity severity, const glm::mat<C, R, T, Q> &message);
        
        /**
         * @brief Logs a formatted message. % are replaced with args... in the order they appear.
         * Use the macro MESSAGE(X, ...), WARN(X, ...) or CRASH(X, ...) to fill in most of the details.
         * @see LoggerMacros.h
         */
        template<typename ...TArgs>
        void log(const char file[], int line, Severity severity, std::string_view format, const TArgs &... args);
        
        /**
         * @brief Logs a container that supports iterators.
         * Use the macro MESSAGE(X, ...), WARN(X, ...) or CRASH(X, ...) to fill in most of the details.
         * @see LoggerMacros.h
         */
        template<typename TIterator>
        void log(const char file[], int line, Severity severity, const TIterator &start, const TIterator &end);
        
        /** Call back to attach to opengl when in debug mode. */
        void openglCallBack(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
        
        [[nodiscard]] const std::vector<Message> &getLogs() const;
        
        [[nodiscard]] std::string_view toStringView(Severity severity) const;
        
        void clearQueue();
        
        void setOutputFlag(OutputSourceFlag flag);
        
    protected:
        void logToConsole(std::string_view message) const;
        void logToFile(std::string_view message) const;
        void logToQueue(std::string_view message, const char file[], int line, Severity severity);
        
        template<typename T, typename ...TArgs>
        std::string formatString(std::string_view format, const T &arg, const TArgs & ...args);
        
        template<typename T>
        std::string formatValue(const T &value);
        
        template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
        std::string formatValue(glm::vec<L, T, Q> vector);
        
        template<typename T, glm::qualifier Q = glm::defaultp>
        std::string formatValue(glm::qua<T, Q> quaternion);
        
        template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q = glm::defaultp>
        std::string formatValue(glm::mat<C, R, T, Q> matrix);
        
        template<>
        std::string formatValue(const std::filesystem::path &value);
        
        
    protected:
        Severity throwLevel { Severity_Major };
        OutputSourceFlag sources { OutputSourceFlag_File | OutputSourceFlag_IoStream | OutputSourceFlag_Queue };
        std::string_view fileName = "log.txt";
        std::vector<Message> messages;
        
        std::set<uint64_t> blackList { 131185, 131218 };
        
        const std::unordered_map<Severity, std::string_view> severityStringMap {
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
        const std::unordered_map<GLenum, Severity> glSeverityCastMap {
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
    
    template<glm::length_t L, typename T, glm::qualifier Q>
    void Logger::log(const char *file, int line, Severity severity, const glm::vec<L, T, Q> &message)
    {
        log(file, line, severity, formatValue(message));
    }
    
    template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
    void Logger::log(const char *file, int line, Severity severity, const glm::mat<C, R, T, Q> &message)
    {
        log(file, line, severity, formatValue(message));
    }
    
    template<typename... TArgs>
    void Logger::log(const char file[], int line, Severity severity, std::string_view format, const TArgs &... args)
    {
        log(file, line, severity, formatString(format, args...));
    }
    
    template<typename T, glm::qualifier Q>
    void Logger::log(const char *file, int line, Severity severity, const glm::qua<T, Q> &message)
    {
        log(file, line, severity, formatValue(message));
    }
    
    template<typename TIterator>
    void Logger::log(const char *file, int line, Severity severity, const TIterator &start, const TIterator &end)
    {
        std::_Adl_verify_range(start, end);
        
        std::string output = "[";
        TIterator element = start;
        while (true)
        {
            output += formatString("%", *element);
            ++element;
            if (element != end)
                output += ", ";
            else
                break;
        }
        
        output += "]";
        
        log(file, line, severity, output);
    }
    
    template<typename T, typename... TArgs>
    std::string Logger::formatString(std::string_view format, const T &arg, const TArgs &... args)
    {
        std::string output;
        for (int i = 0; i < format.length(); ++i)
        {
            if (format[i] == '%')
            {
                output += formatValue(arg);
                if constexpr (sizeof...(args) != 0)  // The last argument will be used if there are too many %.
                    return output + formatString(std::string_view(format.data() + i + 1, format.size() - i), args...);
            }
            else
                output += format[i];
        }
        
        return output;
    }
    
    template<typename T>
    std::string Logger::formatValue(const T &value)
    {
        if constexpr (std::is_arithmetic_v<T>)
            return std::to_string(value);
        else
            return std::string(value);
    }
    
    template<glm::length_t L, typename T, glm::qualifier Q>
    std::string Logger::formatValue(glm::vec<L, T, Q> vector)
    {
        std::string output = "(";
        
        for (glm::length_t i = 0; i < L - 1; ++i)
            output += formatValue(vector[i]) + ", ";
        
        return output + formatValue(vector[L - 1]) + ")";
    }
    
    template<typename T, glm::qualifier Q>
    std::string Logger::formatValue(glm::qua<T, Q> quaternion)
    {
        std::string output = "(";
        
        for (int i = 0; i < 3; ++i)
            output += formatValue(quaternion[i]) + ", ";
        
        return output + formatValue(quaternion[3]) + ") Euler" + formatValue(glm::eulerAngles(quaternion));
    }
    
    template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
    std::string Logger::formatValue(glm::mat<C, R, T, Q> matrix)
    {
        std::string output = "\n";
        
        for (int i = 0; i < C; ++i)
            output += formatValue(glm::row(matrix, i)) + "\n";
        
        return output;
    }
    
    template<>
    std::string Logger::formatValue(const std::filesystem::path &value)
    {
        return value.string();
    }
}

