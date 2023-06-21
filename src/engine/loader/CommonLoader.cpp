/**
 * @file CommonLoader.cpp
 * @author Ryan Purse
 * @date 23/02/2022
 */


#include "CommonLoader.h"
#include <fstream>

namespace load
{
    void parseFile(std::string_view path, const ArgumentList &argumentList)
    {
        std::ifstream fileStream(path.data());
        
        if (fileStream.bad() || fileStream.fail())
        {
            debug::log("Path to file could not be found. Aborting: " + std::string(path), debug::severity::Warning);
            return;
        }
        
        std::string line;
        while (std::getline(fileStream, line))
        {
            if (line.empty())
                continue;
            
            // Split by keywords and expression. Only one keyword and expression each line.
            const auto separator = std::find(line.begin(), line.end(), ' ');
            const std::string keyword(line.begin(), separator);
            const std::string expression(separator + 1, line.end());
            
            // Try call function
            if (argumentList.count(keyword) > 0)
                argumentList.at(keyword)(expression);
            else
                debug::log("Keyword '" + std::string(keyword) + "' does not exist. Ignoring line",
                           debug::severity::Warning);
        }
    }
    
    std::vector<std::string> split(std::string_view args, char delim)
    {
        std::vector<std::string> out;
        
        auto start = args.begin();
        auto end   = std::find(start, args.end(), delim);
        
        while (end != args.end())
        {
            out.emplace_back(start, end);
            start = ++end;  // Skip over delim so that it doesn't get included.
            end = std::find(start, args.end(), delim);
        }
        
        out.emplace_back(start, args.end());  // Always add the last element as a single arg.
        return out;
    }
    
    void doNothing(std::string_view _)
    {
        // Doesn't do anything. We can add this to keywords that we know exist, but we don't want to handle them yet.
    }
    
    std::string convertRelativePath(std::string_view baseFile, std::string_view relativeFile)
    {
        std::filesystem::path baseFilePath(baseFile);
        std::filesystem::path relativeFilePath(relativeFile);
        
        baseFilePath.remove_filename();
        
        return relativeFilePath.is_relative()
               ? baseFilePath.string() + relativeFilePath.string()
               : relativeFilePath.string();
    }
}


