#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

//C++
#include <string>
#include <iostream>
using namespace std;

// connection params
#define ADDRESS "cvap103.nada.kth.se"
#define PORT 5555

#define BUFFERSIZE 10000

void error(const char *msg) {
	perror(msg);
	exit(0);
}

/*!
 * connect to the distant machine
 * @return the socket_id of the connection
 */
int connect_to_server() {
	struct sockaddr_in serv_addr;
	struct hostent *server;

	int socket_id = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_id < 0)
		error("ERROR opening socket");
	server = gethostbyname(ADDRESS);
	if (server == NULL) {
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr,
			server->h_length);
	serv_addr.sin_port = htons(PORT);
	if (connect(socket_id, (sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR connecting");

	return socket_id;
}

/*!
 * read the board with a given index from the server
 * @param socket_id
 * 					the id of the connection
 * @param board
 * 					the id of the board
 */
string read_board_from_server(int socket_id, int board_id) {
	// convert the board index into char*
	char board_str[20];
	sprintf(board_str, "%d", board_id);

	/* write the question number */
	int n = write(socket_id, board_str, strlen(board_str));
	if (n < 0)
		error("ERROR writing to socket");

	/* get the answer */
	char buffer[BUFFERSIZE];
	bzero(buffer, BUFFERSIZE);
	n = read(socket_id, buffer, BUFFERSIZE - 1);
	if (n < 0)
		error("ERROR reading from socket");

	return string(buffer);
}

/*!
 * @param socket_id
 * 					the id of the connection
 * @param solution
 * 					our solution to the problem
 */
void submit_sol_to_server(int socket_id, string& solution) {
	int n = write(socket_id, solution.c_str(), solution.size());
	if (n < 0)
		error("ERROR writing to socket");
}

/*!
 * read the answer from the server
 * @param socket_id
 * 					the id of the connection
 * @return
 * 		the answer from the server
 */
string get_answer_from_server(int socket_id) {
	char buffer[BUFFERSIZE];
	bzero(buffer, BUFFERSIZE);
	int n = read(socket_id, buffer, BUFFERSIZE - 1);
	if (n < 0)
		error("ERROR getting the answer from the server");
	return string(buffer);
}

/*!
 * close the socket
 * @param socket_id
 */
void disconnect_server(int socket_id) {
	shutdown(socket_id, 2);
}

//int main(int argc, char *argv[]) {
//
//	int socket_id = connect_to_server();
//	string board = read_board_from_server(socket_id, 2);
//	cout << "board:" << endl << board << endl;
//	string sol = "U R R D U U L D L L U L L D R R R R L D D R U R U D L L U R";
//	submit_sol_to_server(socket_id, sol);
//	string answer = get_answer_from_server(socket_id);
//	cout << "answer:" << answer << endl;
//	disconnect_server(socket_id);
//
//	socket_id = connect_to_server();
//	string board2 = read_board_from_server(socket_id, 1);
//	cout << "board2:" << endl << board2 << endl;
//	submit_sol_to_server(socket_id, sol);
//	string answer2 = get_answer_from_server(socket_id);
//	cout << "answer2:" << answer2 << endl;
//	disconnect_server(socket_id);
//
//	return 0;
//}
