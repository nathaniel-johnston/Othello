#include <iostream>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <ctime>
#include <string>
#include <sstream>
#include <ugpio/ugpio.h>
#include <fstream>
//THIS WAS OBTAINED FROM AN OMEGA FORUM AND IT IS NOT OUR CODE. FULL CREDIT GOES TO THE DEVELOPER
//http://community.onion.io/topic/2495/programming-serial-uart-in-c-or-c/8
//BY Maximilian Gerhardt
#include "UART.cpp"

#define HIGH 1
#define LOW 0

using namespace std;

const int GRID_SIZE = 8;
const int CORNER_X[] = {0,0,GRID_SIZE-1,GRID_SIZE-1};
const int CORNER_Y[] = {0,GRID_SIZE-1,0,GRID_SIZE-1};
const int P1_KEY = 1;
const int P2_KEY = -P1_KEY;
const int AI_KEY = P2_KEY;
const int HEIGHT = 4;
const int BOARD_CODE = 8;
const int MOVE_CODE = 9;
const int SCROLL_CODE = 88;
const int DELAY_TIME = 10*1000; //millis

struct Grid
{
    int cell[GRID_SIZE][GRID_SIZE];
    int playValue;
    int p1Amount;
    int p2Amount;
};

//system-independent functions
void initialiseGrid(Grid& board);
int incX(int direction);
int incY(int direction);
bool checkDirections(int cellX, int cellY, int direction, Grid& board);
bool check(int cellX, int cellY, bool toUpdate[GRID_SIZE][GRID_SIZE], Grid& board);
void update(bool newUpdate[GRID_SIZE][GRID_SIZE], Grid& board);
vector<int> checkAll(Grid& board);
int convertOneDimension(int x, int y);
bool skipTurn(Grid& board);
void convertCell(int x, int y, Grid& board);
int getNumCells(Grid& board);
int getNumCorners(Grid& board);
float minimax(Grid &board, int depth, bool isMaximizer, float alpha, float beta);
void copyBoard(Grid& board1, Grid& board2);
int convertToCoordinateY(int coordinate);
int convertToCoordinateX(int coordinate);
void makeMove(int move, Grid& board);
bool gameIsOver(Grid& board, int skipCounter);
float evaluate(Grid &board);
void makeAIMove(Grid &board);
float maxOf(float a, float b);
float minOf(float a, float b);

//omega stuff
void runOmega();
void sendBoard(Grid& board, vector<int> moves);
void sendCursor(int coordinate);

//log functions
void logGrid(Grid& board, vector<int> moves);
void log(string name);
void logMessage(string message);
void logBoardInfo(Grid& board, vector<int> moves);

//temp draw func
/*
void drawGrid(Grid& board, vector<int> moves);
void getMove(Grid& board);
void alternatePlayers(Grid& board);
*/

int bestIndex;
bool isEndgame = false;
ofstream logFile("logFile.txt");

void log(string name)
{
	time_t t = time(0);   // get time now
   	struct tm * now = localtime( & t );

	logFile << (now->tm_year + 1900) << "-"
        << (now->tm_mon + 1) << "-"
        <<  now->tm_mday << " "
        <<  now->tm_hour << ":"
        <<  now->tm_min << ":"
        <<  now->tm_sec
        << endl;
	logFile << "Entered " << name << "\n\n\n";
}

void logMessage(string message)
{
	time_t t = time(0);   // get time now
   	struct tm * now = localtime( & t );

	logFile << (now->tm_year + 1900) << "-"
        << (now->tm_mon + 1) << "-"
        <<  now->tm_mday << " "
        <<  now->tm_hour << ":"
        <<  now->tm_min << ":"
        <<  now->tm_sec
        << endl;
	logFile << name << "\n\n\n";
}

void logBoardInfo(Grid& board, vector<int> moves)
{
	time_t t = time(0);   // get time now
   	struct tm * now = localtime( & t );

	logFile << (now->tm_year + 1900) << "-"
        << (now->tm_mon + 1) << "-"
        <<  now->tm_mday << " "
        <<  now->tm_hour << ":"
        <<  now->tm_min << ":"
        <<  now->tm_sec
        << endl;

	logGrid(board, moves);

	logFile << "playValue: " << board.playValue
	<< endl << "p1Amount: "<< board.p1Amount
	<< endl << "p2Amount: "<< board.p2Amount
	<< "\n\n\n";
}

void logGrid(Grid& board, vector<int> moves)
{
    int moveIndex = 0;
    
    logFile << "  ";
    
    for(int i = 0; i < GRID_SIZE; i++)
    {
        logFile << (char)(i + 'A') << " ";
    }
    
    logFile << endl;
    
    for(int y = 0; y < GRID_SIZE; y++)
    {
        logFile << (y + 1) << " ";
        
        for(int x = 0; x < GRID_SIZE; x++)
        {
            if(board.cell[x][y] == P1_KEY)
                logFile << "W ";
            else if(board.cell[x][y] == P2_KEY)
                logFile << "B ";
            else if(moveIndex < moves.size() && moves.at(moveIndex) == convertOneDimension(x,y))
            {
                moveIndex++;
                logFile << "+ ";
            }
            else
                logFile << "- ";
        }
        logFile << endl;
    }
    
    logFile << endl << endl;
}

void runOmega()
{
	const int BACKWARD_PIN = 3;
	const int FORWARD_PIN = 0;
	const int SELECT_PIN = 2;
	
	Grid board;
	vector<int> moves;
	bool skipped = false;
	int select;
	int previousSelect;
	int back;
	int previousBack;
	int forward;
	int previousForward;
	int moveIndex = 0;
	int move;
	int skipCounter = 0;
	
	initialiseGrid(board);
	moves = checkAll(board);
	
	gpio_request(BACKWARD_PIN, NULL);
	gpio_request(FORWARD_PIN, NULL);
	gpio_request(SELECT_PIN, NULL);

	gpio_direction_input(BACKWARD_PIN);
	gpio_direction_input(FORWARD_PIN);
	gpio_direction_input(SELECT_PIN);
	
	if(!uart_open(portname, B9600, 0)) 
		return;

	logMessage("Game started");
	logBoardInfo(board, moves);

	sendBoard(board, moves);
	move = moves.at(0);
	move = 10*convertToCoordinateY(move) + convertToCoordinateX(move);
	sendCursor(move);
	
	while(!(gameIsOver(board, skipCounter)))
	{	
		select = gpio_get_value(SELECT_PIN);
		forward = gpio_get_value(FORWARD_PIN);
		back = gpio_get_value(BACKWARD_PIN);
		
		if(back == LOW && previousBack == HIGH)
		{
			moveIndex--;

			if(moveIndex < 0)
				moveIndex = moves.size() - 1;
			
			move = moves.at(moveIndex);
			move = 10*convertToCoordinateY(move) + convertToCoordinateX(move);
			
			sendCursor(move);
		}
			
		else if(forward == LOW && previousForward == HIGH)
		{
			moveIndex++;
		
			if(moveIndex >= moves.size())
			  moveIndex = 0;
			  
			move = moves.at(moveIndex);
			move = 10*convertToCoordinateY(move) + convertToCoordinateX(move);
			
			sendCursor(move);
		}
		
		else if (select == LOW && previousSelect == HIGH)
		{
			if(!skipTurn(board))
			{
				move = moves.at(moveIndex);
				convertCell(convertToCoordinateX(move), convertToCoordinateY(move), board);
				moves.clear();
			
				moves = checkAll(board);
				moveIndex = 0;

				sendBoard(board, moves);

				skipCounter = 0;

				logMessage("Human move made.");
				logBoardInfo(board, moves);
			}
			else
			{
				skipCounter++;
				logMessage("Human turn skipped");
			}

			if(!skipTurn(board))
			{
				usleep(2500*1000);
				makeAIMove(board);

				moves.clear();
				moves = checkAll(board);
				moveIndex = 0;

				sendBoard(board, moves);
				move = moves.at(0);
				move = 10*convertToCoordinateY(move) + convertToCoordinateX(move);
				sendCursor(move);

				skipCounter = 0;

				logMessage("AI move made");
				logBoardInfo(board, moves);
			}
			else
			{
				skipCounter++;
				logMessage("AI turn skipped");
			}		
		}
		
		previousForward = forward;
		previousBack = back;
		previousSelect = select;
	} // end of while(true)

	logMessage("Game over");
	logFile.close();
}

void sendBoard(Grid& board, vector<int> moves)
{
	log("void sendBoard(Grid& board, vector<int> moves)");


	char data = (char)BOARD_CODE;
	char* dataPtr = &data;

	uart_write(dataPtr, 1);
	usleep(DELAY_TIME);

	for(int y = 0; y < GRID_SIZE; y++)
	{
		for(int x = 0; x < GRID_SIZE; x++)
		{
			data = board.cell[x][y];
      
			if(data == P2_KEY)
				data = 2;

			uart_write(dataPtr, 1);
			usleep(DELAY_TIME);
		}
	}
	
	for(int i = 0; i < moves.size(); i++)
	{
		data = (char)(10*convertToCoordinateY(moves.at(i)) + convertToCoordinateX(moves.at(i)));

		uart_write(dataPtr, 1);
		usleep(DELAY_TIME);
	}
}

void sendCursor(int coordinate)
{
	log("void sendCursor(int coordinate)");

	char data = (char)SCROLL_CODE;

	char* dataPtr = &data;
	uart_write(dataPtr, 1);

	usleep(DELAY_TIME);

	data = (char)coordinate;
	uart_write(dataPtr, 1);
}

void initialiseGrid(Grid& board)
{
    log("void initialiseGrid(Grid& board)");

    //2 tiles each, for p1 and p2
    board.p1Amount = 2;
    board.p2Amount = 2;
    board.playValue = P1_KEY;
    
    //initalizing a board of blank tiles
    for(int y = 0; y < GRID_SIZE; y++)
    {
        for(int x = 0; x < GRID_SIZE; x++)
        {
            board.cell[x][y] = 0;
        }
    }
    
    //setting up board with appropriate player tiles arranged in centre
    board.cell[GRID_SIZE/2][GRID_SIZE/2] = P1_KEY;
    board.cell[GRID_SIZE/2 - 1][GRID_SIZE/2 - 1] = P1_KEY;
    board.cell[GRID_SIZE/2 - 1][GRID_SIZE/2] = P2_KEY;
    board.cell[GRID_SIZE/2][GRID_SIZE/2 - 1] = P2_KEY;
}

//temp draw func
void drawGrid(Grid& board, vector<int> moves)
{
    int moveIndex = 0;
    
    cout << "  ";
    
    for(int i = 0; i < GRID_SIZE; i++)
    {
        cout << (char)(i + 'A') << " ";
    }
    
    cout << endl;
    
    for(int y = 0; y < GRID_SIZE; y++)
    {
        cout << (y + 1) << " ";
        
        for(int x = 0; x < GRID_SIZE; x++)
        {
            if(board.cell[x][y] == P1_KEY)
                cout << "W ";
            else if(board.cell[x][y] == P2_KEY)
                cout << "B ";
            else if(moveIndex < moves.size() && moves.at(moveIndex) == convertOneDimension(x,y))
            {
                moveIndex++;
                cout << "+ ";
            }
            else
                cout << "- ";
        }
        cout << endl;a
    }
    
    cout << endl << endl;
}

void getMove(Grid& board)
{
    string name;
    int y;
    int x;
    
    if(board.playValue == P1_KEY)
    {
        
        cout << "Move: ";
        
        cin >> name;
        
        x = name[0] - 'a';

        y = name[1] - '1';
        
        convertCell(x,y,board);
    }
}

void alternatePlayers(Grid& board)
{	
    bool skipped = false;
    int skipCounter = 0;

    while(true)
    {
    	
    	skipTurn(board);
    	
        drawGrid(board, checkAll(board));
        
        getMove(board);
        
        drawGrid(board, checkAll(board));
        
        skipped = skipTurn(board);
        
        makeAIMove(board);
    }
}

bool gameIsOver(Grid& board, int skipCounter)
{
    if(board.p1Amount + board.p2Amount == GRID_SIZE*GRID_SIZE)
        return true;
    
    if(skipCounter >= 2)
	return true;
    
    return false;
}

int incX(int direction)
{
    int incX[] = {0,1,1,1,0,-1,-1,-1};    //increment X by certain value
    
    //x values arranged like:
    
    //    -1    0    1
    //    -1    X    1
    //    -1    0    1
    
    //incX directional indices are:
    
    //    7    0    1
    //    6         2
    //    5    4    3
    
    //so going in the specified direction will change the X-Coordinate accordingly,
    //i.e. returns appropriate X increment of given direction
    
    return incX[direction];
}

int incY(int direction)
{
    int incY[] = {1,1,0,-1,-1,-1,0,1};    //increment Y by certain value
    
    //y values arranged like:
    
    //     1     1     1
    //     0     Y     0
    //    -1    -1    -1
    
    //incY directional indices are:
    
    //    7    0    1
    //    6         2
    //    5    4    3
    
    //so going in the specified direction will change the Y-Coordinate accordingly,
    //i.e. returns appropriate Y increment of given direction
    
    return incY[direction];
}

bool checkDirections(int cellX, int cellY, int direction, Grid& board)
{
    int cellsMoved = 0;    
    
    while(true)
    {
        cellX += incX(direction);
        cellY += incY(direction);
        
        if(cellX < 0 || cellX > 7 || cellY < 0 || cellY > 7)
        {
            return false;
        }
        
        
        cellsMoved++;
        
        if((board.cell[cellX][cellY] == board.playValue) && (cellsMoved > 1))
            return true;
        else if(board.cell[cellX][cellY] == 0)
            return false;
        else if((board.cell[cellX][cellY] == board.playValue) && (cellsMoved == 1))
            return false;
        
    }
}

bool check(int cellX, int cellY, bool toUpdate[GRID_SIZE][GRID_SIZE], Grid& board)
{
    int originalX = cellX;
    int originalY = cellY;
    bool worked = false;
    
    for(int y = 0; y < GRID_SIZE; y++)
    {
        for(int x = 0; x < GRID_SIZE; x++)
        {
            toUpdate[x][y] = false;
        }
    }
    
    for(int i = 0; i < GRID_SIZE; i++)
    {
        cellX = originalX;
        cellY = originalY;
        
        if(checkDirections(originalX, originalY, i, board))
        {
            
            cellX += incX(i);
            cellY += incY(i);
            
            while(board.cell[cellX][cellY] == board.playValue * -1)
            {
                toUpdate[cellX][cellY] = true;
                
                cellX += incX(i);
                cellY += incY(i);
            }
            
            worked = true;
        }
    }
    
    return worked;
}

void update(bool newUpdate[GRID_SIZE][GRID_SIZE], Grid& board)
{
    log("void update(bool newUpdate[GRID_SIZE][GRID_SIZE], Grid& board)");

    int amountChanged = 0;
    
    for (int y = 0; y < GRID_SIZE; y++)
    {
        for (int x = 0; x < GRID_SIZE; x++)
        {
            if (newUpdate[x][y])
            {
                board.cell[x][y] = board.playValue;
                amountChanged++;
            }
        }
    }
    
    if (board.playValue == P1_KEY)
    {
        
        board.p1Amount = (board.p1Amount + amountChanged + 1);
        board.p2Amount -= amountChanged;
    }
    else
    {
        board.p2Amount = (board.p2Amount + amountChanged + 1);
        board.p1Amount -= amountChanged;
    }
}

int getNumCells(Grid& board)
{
    if(board.playValue == P1_KEY)
        return board.p1Amount;
    
    return board.p2Amount;
}

vector<int> checkAll(Grid& board)
{
    log("vector<int> checkAll(Grid& board)");

    vector<int> validMove;
    bool checkValid[GRID_SIZE][GRID_SIZE];
    
    for(int y = 0; y < GRID_SIZE; y++)
    {
        for(int x = 0; x < GRID_SIZE; x++)
        {
            if(board.cell[x][y] == 0)
            {
                bool worked = check(x, y, checkValid, board);
                
                if(worked)
                {
                    validMove.push_back(convertOneDimension(x, y));
                }
            }
        }
    }
    
    return validMove;
}

int convertOneDimension(int x, int y)
{
    return GRID_SIZE * y + x;
}

bool skipTurn(Grid& board)
{
    if(checkAll(board).size() == 0)
    {
        cout << "No moves available. Skip turn." << endl;
        
        board.playValue *= -1;
        
        return true;
    }
    
    return false;
}

void convertCell(int x, int y, Grid& board)
{
    bool newUpdate[GRID_SIZE][GRID_SIZE];
    bool worked = check(x, y, newUpdate, board);
    
    if(worked)
    {
        board.cell[x][y] = board.playValue;
        update(newUpdate, board);
        
        board.playValue = -board.playValue;
    }
}

void makeMove(int move, Grid& board)
{
    convertCell(convertToCoordinateX(move), convertToCoordinateY(move), board);
}

int convertToCoordinateX(int coordinate)
{
    return coordinate % GRID_SIZE;
}

int convertToCoordinateY(int coordinate)
{
    return coordinate / GRID_SIZE;
}


void copyCells(Grid& board1, Grid& board2)
{
    board2.playValue = board1.playValue;
    board2.p1Amount = board1.p1Amount;
    board2.p2Amount = board2.p2Amount;
    
    for(int y = 0; y < GRID_SIZE; y++)
    {
        for(int x = 0; x < GRID_SIZE; x++)
        {
            board2.cell[x][y] = board1.cell[x][y];
        }
    }
}


//COMPUTER PLAYER METHODS

int getNumCorners(Grid& board)
{
    int numCorners = 0;
    
    for(int i = 0; i < 4; i++)
    {
        if(board.cell[CORNER_X[i]][CORNER_Y[i]] == board.playValue)
            numCorners++;
    }
    
    return numCorners;
}

int main()
{
  if(!logFile.is_open())
		cout << "Could not open" << endl;

    
  runOmega();
    
  return 0;
}

float minimax(Grid &board, int depth, bool isMaximizer, float alpha, float beta){
    
    vector <int> legalMoves = checkAll(board);
    int numChildren = (int)legalMoves.size();
    
    if (depth == 0 || numChildren == 0){
        return evaluate(board);
    }
    
    if(isMaximizer){
        for (int i = 0; i < numChildren; i++){
            
            
            //creating a newBoard with the same board state as board
            //doing this instead of passing by value to minimize
            //load on the stack
            
            Grid newBoard;
            copyCells(board, newBoard);
            makeMove(legalMoves[i], newBoard);
            
            float temp = minimax(newBoard, depth - 1, false, alpha, beta);
            
            if (depth == HEIGHT){
                if (temp > alpha){
                    bestIndex = i;
                }
            }
            
            alpha = maxOf(alpha, temp);
            
            if (beta <= alpha){
                return beta;
            }
        }
        return alpha;
    }
    else{
        for (int i = 0; i < numChildren; i++){
            
            //creating a newBoard with the same board state as board
            //doing this instead of passing by value to minimize
            //load on the stack
            
            Grid newBoard;
            copyCells(board, newBoard);
            makeMove(legalMoves[i], newBoard);
            
            float temp = minimax(newBoard, depth - 1, true, alpha, beta);
            
            beta = minOf(beta, temp);
            
            if (beta <= alpha){
                return alpha;
            }
        }
        return beta;
    }
    
}

//evaluates the current board based on 3 factors:

//    1. Corners Occupied: (100)
//         -Each corner has a weight of 100. Corners are very valuable
//         in Othello because they cannot be reversed.
//
//    2. Mobility: (1)
//        -The number of legal moves the player can make is the main
//         evaluation method. Each legal move the playter can make
//         has a weight of 11.
//
//    3. Disc Count: (1/100)
//        -Discs aren't a very useful factor to determine the strength
//         of a player's position, as discs can easily be flipped later
//         in the game. Each disc has a weight of 1/100.

float evaluate(Grid &board)
{
    const float CORNER_WEIGHT = 100;
    const float MOBILITY_WEIGHT = 1;
    const float DISC_COUNT_WEIGHT = 1/100;
    
    float score = 0;
    float oppScore = 0;
    
    
    int playerKey = board.playValue;
    
    //we only evaluate the score of the board from the AI's perspective
    board.playValue = AI_KEY;
    
    score += CORNER_WEIGHT * getNumCorners(board);
    score += DISC_COUNT_WEIGHT * getNumCells(board);
    
//    score -= AROUND_CORNER_WEIGHT * getNumAroundCorners(board);
    
    int numLegalMoves = (int)checkAll(board).size();
    
    if (!isEndgame){
        if(playerKey == AI_KEY){ //if it's ai's turn to play, add its mobility to AI board score
            score += MOBILITY_WEIGHT * numLegalMoves;
        }
        else{ //if it's opponent's turn to play, subtract opponent's mobility from AI board score
            board.playValue = P1_KEY;
            numLegalMoves = (int)checkAll(board).size();
            score -= MOBILITY_WEIGHT * numLegalMoves;
        }
    }
    
    board.playValue = P1_KEY;
    
    oppScore += CORNER_WEIGHT * getNumCorners(board);
    oppScore += DISC_COUNT_WEIGHT * getNumCells(board);
    
    score = score - oppScore;
    
    board.playValue = playerKey;
    
    return score;
}

void makeAIMove(Grid &board)
{
    if (board.playValue == AI_KEY){
        
        if(GRID_SIZE*GRID_SIZE - board.p1Amount - board.p2Amount <= 14)
            isEndgame = true;
        
        minimax(board, HEIGHT, true, -1000, 1000);
        
        makeMove(checkAll(board).at(bestIndex), board);
    }
}

float maxOf(float a, float b)
{
    if (a > b)
        return a;
    
    return b;
}

float minOf(float a, float b)
{
    if (a > b)
        return b;
    
    return a;
}

