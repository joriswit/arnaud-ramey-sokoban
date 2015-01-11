/*
 * Solver.cpp
 *
 *  Created on: Oct 8, 2009
 *      Author: arnaud
 */

#include "Solver.h"

/*!
 * constructor
 */
Solver::Solver(Situation* i) {
	debug("Solver::constructor()");
	initial_situation = i;
	is_cell_forbidden = new bool[initial_situation->HEIGHT
			* initial_situation->WIDTH];
	distance_table = new coord[initial_situation->HEIGHT
			* initial_situation->WIDTH];
	nb_adjacent_wall = new short[initial_situation->HEIGHT
			* initial_situation->WIDTH];
	is_cell_a_door = new bool[initial_situation->HEIGHT
			* initial_situation->WIDTH];
}

/*!
 * destructor
 */
Solver::~Solver() {
	debug("Solver::destructor()");
	//delete initial_situation;
	delete[] is_cell_forbidden;
	delete[] is_cell_a_door;
	delete[] distance_table;
	delete[] nb_adjacent_wall;
}

/*!
 * @param s1
 * @param s2
 * @return true if s1 starts with s2
 */
static inline bool string_starts_with(string& s1, string& s2) {
	string::iterator it1 = s1.begin(), it2 = s2.begin();
	for (unsigned int i = 0; i < s2.size(); ++i) {
		if (*it1 != *it2)
			return false;
		++it1;
		++it2;
	}
	return true;
}

/*!
 * solver the situation
 * @param was_solution_found if we found a solution
 * @return the tree of situations, from the root to the sol
 */
SituationTree* Solver::solve(bool* was_solution_found) {
	debug("Solver::solve()");

	/* special case : the initial situation is winning */
	if (initial_situation->get_is_won() == true) {
		*was_solution_found = true;
		// return a clone not to delete initial_situation
		return new SituationTree(initial_situation->clone());
	}

	/* clear the answers */
	situations_seen_or_in_queue.clear();
#if SOLVE_TYPE == TYPE_DFS
	cout << "Solver::solving with DFS..." << endl;
	deque<SituationTree*> nodes_to_expand;
#elif SOLVE_TYPE == TYPE_A_STAR
	cout << "Solver::solving with A*..." << endl;
	multiset<SituationTree*, SituationTreeComparer> nodes_to_expand;
#endif
	nodes_to_expand.clear();

	/* find the blocking positions of the boxes */
	compute_blocked_positions();

	/* find the doors */
	compute_doors();

	/* find the distances to the target */
	compute_distance_table();

	/* find the winning string */
	string win_string = compute_string_of_solution();

	/* find the number of wall for each cell */
	compute_nb_adjacent_wall();

	/* create the tree of the initial situation */
	SituationTree* initial_tree = new SituationTree(initial_situation->clone());

	/* init the queue with the initial situation */
#if SOLVE_TYPE == TYPE_DFS
	nodes_to_expand.push_back(initial_tree);
#elif SOLVE_TYPE == TYPE_A_STAR
	nodes_to_expand.insert(initial_tree);
#endif

	SituationTree* sol_tree = NULL;
	*was_solution_found = false;
	int nb_examined_nodes = 0;
	int nb_queued_nodes = 0;
	int nb_expanded_nodes = 0;

	/* search for the solution in the tree */
	while (!*was_solution_found) {
		/* if the queue is empty, there is no solution */
		if (nodes_to_expand.size() == 0) {
			*was_solution_found = false;
			break;
		}

		/* get the first element of the queue */
#if SOLVE_TYPE == TYPE_DFS
		SituationTree* curr_tree = nodes_to_expand.front();
		nodes_to_expand.pop_front();
#elif SOLVE_TYPE == TYPE_A_STAR
		SituationTree* curr_tree = *nodes_to_expand.begin();
		nodes_to_expand.erase(nodes_to_expand.begin());
#endif

		//		/* if we have won => quit */
		//		if (curr_tree->node->get_is_won()) {
		//			*was_solution_found = true;
		//			sol_tree = curr_tree;
		//			break;
		//		}

		/* compute the sons of the current situation */
		curr_tree->expand();
		++nb_expanded_nodes;
		nb_examined_nodes += curr_tree->sons.size();

		/* examine each one of the sons */
		for (vector<SituationTree*>::iterator curr_son =
				curr_tree->sons.begin(); curr_son != curr_tree->sons.end(); ++curr_son) {

			//			cout << (*curr_son)->toString(false) << endl;

			/* get the last diff */
			coord box_new_x, box_new_y; //coord of the box that has just benn moved
			direction dir; // direction of the last movement
			(*curr_son)->compute_diff(0, dir, box_new_x, box_new_y);

			/* check if the new situation is blocking */
#if OPTION_PRUNE_BLOCKING
			int cell_index = box_new_y * initial_situation->WIDTH + box_new_x;

			//if the new move is blocking, skip it
			if (is_cell_forbidden[cell_index] == true) {
#if DEBUG
				cout << "!!!Solver: diff:" << (int) dir << " (" << box_new_x
				<< "," << box_new_y << ") is blocking ! Skipping" << endl;
#endif
				// delete the son and skip to the next
				delete *curr_son;
				continue;
			}
#endif

			/* check if the new situation contains a blocking square */
			if (contains_blocking_square((*curr_son)->node, dir, box_new_x,
					box_new_y) == true) {
#if DEBUG
				cout << "!!! This situation contains a blocking square, "
				<< "Skipping" << (*curr_son)->toString(false) << endl;
#endif
				// delete the son and skip to the next
				delete *curr_son;
				continue;
			}

			/* check if we did not see this situation before */
			string curr_string =
					(*curr_son)->node->compute_string_for_comparison();

			// if we saw it before, skip it
			if (situations_seen_or_in_queue.find(curr_string)
					!= situations_seen_or_in_queue.end()) {
#if DEBUG
				cout << "!!!Solver: curr_string '" << curr_string
				<< "' already seen ! Skipping" << endl;
#endif
				// delete the son and skip to the next
				delete *curr_son;
				continue;
			}

			/* if we have won => quit */
			//			cout << "curr_string:" << curr_string << ", win_string:"
			//					<< win_string << endl;
			// check if the string contains the winning string
			if (string_starts_with(curr_string, win_string)) {
				*was_solution_found = true;
				sol_tree = *curr_son;
				break;
			}

#if DEBUG
			cout << "!!!Solver: new curr_string:" << curr_string << endl;
#endif

			/* otherwise, add them to the queue */
#if SOLVE_TYPE == TYPE_DFS
			nodes_to_expand.push_front(*curr_son);
#elif SOLVE_TYPE == TYPE_A_STAR
			// compute the heuristic
			compute_heuristic(*curr_son, dir, box_new_x, box_new_y);
			nodes_to_expand.insert(*curr_son);
#endif
			// add them to the list of seen situations
			++nb_queued_nodes;
			situations_seen_or_in_queue.insert(
					(*curr_son)->node->compute_string_for_comparison());

		} // end of the loop with the sons

		//		cout << "\n\n\nqueue : ";
		//		for (set<SituationTree*, SituationTreeComparer>::iterator theIterator =
		//				nodes_to_expand.begin(); theIterator != nodes_to_expand.end(); theIterator++)
		//			cout << (*theIterator)->toString(false);
		//		cout << "--------------------" << endl << endl;

		// display some random nodes
		if (nb_expanded_nodes % 10000 == 0)
			cout << "curr_tree:" << curr_tree->toString(false) << endl;

		delete curr_tree;
	} // end of while true

	cout << "size of the queue at the end:" << nodes_to_expand.size() << endl;
	cout << "nb_examined_nodes:" << nb_examined_nodes << endl;
	cout << "nb_queued_nodes:" << nb_queued_nodes << endl;
	cout << "nb_expanded_nodes:" << nb_expanded_nodes << endl;
	cout << "was_solution_found:" << *was_solution_found << endl;

	/* cleaning  */
	// delete the queue
#if SOLVE_TYPE == TYPE_DFS
	for (deque<SituationTree*>::iterator curr_tree = nodes_to_expand.begin(); curr_tree
			< nodes_to_expand.end(); ++curr_tree)
#elif SOLVE_TYPE == TYPE_A_STAR
	for (set<SituationTree*, SituationTreeComparer>::iterator curr_tree =
			nodes_to_expand.begin(); curr_tree != nodes_to_expand.end(); ++curr_tree)
#endif
		delete *curr_tree;
	nodes_to_expand.clear();

	/* no solution => return */
	if (*was_solution_found == false)
		return NULL;

	return sol_tree;
}

/*!
 * first solve the problem, then displays the answer
 */
void Solver::solve_and_display(bool display_sons) {
	debug("solve_and_display()");

	bool is_sol_found;
	SituationTree* sol_path = solve(&is_sol_found);

	cout << "Solving OK" << endl;

	if (is_sol_found == false) {
		cout << "No solution found, ending" << endl;
		return;
	}

	// display the final step
	cout << "******************\n*** Final step ***\n******************"
			<< endl;
	cout << sol_path->toString(true) << endl;

	/* display all steps recursively */
	cout << "******************\n*** All steps ****\n******************"
			<< endl;

	if (display_sons) {
		Situation* s = initial_situation->clone();

		for (int i = sol_path->compute_depth() - 1; i >= 0; --i) {
			// read the direction and coord of the last move
			coord end_x, end_y;
			direction dir;
			sol_path->compute_diff(i, dir, end_x, end_y);

			// find the start positions
			coord start_x, start_y;
			direction opp_dir = Situation::opposite_direction(dir);
			s->coord_in_direction(opp_dir, end_x, end_y, start_x, start_y);

			// modify the situation
			s->move_box_to_direction(start_x, start_y, dir);

			cout << s->toString() << endl;
		}
		delete s;
	} else {
		cout
				<< "Skipped. (change the parameter 'display_sons' to true to display !)"
				<< endl;
	}

	delete sol_path;
}

/*!
 * solve and convert the tree to a string for answering to the server
 * @param was_solution_found
 * @return
 */
string Solver::solve_and_convert_to_string(bool* was_solution_found) {
	debug("solve_and_convert_to_string()");

	bool is_sol_found;
	SituationTree* sol_path = solve(&is_sol_found);

	cout << "Solving OK" << endl;

	if (is_sol_found == false) {
		cout << "No solution found, ending" << endl;
		return "";
	}

	// display the final step
	//	cout << "******************\n*** Final step ***\n******************"
	//			<< endl;
	//	cout << sol_path->toString(true) << endl;
	//
	//	/* display all steps recursively */
	//	cout << "******************\n*** All steps ****\n******************"
	//			<< endl;

	/* recursively build the string */
	ostringstream answer;
	Situation* s1 = initial_situation->clone();

	for (int i = sol_path->compute_depth() - 1; i >= 0; --i) {
		// read the direction and coord of the last move
		coord end_x, end_y;
		direction dir;
		sol_path->compute_diff(i, dir, end_x, end_y);

		// find the start positions
		coord start_x, start_y;
		direction opp_dir = Situation::opposite_direction(dir);
		s1->coord_in_direction(opp_dir, end_x, end_y, start_x, start_y);

		// modify the situation
		Situation* s2 = s1->clone_and_move_box_to_direction(start_x, start_y,
				dir);

		// find the transition and it
		string transition = MovementNode::path_string(s1, s2);
		answer << transition;
		//		cout << "transition:" << transition << endl;

		// clean
		delete s1;
		s1 = s2;
	}
	delete s1;
	delete sol_path;

	return answer.str();
}

/*!
 * @return the string which would identify the situation when it is won
 */
string Solver::compute_string_of_solution() {
	debug("Solver::compute_string_of_solution()");

	/* get a solved version of the initial situation
	 * delete every moving box, and fill every slot */
	Situation* sit_solved = initial_situation->clone();
	state* curr_cell = sit_solved->room;
	for (coord i = 0; i < sit_solved->WIDTH * sit_solved->HEIGHT; ++i) {
		if (*curr_cell == Situation::BOX_MOBILE)
			*curr_cell = Situation::EMPTY;
		else if (*curr_cell == Situation::SLOT_EMPTY)
			*curr_cell = Situation::BOX_IN_SLOT;
		++curr_cell;
	}

	/* compute the string */
	string rep = sit_solved->compute_string_for_comparison();
	// remove the position of the man
	int index_dash = rep.find('-');
	rep = rep.substr(0, index_dash);

	//	cout << "initial_situation solved:" << sit_solved->toString() << endl;
	//	cout << "string_of_solution:" << rep << endl;

	delete sit_solved;
	return rep;
}

bool Solver::contains_blocking_square(Situation* s, direction& dir,
		coord& center_x, coord& center_y) {
	debug("Solver::contains_blocking_square(Situation* s)");
	/*
	 // use the following convention
	 // cLU  cU    cRU
	 // cL   new   cR
	 //      ↑ dir (direction of the mov)
	 // <- dir_left
	 // -> dir_right
	 */

	state s_center = s->get_room(center_x, center_y);

	// get the up state
	coord cU_x, cU_y;
	s->coord_in_direction(dir, center_x, center_y, cU_x, cU_y);
	// if we go out of the room, say it is OK
	if (s->is_coord_in_room(cU_x, cU_y) == false)
		return false;
	state sU = s->get_room(cU_x, cU_y);

	/* check the left side */
	for (short orientation = 0; orientation <= 1; ++orientation) {
		// get the coords of the left side
		direction dir_left;
		if (orientation == 0)
			dir_left = Situation::get_dir_plus_quarter(dir);
		else
			dir_left = Situation::get_dir_minus_quarter(dir);
		coord cL_x, cL_y, cLU_x, cLU_y;
		s->coord_in_direction(dir_left, center_x, center_y, cL_x, cL_y);
		s->coord_in_direction(dir_left, cU_x, cU_y, cLU_x, cLU_y);

		// keep them if needed
		if (s->is_coord_in_room(cL_x, cL_y)
				&& s->is_coord_in_room(cLU_x, cLU_y)) {
			state sL = s->get_room(cL_x, cL_y);
			state sLU = s->get_room(cLU_x, cLU_y);

			// find the number of each type
#if OPTION_PRUNE_SQUARES
			short box_mobile_counter = (sU == Situation::BOX_MOBILE ? 1 : 0);
			box_mobile_counter += (sL == Situation::BOX_MOBILE ? 1 : 0);
			box_mobile_counter += (sLU == Situation::BOX_MOBILE ? 1 : 0);
			short wall_counter = (sU == Situation::WALL ? 1 : 0);
			wall_counter += (sL == Situation::WALL ? 1 : 0);
			wall_counter += (sLU == Situation::WALL ? 1 : 0);
			short box_in_slot_counter = (sU == Situation::BOX_IN_SLOT ? 1 : 0);
			box_in_slot_counter += (sL == Situation::BOX_IN_SLOT ? 1 : 0);
			box_in_slot_counter += (sLU == Situation::BOX_IN_SLOT ? 1 : 0);

			if (s_center == Situation::BOX_MOBILE) {
				// if the three neighbours are full, we are blocked
				if (box_mobile_counter + box_in_slot_counter + wall_counter
						== 3) {
#if DEBUG
					cout << s->toString() << endl;
					cout << "Square blocking situation 1 !" << endl;
#endif
					return true;
				}

				// if up and left are wall => corner
				// seen by the corner
			}

			else if (s_center == Situation::BOX_IN_SLOT) {
				if (box_mobile_counter + box_in_slot_counter + wall_counter
						== 3 && box_mobile_counter >= 1) {
#if DEBUG
					cout << s->toString() << endl;
					cout << "Square blocking situation 2 !" << endl;
#endif
					return true;
				}
			} // end if box mobile OR in slot
#endif

			/*
			 * check for the following configuration
			 *   sLUU = Wall            (*)                 (*)
			 *    sLU = BOX       sU =  Forbidden           (*)
			 *       (*)         s_center= BOX Moved         sR=Wall
			 *                           ^
			 */
			bool up_is_forbidden = is_cell_forbidden[cU_y * s->WIDTH + cU_x];
			if (up_is_forbidden) {
				// get the different states
				coord cLUU_x, cLUU_y;
				s->coord_in_direction(dir, cLU_x, cLU_y, cLUU_x, cLUU_y);

				direction dir_right = Situation::opposite_direction(dir_left);
				coord cR_x, cR_y;
				s->coord_in_direction(dir_right, center_x, center_y, cR_x, cR_y);

				if (s->is_coord_in_room(cLUU_x, cLUU_y) && s->is_coord_in_room(
						cR_x, cR_y)) {
					state sLUU = s->get_room(cLUU_x, cLUU_y);
					state sR = s->get_room(cR_x, cR_y);

					// finally look if we are in the situation
					if (sLU == Situation::BOX_MOBILE && sLUU == Situation::WALL
							&& sR == Situation::WALL) {

#if DEBUG
						cout << s->toString() << endl;
						cout << "Square blocking situation 3 !" << endl;
#endif
						return true;
					} // end of the test on the states
				} // end of the "following" config
			} // end up is forbidden


		} // end if left and leftup accessible
	} // end loop directions


	/*
	 * look for the configuration
	 *
	 *  BOX
	 *  BOX
	 * DOOR
	 *   ^
	 */
#if OPTION_PRUNE_DOORS_BLOCKING
	direction opposite_dir = Situation::opposite_direction(dir);
	coord x_man, y_man;
	s->coord_in_direction(opposite_dir, center_x, center_y, x_man, y_man);
	bool man_is_on_door = is_cell_a_door[y_man * initial_situation->WIDTH
			+ x_man];

	coord cUU_x, cUU_y;
	s->coord_in_direction(dir, cU_x, cU_y, cUU_x, cUU_y);

	// check if we are on a door
	if (s->is_coord_in_room(cUU_x, cUU_y) && man_is_on_door) {
		state sUU = s->get_room(cUU_x, cUU_y);

		// does not work with box in slot
		if (sU == Situation::BOX_MOBILE && sUU == Situation::BOX_MOBILE) {
#if DEBUG
			cout << s->toString() << endl;
			cout << "Square blocking situation door !" << endl;
#endif
			return true;
		}
	} // end on check on door
#endif

	return false;
}

void Solver::compute_blocked_positions() {
	debug("Solver::compute_blocked_positions()");

	// reset the array
	bool* ptr = is_cell_forbidden;
	for (int i = 0; i < initial_situation->HEIGHT * initial_situation->WIDTH; ++i)
		*ptr++ = false;

	// First step : checks if the position is in an angle.
	for (coord y = 0; y < initial_situation->HEIGHT; ++y) {
		for (coord x = 0; x < initial_situation->WIDTH; ++x) {
			is_cell_forbidden[y * initial_situation->WIDTH + x] = is_blocking(
					x, y);
		}
	}

	// Checks if the position is blocking in a "U"
	for (coord x = 0; x < initial_situation->WIDTH; ++x) {
		for (coord y = 0; y < initial_situation->HEIGHT; ++y) {
			state curr_state = initial_situation->get_room(x, y);
			if (curr_state == Situation::SLOT_EMPTY || curr_state
					== Situation::BOX_IN_SLOT)
				continue;

			if (!is_cell_forbidden[y * initial_situation->WIDTH + x]) {
				coord end_x1, end_y1, end_x2, end_y2;
				initial_situation->coord_in_direction(Situation::UP, x, y,
						end_x1, end_y1);
				initial_situation->coord_in_direction(Situation::DOWN, x, y,
						end_x2, end_y2);
				if (initial_situation->is_coord_in_room(end_x1, end_y1)
						&& initial_situation->get_room(end_x1, end_y1)
								== Situation::WALL) {
					blocked_in_an_horizontal_U(x, y, Situation::UP);
				} else if (initial_situation->is_coord_in_room(end_x2, end_y2)
						&& initial_situation->get_room(end_x2, end_y2)
								== Situation::WALL) {
					blocked_in_an_horizontal_U(x, y, Situation::DOWN);
				}
				initial_situation->coord_in_direction(Situation::LEFT, x, y,
						end_x1, end_y1);
				initial_situation->coord_in_direction(Situation::RIGHT, x, y,
						end_x2, end_y2);
				if (initial_situation->is_coord_in_room(end_x1, end_y1)
						&& initial_situation->get_room(end_x1, end_y1)
								== Situation::WALL) {
					blocked_in_an_vertical_U(x, y, Situation::LEFT);
				} else if (initial_situation->is_coord_in_room(end_x2, end_y2)
						&& initial_situation->get_room(end_x2, end_y2)
								== Situation::WALL) {
					blocked_in_an_vertical_U(x, y, Situation::RIGHT);
				}
			}
		}
	}
}

/*!
 * compute the heuristic associated with a tree
 * @param tree
 */
void Solver::compute_heuristic(SituationTree* tree, direction& dir,
		coord& box_new_x, coord& box_new_y) {
	if (tree->was_heuristic_computed)
		return;

	/* compute the heuristic, minimum is the best */
	heuristic answer = 50;
	//coord old_x, old_y;
	//direction opp_dir = tree->node->opposite_direction(dir);
	//tree->node->coord_in_direction(opp_dir, box_new_x, box_new_y, old_x, old_y);

	// adding distance table
#if OPTION_HEUR_USE_DISTANCE_TABLE
	answer += 4 * distance_table[box_new_y * initial_situation->WIDTH
			+ box_new_x];
#endif

	//  adding number of empty slots remaining
#if OPTION_HEUR_USE_NB_EMPTY_SLOT
	answer += 5 * tree->node->compute_nb_of_slot_empty();
#endif

	// giving bonuses according to the number of adjacent walls
	// when putting in a slot
	state box_new_state = tree->node->get_room(box_new_x, box_new_y);
	if (box_new_state == Situation::BOX_IN_SLOT) {
#if OPTION_HEUR_USE_PUSH_BOX_IN_SLOT_WITH_WALL
		if (nb_adjacent_wall[box_new_y * initial_situation->WIDTH + box_new_x]
				== 1) {
			answer -= 5;
		} else if (nb_adjacent_wall[box_new_y * initial_situation->WIDTH
				+ box_new_x] >= 2) {
			answer -= 10;
		}
#endif
	}

	// else : the box new position is not in a slot
	else {
#if OPTION_HEUR_USE_DONT_PUSH_AGAINST_WALL
		coord x, y;
		tree->node->coord_in_direction(dir, box_new_x, box_new_y, x, y);
		state state_end_end = tree->node->get_room(x, y);

		if (state_end_end == Situation::WALL || state_end_end
				== Situation::BOX_MOBILE) {

			answer += 10;

			direction dir_tmp = Situation::get_dir_plus_quarter(dir);
			tree->node->coord_in_direction(dir_tmp, box_new_x, box_new_y, x, y);
			state_end_end = tree->node->get_room(x, y);
			if (state_end_end == Situation::WALL || state_end_end
					== Situation::BOX_MOBILE)
				answer += 5;

			dir_tmp = Situation::get_dir_minus_quarter(dir);
			tree->node->coord_in_direction(dir_tmp, box_new_x, box_new_y, x, y);
			state_end_end = tree->node->get_room(x, y);
			if (state_end_end == Situation::WALL || state_end_end
					== Situation::BOX_MOBILE)
				answer += 5;
		}
#endif
	}

	/* set the heuristic */
	tree->heuristic_value = answer;
	tree->was_heuristic_computed = true;
}

bool Solver::is_blocking(coord &x, coord &y) {
	state curr_state = initial_situation->get_room(x, y);

	if (curr_state == Situation::SLOT_EMPTY || curr_state
			== Situation::BOX_IN_SLOT) {
		return false;
	}
	//
	else if (curr_state == Situation::WALL) {
		return true;
	}
	//
	else if (curr_state == Situation::EMPTY || curr_state
			== Situation::BOX_MOBILE) {
		return is_blocked_in_the_corner(x, y);
	}
	return false;
}

// Check if the position is in an angle.
bool Solver::is_blocked_in_the_corner(coord &x, coord &y) {
	state Wall = Situation::WALL;
	coord end_x_u, end_y_u, end_x_d, end_y_d;
	coord end_x_l, end_y_l, end_x_r, end_y_r;
	state state_up = Wall, state_down = Wall, state_left = Wall, state_right =
			Wall;
	bool is_OK_up, is_OK_down, is_OK_left, is_OK_right;

	initial_situation->coord_in_direction(Situation::UP, x, y, end_x_u, end_y_u);
	is_OK_up = initial_situation->is_coord_in_room(end_x_u, end_y_u);
	if (is_OK_up)
		state_up = initial_situation->get_room(end_x_u, end_y_u);

	initial_situation->coord_in_direction(Situation::DOWN, x, y, end_x_d,
			end_y_d);
	is_OK_down = initial_situation->is_coord_in_room(end_x_d, end_y_d);
	if (is_OK_down)
		state_down = initial_situation->get_room(end_x_d, end_y_d);

	initial_situation->coord_in_direction(Situation::LEFT, x, y, end_x_l,
			end_y_l);
	is_OK_left = initial_situation->is_coord_in_room(end_x_l, end_y_l);
	if (is_OK_left)
		state_left = initial_situation->get_room(end_x_l, end_y_l);

	initial_situation->coord_in_direction(Situation::RIGHT, x, y, end_x_r,
			end_y_r);
	is_OK_right = initial_situation->is_coord_in_room(end_x_r, end_y_r);
	if (is_OK_right)
		state_right = initial_situation->get_room(end_x_r, end_y_r);

	if ((is_OK_up && state_up == Wall) || (is_OK_down && state_down == Wall)) {
		if ((is_OK_left && state_left == Wall) || (is_OK_right && state_right
				== Wall)) {
			return true;
		}
	}
	return false;
}

/*!
 * To calculate the blocked position in a horizontal "U"
 * @param x
 * @param y
 * @param dir direction of the wall regarding the call we analyse
 */
void Solver::blocked_in_an_horizontal_U(coord &x, coord &y, direction &dir) {
	bool blockedRight = false, blockedLeft = false, stop = false;
	coord start_x, start_y, end_x, end_y, wall_x, wall_y;
	start_x = x;
	start_y = y;
	while (stop == false && blockedLeft == false) {
		initial_situation->coord_in_direction(Situation::LEFT, start_x,
				start_y, end_x, end_y);
		if (initial_situation->is_coord_in_room(end_x, end_y)) {
			if (initial_situation->get_room(end_x, end_y)
					== Situation::SLOT_EMPTY) {
				stop = true;
			} else if (initial_situation->get_room(end_x, end_y)
					== Situation::EMPTY || initial_situation->get_room(end_x,
					end_y) == Situation::BOX_MOBILE) {
				if (is_cell_forbidden[end_y * initial_situation->WIDTH + end_x]) {
					blockedLeft = true;
				} else {
					initial_situation->coord_in_direction(dir, end_x, end_y,
							wall_x, wall_y);
					if (initial_situation->is_coord_in_room(wall_x, wall_y)
							&& initial_situation->get_room(wall_x, wall_y)
									!= Situation::WALL) {
						stop = true;
					}
				}
			} else if (initial_situation->get_room(end_x, end_y)
					== Situation::WALL) {
				stop = true;
			}
		} else {
			stop = true;
		}
		start_x = end_x;
		start_y = end_y;
	}
	stop = false;
	start_x = x;
	start_y = y;
	while (stop == false && blockedRight == false) {
		initial_situation->coord_in_direction(Situation::RIGHT, start_x,
				start_y, end_x, end_y);
		if (initial_situation->is_coord_in_room(end_x, end_y)) {
			if (initial_situation->get_room(end_x, end_y)
					== Situation::SLOT_EMPTY) {
				stop = true;
			} else if (initial_situation->get_room(end_x, end_y)
					== Situation::EMPTY || initial_situation->get_room(end_x,
					end_y) == Situation::BOX_MOBILE) {
				if (is_cell_forbidden[end_y * initial_situation->WIDTH + end_x]) {
					blockedRight = true;
				} else {
					initial_situation->coord_in_direction(dir, end_x, end_y,
							wall_x, wall_y);
					if (initial_situation->is_coord_in_room(wall_x, wall_y)
							&& initial_situation->get_room(wall_x, wall_y)
									!= Situation::WALL) {
						stop = true;
					}
				}
			} else if (initial_situation->get_room(end_x, end_y)
					== Situation::WALL) {
				stop = true;
			}
		} else {
			stop = true;
		}
		start_x = end_x;
		start_y = end_y;
	}
	if (blockedLeft == true && blockedRight == true) {
		is_cell_forbidden[y * initial_situation->WIDTH + x] = true;
	}
}

/*!
 * To calculate the blocked position in a vertical "U"
 * @param x
 * @param y
 * @param dir direction of the wall regarding the call we analyse
 */
void Solver::blocked_in_an_vertical_U(coord &x, coord &y, direction &dir) {
	bool blockedDown = false, blockedUp = false, stop = false;
	coord start_x, start_y, end_x, end_y, wall_x, wall_y;
	start_x = x;
	start_y = y;
	while (stop == false && blockedUp == false) {
		initial_situation->coord_in_direction(Situation::UP, start_x, start_y,
				end_x, end_y);
		if (initial_situation->is_coord_in_room(end_x, end_y)) {
			if (initial_situation->get_room(end_x, end_y)
					== Situation::SLOT_EMPTY) {
				stop = true;
			} else if (initial_situation->get_room(end_x, end_y)
					== Situation::EMPTY || initial_situation->get_room(end_x,
					end_y) == Situation::BOX_MOBILE) {
				if (is_cell_forbidden[end_y * initial_situation->WIDTH + end_x]) {
					//cout << "up : " << x << " ," << y << "( ";
					//cout << end_x << " ," << end_y << ")"<< endl;
					blockedUp = true;
				} else {
					initial_situation->coord_in_direction(dir, end_x, end_y,
							wall_x, wall_y);
					if (initial_situation->is_coord_in_room(wall_x, wall_y)
							&& initial_situation->get_room(wall_x, wall_y)
									!= Situation::WALL) {
						stop = true;
					}
				}
			} else if (initial_situation->get_room(end_x, end_y)
					== Situation::WALL) {
				stop = true;
			}
		} else {
			stop = true;
		}
		start_x = end_x;
		start_y = end_y;
	}
	stop = false;
	start_x = x;
	start_y = y;
	while (stop == false && blockedDown == false) {
		initial_situation->coord_in_direction(Situation::DOWN, start_x,
				start_y, end_x, end_y);
		if (initial_situation->is_coord_in_room(end_x, end_y)) {
			if (initial_situation->get_room(end_x, end_y)
					== Situation::SLOT_EMPTY) {
				stop = true;
			} else if (initial_situation->get_room(end_x, end_y)
					== Situation::EMPTY || initial_situation->get_room(end_x,
					end_y) == Situation::BOX_MOBILE) {
				if (is_cell_forbidden[end_y * initial_situation->WIDTH + end_x]) {
					blockedDown = true;
				} else {
					initial_situation->coord_in_direction(dir, end_x, end_y,
							wall_x, wall_y);
					if (initial_situation->is_coord_in_room(wall_x, wall_y)
							&& initial_situation->get_room(wall_x, wall_y)
									!= Situation::WALL) {
						stop = true;
					}
				}
			} else if (initial_situation->get_room(end_x, end_y)
					== Situation::WALL) {
				stop = true;
			}
		} else {
			stop = true;
		}
		start_x = end_x;
		start_y = end_y;
	}
	if (blockedUp == true && blockedDown == true) {
		is_cell_forbidden[y * initial_situation->WIDTH + x] = true;
	}
}

void Solver::compute_distance_table() {
	debug("Solver::compute_distance_table()");

	// reset the array
	coord* ptr = distance_table;
	for (int i = 0; i < initial_situation->HEIGHT * initial_situation->WIDTH; ++i)
		*ptr++ = 1000;

	// First step : checks if the position is in an angle.
	for (coord y = 0; y < initial_situation->HEIGHT; ++y) {
		for (coord x = 0; x < initial_situation->WIDTH; ++x) {
			if (initial_situation->is_coord_in_room(x, y)
					&& initial_situation->get_room(x, y)
							== Situation::SLOT_EMPTY)
				recursive_distance_table(x, y, (heuristic) 0);
		}
	}
}

void Solver::recursive_distance_table(coord &x, coord &y, heuristic value) {
	if (initial_situation->is_coord_in_room(x, y)
			&& initial_situation->get_room(x, y) != Situation::WALL) {
		if (distance_table[y * initial_situation->WIDTH + x] > value) {
			distance_table[y * initial_situation->WIDTH + x] = value;
			coord end_x, end_y;
			++value;
			initial_situation->coord_in_direction(Situation::UP, x, y, end_x,
					end_y);
			recursive_distance_table(end_x, end_y, value);
			initial_situation->coord_in_direction(Situation::DOWN, x, y, end_x,
					end_y);
			recursive_distance_table(end_x, end_y, value);
			initial_situation->coord_in_direction(Situation::LEFT, x, y, end_x,
					end_y);
			recursive_distance_table(end_x, end_y, value);
			initial_situation->coord_in_direction(Situation::RIGHT, x, y,
					end_x, end_y);
			recursive_distance_table(end_x, end_y, value);
		}
	}
}

void Solver::compute_nb_adjacent_wall() {
	debug("Solver::compute_nb_adjacent_wall()");

	// reset the array
	short* ptr = nb_adjacent_wall;
	for (int i = 0; i < initial_situation->HEIGHT * initial_situation->WIDTH; ++i)
		*ptr++ = 0;

	coord end_x, end_y;
	short nb;
	// First step : checks if the position is in an angle.
	for (coord y = 0; y < initial_situation->HEIGHT; ++y) {
		for (coord x = 0; x < initial_situation->WIDTH; ++x) {
			nb = 0;
			initial_situation->coord_in_direction(Situation::UP, x, y, end_x,
					end_y);
			if (initial_situation->is_coord_in_room(end_x, end_y)
					&& initial_situation->get_room(end_x, end_y)
							== Situation::WALL) {
				++nb;
			}
			initial_situation->coord_in_direction(Situation::DOWN, x, y, end_x,
					end_y);
			if (initial_situation->is_coord_in_room(end_x, end_y)
					&& initial_situation->get_room(end_x, end_y)
							== Situation::WALL) {
				++nb;
			}
			initial_situation->coord_in_direction(Situation::LEFT, x, y, end_x,
					end_y);
			if (initial_situation->is_coord_in_room(end_x, end_y)
					&& initial_situation->get_room(end_x, end_y)
							== Situation::WALL) {
				++nb;
			}
			initial_situation->coord_in_direction(Situation::LEFT, x, y, end_x,
					end_y);
			if (initial_situation->is_coord_in_room(end_x, end_y)
					&& initial_situation->get_room(end_x, end_y)
							== Situation::WALL) {
				++nb;
			}
			nb_adjacent_wall[y * initial_situation->WIDTH + x] = nb;
		}
	}
}

/*!
 * find the doors in a room, that means the cells part of a corridor
 */
void Solver::compute_doors() {
	debug("Solver::compute_doors()");
	Situation* room_blank = initial_situation->clone();

	/*  first : delete boxes and reset answer */
	state* room_ptr = room_blank->room;
	bool* is_door_ptr = is_cell_a_door;

	for (int j = 0; j < initial_situation->WIDTH * initial_situation->HEIGHT; ++j) {
		if (*room_ptr == Situation::BOX_MOBILE || *room_ptr
				== Situation::BOX_IN_SLOT || *room_ptr == Situation::SLOT_EMPTY)
			*room_ptr = Situation::EMPTY;
		++room_ptr;
		*is_door_ptr++ = false;
	}
	//	cout << "empty room :" << room_blank->toString() << endl;

	/*  second : find if they are doors */
	for (coord j_box = 0; j_box < initial_situation->HEIGHT; ++j_box) {
		for (coord i_box = 0; i_box < initial_situation->WIDTH; ++i_box) {
			//	coord j_box = 3;
			//	coord i_box = 8;

			state curr_state = room_blank->get_room(i_box, j_box);
			//cout << "curr_state is empty:" << (curr_state == Situation::EMPTY) << endl;
			// if the current cell is not empty : skip
			if (curr_state != Situation::EMPTY)
				continue;

			bool has_found_door = false;
			for (coord j_man = 0; j_man < room_blank->HEIGHT; ++j_man) {
				if (has_found_door)
					break;

				for (coord i_man = 0; i_man < room_blank->WIDTH; ++i_man) {
					if (has_found_door)
						break;

					if (room_blank->get_room(i_man, j_man) != Situation::EMPTY)
						continue;

					has_found_door = false;
					room_blank->move_man_to_position(i_man, j_man);

					// find the number of accessible cells in the empty room
					room_blank->compute_accessible_positions();
					int accessible_empty =
							room_blank->nb_accessible_positions();

					room_blank->set_room(i_box, j_box, Situation::BOX_MOBILE);
					room_blank->compute_accessible_positions();
					int accessible_box = room_blank->nb_accessible_positions();

					//					cout << "accessible_empty:" << accessible_empty << endl;
					//					cout << "accessible_box:" << accessible_box << endl;

					// we lose more than one cell -> room
					if (accessible_box < accessible_empty - 1) {
						/* check if left and right are doors, or up and down */

						// get the coords
						coord xL, xR, xU, xD;
						coord yL, yR, yU, yD;
						initial_situation->coord_in_direction(Situation::UP,
								i_box, j_box, xU, yU);
						initial_situation->coord_in_direction(Situation::DOWN,
								i_box, j_box, xD, yD);
						initial_situation->coord_in_direction(Situation::LEFT,
								i_box, j_box, xL, yL);
						initial_situation->coord_in_direction(Situation::RIGHT,
								i_box, j_box, xR, yR);
						bool okL = initial_situation->is_coord_in_room(xL, yL);
						bool okR = initial_situation->is_coord_in_room(xR, yR);
						bool okU = initial_situation->is_coord_in_room(xU, yU);
						bool okD = initial_situation->is_coord_in_room(xD, yD);

						// get the states
						state sU = (okU ? initial_situation->get_room(xU, yU)
								: Situation::EMPTY);
						state sD = (okD ? initial_situation->get_room(xD, yD)
								: Situation::EMPTY);
						state sL = (okL ? initial_situation->get_room(xL, yL)
								: Situation::EMPTY);
						state sR = (okR ? initial_situation->get_room(xR, yR)
								: Situation::EMPTY);
						bool left_right_OK = (sL == Situation::WALL && sR
								== Situation::WALL);
						bool up_down_OK = (sU == Situation::WALL && sD
								== Situation::WALL);

						// if left-right OK, or up-down OK -> room
						if (left_right_OK || up_down_OK) {
							cout << "new room !" << endl;
							cout << room_blank->toString() << endl;
							has_found_door = true;
						}
					}

					room_blank->set_room(i_box, j_box, Situation::EMPTY);
				} // end loop i_man
			} // end loop j_man

			if (has_found_door)
				is_cell_a_door[j_box * initial_situation->WIDTH + i_box] = true;

		} // end loop i_box
	} // end loop j_box

	/* display */
	//	cout << "is_door_cell:" << endl;
	//	for (coord j_box = 0; j_box < initial_situation->HEIGHT; ++j_box) {
	//		for (coord i_box = 0; i_box < initial_situation->WIDTH; ++i_box) {
	//			if (is_cell_a_door[j_box * initial_situation->WIDTH + i_box])
	//				cout << "X";
	//			else
	//				cout << ".";
	//		}
	//		cout << endl;
	//	}

	delete room_blank;
}
