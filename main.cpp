#include <iostream>
#include <string>
#include <sstream>
#include "Chessboard/Chessboard.h"

enum MODE {
    DEBUG,
    ENGINE
};
constexpr MODE mode = DEBUG;

int main() {
    std::cout << "STARTING MAXIOM_V2\n";
    if (mode == DEBUG) {
        std::cout << "STARTING MAXIOM_V2\n";
        Board board;
        board.setStartPosition();
        beautyPrintBoard(board, 1);

        std::cout << "Press Enter to exit...";
        std::cin.get();
    }
    else if (mode == ENGINE) {
        // Disable output buffering so Arena receives messages instantly
        std::setvbuf(stdout, NULL, _IONBF, 0);


        Board board;

        std::string line;
        while (std::getline(std::cin, line)) {
            std::istringstream stream(line);
            std::string command;
            stream >> command;

            if (command == "uci") {
                // Tell Arena who you are
                std::cout << "id name Maxiom v2.0\n";
                std::cout << "id author Max Rodriguez\n";
                std::cout << "uciok\n";
            }
            else if (command == "isready") {
                // Tell Arena you are ready to receive positions
                std::cout << "readyok\n";
            }
            else if (command == "position") {
                board.handlePositionCommand(stream);
            }
            else if (command == "go") {
                // Arena wants a move! 
                // TODO: Run your search algorithm here.

                // For testing, we just hardcode a starting move
                std::cout << "bestmove e7e5\n";
            }
            else if (command == "quit") {
                break;
            }
        }
    }
    return 0;
}
