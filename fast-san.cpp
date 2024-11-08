#include "fast-san.hpp"
#include "chess.hpp"
#include <cstdint>
#include <string_view>
// Performance-focused, validation-free SAN parser for standard chess
chess::Move FastSAN::parseSan(chess::Board &board, std::string_view fen){
  const bool isWhite = board.sideToMove() == chess::Color::WHITE;
  
  std::uint16_t type = chess::Move::NORMAL;
  std::uint16_t source;
  std::uint16_t target;

  // CASTLES
  if(fen == "O-O"){
   type = chess::Move::CASTLING; 
   if(isWhite){
      // e1 to 81
      source = 4;
      target = 7;
    }
   else{
      // e8 to h8
      source = 4 + 8 * 7;
      target = 7 + 8 * 7;
    
    }
    return chess::Move((type) + (source << 6) + target);
   
  }
  if(fen == "O-O-O"){
   type = chess::Move::CASTLING; 
    if(isWhite){
      // e1 to a1
      source = 4;
      target = 0; 
    }
    else{
      // e8 to a8
      source = 4 + 8 * 7;
      target = 0 + 8 * 7;

    }
    return chess::Move((type << 14) + (source << 6) + target);

  }

  // PAWN MOVES
  
  // PIECE MOVES: NORMAL

  // PIECE MOVES: PROMOTION (handle with original implementation) 
  //move = chess::uci::parseSan(board, fen)
}
