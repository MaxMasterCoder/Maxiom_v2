#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <sstream>

#pragma region SQ
enum class Square {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8
};

namespace SQ {
    inline int squareIndex(int file, int rank) { return rank * 8 + file; }
    inline int fileOf(int sq) { return sq & 7; }
    inline int rankOf(int sq) { return sq >> 3; }

    inline bool inBounds(int file, int rank) {
        return (file >= 0 and file < 8 and rank >= 0 and rank < 8);
    }
}
#pragma endregion
using namespace SQ;

#pragma region BitOp
namespace BitOp {
    inline void setBit(uint64_t& bb, int sq) { bb |= (1ULL << sq); }
    inline void clearBit(uint64_t& bb, int sq) { bb &= ~(1ULL << sq); }
    inline bool getBit(uint64_t bb, int sq) { return (bb >> sq) & 1ULL; }

    // Count how many bits are set (how many pieces)
    inline int popCount(uint64_t bb) {
        return __builtin_popcountll(bb); // GCC/Clang intrinsic, very fast
    }

    // Index of the least significant set bit (e.g. "first piece in this bitboard")
    inline int lsbIndex(uint64_t bb) {
        return __builtin_ctzll(bb); // count trailing zeros
    }

    // Pop (extract and clear) the LSB — the classic bitboard iteration pattern
    inline int popLsb(uint64_t& bb) {
        int sq = lsbIndex(bb);
        bb &= bb - 1;   // clears the lowest set bit — a very common trick
        return sq;
    }
}
#pragma endregion
using namespace BitOp;


struct Board {
    uint64_t whitePawns, whiteKnights, whiteBishops, whiteRooks, whiteQueens, whiteKing;
    uint64_t blackPawns, blackKnights, blackBishops, blackRooks, blackQueens, blackKing;

    // Derived/cached — recompute after every move, hugely speeds up move gen
    uint64_t whiteOccupied, blackOccupied, allOccupied;

    bool whiteToMove;
    // castling rights, en passant square, halfmove clock, etc. come later




    inline void updateOccupancies();

    inline void setStartPosition();

    void setPositionFromFen(const std::string& fen);
    inline void applyUciMove(const std::string& moveStr) {
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
    void handlePositionCommand(std::istringstream& stream) {
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
};



uint64_t knightAttacks[64];
uint64_t kingAttacks[64];
void initKnightAttacks() {
    for (int sq = 0; sq < 64; sq++) {
        uint64_t attacks = 0ULL;
        int f = fileOf(sq), r = rankOf(sq);

        int deltas[8][2] = { {1,2},{2,1},{2,-1},{1,-2},{-1,-2},{-2,-1},{-2,1},{-1,2} };
        for (auto& d : deltas) {
            int nf = f + d[0], nr = r + d[1];
            if (inBounds(nf, nr)) {
                setBit(attacks, squareIndex(nf, nr));
            }
        }
        knightAttacks[sq] = attacks;
    }
}