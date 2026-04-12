#pragma once

#include <stdexcept>
#include <string>

// Egen exception-klasse for feil som skjer i gridet
// Arver fra std::logic_error slik at vi kan bruke .what() til å hente feilmeldingen

class GridException : public std::logic_error
{
    public:
        // Tar imot en feilmelding som string og sender den videre til basisklassen
        explicit GridException(const std::string& message) : std::logic_error(message)
        {}
};
