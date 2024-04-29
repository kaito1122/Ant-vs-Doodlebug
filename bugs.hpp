// Kaito Minami
// CS3520: Programming in C++

#include <ncurses.h>
#include <vector>
#include <memory>
#include <set>
#include <algorithm>

class Bugs {
    public:
        int x;
        int y;
        int starve_count;
        int till_breed;

        Bugs(){starve_count = 0; till_breed = 0;}
        Bugs(int input_x, int input_y) {
            x = input_x; y  = input_y;
            starve_count = 0;
            till_breed = 0;
        }

        // randomly assign coordinates
        void auto_assign(int x_offset, int y_offset, int width, int height, std::vector<std::shared_ptr<Bugs>>& bugs) {
            int rand_x = rand() % (width - 1) + x_offset + 1;
            int rand_y = rand() % (height - 1) + y_offset + 1;
            bool occupied = false;

            for(auto& b: bugs) {
                if (b->x == rand_x and b->y == rand_y) {
                    occupied = true;
                    break;
                }
            }

            if(occupied){
                auto_assign(x_offset, y_offset, width, height, bugs);
            } else {
                x = rand_x;
                y = rand_y;
            }
        }

        std::vector<int> move(int x_offset, int y_offset, int width, int height, std::vector<std::shared_ptr<Bugs>> bugs) {
            std::vector<int> direction;
            int r = rand()%8;
            /* choose direction at random to move
             * (0,1)    down
             * (0,-1)   up
             * (1,0)    right
             * (-1,0)   left
             * (1,1)    down right
             * (1,-1)   up right
             * (-1,1)   down left
             * (-1,-1)  up left
             */
            switch(r) {
                case 0:
                    direction = {0,1};
                    break;
                case 1:
                    direction = {0,-1};
                    break;
                case 2:
                    direction = {1,0};
                    break;
                case 3:
                    direction = {-1,0};
                    break;
                case 4:
                    direction = {1,1};
                    break;
                case 5:
                    direction = {1,-1};
                    break;
                case 6:
                    direction = {-1,1};
                    break;
                case 7:
                    direction = {-1,-1};
                    break;
                default:
                    break;
            }
            direction = angle_check(x_offset, y_offset, width, height, direction, bugs); // if wall, bump
            x += direction[0];
            y += direction[1];
            return direction;
        }

        std::vector<int> angle_check(int x_offset, int y_offset, int width, int height, std::vector<int> direction, std::vector<std::shared_ptr<Bugs>> bugs) {
            // bump one wall
            if(x-1 == x_offset) {
                if(direction[0]==-1 and direction[1]!=0) {
                    direction = {0,direction[1]*2};
                } else if (direction[0]==-1 and direction[1]==0) {
                    direction = {0,0};
                }
            } else if (x+1 == x_offset + width) {
                if(direction[0]==1 and direction[1]!=0) {
                    direction = {0,direction[1]*2};
                } else if (direction[0]==1 and direction[1]==0) {
                    direction = {0,0};
                }
            } else if (y-1 == y_offset) {
                if(direction[1]==-1 and direction[0]!=0) {
                    direction = {direction[0]*2,0};
                } else if (direction[1]==-1 and direction[0]==0) {
                    direction = {0,0};
                }
            } else if (y+1 == y_offset + height) {
                if(direction[1]==1 and direction[0]!=0) {
                    direction = {direction[0]*2,0};
                } else if (direction[1]==1 and direction[0]==0) {
                    direction = {0,0};
                }
            }

            // don't move when occupied
            for(auto& b: bugs){
                if (x+direction[0] == b->x and y+direction[1] == b->y){
                    direction = {0,0};
                }
            }

            // bounce on corner, no double bounce, no out of grid
            if((x+direction[0] <= x_offset or x+direction[0] >= x_offset + width) or (y+direction[1] <= y_offset or y+direction[1] >= y_offset + height)){
                direction = {0,0};
            }

            return direction;
        }

        bool breedable_check(int x_offset, int y_offset, int width, int height, std::vector<std::shared_ptr<Bugs>> bugs) {
            // does 16-neighbor block goes out?
            int x_min = x - 2; int x_max = x + 2; int y_min = y - 2; int y_max = y + 2; int empties = 24;
            if ((x_min <= x_offset) or (y_min <= y_offset) or (x_max >= x_offset + width) or (y_max >= y_offset + height)) {
                return false;
            }

            // how many are empty? do we have it enough?
            for (int i=x_min; i<=x_max; i++) {
                for (int j=y_min; j<=y_max; j++) {
                    for(auto& bug: bugs){
                        if(bug->x == i and bug->y == j){
                            empties--;
                        }
                    }
                }
            }
            return bool(empties >= 1);
        }

        // draws the bug on the board
        void draw_bugs(char symbol){
            mvprintw(y, x, "%c", symbol);
        }

        // breed coordinate outputs
        std::set<std::vector<int>> breed(int offspring, std::vector<std::shared_ptr<Bugs>>& bugs){
            std::set<std::vector<int>> coord_option, final_coord;  // This so that children don't born in same coord
            std::vector<int> input, c;
            for(int i=x-2; i<=x+2; i++) {
                for(int j=y-2; j<=y+2; j++) {
                    for(auto& b: bugs){
                        if(b->x == i and b->y == j){
                            break;
                        }
                        input = {i,j};
                        coord_option.insert(input);
                    }
                }
            }

            if(coord_option.size() < offspring){
                offspring = coord_option.size();
            }

            for(int i=0; i<offspring; i++) {
                int rand_coord = rand() % coord_option.size();
                c = *(std::next(coord_option.begin(), rand_coord));
                final_coord.insert(c);
                coord_option.erase(std::next(coord_option.begin(), rand_coord));
            }
            return final_coord;
        }

        virtual ~Bugs(){}

};

class Ant : public Bugs {
public:
    char gender;

    Ant() {
        int g = rand()%100;
        if(g<70){
            gender = 'f';
        } else {
            gender = 'm';
        }
    }

    Ant(int input_x, int input_y) {
        x = input_x; y  = input_y;
        int g = rand()%100;
        if(g<70){
            gender = 'f';
        } else {
            gender = 'm';
        }


    }

    Ant (char input_g){
        gender = input_g;
    }

    virtual ~Ant(){}
};

class Queen : public Ant {
public:
    bool mated;
    int till_mate;
    bool cataglyphis;

    Queen(){mated=false; till_mate=0; cataglyphis=false;}
    Queen(std::shared_ptr<Ant>& a) {
        x = a->x;
        y = a->y;
        starve_count = a->starve_count;
        till_breed = a->till_breed;
        gender = a->gender;
        mated = false;
        till_mate = 0;

        int c = rand() % 1000;
        if(c == 0){
            cataglyphis=true;
        } else{
            cataglyphis=false;
        }
    }

    Queen(int input_x, int input_y){
        x = input_x;
	y = input_y;
	mated = false;
	till_mate = 0;
	cataglyphis=false;
    }

    // if found male next to her, breed
    // or very rare cataglyphis
    void mate(std::vector<std::shared_ptr<Ant>>& ants) {
        for(auto& ant: ants) {
            if (till_mate == 0 and (((ant->x >= x - 1 and ant->x <= x + 1) and (ant->y >= y - 1 and ant->y <= y + 1) and ant->gender == 'm') or cataglyphis)) {
                mated = true;
            }
        }
    }

    virtual ~Queen(){}
};

class Doodlebug : public Bugs {
public:
    std::vector<int> direction;

    Doodlebug() {direction={0,-1};}
    Doodlebug(int input_x, int input_y){
        x = input_x;
        y = input_y;
        direction = {0,-1};
    }

    // move function with find ant and eat functionality
    virtual void move(int x_offset, int y_offset, int width, int height, std::vector<std::shared_ptr<Bugs>>& bugs, std::vector<std::shared_ptr<Ant>>& ants, std::vector<std::shared_ptr<Queen>>& queens) {
        std::vector<int> dir = angle_check(x_offset, y_offset, width, height, direction, bugs);
        bool eaten = false;

        // eats ants
        for (int i=x-1; i<=x+1; i++) {
            for (int j=y-1; j<=y+1; j++) {
                for(auto it=ants.begin(); it!=ants.end(); it++){
                    auto ant = *it;
                    if(ant->x == i and ant->y == j) {
                        x = i; y = j;
                        ants.erase(it);
                        bugs.erase(std::find_if(bugs.begin(), bugs.end(), [&ant](const std::shared_ptr<Bugs>& bug) {
                            return bug == ant;
                        }));
			            starve_count = 0;
                        eaten = true;
                        break;
                    }
                }
            }
        }

	if(!eaten){
		        // eats ants
          for (int i=x-1; i<=x+1; i++) {
              for (int j=y-1; j<=y+1; j++) {
                  for(auto it=queens.begin(); it!=queens.end(); it++){
                      auto queen = *it;
                      if(queen->x == i and queen->y == j) {
                          x = i; y = j;
                          queens.erase(it);
                          bugs.erase(std::find_if(bugs.begin(), bugs.end(), [&queen](const std::shared_ptr<Bugs>& bug) {
                              return bug == queen;
                          }));
                                      starve_count = 0;
                          eaten = true;
                          break;
                      }
                  }
              }
          }
	}

        // if not eaten, clockwise circulation
        if(!eaten){
            x += dir[0];
            y += dir[1];
        }

        clockwise_circulate();

    }

    // clockwise circulate
    void clockwise_circulate() {
        int temp = direction[0];
        direction[0] = direction[1];
        direction[1] = -temp;
    }

    virtual ~Doodlebug(){}
};

