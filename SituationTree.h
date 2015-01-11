/*
 * SituationTree.h
 *
 *  Created on: Oct 8, 2009
 *      Author: arnaud
 */

#ifndef SITUATIONTREE_H_
#define SITUATIONTREE_H_

#include "Situation.h"

class SituationTree {
public:
	SituationTree(Situation* n);
	~SituationTree();

	Situation *node;
	short nb_zeros;

	string diff_with_father;
	void compute_diff(int index, direction& dir, coord& newX, coord& newY);
	int compute_depth();

	vector<SituationTree*> sons;
	heuristic heuristic_value;
	bool was_heuristic_computed;

	string toString(bool display_sons = true);

	void expand();
	inline void compute_possible_box_son(coord& old_x, coord& old_y, direction dir);
};

class SituationTreeComparer {
public:
	bool operator()(const SituationTree* tree1, const SituationTree* tree2) {
		// if tree1 has a smaller heuristic,
		// it will be at the beginning of the set
		// and so will be prioritary
		return tree1->heuristic_value <= tree2->heuristic_value;
	}
};


#endif /* SITUATIONTREE_H_ */
