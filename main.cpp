
#include <vector>
#include <fstream>
#include <string>
#include <optional>
#include <iostream>
#include <tuple>
#include <memory>
#include <map>
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
    int placedID    = 0;          //unique ID for the piece placed at this spot   
    int key         = 0;          //value for the play spot to match with a domino
    int uID         = 0;          //unique ID for the new space

    PlaySpace(const int& key) : key(key){ }

    void markOccupied(const int& dominoID){
        placedID = dominoID; 
        occupied = true; 
    }
};

class Board{
public:
    //get play space at specific index in a linear search of board indexed by the number of optional<> which contain a value
    PlaySpace at(int index){
        int counter = 0;

        for (int i = 0; i < spaces.size(); i++){
            for (int j = 0; j < spaces[i].size(); j++){
                if (spaces[i][j] && counter == index)
                    return spaces[i][j].value();
                else if (spaces[i][j]){
                    counter++; 
                }
            }
        }
    }

    int numLocations(){
        int counter = 0;

        for (int i = 0; i < spaces.size(); i++){
            for (int j = 0; j < spaces[i].size(); j++){
                if (spaces[i][j])
                    counter++; 
            }
        }
        return counter; 
    }
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

    std::vector<double> solve(){

        //ensure solution is possible
        if (!board.isSolutionPossible(dominoPieces)){
            return std::vector<double>(dominoPieces.size()*2, 0.0); 
        }

        //init structures for ILP
        initProgVars(); 

        glp_smcp smcp;
        //create glpk problem 
        auto lp = glp_create_prob();
        glp_set_prob_name(lp, "DOM_MATH"); 
        glp_set_obj_dir(lp, GLP_MIN); 

        glp_init_smcp(&smcp); 
        smcp.msg_lev = GLP_MSG_OFF; 
        
        //create rules and bounds for rules, upper and lower
        int progVarsRowCounter = 1; 
        int bounding = 0; 
        glp_add_rows(lp, dominoPieces.size()*4); 
        //case 1
        //All pieces must be used so each piece will cover only two locations for some given piece
        //for some given SUM(i) == 2
        for (progVarsRowCounter; progVarsRowCounter < dominoPieces.size(); progVarsRowCounter++) {
            glp_set_row_bnds(lp, progVarsRowCounter, GLP_DB, 2.0, 2.0); 
        }

        
        //case 2
        //All playspaces must be covered 
        //For a given location the SUM == 1
        bounding = progVarsRowCounter + dominoPieces.size();
        for (progVarsRowCounter; progVarsRowCounter < bounding; progVarsRowCounter++){
            glp_set_row_bnds(lp, progVarsRowCounter, GLP_DB, 1.0, 1.0); 
        }

        //case 3
        //Tiles must be placed on locations which are adjacent and have inverted orientations at each location 
        //Two cases for each tiles
        bounding = progVarsRowCounter + dominoPieces.size(); 
        for (progVarsRowCounter; progVarsRowCounter < bounding; progVarsRowCounter++){
            glp_set_row_bnds(lp, progVarsRowCounter, GLP_DB, 1.0, 1.0); 
        }
        bounding = progVarsRowCounter + dominoPieces.size();
        for (progVarsRowCounter; progVarsRowCounter < bounding; progVarsRowCounter++){
            glp_set_row_bnds(lp, progVarsRowCounter, GLP_DB, -1.0, -1.0); 
        }

        std::unique_ptr<int> ia = std::unique_ptr<int>(new int[progVarsRowCounter]);                   //row index of non-zero entry
        std::unique_ptr<int> ja = std::unique_ptr<int>(new int[progVars.size()]);               //column index of non-zero entry
        std::unique_ptr<double> ar = std::unique_ptr<double>(new double[progVars.size()]);          //value 
        double z = 0; 

        //populate A matrix 
        int rowCounter = 0; 
        for (int i = 0; i < progVars.size(); i++){
            //case 1
            

            //case 2

            //case 3
        }
    }

private:
    std::vector<Domino> dominoPieces{};
    std::map<std::tuple<int, int, int>, int> intProgVars{}; //container for variables being used to map to linear programming problem
    std::vector<std::tuple<int, int, int>> progVars{}; 

    void initProgVars(){
        int writeCounter = 0; 
        std::tuple<int, int, int> var{};                    //new var for ILP mapping

        int numBoardLocations = board.numLocations(); 

        //create variables for all possible states of a piece
        progVars.resize(numBoardLocations * (int)dominoPieces.size() * 4); 

        for (int i = 0; i < dominoPieces.size(); i++){
            for (int j = 0; j < 4; j++){
                for (int k = 0; k < numBoardLocations; k++){
                    if ((k == 0 || k == 2) && dominoPieces.at(i).keys.first == board.at(k).key){
                        intProgVars.insert(std::pair<std::tuple<int, int, int>,int>(std::tuple<int,int,int>(i,j,k), progVars.size()));
                        progVars.push_back(std::tuple<int,int,int>(i,j,k));
                    }
                }
            }
        }
    }
    
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

    if (argc != 3){
        std::cout << argc << "wrong number" << std::endl;
        return 0; 
    }
    
    std::string line = "";      //container for reading line from file 
    int val1 = 0, val2 = 0;     //containers for casting piece chars to int before creating domino
    //read pieces file 
    std::ifstream pieceFile(argv[1]); 
    if (pieceFile.is_open()) {
        while(std::getline(pieceFile, line)){
            if (line.size() > 3 || line.at(1) != '-'){
                std::cout << "Unexpected line read from file." << std::endl; 
                std::cout << "The piece file must be the first arguemnt to this program" << std::endl; 
                return -1; 
            }
            val1 = line.at(0) - '0'; 
            val2 = line.at(2) - '0'; 

            puzzle.newDomino(val1, val2); 
        }
        pieceFile.close();
    }else{
        std::cout << "Failed to open piece file: " << argv[1] << std::endl; 
        return -1; 
    }

    int readLineCounter = 0;    //counter for tracking number of lines read from file
    //read board 
    std::ifstream boardFile(argv[2]); 
    if (boardFile.is_open()){
        while(std::getline(boardFile, line)){ 
            for (int i = 0; i < line.size(); i++){
                if (line.at(i) != ' '){
                    val1 = line.at(i) - '0';
                    puzzle.board.addSpace(i, readLineCounter, val1); 
                }
            }
            readLineCounter++; 
        }
        boardFile.close(); 
    }else{
        std::cout << "Failed to open board file: " << argv[2] << std::endl; 
        return -1; 
    }

    std::cout << "Attempting to solve file: " << argv[2] << std::endl;
    puzzle.solve(); 
}