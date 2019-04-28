#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <typeinfo>
#include <Windows.h>

/*
PLEASE READ DESCRIPTION!

The program simulates Conway's Game of Life in an arena. The starting structure (life)
could be random or user-defined, as desired. Every life is in the form of a matrix. The matrix
elements should be binary zeros and ones. Ones indicate living cells, zeros indicate dead cells obviously.


RULES OF CONWAY'S GAME OF LIFE:

#1  Any live cell with fewer than N live neighbours dies, as if caused by underpopulation.
#2  Any live cell with N or N+1 live neighbours lives on to the next generation.
#3  Any live cell with more than N+1 live neighbours dies, as if by overpopulation.
#4  Any dead cell with exactly N+1 live neighbours becomes a live cell, as if by reproduction.

It means, that for N=2:
     C   Neighbours        new C
#1   1   0,1             ->  0  # Underpopulation
#2   1   2,3             ->  1  # Lives to the next generation
#3   1   4,5,6,7,8       ->  0  # Overpopulation
#4   0   3               ->  1  # Reproduction


GAME MODES:
# 1. RANDOM: Set game_mode to 'random' at initialization to play game with a fully randomized life at start
# 2. USER:   Set game_mode to 'user' at initialization to play the game with a user-defined matrix (life, which
             could be inputted from a .txt file. This way, you need to set the 

-------------

Some well-known automatons for N=2 Conway's game:

# GLIDER
a = [[1,0,0],
     [0,1,1],
     [1,1,0]]

# LWSP
b = [[0,0,1,1,0],
     [1,1,0,1,1],
     [1,1,1,1,0],
     [0,1,1,0,0]]

# UNBOUNDED GROWTH
c = [[1,1,1,0,1],
     [1,0,0,0,0],
     [0,0,0,1,1],
     [0,1,1,0,1],
     [1,0,1,0,1]]

# GLIDER GUN
d = [[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0],
     [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0],
     [0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1],
     [0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1],
     [1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
     [1,1,0,0,0,0,0,0,0,0,1,0,0,0,1,0,1,1,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0],
     [0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0],
     [0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
     [0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]]
*/

std::string boundary_condition;                     // Choosen boundary condition for the game
std::string game_mode;                              // Choosen game mode
static int gen_neighbours;                          // Exact number of neighbours to generate new cell (N)
                                                    // If there are gen_neighbours number of cells around the current tile
                                                    // then a new cell borns on a tile. If there are two more (N+2) or one
                                                    // less (N-1), the tile dies (see above in rules).

static int width_arena;                             // Width of the game arena [pixels]
static int height_arena;                            // Height of the game arena [pixels]
static int width_life;                              // Width of the input life [pixels]
static int height_life;                             // Height of the input life [pixels]
static int position_x;                              // Position of input life at start along X axis
static int position_y;                              // Position of input life at start along Y axis

std::vector<std::vector<int>> arena;                // Current state of the whole arena
std::vector<std::vector<int>> arena_new;            // Tracking previous state of arena for continuity
std::vector<std::vector<int>> input_life;           // Initial life, placed on the game arena

double roll_random_number() {
    // Randomize a new life with random zeros and ones
    std::random_device randomized;                           // Will be used to obtain a seed for the random number engine
    std::mt19937 generate(randomized());                     // Standard mersenne_twister_engine seeded with randomized()
    
    // Use dis to transform the random unsigned int generated by gen into a 
    // double in [0, 1). Each call to distribution(generate) generates a new random double.
    std::uniform_real_distribution<> distribution(0, 1);

    double random_number = distribution(generate);

    return random_number;
}

void create_arena_with_life() {

    // We generate the arena then concat the life into that
    // 1. Creating the arena with full of zeros
    for(unsigned int i = 0; i < height_arena; i++) {
        std::vector<int> tempvec;
        for(unsigned int j = 0; j < width_arena; j++) {
            tempvec.push_back(0);
        }
        arena.push_back(tempvec);
        arena_new.push_back(tempvec);
    }

    // 2. Place the life into the arena
    for(unsigned int i = position_y; i < position_y + height_life; i++) {
        for(unsigned int j = position_x; j < position_x + width_life; j++) {
            arena[i][j] = input_life[i-position_y][j-position_x];
        }
    }
}

void initial_random() {

    // Creating the random life
    for(unsigned int i = 0; i < height_life; i++) {
        std::vector<int> tempvec;
        for(unsigned int j = 0; j < width_life; j++) {
            if(roll_random_number() < 0.5) {
                tempvec.push_back(0);
            }
            else {
                tempvec.push_back(1);
            }
        }
        input_life.push_back(tempvec);
    }
}

void initial_user(std::ifstream& input_fs) {
    
    width_life = 0;                         // Initially the width of the input life is 0 [pixels]
    height_life = 0;                        // Initially the heigth of the input life is 0 [pixels]

    //
    // First read in input file, make sure, that it's the correct shape, then
    // test, if it's fit into the arena, if it's placed at the user-defined positon
    //
    // Assure, lines are always
    int first_line = 0;

    // Reading characters one by one in file
    // Loop don't skip whitespaces
    char tempc;                         // Temps storage for read-in characters
    while(input_fs >> std::noskipws >> tempc) {
        // If it's a 0 or 1, then put it into the input matrix
        if(tempc == '0' || tempc == '1') {

            // Convert char -> int
            input_life[height_life][width_life] = tempc - '0';
            width_life++;
        }

        else if(tempc == '\n') {
            height_life++;

            if(first_line == 0) {
                first_line = width_life;
            }

            if(first_line += width_life) {
                std::cout << "Lines are not equally long in input file! Aborting..." << std::endl;
                exit(-1);
            }
            width_life = 0;
        }
    }
    // +1 at EOF, because that's not count as '\n'
    height_life++;

    // Check if its fit into the arena, if it's placed at the user-defined positon
    if(position_x + width_life > width_arena || position_y + height_life > height_arena) {
        std::cout << "Arena's size is too small for the set parameters!\n"
                    << "Try again with a bigger arena or with a smaller life!" << std::endl;

        exit(-1);
    }
}

void step_with_simulation() {

    // Run through all elements of the game arena and count the neighbouring living cells
    // of every tiles in it. Then execute the game rules on the current tile.
    for(unsigned int i = 0; i < height_arena; i++) {
        for(unsigned int j = 0; j < width_arena; j++) {
            
            // For counting number of neighbouring living cells
            int count_neighbours = 0;

            // Run through the neighbouring elements of the current tile (those are 8 tiles)
            for(int r = -1; r <= 1; r++) {
                for(int c = -1; c <= 1; c++) {

                    // Skip the middle tile
                    if(r != 0 && c != 0) {
                        // Indeces of the current tile along X and Y axes
                        int current_tile_x = j+c;
                        int current_tile_y = i+r;
                    
                        // Free boundary conditions
                        // Boundary == 0 everywhere
                        if(boundary_condition == "unbounded") {
                            if(current_tile_x == -1 || current_tile_x == width_arena
                               || current_tile_y == -1 || current_tile_y == height_arena) {
                                continue;
                            }
                        }

                        // Periodic boundary contitions
                        else if(boundary_condition == "periodic") {
                            if(current_tile_x == -1) {
                                current_tile_x = width_arena - 1;
                            }
                            else if(current_tile_x == width_arena) {
                                current_tile_x = 0;
                            }
                            
                            if(current_tile_y == -1) {
                                current_tile_y = height_arena - 1;
                            }
                            else if(current_tile_y == height_arena) {
                                current_tile_y = 0;
                            }
                        }

                        // Constant living boundary conditions
                        else if(boundary_condition == "living") {
                            if(current_tile_x == -1 || current_tile_x == width_arena
                               || current_tile_y == -1 || current_tile_y == height_arena) {
                                count_neighbours++;
                                continue;
                            }
                        }

                        // Constant random boundary conditions
                        else if(boundary_condition == "random") {
                            if(current_tile_x == -1 || current_tile_x == width_arena
                               || current_tile_y == -1 || current_tile_y == height_arena) {
                                if(roll_random_number() < 0.5) {
                                    continue;
                                }
                                else {
                                    count_neighbours++;
                                    continue;
                                }
                            }
                        }

                        if(arena[current_tile_y][current_tile_x] == 1) {
                            count_neighbours += 1;
                        }
                    }
                }
            }

            // Apply rules
            // I. Rule
            if(arena[i][j] == 1 && count_neighbours < gen_neighbours) {
                arena_new[i][j] = 0;
            }
            // II. Rule
            else if(arena[i][j] == 1 && (count_neighbours == gen_neighbours || count_neighbours == gen_neighbours+1)) {
                arena_new[i][j] = 1;
            }
            // III. Rule
            else if(arena[i][j] == 1 && count_neighbours > gen_neighbours+1) {
                arena_new[i][j] = 0;
            }
            // IV. Rule
            else if(arena[i][j] == 0 && count_neighbours == gen_neighbours+1) {
                arena_new[i][j] = 1;
            }
            else {
                arena_new[i][j] = 0;
            }
        }
    }
}


int main(int argc, char* argv[]) {
    std::cout << "Conway's Game of Life (Cellular Automaton)\n"
              << " -----------------------------------------\n";

    boundary_condition = argv[1];                   // Choosen boundary condition for the game
    game_mode = argv[2];                            // Choosen game mode
    gen_neighbours = atoi(argv[3]);                 // Exact number of neighbours to generate new cell (N)
    width_arena = atoi(argv[4]);                    // Width of the game arena [pixels]
    height_arena = atoi(argv[5]);                   // Height of the game arena [pixels]
    position_x = atoi(argv[6]);                     // Position of random life at start along X axis
    position_y = atoi(argv[7]);                     // Position of random life at start along Y axis
    int sim_steps = atoi(argv[8]);                  // Total number of simulated steps

    if(game_mode == "random") {
        width_life = atoi(argv[9]);                 // Width of the random life [pixels]
        height_life = atoi(argv[10]);               // Height of the random life [pixels]

        if(position_x + width_life > width_arena || position_y + height_life > height_arena) {
            std::cout << "Arena's size is too small for the set parameters!\n"
                      << "Try again with a bigger arena or with a smaller life!" << std::endl;

            return(-1);
        }

        else {
            // Generate arena with random life
            initial_random();
        }
    }

    else if(game_mode == "user") {
        std::ifstream input_fs(argv[9]);            // Create input stream for reading file
        
        width_life = 0;
        height_life = 0;

        if (!input_fs.fail()) {
            // Generate arena and put input life into it
            // if it's possible. Else exit the program.
            initial_user(input_fs);

            input_fs.close();
        }

        else {
            std::cout << "File, named '" << argv[9]  << "' does not exist or corrupted! Aboring..." << std::endl;

            width_life = 3;
            height_life = 3;

            std::vector<int> temp;
            temp.push_back(1); temp.push_back(0); temp.push_back(0);
            input_life.push_back(temp);
            
            temp.clear();
            temp.push_back(0); temp.push_back(1); temp.push_back(1);
            input_life.push_back(temp);
            
            temp.clear();
            temp.push_back(1); temp.push_back(1); temp.push_back(0);
            input_life.push_back(temp);
        }
    }

    else {
        std::cout << "Available game modes are 'random' and 'user'!\n"
                  << "Try again and choose a mode from these variations!" << std::endl;
                  
        return(-1);
    }

    create_arena_with_life();

    // Initialize border for various boundary conditions
    if(boundary_condition == "unbounded") {}

    else if(boundary_condition == "periodic") {}

    else if(boundary_condition == "living") {}

    else if(boundary_condition == "random") {}

    // Run the simulation and log outputs
    std::ofstream dataFile;                     // Datafile for outputs
    dataFile.open("..\\out\\cell.dat");

    // Write initial state to log
    for(unsigned int i = 0; i < height_arena; i++) {
            for(unsigned int j = 0; j < width_arena; j++) {
                dataFile << arena[i][j] << ' ';
            }
            dataFile << '\n';
        }
        dataFile << '\n';

    for(unsigned int steps = 0; steps < sim_steps; steps++) {

        step_with_simulation();

        // Refresh the arena's memory
        arena = arena_new;

        for(unsigned int i = 0; i < height_arena; i++) {
            for(unsigned int j = 0; j < width_arena; j++) {
                dataFile << arena[i][j] << ' ';
            }
            dataFile << '\n';
        }
        dataFile << '\n';
    }
}