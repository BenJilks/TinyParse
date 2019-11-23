#pragma once
#include "generator/generator.hpp"

namespace TinyParse
{

    class GeneratorCpp : public Generator
    {
    public:
        GeneratorCpp(string file_path) :
            Generator(file_path) {}

        virtual void generate(string project_name);

    private:
        void generate_definitions();
        void generate_rule_node(Rule::Node *node);
		void generate_list(Rule::Node *node);
		void generate_keyword(Rule::Node *node);
		void generate_token(Rule::Node *node);
        void generate_rule(Rule *rule);

    };

}
