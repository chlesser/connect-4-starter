#include "Connect4.h"
#include <string>
#include <iostream>


Connect4::Connect4()
{
    _grid = new Grid(7, 6);
}

Connect4::~Connect4()
{
    delete _grid;
}

//
// make an X or an O
//
Bit* Connect4::PieceForPlayer(const int playerNumber)
{
    // depending on playerNumber load the "x.png" or the "o.png" graphic
    Bit *bit = new Bit();
    // should possibly be cached from player class?
    bit->LoadTextureFromFile(playerNumber == AI_PLAYER ? "red.png" : "yellow.png");
    bit->setOwner(getPlayerAt(playerNumber == AI_PLAYER ? 1 : 0));
    return bit;
}

void Connect4::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 7;
    _gameOptions.rowY = 6;
    _grid->initializeSquares(80, "square.png");


    if (gameHasAI()) {
        setAIPlayer(AI_PLAYER);
        std::cout << "we have AI" << std::endl;
    }

    startGame();
}


bool Connect4::actionForEmptyHolder(BitHolder &holder)
{
    if (holder.bit()) {
        return false;
    }
    ChessSquare* clickedSquare = dynamic_cast<ChessSquare*>(&holder);
    ChessSquare *below = _grid->getS(clickedSquare->getColumn(), clickedSquare->getRow());
    if (below != nullptr) //if we are not on the bottom...
        if(below->bit() == nullptr) //and we dont have a bit below us...
            return false; //we cannot place. This can be changed to placing at the bottom after I get victory working.
    Bit *bit = PieceForPlayer(getCurrentPlayer()->playerNumber() == 0 ? HUMAN_PLAYER : AI_PLAYER);
    if (bit) {
        bit->setPosition(holder.getPosition());
        holder.setBit(bit);
        endTurn();
        return true;
    }   
    return false;
}

bool Connect4::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    // you can't move anything in connect4
    return false;
}

bool Connect4::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    // you can't move anything in connect4
    return false;
}

//
// free all the memory used by the game on the heap
//
void Connect4::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

//
// helper function for the winner check
//
Player* Connect4::ownerAt(int x, int y) const
{
    auto square = _grid->getSquare(x, y);
    if (!square || !square->bit()) {
        return nullptr;
    }
    return square->bit()->getOwner();
}

Player* Connect4::checkForWinner()
{
    static const int winningQuarts[10][4] =  { {0,1,2,3}, {4,5,6,7}, {8,9,10,11}, {12,13,14,15},  // rows
                                                {0,5,9,13}, {1,5,9,13}, {2,6,10,14}, {3,7,11,15},  // cols
                                                {0,5,10,15}, {12,9,6,3} };         // diagonals
    //this uses the victory checking method by graeme devine.
    //noticably, macroY counts down. This is to ensure that victories on the bottom are checked for first.
    for(int macroY = 3; macroY > 0; macroY--) {
        for(int macroX = 0; macroX < 4; macroX++) {
            //now that the 12 4x4 grids are found, we can check individually.
            for( int i=0; i<10; i++ ) {
                const int *quart = winningQuarts[i];
                Player *player = ownerAt((quart[0] % 4) + macroX, (quart[0] / 4) + macroY);
                if( player && 
                    player == ownerAt((quart[1] % 4) + macroX, (quart[1] / 4) + macroY) && 
                    player == ownerAt((quart[2] % 4) + macroX, (quart[2] / 4) + macroY) && 
                    player == ownerAt((quart[3] % 4) + macroX, (quart[3] / 4) + macroY))
                    return player;
            }
        }
    }
    return nullptr;
}

bool Connect4::checkForDraw()
{
    bool isDraw = true;
    // check to see if the board is full
    _grid->forEachSquare([&isDraw](ChessSquare* square, int x, int y) {
        if (!square->bit()) {
            isDraw = false;
        }
    });
    return isDraw;
}

//
// state strings
//
std::string Connect4::initialStateString()
{
    return  "0000000"
            "0000000"
            "0000000"
            "0000000"
            "0000000"
            "0000000";
}

//
// this still needs to be tied into imguis init and shutdown
// we will read the state string and store it in each turn object
//
std::string Connect4::stateString()
{
    std::string s = initialStateString();
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        Bit *bit = square->bit();
        if (bit) {
            s[y * 7 + x] = std::to_string(bit->getOwner()->playerNumber()+1)[0];
        }
    });
    return s;
}

//
// this still needs to be tied into imguis init and shutdown
// when the program starts it will load the current game from the imgui ini file and set the game state to the last saved state
//
void Connect4::setStateString(const std::string &s)
{
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        int index = y*7 + x;
        int playerNumber = s[index] - '0';
        if (playerNumber) {
            square->setBit( PieceForPlayer(playerNumber-1) );
        } else {
            square->setBit( nullptr );
        }
    });
}


//
// this is the function that will be called by the AI
//
void Connect4::updateAI() 
{
    int bestVal = -1000;
    BitHolder* bestMove = nullptr;
    std::string state = stateString();

    // Traverse all cells, evaluate minimax function for all empty cells
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        int index = y * 7 + x;
        //we add a few more parameters here. First, we only continue if the index is in the final row, or if the next down is another piece.
        if((index + 7 > 42 || state[index + 7] == '0') && state[index] == '0') {
            // Make the move
            state[index] = '2';
            int moveVal = -negamax(state, 0, HUMAN_PLAYER);
            // Undo the move
            state[index] = '0';
            // If the value of the current move is more than the best value, update best
            if (moveVal > bestVal) {
                bestMove = square;
                bestVal = moveVal;
            }
        }
    });


    // Make the best move
    if(bestMove) {
        if (actionForEmptyHolder(*bestMove)) {
        }
    }
}

bool Connect4::isAIBoardFull(const std::string& state) {
    return state.find('0') == std::string::npos;
}

int Connect4::evaluateAIBoard(const std::string& state) {
    return 0;
}
int scoreRow(const std::string& state, int index) {}
int scoreColumn(const std::string& state, int index) {}
int scoreDiag(const std::string& state, int index) {}

//
// player is the current player's number (AI or human)
//
int Connect4::negamax(std::string& state, int depth, int playerColor) 
{
    // Check if AI wins, human wins, or draw
    if(depth == 3) { 
        int score = evaluateAIBoard(state);
        // A winning state is a loss for the player whose turn it is.
        // The previous player made the winning move.
        return -score; 
    }

    if(isAIBoardFull(state)) {
        return 0; // Draw
    }

    int bestVal = -1000; // Min value
    int columnModification[] = { 3,4,2,5,1,6,0 };
    for(int i=0; i<7; i++) {
        int column = columnModification[i];
            // Check if cell is empty
            int index = column;
            while(index < 42 && state[index] == '0')
                index += 7;
            index -= 7;
            if(index > 0) {
                state[index] = playerColor == HUMAN_PLAYER ? '1' : '2'; // Set the cell to the current player's color
                bestVal = std::max(bestVal, -negamax(state, depth + 1, -playerColor));
                // Undo the move for backtracking
                state[index] = '0';
            }
    }

    return bestVal;
}