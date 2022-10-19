
#include <vector>
#include <optional>
#include <iostream>
#include <glpk.h>

struct Domino{
    //keys for the domino
    std::pair<int, int> keys{};
    int uniqueID = 0; 

    //create a new domino
    Domino(const int& key1, const int& key2, const int& uniqueID) : keys{std::pair<int, int>{key1, key2}}, uniqueID(uniqueID){};
};

//each spot on the board will either be occupied or not
struct PlaySpace{
    bool occupied   = false;      //is there a domino on this spot
    int placedID    = 0;           //unique ID for the piece placed at this spot   
    int key         = 0;          //value for the play spot to match with a domino

    PlaySpace(const int& key) : key(key){ }

    void markOccupied(const int& dominoID){
        placedID = dominoID; 
        occupied = true; 
    }
};

class Board{
public:
    /// @brief Prints the current state of the board. Default behavior is to print the board with the keys for the play spaces.
    /// @param printPlacedDominoPieces Should the domino piece uniqueIDs be printed rather than the keys for the play spaces 
    void print(bool printPlacedDominoPieces = false){
        int rowCounter = 0; 
        int outOfBoundsCounter = 0; 
        bool complete = false; 


        if (printPlacedDominoPieces){
            std::cout << "Printing board with unique piece IDs:" << std::endl; 
            std::cout << "NOTE: capital X correlates to an unoccupied spot" << std::endl; 
        }else{
            std::cout << "Printing board:" << std::endl; 
        }

        while(!complete){
            outOfBoundsCounter = 0; 

            //print each row
            for (int i = 0; i < spaces.size(); i++){
                //check if the row being printed within the bounds of the column size
                if (rowCounter < spaces[i].size()){
                    if (spaces[i][rowCounter]){
                        if (printPlacedDominoPieces && spaces[i][rowCounter].value().occupied){
                            std::cout << "  " << spaces[i][rowCounter].value().placedID << "  ";
                        }else if(printPlacedDominoPieces && !spaces[i][rowCounter].value().occupied){
                            std::cout << "  X  ";
                        }else{
                            std::cout << "  " << spaces[i][rowCounter].value().key << "  ";
                        }
                    }else{
                        std::cout << "     "; 
                    }
                }else{
                    //count this as out of bounds
                    std::cout << "     ";
                    outOfBoundsCounter++; 
                }
            }

            std::cout << std::endl; 
            rowCounter++; 

            //if all rows are out of bounds, stop 
            if (outOfBoundsCounter == spaces.size())
                complete = true; 
        }
    }

    //add a new space to the board at the given coordinates
    void addSpace(const int locX, const int locY, const int key){
        //ensure that the board is the correct size
        if (spaces.size() <= locX){
            spaces.resize(locX + 1); 
        }
        if (spaces[locX].size() <= locY){
            spaces[locX].resize(locY + 1); 
        }

        spaces[locX][locY] = std::optional<PlaySpace>(key); 
    }

    //check if all of the play spaces on the board are filled
    bool isComplete(){
        for (int i = 0; i < spaces.size(); i++){
            for (int j = 0; j < spaces[i].size(); j++){
                if (spaces[i][j] && !spaces[i][j].value().occupied)
                {
                    return false; 
                }
            }
        }
        return true;
    }

    //get the placespace by applying the provided orientation to the current position
    std::optional<PlaySpace>* getAdjacentSpaceFromOrientation(const std::pair<int, int>& location, const int& orientation){

        switch(orientation){
            case 0:
                //left-right
                if (location.first + 1 < spaces.size() && spaces[location.first+1][location.second])
                    return &spaces[location.first+1][location.second]; 
                break; 
            case 1:
                //up-down
                if (location.second + 1 < spaces[location.first].size() && spaces[location.first][location.second+1])
                    return &spaces[location.first][location.second+1]; 
                break;
            case 2:
                //left-right 
                if (location.first - 1 >= 0 && location.second < spaces[location.first-1].size() && spaces[location.first-1][location.second])
                    return &spaces[location.first - 1][location.second]; 
                break;
            case 3:
                //down-up
                if (location.second - 1 >= 0 && spaces[location.first][location.second-1])
                    return &spaces[location.first][location.second - 1]; 
                break;
            default:
                throw std::runtime_error("Invalid orientation");
                break; 
        }

        return nullptr; 

    }

    //get all possible positions where the domino could be placed
    std::vector<std::pair<int,int>> possibleDominoPlaces(const Domino& domino, const int& orientation){
        std::vector<std::pair<int,int>> possibleDominoPlaces{}; 
        std::optional<PlaySpace>* currSecondSpace = nullptr; 

        for (int i = 0; i < spaces.size(); i++){
            for (int j = 0; j < spaces[i].size(); j++){
                //ensure optional has value
                if (spaces[i][j] && !spaces[i][j].value().occupied){
                    PlaySpace& space = spaces[i][j].value(); 
                    currSecondSpace = getAdjacentSpaceFromOrientation(std::pair<int,int>(i,j), orientation);
                    if (currSecondSpace && !currSecondSpace->value().occupied){
                        if (space.key == domino.keys.first && currSecondSpace->value().key == domino.keys.second)
                            possibleDominoPlaces.push_back(std::pair<int,int>(i,j));

                    }
                }
            }
        }

        return possibleDominoPlaces;
    }

    //place the domino on the board and update spaces to reflect the placement of piece
    void placeDomino(const Domino& domino, const std::pair<int, int>& location, const int& orientation){
        spaces[location.first][location.second].value().markOccupied(domino.uniqueID); 
        getAdjacentSpaceFromOrientation(location, orientation)->value().markOccupied(domino.uniqueID);
    }

    bool isSolutionPossible(const std::vector<Domino>& pieces){
        int playSpaceCount = 0; 

        for (int i = 0; i < spaces.size(); i++){
            for (int j = 0; j < spaces[i].size(); j++){
                if (spaces[i][j]){
                    playSpaceCount++; 
                }
            }
        }

        if (playSpaceCount / 2 == pieces.size()){
            return true;
        }

        return false; 
    }

private:
    //core of the board
    std::vector<std::vector<std::optional<PlaySpace>>> spaces{std::vector<std::vector<std::optional<PlaySpace>>>(2, std::vector<std::optional<PlaySpace>>(2, std::optional<PlaySpace>()))}; 
};

class MathPuzzle{
public:
    Board board = Board(); 

    //create a new Domino
    void newDomino(const int& key1, const int& key2){
        dominoPieces.push_back(Domino(key1, key2, dominoPieces.size()));  
    }

    void solve(){
        //check that there is enough pieces for a solution 
        if (board.isSolutionPossible(dominoPieces)){
            auto res = processPuzzle(dominoPieces, board, 0); 
            res.print(true);
            if (res.isComplete()){
                std::cout << "Solved" << std::endl;
            }
        }
    }

    MathPuzzle(){
        auto lp = glp_create_prob();
        glp_set_prob_name(lp, "DOMMATH"); 
        glp_set_obj_dir(lp, GLP_MIN); 

        
    }

private:
    std::vector<Domino> dominoPieces{};

    /// @brief 
    /// @param currentDominos 
    /// @param currentBoard 
    /// @param tileOrientation 
    /// @return current state of board with pieces in place 
    Board processPuzzle(std::vector<Domino> currentDominos, Board currentBoard, int tileOrientation){
        if (currentDominos.size() == 0)
            return currentBoard;
        if (tileOrientation > 3){
            return currentBoard;        //uh oh returning unsolved board...will have to check again if it is complete
        }

        Domino currentDomino = currentDominos.back(); 

        //find all possible places for the current piece
        std::vector<Domino> dominoListRmvLast(currentDominos); 
        dominoListRmvLast.pop_back();
        Board correctPlaceBoard{}; 
        Board resCorrectPlace{}; 
        
        //avoid loops in which domino keys are identical and already attempted this type of solution
        if (currentDomino.keys.first == currentDomino.keys.second && tileOrientation == 3 )
            return currentBoard; 

        Board resNotCorrectOrientation = processPuzzle(currentDominos, currentBoard, tileOrientation + 1);
        if (resNotCorrectOrientation.isComplete())
            return resNotCorrectOrientation;

        if (currentDomino.keys.first == 2 && currentDomino.keys.second == 3 && tileOrientation == 2){
            int about = 0; 
        }

        auto possiblePlaces = currentBoard.possibleDominoPlaces(currentDomino, tileOrientation);
        for (auto& space : possiblePlaces){
            //recurse
            correctPlaceBoard = Board(currentBoard);
            correctPlaceBoard.placeDomino(currentDomino, space, tileOrientation);

            //piece is in correct position update the board 
            resCorrectPlace = processPuzzle(dominoListRmvLast, correctPlaceBoard, 0);  

            if (resCorrectPlace.isComplete()){
                return resCorrectPlace; 
            }
        }

        return currentBoard; 
    }
};


int main(int argc, char **argv){
    MathPuzzle puzzle = MathPuzzle(); 

    //test case - 1
    // puzzle.board.addSpace(1, 0, 0);
    // puzzle.board.addSpace(2, 0, 0);
    // puzzle.board.addSpace(0, 1, 0);
    // puzzle.board.addSpace(1, 1, 1);
    // puzzle.board.addSpace(2, 1, 1); 
    // puzzle.board.addSpace(3, 1, 1); 
    // puzzle.board.addSpace(1, 2, 1); 
    // puzzle.board.addSpace(2, 2, 2); 

    // puzzle.newDomino(0, 0); 
    // puzzle.newDomino(0, 1); 
    // puzzle.newDomino(1, 1); 
    // puzzle.newDomino(1, 2); 

    //test case - 2
    // puzzle.board.addSpace(4, 0, 3);
    // puzzle.board.addSpace(5, 0, 3); 
    // puzzle.board.addSpace(3, 1, 0); 
    // puzzle.board.addSpace(4, 1, 3);
    // puzzle.board.addSpace(5, 1, 1);
    // puzzle.board.addSpace(6, 1, 6);
    // puzzle.board.addSpace(3, 2, 1);
    // puzzle.board.addSpace(4, 2, 5); 
    // puzzle.board.addSpace(5, 2, 0);
    // puzzle.board.addSpace(6, 2, 6);
    // puzzle.board.addSpace(1, 3, 2);
    // puzzle.board.addSpace(2, 3, 2);
    // puzzle.board.addSpace(3, 3, 3); 
    // puzzle.board.addSpace(4, 3, 4);
    // puzzle.board.addSpace(5, 3, 4);
    // puzzle.board.addSpace(6, 3, 4);
    // puzzle.board.addSpace(7, 3, 5);
    // puzzle.board.addSpace(8, 3, 4);
    // puzzle.board.addSpace(0, 4, 1);
    // puzzle.board.addSpace(1, 4, 4);
    // puzzle.board.addSpace(2, 4, 0);
    // puzzle.board.addSpace(3, 4, 2);
    // puzzle.board.addSpace(4, 4, 5);
    // puzzle.board.addSpace(5, 4, 0);
    // puzzle.board.addSpace(6, 4, 5);
    // puzzle.board.addSpace(7, 4, 5);
    // puzzle.board.addSpace(8, 4, 6);
    // puzzle.board.addSpace(9, 4, 6);
    // puzzle.board.addSpace(0, 5, 0);
    // puzzle.board.addSpace(1, 5, 4);
    // puzzle.board.addSpace(2, 5, 5);
    // puzzle.board.addSpace(3, 5, 5);
    // puzzle.board.addSpace(4, 5, 2);
    // puzzle.board.addSpace(5, 5, 6);
    // puzzle.board.addSpace(6, 5, 1);
    // puzzle.board.addSpace(7, 5, 2); 
    // puzzle.board.addSpace(8, 5, 3);
    // puzzle.board.addSpace(9, 5, 1);
    // puzzle.board.addSpace(1, 6, 0);
    // puzzle.board.addSpace(2, 6, 0);
    // puzzle.board.addSpace(3, 6, 2);
    // puzzle.board.addSpace(4, 6, 4);
    // puzzle.board.addSpace(5, 6, 4);
    // puzzle.board.addSpace(6, 6, 0);
    // puzzle.board.addSpace(7, 6, 6);
    // puzzle.board.addSpace(8, 6, 1);
    // puzzle.board.addSpace(3, 7, 3); 
    // puzzle.board.addSpace(4, 7, 6);
    // puzzle.board.addSpace(5, 7, 1);
    // puzzle.board.addSpace(6, 7, 1);
    // puzzle.board.addSpace(3, 8, 3);
    // puzzle.board.addSpace(4, 8, 5);
    // puzzle.board.addSpace(5, 8, 2);
    // puzzle.board.addSpace(6, 8, 6);
    // puzzle.board.addSpace(4, 9, 2);
    // puzzle.board.addSpace(5, 9, 3);
    
    // puzzle.newDomino(0, 0);
    // puzzle.newDomino(0, 1);
    // puzzle.newDomino(0, 2);
    // puzzle.newDomino(0, 3);
    // puzzle.newDomino(0, 4);
    // puzzle.newDomino(0, 5);
    // puzzle.newDomino(0, 6);
    // puzzle.newDomino(1, 1);
    // puzzle.newDomino(1, 2);
    // puzzle.newDomino(1, 3);
    // puzzle.newDomino(1, 4);
    // puzzle.newDomino(1, 5);
    // puzzle.newDomino(1, 6);
    // puzzle.newDomino(2, 2);
    // puzzle.newDomino(2, 3);
    // puzzle.newDomino(2, 4);
    // puzzle.newDomino(2, 5);
    // puzzle.newDomino(2, 6);
    // puzzle.newDomino(3, 3);
    // puzzle.newDomino(3, 4);
    // puzzle.newDomino(3, 5);
    // puzzle.newDomino(3, 6);
    // puzzle.newDomino(4, 4);
    // puzzle.newDomino(4, 5);
    // puzzle.newDomino(4, 6);
    // puzzle.newDomino(5, 5); 
    // puzzle.newDomino(5, 6);
    // puzzle.newDomino(6, 6);


    //test case - 3
    puzzle.board.addSpace(4, 0, 6);
    puzzle.board.addSpace(5, 0, 6);
    puzzle.board.addSpace(3, 1, 6);
    puzzle.board.addSpace(4, 1, 0);
    puzzle.board.addSpace(5, 1, 3);
    puzzle.board.addSpace(6, 1, 3);
    puzzle.board.addSpace(3, 2, 5);
    puzzle.board.addSpace(4, 2, 4);
    puzzle.board.addSpace(5, 2, 4);
    puzzle.board.addSpace(6, 2, 3);
    puzzle.board.addSpace(1, 3, 4);
    puzzle.board.addSpace(2, 3, 5);
    puzzle.board.addSpace(3, 3, 5);
    puzzle.board.addSpace(4, 3, 5);
    puzzle.board.addSpace(5, 3, 4);
    puzzle.board.addSpace(6, 3, 4);
    puzzle.board.addSpace(7, 3, 3);
    puzzle.board.addSpace(8, 3, 4);
    puzzle.board.addSpace(0, 4, 2);
    puzzle.board.addSpace(1, 4, 2);
    puzzle.board.addSpace(2, 4, 3);
    puzzle.board.addSpace(3, 4, 0);
    puzzle.board.addSpace(4, 4, 0);
    puzzle.board.addSpace(5, 4, 0);
    puzzle.board.addSpace(6, 4, 1);
    puzzle.board.addSpace(7, 4, 0);
    puzzle.board.addSpace(8, 4, 0);
    puzzle.board.addSpace(9, 4, 0);
    puzzle.board.addSpace(0, 5, 3);
    puzzle.board.addSpace(1, 5, 3);
    puzzle.board.addSpace(2, 5, 2);
    puzzle.board.addSpace(3, 5, 2);
    puzzle.board.addSpace(4, 5, 0);
    puzzle.board.addSpace(5, 5, 1);
    puzzle.board.addSpace(6, 5, 1);
    puzzle.board.addSpace(7, 5, 4);
    puzzle.board.addSpace(8, 5, 5);
    puzzle.board.addSpace(9, 5, 5);
    puzzle.board.addSpace(1, 6, 1);
    puzzle.board.addSpace(2, 6, 5);
    puzzle.board.addSpace(3, 6, 2);
    puzzle.board.addSpace(4, 6, 2);
    puzzle.board.addSpace(5, 6, 2);
    puzzle.board.addSpace(6, 6, 1);
    puzzle.board.addSpace(7, 6, 1);
    puzzle.board.addSpace(8, 6, 1);
    puzzle.board.addSpace(3, 7, 6);
    puzzle.board.addSpace(4, 7, 2);
    puzzle.board.addSpace(5, 7, 6);
    puzzle.board.addSpace(6, 7, 3);
    puzzle.board.addSpace(3, 8, 1);
    puzzle.board.addSpace(4, 8, 6);
    puzzle.board.addSpace(5, 8, 6);
    puzzle.board.addSpace(6, 8, 6);
    puzzle.board.addSpace(4, 9, 5);
    puzzle.board.addSpace(5, 9, 4); 

    puzzle.newDomino(0, 0);
    puzzle.newDomino(0, 1);
    puzzle.newDomino(0, 2);
    puzzle.newDomino(0, 3);
    puzzle.newDomino(0, 4);
    puzzle.newDomino(0, 5);
    puzzle.newDomino(0, 6);
    puzzle.newDomino(1, 1);
    puzzle.newDomino(1, 2);
    puzzle.newDomino(1, 3);
    puzzle.newDomino(1, 4);
    puzzle.newDomino(1, 5);
    puzzle.newDomino(1, 6);
    puzzle.newDomino(2, 2); 
    puzzle.newDomino(2, 3);
    puzzle.newDomino(2, 4);
    puzzle.newDomino(2, 5);
    puzzle.newDomino(2, 6);
    puzzle.newDomino(3, 3);
    puzzle.newDomino(3, 4);
    puzzle.newDomino(3, 5);
    puzzle.newDomino(3, 6); 
    puzzle.newDomino(4, 4);
    puzzle.newDomino(4, 5);
    puzzle.newDomino(4, 6);
    puzzle.newDomino(5, 5);
    puzzle.newDomino(5, 6); 
    puzzle.newDomino(6, 6);

    puzzle.solve(); 
}