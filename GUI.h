/*
 * GUI.h
 *
 *  Created on: Oct 4, 2009
 *      Author: arnaud
 */

#ifndef GUI_H_
#define GUI_H_

#include "libs/CImg.h"
#include "Situation.h"
#include "SituationTree.h"
#include "Solver.h"
#include "IO/IO.h"
using namespace cimg_library;

typedef CImg<unsigned char> CImage;
typedef CImgDisplay CWindow;

class GUI {
public:
	CImage block;
	CImage emptySlot;
	CImage robot;
	CImage slotBlock;
	CImage wall;
	CImage empty;
	CImage blocking;

	int width_pix;
	int height_pix;
	int WIDTH_BOARD;
	int HEIGHT_BOARD;

	int cell_width_pix;
	int cell_height_pix;
	CImage sit_image;
	CWindow window;

	Situation* last_drawn_situation;

	GUI(int w, int h, int WIDTH, int HEIGHT);
	~GUI();
	void make_image(Situation* s, bool has_blocking_cells = false,
			bool* blocking_cells = NULL);
	void draw_cell(coord& i, coord& j, state& state);
	void draw_man(coord& i, coord& j);

	void display_tree(Situation* sit_init, SituationTree* sol_tree);
	void solve_and_display_tree(Situation* sit_init);

	void sleep(int millisec);
};

#endif /* GUI_H_ */
