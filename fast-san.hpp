#pragma once
#include "chess.hpp"
#include <string_view>

namespace FastSAN{
chess::Move parseSan(chess::Board& board, std::string_view fen);
}
