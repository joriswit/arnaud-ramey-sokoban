/*
 * main.cpp
 *
 *  Created on: Sep 24, 2009
 *      Author: arnaud
 */
#include "Situation.h"
#include "SituationTree.h"
#include "Solver.h"
#include "Movement.h"
#include "IO/IO.h"

/*!
 * tests for the strings
 */
void test_strings() {
  cout << endl << endl << "~~~ test_strings() ~~~" << endl;

  Situation* s = IO::parse_file("samples/sample1.txt");
  cout << "s:" << s->toString() << endl;

  coord x = s->man_x + 1, y = s->man_y;
  s->move_man_to_position(x, y);
  cout << "s1:" << s->toString() << endl;
  cout << "s1.string:" << s->compute_string_for_comparison() << endl << endl;
  x = 4;
  y = 2;
  s->move_box_to_direction(x, y, Situation::LEFT);
  cout << "s2:" << s->toString() << endl;
  cout << "s2.string:" << s->compute_string_for_comparison() << endl << endl;

  delete s;
}

/*!
 * test for communicating with the server
 */
void test_comm_server() {
  cout << endl << endl << "~~~ test_comm_server() ~~~" << endl;

  int connection_id;
  Situation* s = IO::get_board_from_server(&connection_id, 1);
  cout << "s:" << s->toString() << endl;

  string rep = "U R R D U U L D L L U L L D R R R R L D D R U R U D L L U R";
  IO::upload_answer_to_server(connection_id, &rep);

  delete s;
}

/*!
 * tests for moving boxes
 */
void test_moving_boxes() {
  cout << endl << endl << "~~~ test_moving_boxes() ~~~" << endl;

  Situation* s = IO::parse_file("samples/sample1.txt");
  cout << "s:" << s->toString() << endl;

  coord x_b = 5, y_b = 2;
  s->move_box_to_direction(x_b, y_b, Situation::DOWN);
  cout << "s:" << s->toString() << endl;

  y_b = 3;
  s->move_box_to_direction(x_b, y_b, Situation::UP);
  cout << s->toString() << endl << endl;

  delete s;
}

void test_clone() {
  cout << endl << endl << "~~~ test_moving_boxes() ~~~" << endl;

  Situation* s = IO::parse_file("samples/sample1.txt");
  cout << "s:" << s->toString() << endl;

  Situation* s2 = s->clone();
  coord x_b = 5, y_b = 2;
  s2->move_box_to_direction(x_b, y_b, Situation::DOWN);
  cout << "s2:" << s2->toString() << endl;

  delete s;
  delete s2;
}

/*!
 * test for expanding a tree
 */
void test_trees() {
  cout << endl << endl << "~~~ test_trees() ~~~" << endl;

  Situation* s = IO::parse_file("samples/sample1.txt");

  SituationTree* tree = new SituationTree(s);
  tree->diff_with_father = "";
  cout << tree->toString() << endl;

  tree->expand();
  cout << tree->toString() << endl;

  for (unsigned int i = 0; i < tree->sons.size(); ++i) {
    cout << tree->sons.at(i)->diff_with_father << endl;
    delete tree->sons.at(i);
  }
  delete tree;
}

/*!
 * test for the win detection
 */
void test_win() {
  cout << endl << endl << "~~~ test_win() ~~~" << endl;

  Situation* s = IO::parse_file("samples/sample1.txt");
  cout << "s:" << s->toString() << endl;

  coord x = 5, y = 2;
  s->move_box_to_direction(x, y, Situation::RIGHT);
  x = 4;
  s->move_box_to_direction(x, y, Situation::RIGHT);
  x++;
  s->move_box_to_direction(x, y, Situation::UP);
  cout << "s modified:" << s->toString() << endl;
  cout << "is won ? " << s->get_is_won() << endl;

  y--;
  s->move_box_to_direction(x, y, Situation::RIGHT);
  cout << "s modified 2:" << s->toString() << endl;
  cout << "is won ? " << s->get_is_won() << endl;

  delete s;
}

/*!
 * test for the solver
 */
void test_solver_easy() {
  cout << endl << endl << "~~~ test_solver_easy() ~~~" << endl;

  Situation* s = IO::parse_file("samples/sample1.txt");
  //	cout << "s:" << s->toString() << endl;

  coord x = 5, y = 2;
  //	x = 4; // very easy problem
  s->set_room(x, y, Situation::EMPTY);
  x = 6;
  y = 1;
  s->set_room(x, y, Situation::BOX_IN_SLOT);
  cout << "s modified:" << s->toString() << endl;
  cout << "is won ? " << s->get_is_won() << endl;

  Solver* solver = new Solver(s);
  solver->solve_and_display();

  delete solver;
  delete s;
}

/*!
 * test for detecting blocking positions
 */
void test_block() {
  cout << endl << endl << "~~~ test_block() ~~~" << endl;

  int connection_id;
  Situation* s = IO::get_board_from_server(&connection_id, 15);
  //Situation* s = IO::parse_file("samples/sample7.txt");
  cout << "s:" << s->toString() << endl;
  coord x = 4, y = 1;
  cout << "state:" << (int) s->get_room(x, y) << endl;

  Solver* solver = new Solver(s);
  solver->compute_blocked_positions();

  cout << "Result" << endl << endl;
  bool* pointeur = solver->is_cell_forbidden;
  for (coord j = 0; j < solver->initial_situation->HEIGHT; ++j) {
    for (coord i = 0; i < solver->initial_situation->WIDTH; ++i) {
      if (pointeur[j * solver->initial_situation->WIDTH + i] == true) {
        if (solver->initial_situation->get_room(i, j)
            == Situation::WALL)
          cout << "#";
        else
          cout << "+";
      } else {
        cout << " ";
      }
    }
    cout << endl;
  }

  delete solver;
  delete s;
}

void test_movement() {
  cout << endl << endl << "~~~ test_movement() ~~~" << endl;

  Situation* s = IO::parse_file("samples/sample1.txt");
  cout << "s:" << s->toString() << endl;

  coord x = 5, y = 2;
  Situation* s2 = s->clone_and_move_box_to_direction(x, y, Situation::DOWN);
  cout << "s2:" << s2->toString() << endl;

  cout << "MovementNode::path_string(s, s2):" << MovementNode::path_string(s,
                                                                           s2) << endl;
  //
  //	cout << "Sol depth:" << ans.size() << endl;
  //	for (vector<Situation>::iterator it = ans.begin(); it < ans.end(); ++it) {
  //		cout << it->toString() << endl;
  //	}

  delete s;
  delete s2;
}

/*!
 * test for heurisitics
 */
void test_heuristic() {
  cout << endl << endl << "~~~ test_heuristic() ~~~" << endl;

  Situation* s = IO::parse_file("samples/sample4.txt");
  cout << "s:" << s->toString() << endl;

  Solver* solver = new Solver(s);

  solver->compute_distance_table();

  cout << "Result" << endl << endl;
  coord* pointeur = solver->distance_table;
  for (coord j = 0; j < solver->initial_situation->HEIGHT; ++j) {
    for (coord i = 0; i < solver->initial_situation->WIDTH; ++i) {
      if (pointeur[j * solver->initial_situation->WIDTH + i] == 1000) {
        cout << "#";
      } else {
        cout << pointeur[j * solver->initial_situation->WIDTH + i];
      }
    }
    cout << endl;
  }

  solver->compute_nb_adjacent_wall();
  cout << endl;
  cout << endl;
  short* ptr = solver ->nb_adjacent_wall;
  for (coord j = 0; j < solver->initial_situation->HEIGHT; ++j) {
    for (coord i = 0; i < solver->initial_situation->WIDTH; ++i) {
      cout << ptr[j * solver->initial_situation->WIDTH + i];
    }
    cout << endl;
  }

  delete s;
}

/*!
 * tests for double elements in sets
 */
void test_sets() {
  cout << endl << endl << "~~~ test_sets() ~~~" << endl;

  SituationTree* tree1 =
      new SituationTree(IO::parse_file("samples/sample1.txt"));
  tree1->heuristic_value = 10;
  tree1->was_heuristic_computed = true;

  coord x = 5, y = 2;
  SituationTree* tree2 = new SituationTree(
        tree1->node->clone_and_move_box_to_direction(x, y, Situation::UP));
  tree2->heuristic_value = 12;
  tree2->was_heuristic_computed = true;

  SituationTree* tree3 =
      new SituationTree(tree1->node->clone_and_move_box_to_direction(x,
                                                                     y, Situation::DOWN));
  tree3->was_heuristic_computed = true;
  tree3->heuristic_value = 13;

  set<SituationTree*, SituationTreeComparer> set_trees;
  set_trees.insert(tree1);
  set_trees.insert(tree2);
  set_trees.insert(tree3);

  for (set<SituationTree*, SituationTreeComparer>::iterator theIterator =
       set_trees.begin(); theIterator != set_trees.end(); theIterator++) {
    cout << (*theIterator)->toString(true);
  }

  delete tree1;
  delete tree2;
  delete tree3;
}

/*!
 * test to see if everthing is not dying
 * when the initial situation is in fact the solution
 */
void test_solver_init_is_win() {
  cout << endl << endl << "~~~ test_solver_init_is_win() ~~~" << endl;

  Situation* s = IO::parse_file("samples/sample1.txt");
  //	cout << "s:" << s->toString() << endl;

  coord x = 4, y = 2;
  s->set_room(x, y, Situation::EMPTY);
  ++x;
  s->set_room(x, y, Situation::EMPTY);
  x = 6;
  y = 1;
  s->set_room(x, y, Situation::BOX_IN_SLOT);
  ++y;
  s->set_room(x, y, Situation::BOX_IN_SLOT);
  cout << "s modified:" << s->toString() << endl;

  Solver* solver = new Solver(s);
  solver->solve_and_display();

  delete solver;
  delete s;
}

/*!
 * display all the levels in the file 'samples-100-levels.txt'
 */
void test_read_all_levels() {
  cout << endl << endl << "~~~ test_read_all_levels() ~~~" << endl;

  for (int level_id = 1; level_id <= 90; ++level_id) {
    Situation* s = IO::parse_file("samples/samples-100-levels.txt", level_id);
    cout << "Level " << level_id << " : " << s->toString() << endl;
    delete s;
  }
}

void test_squares() {
  cout << endl << endl << "~~~ test_squares() ~~~" << endl;

  //	Situation* s = IO::parse_file("samples/sample6.txt");
  //	Situation* s = IO::parse_file("samples/samples-100-levels.txt", 2);
  int conn_id;
  Situation* s = IO::get_board_from_server(&conn_id, 10);

  cout << "s:" << s->toString() << endl;
  Solver* solver = new Solver(s);
  solver->solve_and_display(false);

  delete solver;
  delete s;
}

void test_doors() {
  cout << endl << endl << "~~~ test_doors() ~~~" << endl;

  //	Situation* s = IO::parse_file("samples/sample6.txt");
  //	Situation* s = IO::parse_file("samples/samples-100-levels.txt", 2);
  int conn_id;
  Situation* s = IO::get_board_from_server(&conn_id, 4);

  cout << "s:" << s->toString() << endl;
  Solver* solver = new Solver(s);
  //	solver->compute_doors();
  solver->solve_and_display(false);

  delete solver;
  delete s;
}

#define INPUT_TYPE_FILE 1
#define INPUT_TYPE_FILE_MULTI 2
#define INPUT_TYPE_SERVER 3

/*!
 * test for the solver
 */
void test_solver(bool get_string_version, int input_type, int level_id = -1,
                 string filename = "") {
  printf("test_solver(get_string_version:%i, input_type:%i, level_id:%i, filename:'%s')\n",
         get_string_version, input_type, level_id, filename.c_str());
  Situation* s = NULL;
  int connection_id;
  // if you want to load a file
  if (input_type == INPUT_TYPE_FILE)
    s = IO::parse_file(filename);
  else if (input_type == INPUT_TYPE_FILE_MULTI)
    s = IO::parse_file(filename, level_id);
  else if (input_type == INPUT_TYPE_SERVER) {
    s = IO::get_board_from_server(&connection_id, level_id);
  }

  cout << s->toString() << endl;

  Solver* solver = new Solver(s);
  //	coord x = 2, y = 2;
  //	s.move_man_to_position(x, y);
  //	cout << "s:" << s.toString() << endl;
  if (get_string_version) {
    bool was_sol_found;
    string string_ans = solver->solve_and_convert_to_string(&was_sol_found);
    cout << "string_ans:" << string_ans << endl;
    if (input_type == INPUT_TYPE_SERVER)
      // try to send the answer :
      IO::upload_answer_to_server(connection_id, &string_ans);
  }

  else
    solver->solve_and_display(false);

  delete solver;
  delete s;
}

void interface() {
  cout << "Which map do you want to resolve (from the server) ?" << endl;
  cout << "1: solve a board from the server" << endl;
  cout << "2: launch a test" << endl;

  cout << "Choice ?";
  int choice;
  cin >> choice;
  if (choice == 1) {
    cout << "Number of the map ?" << endl;
    int numMap;
    cin >> numMap;
    test_solver(numMap, true);
  }

  if (choice == 2) {
    cout << "Which test ?" << endl;
    cout << "1: test_strings();" << endl;
    cout << "2: test_comm_server();" << endl;
    cout << "3: test_moving_boxes();" << endl;
    cout << "4: test_clone();" << endl;
    cout << "5: test_trees();" << endl;
    cout << "6: test_win();" << endl;
    cout << "7: test_block();" << endl;
    cout << "8: test_movement()" << endl;
    cout << "9: test_heuristic();" << endl;
    cout << "10: test_sets();" << endl;
    cout << "11: test_solver_init_is_win();" << endl;
    cout << "12: test_read_all_levels();" << endl;
    cout << "13: test_squares();" << endl;
    cout << "14: test_doors();" << endl;
    cout << "test_solver_easy();" << endl;

    cout << "Choice ?";
    int numTest;
    cin >> numTest;

    if (numTest == 1)
      test_strings();
    if (numTest == 2)
      test_comm_server();
    if (numTest == 3)
      test_clone();
    if (numTest == 4)
      test_trees();
    if (numTest == 5)
      test_win();
    if (numTest == 6)
      test_block();
    if (numTest == 7)
      test_movement();
    if (numTest == 8)
      test_heuristic();
    if (numTest == 9)
      test_sets();
    if (numTest == 10)
      test_solver_init_is_win();
    if (numTest == 11)
      test_read_all_levels();
    if (numTest == 12)
      test_squares();
    if (numTest == 13)
      test_doors();
    if (numTest == 14)
      test_solver_easy();
  } // end if choice == 2
}

/*!
 * tests
 */
int main(int argc, char **argv) {
  debug("main()");

  //	test_strings();
  //	test_comm_server();
  //	test_moving_boxes();
  //	test_clone();
  //	test_trees();
  //	test_win();
  //	test_block();
  //	test_movement();
  //	test_heuristic();
  //	test_sets();
  //	test_solver_init_is_win();
  //	test_read_all_levels();
  //	test_squares();
  //	test_doors();

  //	test_solver_easy();
  //	test_solver(true, INPUT_TYPE_SERVER, 6);
  //test_solver(true, INPUT_TYPE_FILE, -1, "samples/sample2.txt");
  //	test_solver(true, INPUT_TYPE_FILE_MULTI, 1, "samples/samples-100-levels.txt");

  if (argc != 2 && argc != 3) {
    printf("Synopsis: %s LEVELFILE\n", argv[0]);
    printf("or        %s LEVELSFILE LEVELINDEX\n", argv[0]);
    return -1;
  }
  string filename = argv[1];
  if (argc == 2)
    test_solver(true, INPUT_TYPE_FILE, -1, filename);
  else if (argc == 3)
    test_solver(false, INPUT_TYPE_FILE_MULTI, atoi(argv[2]), filename);
  else
    interface();
  return 0;
}
