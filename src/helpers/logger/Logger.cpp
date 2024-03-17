/**
 * @file Logger.cpp
 * @author Ryan Purse
 * @date 06/08/2023
 */


#include "Logger.h"
#include "Statistics.h"

#include <assimp/Logger.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/DefaultLogger.hpp>

#include <sstream>
#include <fstream>


namespace debug
{
    Logger *logger;
    
    Logger::Logger()
    {
        Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
        
        // Streams are handled by the Default Logger's destructor.
        // Assimp::DefaultLogger::get()->attachStream(new StreamOutput(Severity_Notification), Assimp::Logger::Info | Assimp::Logger::Debugging);
        // Assimp::DefaultLogger::get()->attachStream(new StreamOutput(Severity_Warning), Assimp::Logger::Warn);
        // Assimp::DefaultLogger::get()->attachStream(new StreamOutput(Severity_Major), Assimp::Logger::Err);
    }
    
    Logger::~Logger()
    {
        Assimp::DefaultLogger::kill();
    }
    
    void Logger::log(const char file[], int line, Severity severity, std::string_view message)
    {
        std::stringstream ss;
        ss << "[" << severityStringMap.at(severity) << "] ";
        ss << "(" << file << " at line " << line << ")\n";
        ss << message << "\n";
        
        const std::string output = ss.str();
        logToConsole(output);
        logToFile(output);
        logToQueue(message, file, line, severity);
        
        if (severity >= throwLevel)
        {
            std::cout << output;
            throw LogException();
        }
    }
    
    void Logger::logToConsole(std::string_view message) const
    {
        if (sources & OutputSourceFlag_IoStream)
            std::cout << message;
    }
    
    void Logger::logToFile(std::string_view message) const
    {
        if (!(sources & OutputSourceFlag_File))
            return;
        
        std::ofstream file(fileName.data(), std::ios_base::app);
        file << message;
        file.close();
    }
    
    void Logger::logToQueue(std::string_view message, const char file[], int line, Severity severity)
    {
        if ((sources & OutputSourceFlag_Queue))
            messages.emplace_back(Message { line, severity, std::filesystem::path(file), std::string(message) });
    }

    void Logger::openglCallBack(
        GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,
        const void *userParam)
    {
        if (blackList.find(id) != blackList.end())
            return;  // Ignore black listed elements.
        if (type == GL_DEBUG_TYPE_PUSH_GROUP || type == GL_DEBUG_TYPE_POP_GROUP)
            return;

        const Severity level = glSeverityCastMap.at(severity);
        
        std::stringstream ss;
        ss << "[OpenGL | " << severityStringMap.at(level) << "]\n";
        ss << "  Source: " <<  glSourceMap.at(source) << "\n";
        ss << "    Type: " << glTypeMap.at(type) << "\n";
        ss << " Message: " << message << "\n";
        
        const std::string output = ss.str();
        logToConsole(output);
        logToFile(output);
        logToQueue(message, "", -1, level);
        
        if (level >= throwLevel)
        {
            std::cout << output;
            throw LogException();
        }
    }
    
    const std::vector<Message> &Logger::getLogs() const
    {
        return messages;
    }
    
    std::string_view Logger::toStringView(Severity severity) const
    {
        return severityStringMap.at(severity);
    }
    
    void Logger::clearQueue()
    {
        messages.clear();
    }
    
    void Logger::setOutputFlag(OutputSourceFlag flag)
    {
        sources = flag;
    }
    
    
    StreamOutput::StreamOutput(Severity severity)
        : mSeverity(severity)
    {
    
    }
    
    void StreamOutput::write(const char *message)
    {
        switch (mSeverity)
        {
            default:
            case Severity_Notification:
            case Severity_Unknown:
                MESSAGE("%", message);
                break;
            case Severity_Warning:
            case Severity_Minor:
            case Severity_Major:
                WARN("%", message);
                break;
            case Severity_Fatal:
                CRASH("%", message);
        }
    }
}
