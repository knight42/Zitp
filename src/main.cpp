#include <iostream>
#include <unistd.h>
//#include <algorithm>
//#include <vector>
//#include <queue>
//#include <deque>
//#include <unordered_map>
//#include <list>
//#include <string>
//#include <iomanip>
//#include <cstdio>
//#include <cstdlib>
//#include <cstring>
//#include <cmath>
//#define
using std::cin;
using std::cout;
using std::endl;
//using std::ifstream;
//using std::ofstream;
//using std::string;
//using std::queue;
//using std::unordered_map;

//typedef int8_t i8;
//typedef int16_t i16;
//typedef int32_t i32;
//typedef int64_t i64;
//typedef intptr_t isize;
//typedef uint8_t u8;
//typedef uint16_t u16;
//typedef uint32_t u32;
//typedef uint64_t u64;
//typedef uintptr_t usize;

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
                puts("Usage: -i <input.txt> -o <output.txt> -p <program.txt>");
                return 0;
            default:
                return 1;
        }
    }

    return 0;
}

