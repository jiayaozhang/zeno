#include <z2/dop/DopContext.h>
#include <z2/dop/DopNode.h>
#include <ranges>


namespace z2::dop {


/*void DopContext::Ticket::wait() const {
    if (!ctx->visited.contains(node)) {
        ctx->visited.insert(node);
        node->_apply_func(ctx);
    }
}*/


bool DopDepsgraph::contains_node(DopNode *node) const {
    return nodes.contains(node);
}


void DopDepsgraph::insert_node(DopNode *node, std::set<DopNode *> &&deps) {
    nodes.emplace(node, std::move(deps));
    order.push_back(node);
}


void DopDepsgraph::execute() {
#ifndef __CLANGD__
    for (auto *node: order) {
        node->execute();
    }
#endif
}


}