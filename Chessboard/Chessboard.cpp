#include "Chessboard.h"

inline void Board::applyUciMove(const std::string& moveStr) {
    // moveStr like "e2e4", "e7e8q" (promotion)
    int fromFile = moveStr[0] - 'a';
    int fromRank = moveStr[1] - '1';
    int toFile = moveStr[2] - 'a';
    int toRank = moveStr[3] - '1';
    int from = squareIndex(fromFile, fromRank);
    int to = squareIndex(toFile, toRank);
    char promo = (moveStr.size() > 4) ? moveStr[4] : '\0';

    // TODO: find which piece bitboard has `from` set, clear that bit,
    // clear any enemy piece bit at `to` (capture), set the piece bit at `to`.
    // Handle promo, castling, en passant as special cases.
    // Flip whiteToMove.

    updateOccupancies();
}
void Board::handlePositionCommand(std::istringstream& stream) {
    std::string token;
    stream >> token; // "startpos" or "fen"

    if (token == "startpos") {
        setStartPosition();
        stream >> token; // should be "moves" if present, or nothing
    }
    else if (token == "fen") {
        std::string fen;
        // FEN has 6 space-separated fields; collect them until we hit "moves" or run out
        while (stream >> token && token != "moves") {
            fen += token + " ";
        }
        setPositionFromFen(fen); // you'll write this — parse each FEN field
    }

    // At this point token is either "moves" or stream is exhausted
    if (token == "moves") {
        std::string moveStr;
        while (stream >> moveStr) {
            applyUciMove(moveStr);
        }
    }
}