#include <zeno2/UI/UiDopSocket.h>
#include <zeno2/UI/UiDopNode.h>
#include <zeno2/UI/UiDopGraph.h>


namespace zeno2::UI {


UiDopSocket::UiDopSocket() {
    bbox = {-R, -R, 2 * R, 2 * R};
    zvalue = 2.f;
}


Color UiDopSocket::get_color() const {
    if (hovered) {
        return {0.75f, 0.5f, 0.375f};
    } else if (failed) {
        return {0.825f, 0.225f, 0.125f};
    } else if (links.size()) {
        return {0.375f, 0.5f, 1.0f};
    } else {
        return {0.375f, 0.375f, 0.375f};
    }
}


void UiDopSocket::clear_links() {
    auto graph = get_parent()->get_parent();
    if (links.size()) {
        for (auto link: std::set(links)) {
            graph->remove_link(link);
        }
    }
}


void UiDopSocket::paint() const {
    glColor3f(0.75f, 0.75f, 0.75f);
    glRectf(bbox.x0, bbox.y0, bbox.x0 + bbox.nx, bbox.y0 + bbox.ny);
    glColor3fv(get_color().data());
    glRectf(bbox.x0 + BW, bbox.y0 + BW, bbox.x0 + bbox.nx - BW, bbox.y0 + bbox.ny - BW);
}


void UiDopInputSocket::paint() const {
    UiDopSocket::paint();

    if (get_parent()->hovered) {
        auto font = get_default_font();
        font.set_font_size(FH);
        font.set_fixed_height(2 * R);
        font.set_fixed_width(NW, FTGL::ALIGN_LEFT);
        glColor3f(1.f, 1.f, 1.f);
        font.render(R * 1.3f, -R + FH * 0.15f, name);
    }
}


void UiDopOutputSocket::paint() const {
    UiDopSocket::paint();

    if (get_parent()->hovered) {
        auto font = get_default_font();
        font.set_font_size(FH);
        font.set_fixed_height(2 * R);
        font.set_fixed_width(NW, FTGL::ALIGN_RIGHT);
        glColor3f(1.f, 1.f, 1.f);
        font.render(-NW - R * 1.5f, -R + FH * 0.15f, name);
    }
}


Color UiDopInputSocket::get_color() const {
    if (value.size() && !hovered && !failed && !links.size())
        return {0.75f, 0.25f, 0.6f};
    return UiDopSocket::get_color();
}


}  // namespace zeno2::UI