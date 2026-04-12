#pragma once

#include <string>

// Alle mulige tilstander en celle i gridet kan ha
// Bruker enum class for å unngå at de blandes med vanlige int-verdier
enum class CellState
{
    Empty,      // White  - kan gå
    Wall,       // Yellow   - kan ikke gå
    Start,      // Green - BFS start
    End,        // Red   - BFS slutt
    Visited,    // Blue   - BFS besøkt
    Path        // Purple - BFS sti
};


// Konverterer en bokstav fra fil til riktig CellState
// Brukes når vi laster inn demo-labyrinten fra en hardkodet vektor
inline CellState char_to_state(const char& state)
{
    if(state == 'W') return CellState::Empty;
    if(state == 'Y') return CellState::Wall;
    if(state == 'G') return CellState::Start;
    if(state == 'R') return CellState::End;
    if(state == 'C') return CellState::Visited;
    if(state == 'P') return CellState::Path;
    return CellState::Empty; // standard hvis vi ikke kjenner igjen tegnet
}

// Gjør det motsatte - konverterer CellState tilbake til en bokstav
// Trengs når vi lagrer gridet til fil
inline char state_to_char(CellState s)
{
    switch(s)
    {
        case CellState::Empty:   return 'W';
        case CellState::Wall:    return 'Y';
        case CellState::Start:   return 'G';
        case CellState::End:     return 'R';
        case CellState::Visited: return 'C';
        case CellState::Path:    return 'P';
    }
    return 'W'; // burde aldri skje, men kompilatoren klager uten dette
}
