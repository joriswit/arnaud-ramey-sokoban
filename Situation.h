/*
 * Situation.h
 *
 *  Created on: Sep 24, 2009
 *      Author: arnaud
 */

#ifndef SITUATION_H_
#define SITUATION_H_

#define DEBUG 0

// STL imports
#include <stdio.h>  // for exit()
#include <stdlib.h> // for exit()
#include <iostream>
#include <vector> 	// for vectors
#include <set> 		// for sets
#include <deque>	// for double ended queues
#include <fstream> 	// for the input string streams
#include <sstream> 	// for the output string streams
#include <iomanip> 	// for the leading zeros
#include <math.h>	// for log10
using namespace std;

// coord = unsigned short, this is more readable
typedef unsigned short coord;
typedef unsigned short heuristic;
typedef unsigned char state;
typedef unsigned char direction;
typedef std::pair<coord, coord> coord_pair;

/*!
 * the difference between two situations
 */
//class SituationDiff {
//public:
//	direction dir;
//	coord new_x;
//	coord new_y;
//
//	SituationDiff(direction& d, coord& nx, coord& ny) {
//		dir = d;
//		new_x = nx;
//		new_y = ny;
//	}
//
//	/*!
//	 * clone operator
//	 */
//	SituationDiff clone() {
//		SituationDiff rep (dir, new_x, new_y);
//		return rep;
//	}
//
//	string toString() {
//		ostringstream rep;
//		rep << "dir=" << (int) dir << ", new box=(" << new_x << ", " << new_y
//				<< ")";
//		return rep.str();
//	}
//};

class Situation {
public:
	static state EMPTY;
	static state WALL;
	static state SLOT_EMPTY;
	static state BOX_MOBILE;
	static state BOX_IN_SLOT;

	static char EMPTY__CHAR;
	static char WALL__CHAR;
	static char SLOT_EMPTY__CHAR;
	static char BOX_MOBILE__CHAR;
	static char BOX_IN_SLOT__CHAR;
	static char MAN__CHAR;

	/*!
	 * the constants for the diretion
	 */
	static direction UP;
	static direction DOWN;
	static direction LEFT;
	static direction RIGHT;

	Situation(short w, short h, bool clear_room);
	Situation* clone();
	~Situation();

	/*! the size of the room */
	coord WIDTH, HEIGHT;

	/*! the array containing the state of the room */
	state* room;

	/*! the robot position */
	coord man_x, man_y;

	/*! the number of previous moves of boxes */
	//unsigned int nb_box_moves;

	void set_room(coord& x, coord& y, state& new_state);
	state get_room(coord& x, coord& y);

	/* basic stuff */
	void coord_in_direction(direction& dir, coord& start_x, coord& start_y,
			coord& rep_x, coord& rep_y);
	bool is_coord_in_room(coord& x, coord& y);
	bool is_coord_in_room_and_free(coord& x, coord& y);
	bool is_direction_in_room_and_free(coord& start_x, coord& start_y,
			direction& dir);
	void directions_in_room_and_free(coord& start_x, coord& start_y, vector<
			direction>* rep);
	void directions_in_room_and_free(vector<direction>* rep);

	void move_man_to_position(coord& new_x, coord& new_y);
	void move_man_to_direction(direction& dir);

	/* accessible positions */
	bool* accessible_positions;
	bool need_to_compute_accessible_positions;
	void compute_accessible_positions();
	void compute_accessible_positions(coord& start_x, coord& start_y);
	int nb_accessible_positions();

	/* boxes moves */
	bool is_box_movable_to_direction(coord& start_x, coord& start_y,
			direction& dir);
	void move_box_to_direction(coord& start_x, coord& start_y, direction& dir);
	Situation* clone_and_move_box_to_direction(coord& start_x, coord& start_y,
			direction& dir);


	/* strings for equivalency */
	string compute_string_for_comparison();
	bool equivalent(Situation& s2);

	string toString();

	bool get_is_won();
	short compute_nb_of_slot_empty();

	static direction opposite_direction(direction& dir);
	static direction get_dir_plus_quarter(direction &dir);
	static direction get_dir_minus_quarter(direction &dir);

private:
	bool is_won;
	bool need_to_compute_is_won;
	void compute_is_won();
};

inline static void debug(string s) {
#if DEBUG
	cout << "!!!" << s << endl;
#endif
}

#endif /* SITUATION_H_ */
