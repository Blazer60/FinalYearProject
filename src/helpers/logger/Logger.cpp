/**
 * @file Logger.cpp
 * @author Ryan Purse
 * @date 06/08/2023
 */


#include <sstream>
#include <fstream>
#include "Logger.h"

namespace engine
{
    Logger *logger;
    
    void Logger::log(const char file[], int line, Severity_ severity, std::string_view message)
    {
        std::stringstream ss;
        ss << "[" << severityStringMap.at(severity) << "] ";
        ss << "(" << file << " at line " << line << ")\n";
        ss << message << "\n";
        
        const std::string output = ss.str();
        logToConsole(output);
        logToFile(output);
        
        if (severity >= throwLevel)
            throw LogException();
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
    
    void Logger::openglCallBack(
        GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,
        const void *userParam)
    {
        if (blackList.find(id) != blackList.end())
            return;  // Ignore black listed elements.
        
        Severity_ level = glSeverityCastMap.at(severity);
        
        std::stringstream ss;
        ss << "[OpenGL | " << severityStringMap.at(level) << "]\n";
        ss << "  Source: " <<  glSourceMap.at(source) << "\n";
        ss << "    Type: " << glTypeMap.at(type) << "\n";
        ss << " Message: " << message << "\n";
        
        const std::string output = ss.str();
        logToConsole(output);
        logToFile(output);
        
        if (level >= throwLevel)
            throw LogException();
    }
}
