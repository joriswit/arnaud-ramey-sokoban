/*
 * Plugin.cpp
 *
 *  Created on: Apr 17, 2016
 *      Author: joriswit
 */

#include "Solver.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <algorithm>
#pragma pack(push, 1)
#include "sokobanpluginsdll.h"
#pragma pack(pop)
#include "arnaud-ramey-sokoban_export.h"

extern "C" {

int ARNAUD_RAMEY_SOKOBAN_EXPORT __stdcall SolveEx(unsigned int width,
												 unsigned int height,
												 char* board,
												 char* pcSolution,
												 unsigned int uiSolutionBufferSize,
												 struct PluginStatus *psStatus,
												 PLUGINCALLBACK *pc) {
	
	Situation* s = new Situation(width, height, true);
	for (coord j = 0; j < height; ++j) {
		for (coord i = 0; i < width; ++i) {
			char curr_char = board[j * width + i];
			if (curr_char == '#')
				s->set_room(i, j, Situation::WALL);
			else if (curr_char == '.')
				s->set_room(i, j, Situation::SLOT_EMPTY);
			else if (curr_char == '$')
				s->set_room(i, j, Situation::BOX_MOBILE);
			else if (curr_char == '*')
				s->set_room(i, j, Situation::BOX_IN_SLOT);
			else
				s->set_room(i, j, Situation::EMPTY);

			if (curr_char == '@' || curr_char == '+') {
				s->man_x = i;
				s->man_y = j;
			}
		}
	}

	Solver* solver = new Solver(s);
	bool was_solution_found = false;
	string solution = solver->solve_and_convert_to_string(&was_solution_found);

	delete solver;
	delete s;

	if (was_solution_found)
	{
		solution.erase(remove_if(solution.begin(), solution.end(), isspace), solution.end());
		psStatus->uiFlags = SOKOBAN_PLUGIN_FLAG_SOLUTION;

		if (!strcpy_s(pcSolution, uiSolutionBufferSize, solution.c_str())) {
			return SOKOBAN_PLUGIN_RESULT_SUCCESS;
		} else {
			return SOKOBAN_PLUGIN_RESULT_GAMETOOLONG;
		}
	} else {
		return SOKOBAN_PLUGIN_RESULT_GIVEUP;
	}
}

void ARNAUD_RAMEY_SOKOBAN_EXPORT __stdcall GetConstraints(unsigned int* puiMaxWidth, unsigned int* puiMaxHeight, unsigned int* puiMaxBoxes) {
	*puiMaxWidth = 0;
	*puiMaxHeight = 0;
	*puiMaxBoxes = 0;
}

void ARNAUD_RAMEY_SOKOBAN_EXPORT __stdcall GetPluginName(char* pcString, unsigned int uiStringBufferSize) {
	strcpy_s(pcString, uiStringBufferSize, "Arnaud Ramey Sokoban solver");
}

void ARNAUD_RAMEY_SOKOBAN_EXPORT __stdcall Configure(HWND hwndParent) {
	MessageBox(hwndParent, "No configuration options", "Sokoban solver", MB_OK);
}

void ARNAUD_RAMEY_SOKOBAN_EXPORT __stdcall ShowAbout(HWND hwndParent) {
	MessageBox(hwndParent, "                  +----------------------+\n                  |       sokoban        |\n                  +----------------------+\n\nThis is a Sokoban solver.\nThere is also a graphical user interface (GUI).", "Sokoban solver", MB_OK);
}

}