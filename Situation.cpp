/*
 * Situation.cpp
 *
 *  Created on: Sep 24, 2009
 *      Author: arnaud
 */

#include "Situation.h"

state Situation::EMPTY = 0;
state Situation::WALL = 1;
state Situation::SLOT_EMPTY = 2;
state Situation::BOX_MOBILE = 3;
state Situation::BOX_IN_SLOT = 4;

state Situation::UP = 1;
state Situation::DOWN = 2;
state Situation::LEFT = 3;
state Situation::RIGHT = 4;

char Situation::EMPTY__CHAR = ' ';
char Situation::WALL__CHAR = '#';
char Situation::SLOT_EMPTY__CHAR = '.';
char Situation::BOX_MOBILE__CHAR = '$';
char Situation::BOX_IN_SLOT__CHAR = 'X';
char Situation::MAN__CHAR = '@';

/*!
 * constructor
 * @param w the width
 * @param h the height
 * @return
 */
Situation::Situation(short w, short h, bool clear_room) {
	debug("Situation::constructor()");
	WIDTH = w;
	HEIGHT = h;
	room = new state[w * h];

	// fill room
	state* room_ptr = room;
	for (int i = 0; i < w * h; ++i)
		*room_ptr++ = EMPTY;

	accessible_positions = new bool[WIDTH * HEIGHT];
	need_to_compute_accessible_positions = true;
	need_to_compute_is_won = true;
}

/*!
 * constructor with a clone operation
 * @return a copy of the situation
 */
Situation* Situation::clone() {
	debug("Situation::clone()");
	// make a empty answer
	Situation* rep = new Situation(WIDTH, HEIGHT, false);
	// copy the room
	state *this_room = room, *rep_room = rep->room;
	for (int i = 0; i < WIDTH * HEIGHT; ++i) {
		*rep_room = *this_room;
		++this_room;
		++rep_room;
	}
	// copy the man position
	rep->man_x = man_x;
	rep->man_y = man_y;

	return rep;
}

/*!
 * the destructor
 */
Situation::~Situation() {
	debug("Situation::destructor()");
	delete[] room;
	delete[] accessible_positions;
}

/*!
 * set a case of the room
 * @param x
 * @param y
 * @param new_state
 */
void Situation::set_room(coord& x, coord& y, state& new_state) {
	room[y * WIDTH + x] = new_state;
	need_to_compute_accessible_positions = true;
	need_to_compute_is_won = true;
}

/*!
 * get a case of the room
 * @param x
 * @param y
 * @return
 */
state Situation::get_room(coord& x, coord& y) {
	return room[y * WIDTH + x];
}

/*!
 * get the new coordinates when you move from start in the direction dir
 * @param dir
 * @param start_x
 * @param start_y
 * @param rep_x
 * @param rep_y
 */
void Situation::coord_in_direction(direction& dir, coord& start_x,
		coord& start_y, coord& rep_x, coord& rep_y) {
	rep_x = start_x + (dir == LEFT ? -1 : (dir == RIGHT ? +1 : 0));
	rep_y = start_y + (dir == UP ? -1 : (dir == DOWN ? +1 : 0));
}

/*!
 * @param x
 * @param y
 * @return true if the coord are in the room
 */
bool Situation::is_coord_in_room(coord& x, coord& y) {
	if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
		return false;
	return true;
}

/*!
 * @param x
 * @param y
 * @return true if the coord are in the room
 */
bool Situation::is_coord_in_room_and_free(coord& x, coord& y) {
	if (!is_coord_in_room(x, y))
		return false;

	// check if the destination is clean
	state end_state = get_room(x, y);
	if (end_state != EMPTY && end_state != SLOT_EMPTY)
		return false;

	return true;
}

/*!
 * @param dir
 * @return true if the man can move in the direction dir from start
 */
bool Situation::is_direction_in_room_and_free(coord& start_x, coord& start_y,
		direction& dir) {
	// watch if we got out of the room
	coord end_x, end_y;
	coord_in_direction(dir, start_x, start_y, end_x, end_y);
	if (!is_coord_in_room_and_free(end_x, end_y))
		return false;

	// we pass all the tests => return OK
	return true;
}

/*!
 * @param start_x
 * @param start_y
 * @param rep the answer, it will contain all the directions allowed moving from start
 */
void Situation::directions_in_room_and_free(coord& start_x, coord& start_y,
		vector<direction>* rep) {
	rep->clear();
	if (is_direction_in_room_and_free(start_x, start_y, UP))
		rep->push_back(UP);
	if (is_direction_in_room_and_free(start_x, start_y, DOWN))
		rep->push_back(DOWN);
	if (is_direction_in_room_and_free(start_x, start_y, LEFT))
		rep->push_back(LEFT);
	if (is_direction_in_room_and_free(start_x, start_y, RIGHT))
		rep->push_back(RIGHT);
}

/*!
 * @param rep the answer, it will contain all the directions allowed
 */
void Situation::directions_in_room_and_free(vector<direction>* rep) {
	directions_in_room_and_free(man_x, man_y, rep);
}

/*!
 *move the man to a new position
 * @param new_x
 * @param new_y
 */
void Situation::move_man_to_position(coord& new_x, coord& new_y) {
	man_x = new_x;
	man_y = new_y;
	need_to_compute_accessible_positions = true;
}

/*!
 * move the robot in the chosen direction, and if needed moves the boxes
 * @param dir
 */
void Situation::move_man_to_direction(direction& dir) {
	// find the end position
	coord end_x, end_y;
	coord_in_direction(dir, man_x, man_y, end_x, end_y);

	// if there is a box at the end, move it
	state end_state = get_room(end_x, end_y);
	if (end_state == BOX_MOBILE || end_state == BOX_IN_SLOT)
		move_box_to_direction(end_x, end_y, dir);

	// move the man to this position
	move_man_to_position(end_x, end_y);
}

/*!
 * move a box in a given direction.
 * There is no check about the legitimity of this action !
 *
 * @param start_x the position of the box
 * @param start_y the position of the box
 * @param dir the direction where you want to move the box
 */
void Situation::move_box_to_direction(coord& start_x, coord& start_y,
		direction& dir) {

	state old_start_state = get_room(start_x, start_y);

	// find where we go
	coord end_x, end_y;
	coord_in_direction(dir, start_x, start_y, end_x, end_y);
	state old_end_state = get_room(end_x, end_y);

	// if the end position is a slot, put the box in the slot
	if (old_end_state == SLOT_EMPTY) {
		set_room(end_x, end_y, BOX_IN_SLOT);
		need_to_compute_is_won = true;
	} else
		set_room(end_x, end_y, BOX_MOBILE);

	// clear the old position
	set_room(start_x, start_y, (old_start_state == BOX_IN_SLOT ? SLOT_EMPTY
			: EMPTY));

	// move the man to this position
	move_man_to_position(start_x, start_y);
}

/*!
 * make the same as move_box_to_direction,
 * but make it on a clone of the current situation
 * and return this clone
 * @param start_x
 * @param start_y
 * @param dir
 * @return
 */
Situation* Situation::clone_and_move_box_to_direction(coord& start_x,
		coord& start_y, direction& dir) {
	Situation* rep = clone();
	rep->move_box_to_direction(start_x, start_y, dir);
	return rep;
}

/*!
 * compute the accessible positions from the man position
 */
void Situation::compute_accessible_positions() {
	compute_accessible_positions(man_x, man_y);
}

/*!
 * @return the number of accessible positions from
 * the position of the man
 */
int Situation::nb_accessible_positions() {
	compute_accessible_positions();
	bool* current_accessible_pos = accessible_positions;
	int ans = 0;
	for (int i = 0; i < WIDTH*HEIGHT; ++i) {
		ans += *current_accessible_pos;
		++current_accessible_pos;
	}
	return ans;
}

/*!
 * compute the accessible positions from start.
 * The result is stored in accessible_positions !
 * @param start_x
 * @param start_y
 */
void Situation::compute_accessible_positions(coord& start_x, coord& start_y) {
	if (!need_to_compute_accessible_positions)
		return;

	debug("Situation::compute_accessible_positions()");

	// reset accessible positions : set the ans to false everywhere
	bool* ans_ptr = accessible_positions;
	for (int i = 0; i < WIDTH * HEIGHT; ++i)
		*ans_ptr++ = false;

	// prepare the queue
	deque<coord_pair> queue;
	queue.push_back(coord_pair(start_x, start_y));

	/* while there are some cells in the queue, add their neighbours */
	while (queue.size() > 0) {
		coord_pair current_pos = queue.front();
		queue.pop_front();

		bool* current_state = &accessible_positions[current_pos.second * WIDTH
				+ current_pos.first];

		//cout << "current_pos:" << current_pos.first << "," << current_pos.second << endl;

		// if we already made this node => skip
		if (*current_state == true)
			continue;

		// change the state of this cell
		*current_state = true;

		// add the accessible neighbours
		coord dest_x, dest_y;

		coord_in_direction(UP, current_pos.first, current_pos.second, dest_x,
				dest_y);
		if (is_coord_in_room_and_free(dest_x, dest_y))
			queue.push_back(coord_pair(dest_x, dest_y));

		coord_in_direction(DOWN, current_pos.first, current_pos.second, dest_x,
				dest_y);
		if (is_coord_in_room_and_free(dest_x, dest_y))
			queue.push_back(coord_pair(dest_x, dest_y));

		coord_in_direction(LEFT, current_pos.first, current_pos.second, dest_x,
				dest_y);
		if (is_coord_in_room_and_free(dest_x, dest_y))
			queue.push_back(coord_pair(dest_x, dest_y));

		coord_in_direction(RIGHT, current_pos.first, current_pos.second,
				dest_x, dest_y);
		if (is_coord_in_room_and_free(dest_x, dest_y))
			queue.push_back(coord_pair(dest_x, dest_y));
	}

	/* display */
	//#if DEBUG
	//	ans_ptr = accessible_positions;
	//	cout << "** Accessible cells :" << endl;
	//	for (coord y = 0; y < HEIGHT; ++y) {
	//		for (coord x = 0; x < WIDTH; ++x)
	//			cout << (*ans_ptr++ ? "1" : ".");
	//		cout << endl;
	//	}
	//#endif

	need_to_compute_accessible_positions = false;
}

direction Situation::opposite_direction(direction& dir) {
	if (dir == Situation::LEFT)
		return Situation::RIGHT;
	else if (dir == Situation::RIGHT)
		return Situation::LEFT;
	else if (dir == Situation::UP)
		return Situation::DOWN;
	else if (dir == Situation::DOWN)
		return Situation::UP;

	cout << "Ohoh, impossible to return the opposite of " << dir << endl;
	return -1;
}

/*!
 * check if a box is movable in a given direction.
 *
 * @param start_x the position of the box
 * @param start_y the position of the box
 * @param dir the direction where you want to move the box
 */
bool Situation::is_box_movable_to_direction(coord& start_x, coord& start_y,
		direction& dir) {
	/* find if the movement of the box is allow by the environment */
	// find where we go
	coord end_x, end_y;
	coord_in_direction(dir, start_x, start_y, end_x, end_y);
	// if we go out of the room => return false
	if (!is_coord_in_room(start_x, start_y))
		return false;

	// if the end position is not clear => return false
	state end_state = get_room(end_x, end_y);
	if (end_state != EMPTY && end_state != SLOT_EMPTY)
		return false;

	/* check if the opposite direction is accessible and free */
	direction opposite_dir = opposite_direction(dir);
	coord opposite_x, opposite_y;
	coord_in_direction(opposite_dir, start_x, start_y, opposite_x, opposite_y);

	// if the opposite is outside => return false
	if (!is_coord_in_room(opposite_x, opposite_y))
		return false;
	// if there is no way to push this box => return false
	if (!accessible_positions[opposite_y * WIDTH + opposite_x])
		return false;

	// otherwise => true
	return true;
}

/*!
 * @return a string representing the state of the room
 * 2 situations are equivalent if they have the same string
 */
string Situation::compute_string_for_comparison() {
	debug("Situation::compute_string_for_comparison()");
	ostringstream rep;

	// add the information about the boxes
	state* curr = room;
	short nb_zeros = (short) (1 + log10(WIDTH * HEIGHT));

	for (int i = 0; i < WIDTH * HEIGHT; ++i) {
		if (*curr == BOX_MOBILE || *curr == BOX_IN_SLOT)
			rep << setfill('0') << setw(nb_zeros) << i;
		++curr;
	}

	// add the information about the position of the man
	// we need the accessible cells,
	compute_accessible_positions();
	rep << '-';

	// we only add the first accessible cell (2009/10/09)
	bool* acc_ptr = accessible_positions;
	for (int i = 0; i < WIDTH * HEIGHT; ++i) {
		if (*acc_ptr++) {
			rep << setfill('0') << setw(nb_zeros) << i;
			break;
		}
	}

	return rep.str();
}

/*!
 * @param s2 another situation
 * @return true if the situations are equivalent
 */
bool Situation::equivalent(Situation& s2) {
	if (compute_string_for_comparison().compare(
			s2.compute_string_for_comparison()) == 0)
		return true;
	return false;
}

/*!
 * @return a string representation of the box
 */
string Situation::toString() {
	ostringstream rep;

	rep << "-> Situation, w=" << WIDTH << ", h=" << HEIGHT << endl;

	state* curr = room;
	for (int i = 0; i < WIDTH * HEIGHT; ++i) {
		/* add the good character */
		// is it the man ?
		if (i / WIDTH == man_y && i % WIDTH == man_x)
			rep << MAN__CHAR;

		// otherwise : what is it ?
		else if (*curr == EMPTY)
			rep << EMPTY__CHAR;
		else if (*curr == WALL)
			rep << WALL__CHAR;
		else if (*curr == SLOT_EMPTY)
			rep << SLOT_EMPTY__CHAR;
		else if (*curr == BOX_IN_SLOT)
			rep << BOX_IN_SLOT__CHAR;
		else if (*curr == BOX_MOBILE)
			rep << BOX_MOBILE__CHAR;

		// jump a line at the end of a row
		int i_next = i + 1;
		if (i_next % WIDTH == 0 && i_next < WIDTH * HEIGHT)
			rep << endl;
		curr++;
	}

	return rep.str();
}

/*!
 * @return true if the game is won
 */
bool Situation::get_is_won() {
	if (need_to_compute_is_won == true) {
		compute_is_won();
	}
	return is_won;
}

void Situation::compute_is_won() {
	need_to_compute_is_won = false;
	state* room_ptr = room;

	for (int y = 0; y < HEIGHT * WIDTH; ++y) {
		if (*room_ptr == BOX_MOBILE) {
			is_won = false;
			return;
		}
		++room_ptr;
	}
	is_won = true;
}

short Situation::compute_nb_of_slot_empty() {
	short answer = 0;
	state* room_ptr = room;

	for (int y = 0; y < HEIGHT * WIDTH; ++y) {
		if (*room_ptr == SLOT_EMPTY) {
			++answer;
		}
		++room_ptr;
	}
	return answer;
}

direction Situation::get_dir_plus_quarter(direction &dir){
	if(dir==UP)
		return LEFT;
	else if(dir==LEFT)
		return DOWN;
	else if(dir==DOWN)
		return RIGHT;
	else
		return UP;
}

direction Situation::get_dir_minus_quarter(direction &dir){
	if(dir==UP)
		return RIGHT;
	else if(dir==LEFT)
		return UP;
	else if(dir==DOWN)
		return LEFT;
	else
		return DOWN;
}
