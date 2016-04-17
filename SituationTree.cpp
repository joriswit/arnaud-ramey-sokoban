/*
 * SituationTree.cpp
 *
 *  Created on: Oct 8, 2009
 *      Author: arnaud
 */

#include "SituationTree.h"
#include <algorithm>

/*!
 * constructor
 * @param n the node (Situation)
 */
SituationTree::SituationTree(Situation* n) {
	this->node = n;
	this->heuristic_value = 0;
	this->was_heuristic_computed = false;
	nb_zeros = (short) (1 + log10(max(node->WIDTH, node->HEIGHT)));
}

/*!
 * destructor
 */
SituationTree::~SituationTree() {
	delete node;
}

/*!
 * expand the sons of the node
 */
void SituationTree::expand() {
	// compute the possible sons of the situation in the node
	sons.clear();
	state* room_ptr = node->room;

	node->compute_accessible_positions();

	for (coord y = 0; y < node->HEIGHT; ++y) {
		for (coord x = 0; x < node->WIDTH; ++x) {
			// if we don't have a box => skip
			if (*room_ptr != Situation::BOX_MOBILE && *room_ptr
					!= Situation::BOX_IN_SLOT) {
				++room_ptr;
				continue;
			}

			//			cout << "x:" << x << ", y:" << y << endl;
			compute_possible_box_son(x, y, Situation::UP);
			compute_possible_box_son(x, y, Situation::DOWN);
			compute_possible_box_son(x, y, Situation::LEFT);
			compute_possible_box_son(x, y, Situation::RIGHT);

			++room_ptr;
		}
	}
}

/*!
 * test if it is possible to move the box in old_* in the direction dir
 * and if it is the case, add it to the answer
 * @param old_x
 * @param old_y
 * @param dir
 */
void SituationTree::compute_possible_box_son(coord& old_x, coord& old_y,
		direction dir) {
	// if we cannot move => return
	if (!node->is_box_movable_to_direction(old_x, old_y, dir))
		return;

	// add the new situation to the answers
	Situation* son_node = node->clone_and_move_box_to_direction(old_x, old_y,
			dir);

	// add the new coord to the list of coords
	coord end_x, end_y;
	node->coord_in_direction(dir, old_x, old_y, end_x, end_y);

	// create the tree
	SituationTree* son_tree = new SituationTree(son_node);

	// copy the history
	ostringstream son_hist;
	son_hist << diff_with_father << (int) dir;
	// add the new movement
	son_hist << setfill('0') << setw(nb_zeros) << end_x << setw(nb_zeros)
			<< end_y;
	son_tree->diff_with_father = son_hist.str();

	// add to sons
	sons.push_back(son_tree);
}

/*!
 * compute from the string the last diff with the dad
 * @param index the index, 0=last
 * @param dir
 * @param newX
 * @param newY
 */
void SituationTree::compute_diff(int index, direction& dir, coord& newX,
		coord& newY) {
	int start = diff_with_father.size() - (index + 1) * (1 + 2 * nb_zeros);
	dir = atoi(diff_with_father.substr(start, 1).c_str());
	newX = atoi(diff_with_father.substr(start + 1, nb_zeros).c_str());
	newY
			= atoi(
					diff_with_father.substr(start + 1 + nb_zeros, nb_zeros).c_str());
}

/*!
 * @return the depth of the tree, in relation with the initial situation
 */
int SituationTree::compute_depth() {
	int size_one_diff = 1 + 2 * nb_zeros;
	return diff_with_father.size() / size_one_diff;
}

/*!
 * the string version
 * @param display_sons true to display the sons
 */
string SituationTree::toString(bool display_sons) {
	ostringstream rep;
	rep << "*** Tree: ***\n" << node->toString() << endl;
	rep << " -> Depth:" << compute_depth() << endl;
	rep << " -> Heuristic:";
	if (was_heuristic_computed)
		rep << heuristic_value << endl;
	else
		rep << "not computed" << endl;

	if (diff_with_father.size() > 0) {
		rep << " -> Diff with father:";
		direction diff_dir;
		coord diff_x, diff_y;
		compute_diff(0, diff_dir, diff_x, diff_y);
		rep << "dir=" << (int) diff_dir << ", new box=(" << diff_x << ", "
				<< diff_y << ")";
		rep << " (full= " << diff_with_father << ")" << endl;
	}

	// recrusively display the sons
	if (display_sons) {
		rep << " -> Sons : ";
		if (sons.size() == 0)
			rep << "no sons" << endl;
		else {
			rep << "(" << sons.size() << ") detail :" << endl;
			for (vector<SituationTree*>::iterator son = sons.begin(); son
					< sons.end(); ++son)
				rep << (*son)->toString() << endl;
		}
	}
	return rep.str();
}
