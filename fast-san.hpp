#pragma once
#include "chess.hpp"
#include <string_view>

namespace FastSAN{
    void parseSAN(chess::Board& board, chess::Move& move, std::string_view fen);
}
