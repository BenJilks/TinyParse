#pragma once
#include "rule.hpp"
#include <fstream>
#include <vector>
#include <tuple>
#include <string>
#include <map>
using std::vector;
using std::pair;
using std::map;
using std::string;

namespace TinyParse
{

    class Generator
    {
    public:
        Generator();
        Generator(string file_path);
        ~Generator();

        void add_rule(Rule rule);
        void add_define(string name, string value);
        virtual void generate(string project_name) = 0;

    protected:
        vector<Rule> rules;
        map<string, string> defines;
        void write_string(string msg);
        void write_line(string line);
        void write_file(string file_path);
        void start_scope() { scope++; }
        void end_scope() { scope--; }

        bool has_define(string value);
        const Rule *find_rule(string name) const;
        Rule::Node *get_entry_node(Rule::Node *node);

    private:
        std::ofstream out_file;
        std::ostream &out;
        int scope;

    };

}
