#ifndef ZITP_H
#define ZITP_H

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <memory>

#include "Term.hpp"
#include "value.hpp"

class Zitp {
private:
    std::string input_file = "input.txt";
    std::string output_file = "output.txt";
    std::string prog_file = "program.txt";
    std::ifstream _input;
    std::ofstream _output;

    i32 read_int();
    void print_int(i32 val);

    void init_params(const FuncValue *fv, Term *argus, Scope *born, Scope *current);
    std::shared_ptr<Value> eval_expr(Term *t, Scope *current);
    std::shared_ptr<Value> execute_program(Term *t, Scope *root);

public:
    Term *ast;

    Zitp(const char *prog, const char *in, const char *out):
        ast(nullptr)
    {
        if (prog) {
            prog_file = std::string(prog);
        }
        if (in) {
            input_file = std::string(in);
        }
        if (out) {
            output_file = std::string(out);
        }
    }

    bool parse_ast() {
        std::ifstream ifs(prog_file);
        if (!ifs) {
            std::cerr << prog_file << " cannot be found" << std::endl;
            return false;
        }
        ast = parse(ifs);
        return ast != nullptr;
    }

    void run();
};
#endif
