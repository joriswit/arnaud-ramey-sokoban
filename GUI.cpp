/*
 * GUI.cpp
 *
 *  Created on: Oct 4, 2009
 *      Author: arnaud
 */

#include "GUI.h"
#include "sokoban_path.h"

static string PATH_IMAGES = SOKOBAN_PATH "images/";

static CImage load_image(string filename, int w, int h) {
  CImage rep((PATH_IMAGES + filename).c_str());
  rep.resize(w, h);
  return rep;
}

/*!
 * constructor
 * @param w
 * @param h
 * @param WIDTH
 * @param HEIGHT
 */
GUI::GUI(int w, int h, int WIDTH, int HEIGHT) {
  debug("GUI constructor");

  width_pix = w;
  height_pix = h;
  WIDTH_BOARD = WIDTH;
  HEIGHT_BOARD = HEIGHT;

  cell_width_pix = w / WIDTH;
  cell_height_pix = h / HEIGHT;

  // load the images
  block = load_image("block.png", cell_width_pix, cell_height_pix);
  emptySlot = load_image("emptySlot.png", cell_width_pix, cell_height_pix);
  robot = load_image("robot.png", cell_width_pix, cell_height_pix);
  slotBlock = load_image("slotBlock.png", cell_width_pix, cell_height_pix);
  wall = load_image("wall.png", cell_width_pix, cell_height_pix);
  empty = load_image("empty.png", cell_width_pix, cell_height_pix);
  blocking = load_image("blocking.png", cell_width_pix, cell_height_pix);

  // reset the answer image
  sit_image = CImage(width_pix, height_pix, 1, 3);
  sit_image.fill(0);

  // init the last drawn situation
  last_drawn_situation = NULL;

  // prepair the window
  window = CWindow(w, h, "Sokoban GUI");
}

/*!
 * destructor
 */
GUI::~GUI() {
  debug("GUI destructor");
  delete last_drawn_situation;
}

static state EMPTY_BLOCKING_CELL = 100;

/*!
 * draw a specific image
 * @param s
 */
void GUI::make_image(Situation* s, bool has_blocking_cells,
                     bool* blocking_cells) {
  debug("GUI::make_image(Situation* s)");

  for (coord j = 0; j < HEIGHT_BOARD; ++j) {
    for (coord i = 0; i < WIDTH_BOARD; ++i) {
      state curr_state = s->get_room(i, j);
      // draw only if it change
      if (last_drawn_situation == NULL || last_drawn_situation->get_room(
            i, j) != curr_state) {

        // check if we need to draw blocking cells
        if (has_blocking_cells && curr_state == Situation::EMPTY) {
          if (blocking_cells[j * WIDTH_BOARD + i] == true)
            draw_cell(i, j, EMPTY_BLOCKING_CELL);
          else
            draw_cell(i, j, curr_state);
        } else
          draw_cell(i, j, curr_state);
      }
    }
  }

  // redraw the man if needed
  coord man_x = s->man_x;
  coord man_y = s->man_y;

  if (last_drawn_situation != NULL) {
    coord old_man_x = last_drawn_situation->man_x;
    coord old_man_y = last_drawn_situation->man_y;
    if (old_man_x != man_x || old_man_y != man_y) {
      state new_state = s->get_room(old_man_x, old_man_y);
      draw_cell(old_man_x, old_man_y, new_state);
      draw_man(man_x, man_y);
    }
  } else
    // no last situation, draw man
    draw_man(man_x, man_y);

  // draw text
  unsigned char green[] = { 0, 255, 0 };
  sit_image.draw_text(10, 10,
                      "Press 'PAGE UP/DOWN' for rewind or next step, SPACE for autoplay",
                      green);

  if (last_drawn_situation != NULL)
    delete last_drawn_situation;
  last_drawn_situation = s->clone();
}

/*!
 * sleep a while
 * @param millisec
 */
void GUI::sleep(int millisec) {
  window.display(sit_image);
  window.wait(millisec);
}

/*!
 * draw only a cell
 * @param i
 * @param j
 * @param state
 */
void GUI::draw_cell(coord& i, coord& j, state& state) {
#if DEBUG
  cout << "GUI::draw_cell(" << i << ", " << j << ", " << (int) state << ")"
       << endl;
#endif

  CImage* ptr = &empty;
  if (state == Situation::EMPTY) {
  } else if (state == Situation::WALL)
    ptr = &wall;
  else if (state == Situation::BOX_MOBILE)
    ptr = &block;
  else if (state == Situation::SLOT_EMPTY)
    ptr = &emptySlot;
  else if (state == Situation::BOX_IN_SLOT)
    ptr = &slotBlock;
  else if (state == EMPTY_BLOCKING_CELL)
    ptr = &blocking;

  sit_image.draw_image(i * cell_width_pix, j * cell_height_pix, *ptr);
}

/*!
 * draw the man at a given position
 * @param i
 * @param j
 */
void GUI::draw_man(coord& i, coord& j) {
#if DEBUG
  cout << "GUI::draw_man(" << i << ", " << j << ")" << endl;
#endif
  sit_image.draw_image(i * cell_width_pix, j * cell_height_pix, robot);
}

#define ACTION_DO_NOTHING 0
#define ACTION_INCR 1
#define ACTION_DECR 2

/*!
 * display a solution tree
 * @param sit_init
 * @param sol_tree
 */
void GUI::display_tree(Situation* sit_init, SituationTree* sol_tree) {
  Situation* s = sit_init->clone();

  int MOVE_INDEX_MAX = sol_tree->compute_depth() - 1, MOVE_INDEX_MIN = 0;
  int move_index = MOVE_INDEX_MAX;
  int action = ACTION_DO_NOTHING;
  bool autoplay = false;

  while (!window.is_closed) {
    coord end_box_x, end_box_y;
    coord start_box_x, start_box_y;
    direction dir, opp_dir;

    if (action == ACTION_INCR && move_index >= MOVE_INDEX_MIN) {
      // read the direction and coord of the last move
      sol_tree->compute_diff(move_index, dir, end_box_x, end_box_y);

      // find the start positions
      opp_dir = Situation::opposite_direction(dir);
      s->coord_in_direction(opp_dir, end_box_x, end_box_y, start_box_x,
                            start_box_y);

      // modify the situation
      s->move_box_to_direction(start_box_x, start_box_y, dir);
      cout << s->toString() << endl;

      move_index--;
    }
    if (action == ACTION_DECR && move_index < MOVE_INDEX_MAX) {
      move_index++;
      // read the direction and coord of the last move
      sol_tree->compute_diff(move_index, dir, end_box_x, end_box_y);

      // find the start positions
      opp_dir = Situation::opposite_direction(dir);
      s->coord_in_direction(opp_dir, end_box_x, end_box_y, start_box_x,
                            start_box_y);
      coord man_old_x, man_old_y;
      s->coord_in_direction(opp_dir, start_box_x, start_box_y, man_old_x,
                            man_old_y);

      // modify the situation
      s->move_box_to_direction(end_box_x, end_box_y, opp_dir);
      s->move_man_to_position(man_old_x, man_old_y);
      cout << s->toString() << endl;
    }

    // refresh the image
    make_image(s);
    window.display(sit_image);
    action = ACTION_DO_NOTHING;

    if (autoplay) {
      action = ACTION_INCR;
      window.wait(20);
      if (window.is_key(cimg::keySPACE, true))
        autoplay = false;
    } else {
      window.wait();
      // control the keys
      if (window.is_key(cimg::keyPAGEDOWN, true))
        action = ACTION_INCR;

      if (window.is_key(cimg::keyPAGEUP, true))
        action = ACTION_DECR;

      if (window.is_key(cimg::keySPACE, true))
        autoplay = true;
    }
  }

  // wait for an input
  delete s;
}

/*!
 * display a solution tree
 * @param sit_init
 * @param sol_tree
 */
void GUI::solve_and_display_tree(Situation* sit_init) {
  Solver* solver = new Solver(sit_init);
  bool is_sol_found;
  SituationTree* sol_tree = solver->solve(&is_sol_found);
  delete solver;
  cout << sol_tree->toString(false) << endl;

  if (is_sol_found)
    display_tree(sit_init, sol_tree);
  else
    cout << "GUI : No solution to display" << endl;
  delete sol_tree;
}

