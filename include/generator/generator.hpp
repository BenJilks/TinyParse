#pragma once
#include "rule.hpp"
#include <fstream>
#include <vector>
#include <tuple>
#include <string>
using std::vector;
using std::pair;
using std::string;

namespace TinyParse
{

    class Generator
    {
    public:
        Generator();
        Generator(string file_path);
        ~Generator();

        void add_rule(Rule *rule);
        virtual void generate(string project_name) = 0;

    protected:
        vector<Rule*> rules;
        void write_string(string msg);
        void write_line(string line);
        void write_file(string file_path);

    private:
        std::ofstream out_file;
        std::ostream &out;

    };

}
