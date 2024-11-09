#include "fast-san.hpp"
#include "chess.hpp"
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <string_view>
bool rookMatch(const int &src, const int &target) {
  return (src / 8 == target / 8) || (src % 8 == target % 8);
}
bool bishopMatch(const int &src, const int &target) {
  return std::abs((src / 8) - (target / 8)) ==
         std::abs((src % 8) - (target % 8));
}
bool knightMatch(const int &src, const int &target) {
  return (std::abs((src / 8) - (target / 8)) == 2 &&
          abs((src % 8) - (target % 8)) == 1) ||
         (std::abs((src / 8) - (target / 8)) == 1 &&
          abs((src % 8) - (target % 8)) == 2);
}
bool queenMatch(const int &src, const int &target) {
  return rookMatch(src, target) || bishopMatch(src, target);
}
bool kingMatch(const int &src, const int &target) {
  int rowDiff = std::abs((src / 8) - (target / 8));
  int colDiff = std::abs((src % 8) - (target % 8));
  return rowDiff <= 1 && colDiff <= 1 && (src != target);
}
chess::PieceType parseType(char type) {
  chess::PieceType pt;
  switch (type) {
  case 'N':
    pt = chess::PieceType::KNIGHT;
    break;
  case 'B':
    pt = chess::PieceType::BISHOP;
    break;
  case 'R':
    pt = chess::PieceType::ROOK;
    break;
  case 'Q':
    pt = chess::PieceType::QUEEN;
    break;
  case 'K':
    pt = chess::PieceType::KING;
  default:
    break;
  }

  return pt;
}
chess::Bitboard getPieces(chess::Board &board, chess::PieceType type,
                          bool isWhite) {
  auto color = isWhite ? chess::Color::WHITE : chess::Color::BLACK;
  auto bitBoard = board.pieces(type, color);
  return bitBoard;
}
// Performance-focused, validation-free SAN parser for standard chess
chess::Move FastSAN::parseSan(chess::Board &board, std::string_view fen) {
  const bool isWhite = board.sideToMove() == chess::Color::WHITE;

  std::uint16_t type = chess::Move::NORMAL;
  std::uint16_t source;
  std::uint16_t target;

  // CASTLES
  if (fen == "O-O") {
    type = chess::Move::CASTLING;
    if (isWhite) {
      // e1 to 81
      source = 4;
      target = 7;
    } else {
      // e8 to h8
      source = 4 + 8 * 7;
      target = 7 + 8 * 7;
    }
    return chess::Move((type) + (source << 6) + target);
  }
  if (fen == "O-O-O") {
    type = chess::Move::CASTLING;
    if (isWhite) {
      // e1 to a1
      source = 4;
      target = 0;
    } else {
      // e8 to a8
      source = 4 + 8 * 7;
      target = 0 + 8 * 7;
    }
    return chess::Move((type) + (source << 6) + target);
  }

  // PAWN MOVES
  if (fen[0] < 'i' && fen[0] >= 'a') {
    int direction = isWhite ? 1 : -1;
    if (fen.find("=") != std::string_view::npos) {
      // Handle promotion with orginal implementation
      return chess::uci::parseSan(board, fen);
    }
    int srcFile = fen[0] - 'a';
    int srcRank;
    bool capture = false;
    int file, rank;
    for (auto &chr : fen.substr(1, fen.length() - 1)) {
      if (chr == 'x') {
        capture = true;
        continue;
      } else if (chr < '9' && chr > '0') {
        rank = chr - '1';
      } else if (chr < 'i' && chr >= 'a') {
        file = chr - 'a';
        std::cout << file << std::endl << std::flush;
      }
      if (!capture) {
        file = srcFile;
        bool needsCheck = (isWhite && rank == 3) || (!isWhite && rank == 4);
        if (needsCheck) {
          std::cout << file << std::endl;
          if (board.at(file + 8 * (rank - direction)) ==
                  chess::Piece::WHITEPAWN ||
              board.at(file + 8 * (rank - direction)) ==
                  chess::Piece::BLACKPAWN) {
            srcRank = rank - direction;
          } else {
            srcRank = rank - direction * 2;
          }
        } else {
          srcRank = rank - direction;
        }
        source = srcFile + srcRank * 8;
        target = file + rank * 8;
        return chess::Move((type) + (source << 6) + target);
      }
    }
  }

  // PIECE MOVES
  if (fen[0] < 'Z' && fen[0] > 'A') {
    char piece = fen[0];
    int tfile, trank, spos, tpos;
    int len = fen.size();
    if (fen.find("+") != std::string_view::npos ||
        fen.find("#") != std::string_view::npos) {
      len--;
    }
    if (fen[1] == 'x') {
      if (len > 4) {
        // special case, handle with original implementation
        return chess::uci::parseSan(board, fen);
      }
      tfile = fen[2] - 'a';
      trank = fen[3] - '1';
    } else {
      if (len > 3) {
        return chess::uci::parseSan(board, fen);
      }
      tfile = fen[1] - 'a';
      trank = fen[2] - '1';
    }
    tpos = tfile + trank * 8;
    chess::PieceType pieceType = parseType(piece);
    auto piecebb = getPieces(board, pieceType, isWhite);

    while (!piecebb.empty()) {
      spos = piecebb.pop();
      std::cout << "TPOS " << tpos << std::endl;
      std::cout << "SPOS " << spos << std::endl;
      std::cout << "PIECE " << piece << std::endl;
      if ((piece == 'N' && knightMatch(spos, tpos)) ||
          (piece == 'B' && bishopMatch(spos, tpos)) ||
          (piece == 'R' && rookMatch(spos, tpos)) ||
          (piece == 'Q' &&
           (bishopMatch(spos, tpos) || rookMatch(spos, tpos))) ||
          (piece == 'K' && kingMatch(spos, tpos))) {

            return chess::Move(tpos + (spos << 6));

      }
    }
    // We shouldn't be here..
    throw std::exception();
  }
}
