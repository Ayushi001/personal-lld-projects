/******************************************************************************
 Tic Tac Toe, known colloquially as "Xs and Os," is a two-player game typically played on a 3x3 grid.
 The objective is simple: be the first to form a horizontal, vertical, or diagonal line of three of your marks (either "X" or "O").
 The elegance of the game lies in its deceptive complexity, while the rules are straightforward,
 devising an unbeatable strategy demands a keen understanding of the game's dynamics.

Rules of the game :
Firstly let's understand the rules of the game:

• Setup: The game is played on a 3 * 3 grid. One player uses 'X' another player uses 'O' and each player takes turns making their moves.

• Winner: The game is won by the player placing his or her symbol in a row, column, or diagonal.
The first player to get three symbols in a row wins the game. When the player reaches this, the game ends immediately.

• Draw: If all the grid cells are filled and no player has three symbols in a row, the game will be a tie or a draw.

• Illegal Moves: A player cannot place his or her symbol on a tile occupied by an opponent's symbol or their own symbol.
The move must be made to an empty cell.
*******************************************************************************/

/*
Entities:
Board: vector<vector<char>> board, stack<Move> history
  - void makeMove(Move m)
  - Move undoMove() // pops the last state from history
  - void redoMove(Move m)
  - bool hasWon(Move m) : returns true if the player who made
  - bool isDraw() : returns true if all board is filled and no one has won
  - bool isValidMod(Move m)
Move: Player p, int row, int column : Mememto of Mememto pattern
Player: id, symbol
GameController: Board board, vector<Player*> players, int currPlayerIndex,
   - startGame() : runs the game simulation till it ends
   - resetGame(): clears the board and starts new game

Interface GameState:
   - void next(GameStateContext context,isDraw,isWon)
   - bool isGameOver()  : returns true if currentState is a GameOver state: DrawState/WonState

class UnstartedState: public GameState
     void next(GameStateContext context,isDraw, hasWon)
     {
         context.setState( new InprogressState())
     }

class InprogressState: public GameState{
    void next(GameStateContext context, isDraw, hasWon)
    {
        if (isWon)
        {
            context.setState(new WonState())
        }
    }
}
class DrawState: public GameState
class WonState: public GameState

// StateHandler which is going to delegate state specific behavior to state's next
GameStateContext: GameState currentState
   void setState(GameState nextState)
   {
       currentState=nextState
   }
   void next(bool isDraw, bool isWon)
   {
       currentState.next(this, isDraw, isWon )
   }
   bool isGameOver()
   {
       return currentState.isGameOver()
   }
   GameState getCurrentState()

Design Patterns:
- Memento Pattern
- State pattern to get game state: UNSTARTED, INPROGRESS, DRAW, WON
- can apply observer pattern at the end to notify all players (which act as obersers/listeners)
whenever a game state has changed
*/
#include <bits/stdc++.h>
using namespace std;

/*
Move: Player p, int row, int column : Mememto of Mememto pattern
Player: id, symbol
*/
class PlayerStrategy;
class Player {
	PlayerStrategy* strategy;
public:
	int id;
	char symbol;
	Player(int id, char symbol, PlayerStrategy* strategy):id(id),symbol(symbol),strategy(strategy) {
	}
	PlayerStrategy* getStrategy()
	{
		return strategy;
	}
};
class Move {
public:
	Player* player;
	int row, column;
	Move(Player* player,int row, int col): player(player), row(row), column(col) {

	}
};
//Strategy design pattern
class PlayerStrategy {
public:
	// creates moves for the given player
	virtual Move* createMove(Player* player, int boardSize)=0;
};
class HumanPlayerStrategy: public PlayerStrategy {
public:
	Move* createMove(Player* player, int boardSize)
	{
		int r,c;
		cout<<"Enter row for human player: "<<player->id<<'\n';
		cin>>r;
		cout<<"Enter column for human player: "<<player->id<<'\n';
		cin>>c;
		Move* move=new Move(player,r,c);
		return move;
	}
};
class RandomPlayerStrategy: public PlayerStrategy {
public:
	Move* createMove(Player* player,int boardSize)
	{
		int r=rand()%boardSize;
		int c=rand()%boardSize;
		return new Move(player,r,c);
	}
};
// Implementing State pattern
class GameState; // forward declaration
class GameStateContext {
	GameState* currentState;
public:
	// forward declaration only, defined below
	GameStateContext();
	// 	{
	// 		currentState=new UnstartedState();
	// 	}
	void setState(GameState*nextState) {
		currentState=nextState;
	}
	GameState* getState()
	{
		return currentState;
	}
};
// List of all possible states
class GameState {
public:
	virtual void next(GameStateContext* ctx, bool isDraw=false, bool hasWon=false )=0;
	virtual bool isGameOver()=0;
	virtual string getStateText()=0;
};
class WonState:public GameState {
public:
	WonState() {}
	void next(GameStateContext *ctx, bool isDraw, bool hasWon) override {
		// no next state
	}
	bool isGameOver() override {
		return true;
	}
	string getStateText()
	{
		return "WonState";
	}
};
class DrawState: public GameState {
public:
	DrawState() {}
	void next(GameStateContext*ctx, bool isDraw, bool hasWon) override {
		// no next state
	}
	bool isGameOver() override {
		return true;
	}
	string getStateText()
	{
		return "DrawState";
	}
};
class InprogressState: public GameState {
public:
	InprogressState() {}
	void next(GameStateContext *ctx, bool isDraw, bool hasWon) override {
		if(isDraw)
			ctx->setState(new DrawState());
		else if(hasWon)
			ctx->setState(new WonState());
	}
	bool isGameOver() override {
		return false;
	}
	string getStateText()
	{
		return "InprogressState";
	}
};
class UnstartedState: public GameState {
public:
	UnstartedState() {}
	void next(GameStateContext * ctx, bool isDraw,bool hasWon) override
	{
		ctx->setState(new InprogressState());
	}
	bool isGameOver()override {
		return false;
	}
	string getStateText()
	{
		return "UnstartedState";
	}
};
GameStateContext::GameStateContext() {
	currentState=new UnstartedState();
}
//Observer pattern for all listeners of board event changes
class GameObserver {
public:
	virtual void notifyMove(vector<vector<char>> &board)=0;
	virtual void onStateChange(GameState* gameState)=0;
};
class ConsoleObserver:public GameObserver {
public:
	void notifyMove(vector<vector<char>> &board)
	{
		cout<<"\nMOVE MADE, current board status:\n";
		// board->displayBoard();
		for(int i=0; i<board.size(); i++)
		{
			cout<<'\n';
			for(int j=0; j<board[0].size(); j++)
			{
				cout<<board[i][j]<<' ';
			}
		}
	}
	void onStateChange(GameState* gameState)
	{
		cout<<"\nSTATE CHANGED, current state = "<<gameState->getStateText();
	}
};
/*
Board: vector<vector<char>> board, stack<Move> history
  - void makeMove(Move m)
  - Move undoMove() // pops the last state from history
  - void redoMove(Move m)
  - bool hasWon(Move m) : returns true if the player who made
  - bool isDraw() : returns true if all board is filled and no one has won
  - bool isValidMod(Move m)
*/
class Board {
	vector<vector<char>> board;
	int size;
	stack<Move*> history;
	GameStateContext *ctx;
	GameObserver* gameObserver;
	bool isMoveValid(Move* m)
	{
		if(m->row>=size||m->row<0||m->column>=size||m->row<0)
			return false;
		if(board[m->row][m->column]!='\0')
			return false;
		return true;
	}
	void saveMove(Move* m)
	{
		history.push(m);
	}
	bool hasWon(Move *m)
	{
		char symbol=m->player->symbol;
		bool isWon=true;
		// row check
		for(int j=0; j<size; j++)
		{
			if(board[m->row][j]!=symbol)
			{
				isWon=false;
				break;
			}
		}
		if(isWon)return true;

		isWon=true;
		// column check
		for(int i=0; i<size; i++)
		{
			if(board[i][m->column]!=symbol)
			{
				isWon=false;
				break;
			}
		}
		if(isWon)return true;

		/*
		  0 1 2 3
		0
		1
		2
		3
		*/
		// diagonal check
		if(m->row!=m->column || m->row+m->column!=size-1)
			return false; // the placed symbol is not on either of diagnols
		isWon=true;
		for(int i=0,j=0; i<size&&j<size; i++,j++)
		{
			if(board[i][j]!=symbol)
			{
				isWon=false;
				break;
			}
		}
		if(isWon)
			return true;

		// anti diagnol check
		for(int i=0,j=size-1; i<size&&j>=0; i++,j--)
		{
			if(board[i][j]!=symbol)
			{
				return false;
			}
		}
		return true;
	}
	bool isDraw(Move *m) {
		if(history.size()==size*size && !hasWon(m))
			return true;
		return false;
	}
public:
	Board(int n, GameObserver* gameObserver): size(n), gameObserver(gameObserver)
	{
		// initialise an n*n Board
		board.assign(n,vector<char>(n));
		for(int i=0; i<n; i++)
			for(int j=0; j<n; j++)
			{
				board[i][j]='\0';
			}
		// wrong practice ideally should be passed from constructor params so it can be mocked in unit tests
		// gameObserver= new ConsoleObserver();
	}
	bool makeMove(Move* m)
	{
		if(!isMoveValid(m))
		{
			cout<<"ERROR: Invalid Move\n";
			return false;
		}
		board[m->row][m->column]=m->player->symbol;
		saveMove(m);
		gameObserver->notifyMove(board);
		if(hasWon(m))
		{
			// change game state and notify observers which player id won
			ctx->getState()->next(ctx,false,true);
			gameObserver->onStateChange(ctx->getState());
			// cout<<"GAME WON";
			return true;
		}
		if(isDraw(m))
		{
			// change game state and notify observers
			ctx->getState()->next(ctx,true,false);
			// 	cout<<"GAME Draw";
			gameObserver->onStateChange(ctx->getState());
			return true;
		}
		return true;
	}
	void undoMove() {
		Move* lastMove=history.top();
		history.pop();
		board[lastMove->row][lastMove->column]='\0';
	}
	void setGameStateContext(GameStateContext* ctx)
	{
		this->ctx=ctx;
	}
	void resetBoard() {
		while(!history.empty())
		{
			Move* move=history.top();
			board[move->row][move->column]='\0';
			history.pop();
		}
	}
	int getSize() {
		return size;
	}
};


// GameController Facade pattern
class GameController {
	Board* board;
	vector<Player*> players;
	int currPlayerIndex;
	int playerCnt;
	GameStateContext* ctx;
public:
	GameController(Board*board,vector<Player*> players):board(board),players(players),currPlayerIndex(0) {
		playerCnt=players.size();
		ctx=new GameStateContext();
		board->setGameStateContext(ctx);
	}
	void startGame() {
		ctx->getState()->next(ctx); // state set to InprogressState
		while(!ctx->getState()->isGameOver())
		{
			Player* currentPlayer=players[currPlayerIndex];

			Move* move=currentPlayer->getStrategy()->createMove(currentPlayer,board->getSize());
			bool isSuccess=board->makeMove(move);
			if(isSuccess)
			{
				currPlayerIndex++;
				currPlayerIndex%=playerCnt;
			}
		}
	}
	void resetGame() {
		board->resetBoard();
		currPlayerIndex=0;
		ctx->setState(new UnstartedState());
	}

};
int main()
{
	GameObserver* consoleObs=new ConsoleObserver();
	Board* board= new Board(3,consoleObs);
	vector<Player*> players;
	for(int i=0; i<2; i++)
	{
		PlayerStrategy *strategy=NULL;
		if(i==0)
			strategy=new HumanPlayerStrategy();
		else
			strategy=new RandomPlayerStrategy();
		Player* player=new Player(i,'a'+i,strategy);
		players.push_back(player);
	}
	GameController *console=new GameController(board, players);
	console->startGame();
	// add observer design pattern to get console log notifications to each player for every state change
	// and display board on every notification
	// 	std::cout<<"Hello World";

	return 0;
}


/*
OUTPUT:
Enter row for human player: 0
0
Enter column for human player: 0
0

MOVE MADE, current board status:

a   
   
   
MOVE MADE, current board status:

a   
 b  
   Enter row for human player: 0
0
Enter column for human player: 0
4
ERROR: Invalid Move
Enter row for human player: 0
0
Enter column for human player: 0
2

MOVE MADE, current board status:

a  a 
 b  
   
MOVE MADE, current board status:

a b a 
 b  
   Enter row for human player: 0
3
Enter column for human player: 0
4
ERROR: Invalid Move
Enter row for human player: 0
2
Enter column for human player: 0
2

MOVE MADE, current board status:

a b a 
 b  
  a 
MOVE MADE, current board status:

a b a 
 b  
 b a 
STATE CHANGED, current state = WonState
*/


