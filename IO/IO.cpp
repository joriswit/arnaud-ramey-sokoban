/*
 * IO.cpp
 *
 *  Created on: Sep 24, 2009
 *      Author: arnaud
 */

#include "IO.h"
#include "client.cpp"

/*!
 * read a file and transform it into a situation
 * @param filename the file to read
 * @param wanted_level the wanted level.
 * The level must start with "; <level>" and end with ";"
 */
Situation* IO::parse_file(string filename, int wanted_level) {
	ostringstream s;
	//	s << "IO::parse_file(" << filename << ", " << wanted_level << ")";
	//debug(s.str());
	cout << s.str() << endl;

	ifstream myfile(filename.c_str());
	if (!myfile.is_open()) {
		cout << "Unable to open file '" << filename << "' !";
		return NULL;
	}

	vector<string> lines;
	// determine when we start keeping the lines
	bool level_started = true;
	ostringstream level_start_string;
	level_start_string << "; " << wanted_level;
	//	cout << "level_start_string;" << level_start_string.str() << endl;
	if (wanted_level > 0) {
		level_started = false;
	}

	while (!myfile.eof()) {
		string line;
		getline(myfile, line);
		//		cout << "current line:" << line << endl;

		if (wanted_level > 0) {
			// determine if we start reading the line
			if (level_started == false && line.find(level_start_string.str())
					== 0)
				level_started = true;
			// determine if we stop reading the line
			else if (level_started == true && line[0] == ';')
				break;
		}

		//if the line is not empty and not a comment => add it
		if (level_started && line.size() > 0 && line[0] != '/' && line[0]
				!= ';') {
			//			cout << "added line : " << line << endl;
			lines.push_back(line);
		}
	}
	myfile.close();

	return parse_lines(&lines, Situation::WALL__CHAR,
			Situation::SLOT_EMPTY__CHAR, Situation::BOX_MOBILE__CHAR,
			Situation::BOX_IN_SLOT__CHAR, Situation::MAN__CHAR);

}

/*!
 * connect to the server and get a board
 * @param connection_id the id of the connection.
 * You need it to answer
 * @param board_id
 * The id of the board
 * @return the board
 */
Situation* IO::get_board_from_server(int* connection_id, int board_id) {
	*connection_id = connect_to_server();
	string board_from_server = read_board_from_server(*connection_id, board_id);
	//	debug("IO:: board_from_server:\n" + board_from_server);
	return convert_server_string_to_vector(board_from_server);
}

/*!
 * upload the answer string to the server
 * @param connection_id
 * the id of the connection
 * @param answer
 * the answer string
 */
void IO::upload_answer_to_server(int connection_id, string* answer) {
	submit_sol_to_server(connection_id, *answer);
	cout << " -> Answer uploaded." << endl;
	string answer_server = get_answer_from_server(connection_id);
	cout << " -> Server answer : " << answer_server << endl;
	disconnect_server(connection_id);
}

/*!
 * an internal conversion function
 * @param board
 * @return
 */
Situation* IO::convert_server_string_to_vector(string board) {
	vector<string> lines;
	ostringstream curr_line;

	for (string::iterator curr_char = board.begin(); curr_char < board.end(); ++curr_char) {
		/* if new line */
		if (*curr_char == '\n') {
			// add the current line
			lines.push_back(curr_line.str());
			// reset current line
			curr_line.str("");
		}
		/* no new line => append */
		else {
			curr_line << *curr_char;
		}
	}

	return parse_lines(&lines, '#', '/', 'o', 'O', 'x');
}

/*!
 *
 *the parsing function itself
 * @param lines
 * @param w the with
 * @param h the height
 * @param wall_c
 * @param slotempty_c
 * @param boxmob_c
 * @param boxinslot_c
 * @param man_c
 * @return
 */
Situation* IO::parse_lines(vector<string>* lines, char wall_c,
		char slotempty_c, char boxmob_c, char boxinslot_c, char man_c) {

	// first : find w and h
	coord w = 0, h;
	h = lines->size();
	for (vector<string>::iterator it = lines->begin(); it < lines->end(); ++it) {
		w = max(w, (coord) it->size());
	}

	Situation* rep = new Situation(w, h, true);
	for (coord j = 0; j < h; ++j) {
		string curr_line = lines->at(j);

		for (coord i = 0; i < curr_line.size(); ++i) {
			char curr_char = curr_line.at(i);
			if (curr_char == wall_c)
				rep->set_room(i, j, Situation::WALL);
			else if (curr_char == slotempty_c)
				rep->set_room(i, j, Situation::SLOT_EMPTY);
			else if (curr_char == boxmob_c)
				rep->set_room(i, j, Situation::BOX_MOBILE);
			else if (curr_char == boxinslot_c)
				rep->set_room(i, j, Situation::BOX_IN_SLOT);
			else
				rep->set_room(i, j, Situation::EMPTY);

			if (curr_char == man_c) {
				rep->man_x = i;
				rep->man_y = j;
			}
		}
	}
	return rep;
}
