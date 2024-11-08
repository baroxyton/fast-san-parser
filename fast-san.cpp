#include "fast-san.hpp"
#include <string_view>
void FastSAN::parseSAN(chess::Board &board, chess::Move &move, std::string_view fen){
  const int isWhite = board.sideToMove() == chess::Color::WHITE;
  
}
