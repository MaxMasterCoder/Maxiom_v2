#include "Chessboard.h"
#include <cctype>

void Board::setPositionFromFen(const std::string& fen) {
    // Clear everything first
    whitePawns = whiteKnights = whiteBishops = whiteRooks = whiteQueens = whiteKing = 0ULL;
    blackPawns = blackKnights = blackBishops = blackRooks = blackQueens = blackKing = 0ULL;

    std::istringstream ss(fen);
    std::string placement, side = "w", castling = "-", enPassant = "-";
    int halfmove = 0, fullmove = 1;
    ss >> placement >> side >> castling >> enPassant >> halfmove >> fullmove;

    int rank = 7, file = 0;
    for (char c : placement) {
        if (c == '/') {
            rank--;
            file = 0;
        }
        else if (std::isdigit(static_cast<unsigned char>(c))) {
            file += c - '0';               // run of empty squares
        }
        else {
            if (!inBounds(file, rank)) break;   // malformed FEN guard
            int sq = squareIndex(file, rank);
            switch (c) {
            case 'P': setBit(whitePawns, sq);   break;
            case 'N': setBit(whiteKnights, sq); break;
            case 'B': setBit(whiteBishops, sq); break;
            case 'R': setBit(whiteRooks, sq);   break;
            case 'Q': setBit(whiteQueens, sq);  break;
            case 'K': setBit(whiteKing, sq);    break;
            case 'p': setBit(blackPawns, sq);   break;
            case 'n': setBit(blackKnights, sq); break;
            case 'b': setBit(blackBishops, sq); break;
            case 'r': setBit(blackRooks, sq);   break;
            case 'q': setBit(blackQueens, sq);  break;
            case 'k': setBit(blackKing, sq);    break;
            }
            file++;
        }
    }

    whiteToMove = (side == "w");

    // Castling / en passant / clocks are parsed above but not stored yet.
    // Once you add fields to Board, set them here, e.g.:
    castleRights[WK] = castling.find('K') != std::string::npos;
    castleRights[WQ] = castling.find('Q') != std::string::npos;
    castleRights[BK] = castling.find('k') != std::string::npos;
    castleRights[BQ] = castling.find('q') != std::string::npos;
    epSquare = (enPassant == "-") ? -1 : squareIndex(enPassant[0] - 'a', enPassant[1] - '1');
    halfMoveClock = halfmove;

    updateOccupancies();
}

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