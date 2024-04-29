/* Kaito Minami
 * CS3520: Programming in C++
 *
 * UML
 *
 * [Bugs                      ]         [Doodlebug : Bugs           ]
 * [--------------------------]         [---------------------------]
 * [x: int                    ]         [direction: vector<int>     ]
 * [y: int                    ] <-------[===========================]
 * [starve_count: int         ]         [Doodlebug()                ]
 * [till_breed: int           ]         [virtual move(): vector<int>]
 * [==========================]         [clockwise_circulate(): void]
 * [Bugs()                    ]         [virtual ~Doodlebug()       ]
 * [move(): vector<int>       ]
 * [angle_check(): vector<int>]
 * [breed(): set<vector<int>> ]
 * [breedable_check(): bool   ]
 * [draw_bugs(): void         ]
 * [auto_assign(x,y): void    ]
 * [~Bugs()                   ]
 *       ^
 *       |
 *       |
 * [Ant : Bugs    ]                      [Queen : Ant     ]
 * [--------------]                      [----------------]
 * [gender: char  ] <--------------------[mated: bool     ]
 * [==============]                      [================]
 * [Ant()         ]                      [Queen()         ]
 * [virtual ~Ant()]                      [mate(): void    ]
 *                                       [virtual ~Queen()]
 *
 */

#include <ncurses.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <vector>
#include <set>
#include <algorithm>
#include "game_window.hpp"
#include "bugs.hpp"

// copy code from snake_game
enum State{INIT, DOODLEBUG, ANT, DEAD, EXIT};

void game(){
    enum State state = INIT; // Set the initial state
    static int x_max, y_max; // Max screen size variables
    static int x_offset, y_offset; // distance between the top left corner of your screen and the start of the board
    gamewindow_t *window; // Name of the board

    std::vector<std::shared_ptr<Bugs>> all_bugs;
    std::vector<std::shared_ptr<Doodlebug>> doodles;
    std::vector<std::shared_ptr<Ant>> ants;
    std::vector<std::shared_ptr<Queen>> queens;

    const int height = 30; // 100
    const int width = 70; // 100

    struct timespec timeret;
    timeret.tv_sec = 0;
    timeret.tv_nsec = 999999999/4;

    std::string win;
    int turn = 0;

    while(state != EXIT){
        switch(state){
            case INIT: {
                // pretty sure those not needed but i just leave it
                initscr();
                start_color();
                nodelay(stdscr, TRUE);
                noecho();
                getmaxyx(stdscr, y_max, x_max);
                keypad(stdscr, TRUE);
                curs_set(0);
                timeout(100);

                // Setting height and width of the board
                x_offset = (x_max / 2) - (width / 2);
                y_offset = (y_max / 2) - (height / 2);

                //Init board
                window = init_GameWindow(x_offset, y_offset, width, height); // memory leak

                // Init bugs
                int dood, m_ant, f_ant, queen;
                std::cout<<"How many doodlebugs?"<<std::endl;
                std::cin>>dood; // 1
                std::cout<<"How many male ants?"<<std::endl;
                std::cin>>m_ant; // 20
		std::cout<<"How many female ants?"<<std::endl;
                std::cin>>f_ant; // 60
                std::cout<<"How many queen ants?"<<std::endl;
                std::cin>>queen; // 20

                // initial declaration
                for(int d=0; d<dood; d++) {
                    std::shared_ptr<Doodlebug> new_dood(new Doodlebug());
                    new_dood->auto_assign(x_offset, y_offset, width, height, all_bugs);
                    all_bugs.push_back(new_dood);
                    doodles.push_back(new_dood);
                }
                for(int a=0; a<m_ant; a++) {
                    std::shared_ptr<Ant> new_ant(new Ant('m'));
                    new_ant->auto_assign(x_offset, y_offset, width, height,all_bugs);
                    all_bugs.push_back(new_ant);
                    ants.push_back(new_ant);
                }
		for(int a=0; a<f_ant; a++) {
                    std::shared_ptr<Ant> new_ant(new Ant('f'));
                    new_ant->auto_assign(x_offset, y_offset, width, height,all_bugs);
                    all_bugs.push_back(new_ant);
                    ants.push_back(new_ant);
                }
                for(int d=0; d<queen; d++) {
                    std::shared_ptr<Queen> new_queen(new Queen());
                    new_queen->auto_assign(x_offset, y_offset, width, height,all_bugs);
                    all_bugs.push_back(new_queen);
                    queens.push_back(new_queen);
                }

                // draw
                draw_Gamewindow(window);
                for(auto& a: ants) {
                    if(a->gender=='f') {
                        a->draw_bugs('w');
                    } else {
                        a->draw_bugs('o');
                    }
                }
                for(auto& q: queens) {
                    q->draw_bugs('Q');
                }
                for(auto& d: doodles) {
                    d->draw_bugs('X');
                }

                state = DOODLEBUG;
                break;
            }
            case DOODLEBUG: {
                // doodlebug moves first
                // checks if ants eaten, breed check, starve check
                std::vector<std::shared_ptr<Doodlebug>> new_doodles;
                new_doodles.clear();
                for(auto it=doodles.begin(); it!=doodles.end(); ){
                    auto d = *it;
                    d->starve_count++;
                    d->till_breed++;
                    d->move(x_offset, y_offset, width, height, all_bugs, ants, queens); // moves and eats

                    // baby born
                    if(d->till_breed >= 10 and d->breedable_check(x_offset, y_offset, width, height, all_bugs)) {
                        std::set<std::vector<int>> coord = d->breed(1, all_bugs);
                        for(auto it=coord.begin(); it!=coord.end(); it++){
                            auto vec = *it;
                            std::shared_ptr<Doodlebug> new_d(new Doodlebug(vec[0], vec[1]));
                            new_doodles.push_back(new_d);
                        }
                        d->till_breed = 0;
                    }

                    // starved to death
                    if(d->starve_count == 20) {
                        doodles.erase(it);
                        all_bugs.erase(std::find_if(all_bugs.begin(), all_bugs.end(), [&d](const std::shared_ptr<Bugs>& bug) {
                            return bug == d;
                        }));
                    } else {
                        ++it;
                    }
                }
                // babies insert
                doodles.insert(doodles.end(), new_doodles.begin(), new_doodles.end());
                all_bugs.insert(all_bugs.end(), new_doodles.begin(), new_doodles.end());

                // draw
                clear();
                draw_Gamewindow(window);
                for(auto& a: ants) {
                    if(a->gender=='f') {
                        a->draw_bugs('w');
                    } else {
                        a->draw_bugs('o');
                    }
                }
                for(auto& q: queens) {
                    q->draw_bugs('Q');
                }
                for(auto& d: doodles) {
                    d->draw_bugs('X');
                }
                mvprintw(0,0,"Doodlebug: %i, Ant: %i, Queen: %i, Turn: %i", doodles.size(), ants.size()+queens.size(), queens.size(), turn);

                state = ANT;

                // if ant or doodlebug empty, end game
                if(ants.empty() and queens.empty()) {
                    win = "Doodlebug";
                    state = DEAD;
                } else if (doodles.empty()) {
                    win = "Ant";
                    state = DEAD;
                }
                break;
            }
            case ANT: {
                // ant moves
                for(auto it=ants.begin(); it!=ants.end(); ){
                    auto a = *it;
                    // moving
                    std::vector<int> moved = a->move(x_offset, y_offset, width, height, all_bugs);
		    bool erased = false;
                    // if moved, doesn't starve. if not moved, starves.
                    if(moved[0] == 0 and moved[1] == 0){
                        a->starve_count++;
                    } else {
                        a->starve_count = 0;
                    }

                    // grows to queen at >0.005% chance
                    if (a->gender == 'f') {
                        int r = rand() % 1000;
                        if(r < 5) {
                            std::shared_ptr<Queen> new_queen(new Queen(a));
                            all_bugs.erase(std::find_if(all_bugs.begin(), all_bugs.end(), [&a](const std::shared_ptr<Bugs>& bug) {
                                return bug == a;
                            }));
                            queens.push_back(new_queen);
                            all_bugs.push_back(new_queen);
			    erased = true;
                        }
                    }

                    // starved to death
                    if(a->starve_count == 5) {
                        erased = true;
                        all_bugs.erase(std::find_if(all_bugs.begin(), all_bugs.end(), [&a](const std::shared_ptr<Bugs>& bug) {
                            return bug == a;
                        }));
                    } 
		    if (erased){
		        ants.erase(it);
		    }
		    else {
                        ++it;
                    }
                }

                // queen moves
                std::vector<std::shared_ptr<Ant>> new_ants;
                new_ants.clear();
                for(auto it=queens.begin(); it!=queens.end(); ){
                    auto q = *it;
                    // if not mated, moves. if mated, not moves till birth
                    if(!q->mated) {
                        q->move(x_offset, y_offset, width, height, all_bugs);
                    } else {
                        q->till_breed++;
                    }
                    q->mate(ants);

                    if(q->till_mate==30){
                        q->till_mate = 0;
                    }
                    if(q->till_mate>0){
                        q->till_mate++;
                    }

                    // babies born
                    if(q->till_breed >= 30 and q->breedable_check(x_offset, y_offset, width, height, all_bugs)) {
                        std::set<std::vector<int>> coord = q->breed(10, all_bugs);
			for(auto it=coord.begin(); it!=coord.end(); it++){
                            auto vec = *it;
			    int r = rand() % 1000;
			    if(r < 5) {
                                std::shared_ptr<Queen> new_queen(new Queen(vec[0], vec[1]));
                                queens.push_back(new_queen);
                                all_bugs.push_back(new_queen);
                            } else {
			        std::shared_ptr<Ant> new_a(new Ant(vec[0], vec[1]));
                                new_ants.push_back(new_a);
			    }
                        }

                        q->till_breed = 0;
                        q->mated = false;
                        q->till_mate++;
                    }

                    // couldn't birth, die.
                    if(q->till_breed == 90) {
                        queens.erase(it);
                        all_bugs.erase(std::find_if(all_bugs.begin(), all_bugs.end(), [&q](const std::shared_ptr<Bugs>& bug) {
                            return bug == q;
                        }));
                    } else {
                        ++it;
                    }
                }
                // babies insert
                ants.insert(ants.end(), new_ants.begin(), new_ants.end());
                all_bugs.insert(all_bugs.end(), new_ants.begin(), new_ants.end());

                // how many days passed
                turn++;

                // draw
                clear();
                draw_Gamewindow(window);
                for(auto& a: ants) {
                    if(a->gender=='f') {
                        a->draw_bugs('w');
                    } else {
                        a->draw_bugs('o');
                    }
                }
                for(auto& q: queens) {
                    q->draw_bugs('Q');
                }
                for(auto& d: doodles) {
                    d->draw_bugs('X');
                }
                mvprintw(0,0,"Doodlebug: %i, Ant: %i, Queen: %i, Turn: %i", doodles.size(), ants.size()+queens.size(), queens.size(), turn);

                state = DOODLEBUG;

                // if ant empty, end game
                if(ants.empty() and queens.empty()) {
                    win = "Doodlebug";
                    state = DEAD;
                }
                break;
            }
            case DEAD: {
                // winner
                mvprintw(1,0,"Simulation finished. The winner is: %s",win.c_str());

                // actually end game
                endwin();
                state = EXIT;
                break;
            }
        }
        refresh();
        nanosleep(&timeret, NULL);
    }
}

int main(){
    game();
    return 0;
}
