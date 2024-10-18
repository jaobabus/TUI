#include "../include/consolezbuffer.hpp"




ConsoleZBuffer::ConsoleZBuffer() : ConsoleView("Z buffer") {
    Log->trace("ConsoleZBuffer::created");
}

ConsoleZBuffer::~ConsoleZBuffer() {
    Log->trace("ConsoleZBuffer::destroyed");
}

void ConsoleZBuffer::add_view(std::shared_ptr<ConsoleViewWidget> &&view) {
    Log->trace("ConsoleZBuffer::add_view");
    views.push_back(std::move(view));
}

void ConsoleZBuffer::notify(const ViewNotifier *notifier) {
    Log->trace("ConsoleZBuffer::notify");
    // пока что игнорим draw у нас полностью переписывает все
}

void ConsoleZBuffer::draw(ConsoleArea *area) const {
    // Log->trace("ConsoleZBuffer::draw");
    area->clear();
    for (auto it = views.rbegin(); it != views.rend(); ++it) {
        (*it)->draw(area);
    }
}

ConsoleViewWidget *ConsoleZBuffer::get_widget_at(Vector2u position) {
    for (auto it = views.rbegin(); it != views.rend(); ++it) {
        auto pos = (*it)->get_position();
        auto off = position - pos;
        if (position >= pos and (*it)->get_view()->contains(off)) {
            return it->get();
        }
    }
    return nullptr;
}

bool ConsoleZBuffer::contains(Vector2u pos) const { return false; }

const std::vector<std::shared_ptr<ConsoleViewWidget> > &ConsoleZBuffer::get_views() const { return views; }


void ConsoleViewWidget::draw(ConsoleArea* area) const {
    if (view) {
        // Log->trace("ConsoleViewWidget::draw");
        ConsoleAreaRect rect(area, position, area->size - position);
        view->draw(&rect);
        if (highlighted) {
            auto size = area->size - position;
            auto range = [](int st, int sp, int step) {
                std::vector<int> range;
                range.resize((sp - st) / step);
                auto it = range.begin();
                for (int i = st; i != sp; i += step)
                    *it++ = i;
                return range;
            };
            auto transform = [&rect](const std::vector<int>& range, auto&& position, auto&& op) {
                for (auto e : range) {
                    auto pos = position(e);
                    auto ch = rect.get_char(pos);
                    op(ch);
                    rect.set_char(pos, std::move(ch));
                }
            };
            auto hl = [](CharUnit& unit) { unit.meta.background = (uint8_t)236; };
            transform(range(0, size.x - 1, 1), [&](int i) { return Vector2u{(unsigned)i, 0}; }, hl);
            transform(range(0, size.y - 1, 1), [&](int i) { return Vector2u{0, (unsigned)i}; }, hl);
            transform(range(size.x - 1, 0, -1), [&](int i) { return Vector2u{(unsigned)i, 0}; }, hl);
            transform(range(size.y - 1, 0, -1), [&](int i) { return Vector2u{0, (unsigned)i}; }, hl);
        }
    }
}
