#include "Movement.h"

coordMove::coordMove(int a, int b) {
	debug("coordMove::constructor");
	begin = a;
	end = b;
}

coordMove::~coordMove() {
	debug("coordMove::destructor");
}

coordMove MovementNode::CalcPosition(Situation *sit1, Situation *sit2) {
	debug("MovementNode::CalcPosition()");

	coord box_sit1_x = 0, box_sit2_x = 0, box_sit1_y = 0, box_sit2_y = 0;
	for (coord i = 0; i < sit1->WIDTH; i++) {
		for (coord j = 0; j < sit1->HEIGHT; j++) {
			coord cell1 = sit1->get_room(i, j);
			coord cell2 = sit2->get_room(i, j);
			if (cell1 != cell2) {
				if (cell1 == sit1->BOX_MOBILE || cell1 == sit1->BOX_IN_SLOT) {
					box_sit1_x = i;
					box_sit1_y = j;
				}
				if (cell2 == sit2->BOX_MOBILE || cell2 == sit2->BOX_IN_SLOT) {
					box_sit2_x = i;
					box_sit2_y = j;
				}
			}
		}
	}

	int box_sit1_coord = ((int) box_sit1_y) * sit1->WIDTH + (int) box_sit1_x;
	int box_sit2_coord = ((int) box_sit2_y) * sit1->WIDTH + (int) box_sit2_x;
	return coordMove(box_sit1_coord, box_sit2_coord);
}

MovementNode::MovementNode() {
	debug("MovementNode::constructor 1");
	this->coordo = 0;
	this->cost = 0;
	this->heurist = 0;
	this->father = NULL;
}

MovementNode::MovementNode(int coo, int cou, int heu) {
	debug("MovementNode::constructor 2");
	this->coordo = coo;
	this->cost = cou;
	this->heurist = heu;
	this->father = NULL;
}

MovementNode::MovementNode(int coo, int cou, int heu, MovementNode* fath) {
	debug("MovementNode::constructor 3");
	this->coordo = coo;
	this->cost = cou;
	this->heurist = heu;
	this->father = fath;
}

/*!
 * destructor
 */
MovementNode::~MovementNode() {
	debug("MovementNode::destructor");
}

int MovementNode::heuristic(int coord1, int coord2, Situation *sit1) {
	debug("MovementNode::heuristic()");
	coord x1 = coord1 / sit1->WIDTH;
	coord y1 = coord1 % sit1->WIDTH;
	coord x2 = coord2 / sit1->WIDTH;
	coord y2 = coord2 % sit1->WIDTH;

	int res = (int) (abs(x1 - x2) + abs(y1 - y2));

	//	cout << "coord1:" << coord1 << endl;
	//	cout << "heuristic evaluated node:" << res << endl;
	return res;
}

void MovementNode::Astar(MovementNode *node1, MovementNode *node2,
		Situation *sit1, deque<MovementNode*> *result) {
	debug("MovementNode::Astar()");
	//	cout << "currentnode (start):" << node1->toString() << endl;
	//	cout << "destination (start):" << node2->toString() << endl;

	result->clear();
	openlist.clear();
	closedlist.clear();
	openlist.insert(node1);
	// init at null
	MovementNode* current_node = NULL;

	while (current_node == NULL || current_node->coordo != node2->coordo) {
		current_node = *openlist.begin();
		openlist.erase(openlist.begin());
		coord current_x = current_node->coordo % sit1->WIDTH;
		coord current_y = current_node->coordo / sit1->WIDTH;

		/* display */
		//		cout << endl << endl;
		//		cout << "*** openlist :" << endl;
		//		cout << " - size :" << openlist.size() << endl;
		//		for (multiset<Node*, classcomp>::iterator it = openlist.begin(); it
		//				!= openlist.end(); ++it)
		//			cout << " - " << (*it)->toString() << endl;
		//		cout << "current_node:" << current_node->toString() << endl;


		for (int dir_int = 0; dir_int < 4; ++dir_int) {
			//			cout << endl << "dir_int:" << dir_int << endl;

			direction curr_dir;
			if (dir_int == 0)
				curr_dir = Situation::UP;
			if (dir_int == 1)
				curr_dir = Situation::DOWN;
			if (dir_int == 2)
				curr_dir = Situation::LEFT;
			if (dir_int == 3)
				curr_dir = Situation::RIGHT;

			if (sit1->is_direction_in_room_and_free(current_x, current_y,
					curr_dir)) {
				//			cout << "currentcoord" << current_node->coordo << endl;
				coord new_x, new_y;
				sit1->coord_in_direction(curr_dir, current_x, current_y, new_x,
						new_y);
				int new_coord = new_y * sit1->WIDTH + new_x;
				MovementNode *new_node_1 = new MovementNode( //
						new_coord, //
						current_node->cost + 1, //
						current_node->cost + 1 + heuristic(new_x, new_y, sit1), //
						current_node//
						);

				newNode(new_node_1, &closedlist, &openlist, node1);

				/* display the open list */
				//				cout << "*** openlist :" << endl;
				//				cout << " - size :" << openlist.size() << endl;
				//				for (multiset<Node*, classcomp>::iterator it = openlist.begin(); it
				//						!= openlist.end(); ++it)
				//					cout << " - " << (*it)->toString() << endl;
			};
		} // end loop dir_int

		closedlist.push_back(current_node);
	}

	/* some display */
	//	cout << "while 1 finished" << endl;
	//	cout << "building path : " << endl;
	//	cout << " - currentnode : " << current_node->toString() << endl;
	//	cout << " - node1 : " << node1->toString() << endl;
	//	cout << " - node2 : " << node2->toString() << endl;
	//	sleep(1);

	// find back the path from Node2 to node1
	while (current_node->coordo != node1->coordo) {
		result->push_front(current_node);
		current_node = (current_node->father);
		//		cout << " -> current_node:" << current_node->toString() << endl;
		//		sleep(2);
	}

}

void MovementNode::newNode(MovementNode *newn,
		vector<MovementNode*> *closedlist0, multiset<MovementNode*,
				MovementNode_comparer> *openlist0, MovementNode *begin) {
	debug("MovementNode::newNode()");

	/* display closed list */
	//	cout << "closed list coords:";
	//	for (vector<Node*>::iterator it_closed = closedlist0->begin(); it_closed
	//			< closedlist0->end(); ++it_closed)
	//		cout << (*it_closed)->coordo << ", ";
	//	cout << endl;

	/* search in the closed list */
	bool found_in_closed = false;
	for (vector<MovementNode*>::iterator it_closed = closedlist0->begin(); it_closed
			< closedlist0->end(); ++it_closed) {
		if ((*it_closed)->coordo == newn->coordo && (*it_closed)->cost
				< newn->cost) {
			found_in_closed = true;
			break;
		}
	}
	//	cout << "node found_in_closed ? " << found_in_closed << endl;

	/* if not found, search in the open list */
	bool found_in_open = false;
	if (!found_in_closed) {

		// search the element in the open list
		for (multiset<MovementNode*, MovementNode_comparer>::iterator it =
				openlist0->begin(); it != openlist0->end(); ++it) {
			if ((*it)->coordo == newn->coordo) {
				if ((*it)->heurist > newn->heurist) {
					(*it)->heurist = newn->heurist;
					(*it)->father = newn->father;
					found_in_open = true;

					//cout << "updating an old node:" << (*it)->toString() << endl;
				}
			}
		}

		if (found_in_open == false) {
			//			cout << "inserting newn, newn=" << newn->toString() << endl;
			openlist0->insert(newn);
		}
	}

	if (found_in_closed || found_in_open)
	// we can delete newn
		delete newn;
}

direction convert_coord_diffs_to_direction(int begin_coordo, int end_coordo,
		int w) {
	coord end_x = end_coordo % w;
	coord end_y = end_coordo / w;
	coord begin_x = begin_coordo % w;
	coord begin_y = begin_coordo / w;
	int diff_x = end_x - begin_x;
	int diff_y = end_y - begin_y;

	// determine the corresponding direction
	direction curr_dir;
	if (diff_x == 0)
		curr_dir = (diff_y == -1 ? Situation::UP : Situation::DOWN);
	else
		curr_dir = (diff_x == -1 ? Situation::LEFT : Situation::RIGHT);
	//		cout << "curr_dir:" << (int) curr_dir << endl;
	return curr_dir;
}

void MovementNode::Path(Situation *sit1, Situation *sit2,
		vector<direction>* ans) {
	debug("MovementNode::Path()");

	coordMove box_move = CalcPosition(sit1, sit2);
	coord man_dest_coord = 2 * (box_move.begin) - box_move.end;

	/* display */
	//	cout << "sit1:" << sit1->toString() << endl;
	//	cout << "sit2:" << sit2->toString() << endl;
	//	cout << "man_dest_coord:" << man_dest_coord << endl;

	MovementNode *node1 = new MovementNode(sit1->man_y * sit1->WIDTH
			+ sit1->man_x, 0, 0);
	MovementNode *node2 = new MovementNode((int) man_dest_coord, 0, 0);
	deque<MovementNode*> path;
	path.clear();
	Astar(node1, node2, sit1, &path);

	/* display path */
	//	cout << "*** path:" << endl;
	//	for (deque<MovementNode*>::iterator it = path.begin(); it < path.end(); ++it) {
	//		cout << " -> " << (*it)->toString() << endl;
	//	}

	/*
	 * compute the directions
	 */
	ans->clear();
	for (deque<MovementNode*>::iterator it = path.begin(); it < path.end(); ++it) {
		// determine the differences in x and y
		coord end_coordo = (*it)->coordo, begin_coordo = (*it)->father->coordo;
		direction curr_dir = convert_coord_diffs_to_direction(begin_coordo,
				end_coordo, sit1->WIDTH);
		// push the result
		ans->push_back(curr_dir);
	}

	/* add the direction between the final state and sit2 */
	direction last_dir = convert_coord_diffs_to_direction(box_move.begin,
			box_move.end, sit1->WIDTH);
	//	cout << "last_dir:" << (int) last_dir << endl;
	ans->push_back(last_dir);

	/* clean the useless nodes */
	set<MovementNode*> to_clean;
	to_clean.insert(openlist.begin(), openlist.end());
	to_clean.insert(closedlist.begin(), closedlist.end());
	to_clean.insert(node1);
	to_clean.insert(node2);
	//	cout << "to clean:" << endl;
	for (set<MovementNode*>::iterator it = to_clean.begin(); it
			!= to_clean.end(); ++it) {
		//		cout << " ->" << (*it)->toString() << endl;
		delete *it;
	}
}

string MovementNode::path_string(Situation *sit1, Situation *sit2) {
	debug("MovementNode::path_string()");

	// first find directions
	MovementNode* n = new MovementNode();
	vector<direction> directions;
	n->Path(sit1, sit2, &directions);
	delete n;

	// second : convert to string buffer
	ostringstream buffer;
	for (vector<direction>::iterator curr_dir = directions.begin(); curr_dir
			< directions.end(); ++curr_dir) {
		if (*curr_dir == Situation::UP)
			buffer << "U ";
		else if (*curr_dir == Situation::DOWN)
			buffer << "D ";
		else if (*curr_dir == Situation::LEFT)
			buffer << "L ";
		else
			buffer << "R ";
	}

	// third : convert to string
	return buffer.str();
}

string MovementNode::toString() {
	ostringstream rep;
	rep << "coordo :" << coordo;
	rep << ", heurist :" << heurist;
	rep << ", cout :" << cost;
	rep << ", father->coordo :" << (father == NULL ? -1 : father->coordo);
	return rep.str();
}

