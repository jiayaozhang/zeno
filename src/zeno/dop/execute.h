#pragma once


#include <zeno/dop/Node.h>


namespace zeno::dop {


ztd::zany getval(Input const &input);
ztd::zany resolve(Input const &input);
ztd::zany resolve(Input const &input, std::set<Node *> &visited);
void touch(Input const &input, std::vector<Node *> &tolink, std::set<Node *> &visited);
void sortexec(Node *root, std::vector<Node *> &tolink, std::set<Node *> &visited);


}