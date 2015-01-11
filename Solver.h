/*
 * Solver.h
 *
 *  Created on: Oct 8, 2009
 *      Author: arnaud
 */

#ifndef SOLVER_H_
#define SOLVER_H_

#include "Situation.h"
#include "SituationTree.h"
#include "Movement.h"

// solver type
#define TYPE_DFS                                    1
#define TYPE_A_STAR                                 2
#define SOLVE_TYPE                                  TYPE_DFS

// pruning options
#define OPTION_PRUNE_BLOCKING                       1
#define OPTION_PRUNE_SQUARES                        1
#define OPTION_PRUNE_TRIANGLE_SITUATIONS            1
#define OPTION_PRUNE_DOORS_BLOCKING                 1

// heuristic options
#define OPTION_HEUR_USE_DISTANCE_TABLE              1
#define OPTION_HEUR_USE_NB_EMPTY_SLOT               1
#define OPTION_HEUR_USE_PUSH_BOX_IN_SLOT_WITH_WALL  1
#define OPTION_HEUR_USE_DONT_PUSH_AGAINST_WALL      1

class Solver {
public:
	Solver(Situation* i);
	~Solver();

	SituationTree* solve(bool* was_solution_found);
	void solve_and_display(bool display_sons = true);
	string solve_and_convert_to_string(bool* was_solution_found);

//private:

	Situation* initial_situation;

	set<string> situations_seen_or_in_queue;

	string compute_string_of_solution();
	void compute_heuristic(SituationTree* tree, direction& dir,
			coord& box_new_x, coord& box_new_y);

	bool* is_cell_forbidden;
	void compute_blocked_positions();
	bool is_blocking(coord &x, coord &y);
	bool is_blocked_in_the_corner(coord &x, coord &y);
	void blocked_in_an_horizontal_U(coord &x, coord &y, direction &dir);
	void blocked_in_an_vertical_U(coord &x, coord &y, direction &dir);
	bool contains_blocking_square(Situation* s, direction& dir,
			coord& center_x, coord& center_y);

	coord* distance_table;
	void compute_distance_table();
	void recursive_distance_table(coord &x, coord &y, heuristic value);

	short* nb_adjacent_wall;
	void compute_nb_adjacent_wall();

	bool* is_cell_a_door;
	void compute_doors();
};

#endif /* SOLVER_H_ */
