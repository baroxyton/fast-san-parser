#include "chess.hpp"
#include "fast-san.hpp"
#include <fstream>
#include <sstream>

int main() {
  chess::Board board{
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};
  chess::Board startBoard{
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};
  std::string game;
  std::ifstream infile("./bench_games.txt");
  while (std::getline(infile, game)) {
    board = startBoard;
    std::istringstream gamestream{game};
    std::string san;
    while (getline(gamestream, san, ' ')) {
      board.makeMove(FastSAN::parseSan(board, san));

    }
  }
}
