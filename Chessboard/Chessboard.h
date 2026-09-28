#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <sstream>
#include <bit>
#include <iostream>
#include <array>


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
        return std::popcount(bb);
    }

    // Index of the least significant set bit (e.g. "first piece in this bitboard")
    inline int lsbIndex(uint64_t bb) {
        return std::countr_zero(bb);
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

enum rights {
    WK,WQ,BK,BQ
};
struct Board {
    uint64_t whitePawns, whiteKnights, whiteBishops, whiteRooks, whiteQueens, whiteKing;
    uint64_t blackPawns, blackKnights, blackBishops, blackRooks, blackQueens, blackKing;

    // Derived/cached — recompute after every move, hugely speeds up move gen
    uint64_t whiteOccupied, blackOccupied, allOccupied;

    bool whiteToMove;
    bool castleRights[4];
    int epSquare;
    int halfMoveClock;
    // castling rights, en passant square, halfmove clock, etc. come later




    inline void updateOccupancies() {
        whiteOccupied = whitePawns | whiteKnights | whiteBishops |
            whiteRooks | whiteQueens | whiteKing;
        blackOccupied = blackPawns | blackKnights | blackBishops |
            blackRooks | blackQueens | blackKing;
        allOccupied = whiteOccupied | blackOccupied;
    }

    inline void setStartPosition() {
        whitePawns = 0x000000000000FF00ULL; // rank 2
        whiteRooks = 0x0000000000000081ULL; // a1, h1
        whiteKnights = 0x0000000000000042ULL; // b1, g1
        whiteBishops = 0x0000000000000024ULL; // c1, f1
        whiteQueens = 0x0000000000000008ULL; // d1
        whiteKing = 0x0000000000000010ULL; // e1

        blackPawns = 0x00FF000000000000ULL; // rank 7
        blackRooks = 0x8100000000000000ULL; // a8, h8
        blackKnights = 0x4200000000000000ULL; // b8, g8
        blackBishops = 0x2400000000000000ULL; // c8, f8
        blackQueens = 0x0800000000000000ULL; // d8
        blackKing = 0x1000000000000000ULL; // e8

        whiteToMove = true;
        updateOccupancies();
    }

    void setPositionFromFen(const std::string& fen);
    inline void applyUciMove(const std::string& moveStr);
    void handlePositionCommand(std::istringstream& stream);
};

inline void beautyPrintBoard(const Board& board, int interpiece = 0) {
    std::array<char, 64> boardConstruct;
    boardConstruct.fill('-');

    auto addPiece = [&](const uint64_t& piece, char character) {
        uint64_t piece_copy = piece;
        while (piece_copy) {
            int sq = popLsb(piece_copy);
            boardConstruct[sq] = character;
        }
        };

    // --- White Pieces ---
    addPiece(board.whitePawns, 'P');
    addPiece(board.whiteKnights, 'N');
    addPiece(board.whiteBishops, 'B');
    addPiece(board.whiteRooks, 'R');
    addPiece(board.whiteQueens, 'Q');
    addPiece(board.whiteKing, 'K');

    // --- Black Pieces ---
    addPiece(board.blackPawns, 'p');
    addPiece(board.blackKnights, 'n');
    addPiece(board.blackBishops, 'b');
    addPiece(board.blackRooks, 'r');
    addPiece(board.blackQueens, 'q');
    addPiece(board.blackKing, 'k');
    
    std::cout << std::string(8 + interpiece * 7, '-') << std::endl;
    for (int r = 7; r >= 0; r--) {
        for (int f = 0; f < 8; f++) {
            int i = SQ::squareIndex(f, r);

            std::cout << boardConstruct[i];
            if (f < 7) {
                std::cout << std::string(interpiece, ' ');
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::string(8 + interpiece * 7, '-') << std::endl;
}


inline uint64_t knightAttacks[64];
inline uint64_t kingAttacks[64];
inline void initKnightAttacks() {
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