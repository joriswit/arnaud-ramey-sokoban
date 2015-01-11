#include "GUI.h"

/*!
 * test the construction of images
 */
void test_images() {
  debug("test_images()");

  // Define a 640x400 color image with 8 bits per color component.
  CImg<unsigned char> img(640, 400, 1, 3);
  // Set pixel values to 0 (color : black)
  img.fill(0);
  // Define a purple color
  unsigned char purple[] = { 255, 0, 255 };

  // Draw a purple "Hello world" at coordinates (100,100).
  img.draw_text(100, 100, "Hello World", purple);

  CImg<unsigned char> i2("samples/images/block.png");

  // Display the image in a display window.
  img.display("window1");
  i2.display("window2");
}

/*!
 * test the display of two images
 */
void test_one_image() {
  debug("test_one_image()");

  Situation* s = IO::parse_file("samples/sample1.txt");
  cout << s->toString() << endl;

  GUI* gui = new GUI(640, 480, s->WIDTH, s->HEIGHT);
  gui->make_image(s);
  gui->sleep(500);

  s->move_man_to_direction(Situation::UP);
  cout << s->toString() << endl;
  gui->make_image(s);
  gui->sleep(500);

  delete gui;
  delete s;
}

/*!
 * test to see the resolution of a board
 */
void test_solve(const std::string & board_filename) {
  debug("test_display_tree()");
  Situation* s = IO::parse_file(board_filename);
  // 	int connection_id;
  // 	Situation* s = IO::get_board_from_server(&connection_id, board_filename);
  cout << s->toString() << endl;

  GUI* gui = new GUI(640, 480, s->WIDTH, s->HEIGHT);
  gui->solve_and_display_tree(s);

  delete gui;
  delete s;
}

/*!
 * test for blocking cells
 */
void test_blocking_cells(const std::string & board_filename) {
  debug("test_blocking_cells()");
  Situation* s = IO::parse_file(board_filename);
  // 	int connection_id;
  // 	Situation* s = IO::get_board_from_server(&connection_id, board_filename);
  Solver* solver = new Solver(s);
  solver->compute_blocked_positions();

  GUI* gui = new GUI(640, 480, s->WIDTH, s->HEIGHT);
  gui->make_image(s, true, solver->is_cell_forbidden);
  gui->sleep(1000);
  gui->window.wait();

  delete gui;
  delete solver;
  delete s;
}

int main(int argc, char **argv) {
  //	test_images();
  //test_one_image();

  if (argc != 2) {
    printf("syntax : %s LEVELFILE\n", argv[0]);
    return -1;
  }
  const std::string board_filename = argv[1];
  cout << "Chosen level:" << board_filename << endl;
  cout << "-> 1: solve level" << endl;
  cout << "-> 2: show forbidden cells" << endl;
  int choice;
  cin >> choice;
  if (choice == 1)
    test_solve(board_filename);
  else
    test_blocking_cells(board_filename);
  return 0;
}
