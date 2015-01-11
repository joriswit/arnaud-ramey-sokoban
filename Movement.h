#include "Situation.h"
using namespace std;

#ifndef MOVEMENT_H_
#define MOVEMENT_H_

class coordMove {
public:
	coordMove(int a, int b);
	~coordMove();
	int begin;
	int end;

};


//the node classe define a cell in the algorithm for the shortest path between two points
class MovementNode {
public:
	//the final functions
	void Path(Situation *sit1, Situation *sit2, vector<direction>* ans);
	static string path_string(Situation *sit1, Situation *sit2);

private:
	class MovementNode_comparer {
	public:
		bool operator()(const MovementNode* lhs, const MovementNode* rhs) const {
			return lhs->heurist < rhs->heurist;
		}
	};

	int coordo;

	//heuristic = cost + constant*manhattan distance
	int heurist;

	//here, the cost is the number of movements to reach the cell
	int cost;

	//the previous node to reach this cell
	MovementNode* father;

	MovementNode();
	MovementNode(int coo, int cou, int heu);
	MovementNode(int coo, int cou, int heu, MovementNode* fath);
	~MovementNode();

	//will return the pair of coordinates of the previous position of the moved box, and its new position
	coordMove CalcPosition(Situation *sit1, Situation *sit2);

	//calculate the f function between two positions (basically the manhattan distance
	int heuristic(int coord1, int coord2, Situation *sit1);

	//Astar calculate the vector of node leading from node1 to node2
	//Basically, there are two lists : an openlist and a closedlist
	//the openlist is sorted by value of heuristic
	//the algorithm opens the node with the lowest heuristic in the openlist, check all of its neighbours (if there are soon in the closedlist, or in the openlist
	//if there are in the closedlist with a lowest cost, nothing is done
	//if there are in the openlist with a lowest heuristic, nothing is one
	//otherwise, they are added/remplaced, indicating their father (the current node)
	//then the opened node is put in the closedlist
	//we go on opening nodes from the sorted openlist, until the first node is the destination
	void Astar(MovementNode *node1, MovementNode *node2, Situation *sit1,
			deque<MovementNode*> *result);
	multiset<MovementNode*, MovementNode_comparer> openlist;
	vector<MovementNode*> closedlist;

	//newNode check if the current node is :
	//-soon in the closedlist, and with a lowest cost
	//-soon in the openlist, with a lowest heuristic
	void newNode(MovementNode *newn, vector<MovementNode*> *closedlist0,
			multiset<MovementNode*, MovementNode_comparer> *openlist0, MovementNode *begin);

	/* string version */
	string toString();

};



#endif /*MOVEMENT_H_ */
