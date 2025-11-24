#include "fast-san-parser/fast-san.hpp"
#include "chess.hpp"
#include <cstdint>
#include <cstdlib>
#include <string_view>

bool rookMatch(const int &src, const int &target) {
    return (src / 8 == target / 8) || (src % 8 == target % 8);
}
bool bishopMatch(const int &src, const int &target) {
    return std::abs((src / 8) - (target / 8)) == std::abs((src % 8) - (target % 8));
}
bool knightMatch(const int &src, const int &target) {
    return (std::abs((src / 8) - (target / 8)) == 2 && abs((src % 8) - (target % 8)) == 1) ||
           (std::abs((src / 8) - (target / 8)) == 1 && abs((src % 8) - (target % 8)) == 2);
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
        break;
    default:
        pt = chess::PieceType::NONE;
        break;
    }

    return pt;
}

chess::Bitboard getPieces(chess::Board &board, chess::PieceType type, bool isWhite) {
    auto color = isWhite ? chess::Color::WHITE : chess::Color::BLACK;
    auto bitBoard = board.pieces(type, color);
    return bitBoard;
}

chess::Move FastSAN::parseSan(chess::Board &board, std::string_view fen) {
    if (fen.empty()) {
        return chess::uci::parseSan(board, fen);
    }

    const bool isWhite = board.sideToMove() == chess::Color::WHITE;

    std::uint16_t type = chess::Move::NORMAL;
    std::uint16_t source = 0;
    std::uint16_t target = 0;

    // CASTLES
    if (fen == "O-O") {
        type = chess::Move::CASTLING;
        if (isWhite) {
            source = 4;
            target = 7;
        } else {
            source = 4 + 8 * 7;
            target = 7 + 8 * 7;
        }
        return chess::Move((type) + (source << 6) + target);
    }
    if (fen == "O-O-O") {
        type = chess::Move::CASTLING;
        if (isWhite) {
            source = 4;
            target = 0;
        } else {
            source = 4 + 8 * 7;
            target = 0 + 8 * 7;
        }
        return chess::Move((type) + (source << 6) + target);
    }

    // PAWN MOVES
    if (fen[0] < 'i' && fen[0] >= 'a') {
        if (fen.size() < 2) {
            return chess::uci::parseSan(board, fen);
        }

        int direction = isWhite ? 1 : -1;
        if (fen.find("=") != std::string_view::npos) {
            return chess::uci::parseSan(board, fen);
        }
        int srcFile = fen[0] - 'a';
        int srcRank = -1;
        bool capture = false;
        int file = -1;
        int rank = -1;
        
        for (auto &chr : fen) {
            if (chr == 'x') {
                capture = true;
                continue;
            } else if (chr < '9' && chr > '0') {
                rank = chr - '1';
            } else if (chr < 'i' && chr >= 'a') {
                file = chr - 'a';
            }
        }

        if (rank == -1 || file == -1) {
            return chess::uci::parseSan(board, fen);
        }

        if (capture) {
            srcRank = rank - direction;
            target = file + rank * 8;
            if (target == board.enpassantSq().index()) {
                type = chess::Move::ENPASSANT;
            }
            source = srcFile + srcRank * 8;
            return chess::Move(type + (source << 6) + target);
        } else {
            file = srcFile;
            bool needsCheck = (isWhite && rank == 3) || (!isWhite && rank == 4);
            if (needsCheck) {
                int oneStepIdx = file + 8 * (rank - direction);
                if (oneStepIdx < 0 || oneStepIdx >= 64) {
                    return chess::uci::parseSan(board, fen);
                }

                if (board.at(oneStepIdx) == chess::Piece::WHITEPAWN ||
                    board.at(oneStepIdx) == chess::Piece::BLACKPAWN) {
                    srcRank = rank - direction;
                } else {
                    srcRank = rank - direction * 2;
                }
            } else {
                srcRank = rank - direction;
            }
            source = srcFile + srcRank * 8;
            target = file + rank * 8;

            if (source < 0 || source >= 64 || target < 0 || target >= 64) {
                 return chess::uci::parseSan(board, fen);
            }

            return chess::Move((type) + (source << 6) + target);
        }
    }

    // PIECE MOVES
    if (fen[0] < 'Z' && fen[0] > 'A') {
        if (fen.size() < 3) {
            return chess::uci::parseSan(board, fen);
        }

        char piece = fen[0];
        int tfile, trank, spos, tpos;
        
        if (fen[1] == 'x') {
            if (fen.size() < 4) {
                return chess::uci::parseSan(board, fen);
            }
            size_t len = fen.size();
            for (auto &chr : fen) {
                if (chr == '+' || chr == '#' || chr == '!' || chr == '?') {
                    len--;
                }
            }
            if (len > 4) {
                return chess::uci::parseSan(board, fen);
            }
            tfile = fen[2] - 'a';
            trank = fen[3] - '1';
        } else {
            size_t len = fen.size();
            for (auto &chr : fen) {
                if (chr == '+' || chr == '#' || chr == '!' || chr == '?') {
                    len--;
                }
            }
            if (len > 3) {
                return chess::uci::parseSan(board, fen);
            }
            tfile = fen[1] - 'a';
            trank = fen[2] - '1';
        }
        
        if (tfile < 0 || tfile > 7 || trank < 0 || trank > 7) {
            return chess::uci::parseSan(board, fen);
        }
        
        tpos = tfile + trank * 8;
        chess::PieceType pieceType = parseType(piece);
        
        if (pieceType == chess::PieceType::NONE) {
            return chess::uci::parseSan(board, fen);
        }

        auto piecebb = getPieces(board, pieceType, isWhite);

        int numResults = 0;
        if (piecebb.count() == 1) {
            spos = piecebb.pop();
            numResults++;
        }
        while (!piecebb.empty()) {
            int sq = piecebb.pop();
            if ((piece == 'N' && knightMatch(sq, tpos)) || (piece == 'B' && bishopMatch(sq, tpos)) ||
                (piece == 'R' && rookMatch(sq, tpos)) || (piece == 'Q' && queenMatch(sq, tpos)) ||
                (piece == 'K' && kingMatch(sq, tpos))) {

                numResults++;
                spos = sq;
            }
        }
        if (numResults == 1) {
            return chess::Move(tpos + (spos << 6));
        }
        return chess::uci::parseSan(board, fen);
    }
    return chess::uci::parseSan(board, fen);
}
