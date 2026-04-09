#pragma once

#include <stdexcept>
#include <string>

class GridException : public std::logic_error
{
    public:
        explicit GridException(const std::string& message) : std::logic_error(message)
        {}
};
