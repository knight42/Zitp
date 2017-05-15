#include <iostream>
#include <unistd.h>

#include "zitp.hpp"

using std::cout;
using std::endl;
using std::ifstream;

int main(int argc, char *argv[]) {
    char *infile(nullptr),
         *outfile(nullptr),
         *prog(nullptr);

    int c;
    while ((c = getopt(argc, argv, "hi:o:p:")) != -1) {
        switch (c) {
            case 'i':
                infile = optarg;
                break;
            case 'o':
                outfile = optarg;
                break;
            case 'p':
                prog = optarg;
                break;
            case 'h':
                cout << "Usage: -i <input.txt> -o <output.txt> -p <program.txt>" << endl;
                return 0;
            default:
                return 1;
        }
    }

    ifstream input(prog ? prog : "program.txt");
    Zitp *z = new Zitp(prog, infile, outfile);
    z->parse_ast();
    #if DEBUG_MODE
    z->ast->print();
    #endif
    z->run();
	cout << "Program exited." << endl;
    return 0;
}

