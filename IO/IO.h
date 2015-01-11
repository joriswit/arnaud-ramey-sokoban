/*
 * IO.h
 *
 *  Created on: Sep 24, 2009
 *      Author: arnaud
 */

#ifndef IO_H_
#define IO_H_

#include "../Situation.h"

class IO {
public:
	static Situation* parse_file(string filename, int wanted_level = -1);

	static Situation* get_board_from_server(int* connection_id, int board_id);
	static void upload_answer_to_server(int connection_id, string* answer);

private:
	static Situation* convert_server_string_to_vector(string board);
	static Situation* parse_lines(vector<string>* lines,
			char wall_c, char slotempty_c, char boxmob_c, char boxinslot_c,
			char man_c);
};

#endif /* IO_H_ */
