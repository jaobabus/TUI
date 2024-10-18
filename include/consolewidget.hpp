#ifndef CONSOLEWIDGET_HPP
#define CONSOLEWIDGET_HPP

#include <vector>
#include <string>

#include "consolearea.hpp"
#include "consoleevent.hpp"



class ConsoleWidget;


class ConsoleAreaRect : public ConsoleArea
{
public:
    ConsoleAreaRect(ConsoleArea* view, Vector2u position, Vector2u size)
        : ConsoleArea(size), view(view), position(position) {}

public:
    void set_char(Vector2u pos,
                  CharUnit&& ch)
    {
        if (pos < size)
            view->set_char(pos + position, std::move(ch));
    }
    CharUnit const& get_char(Vector2u pos) const
    {
        if (pos < size)
            return view->get_char(pos + position);
        else
            return CharUnit::none;
    }

public:
    ConsoleArea* const view;
    Vector2u position;

};


class ViewNotifier;

class ConsoleView : public ConsoleEventReceiver, public std::enable_shared_from_this<ConsoleView>
{
public:
    ConsoleView(std::string&& label) : label(std::move(label)) {}
    virtual ~ConsoleView() = default;

public:
    virtual void draw(ConsoleArea* area) const = 0;
    virtual void notify(const ViewNotifier* notifier) {}
    virtual bool contains(Vector2u pos) const = 0;

public:
    const std::string label;

protected:
    bool mouse_inside = false;
};



class ViewNotifier
{
public:
    // Type aliases
    using WeakPtr = std::weak_ptr<ViewNotifier>;
    using SharedPtr = std::shared_ptr<ViewNotifier>;

public:
    // Constructor and Destructor
    ViewNotifier() {
        Log->trace("ViewNotifier::created");
    }

    virtual ~ViewNotifier() {
        Log->trace("ViewNotifier::destroyed");
    }

public:
    // Methods
    void notify() {
        // Log->trace("ViewNotifier::notify called");
        for (const auto& weak_view : view_ptrs) {
            if (auto view = weak_view.lock()) {
                // Log->trace("ViewNotifier::notify - valid view, performing update");
                view->notify(this); // Replace nullptr with actual ConsoleArea if needed
            }
        }
        // Remove expired weak pointers from the list
        view_ptrs.erase(std::remove_if(view_ptrs.begin(), view_ptrs.end(), [](const std::weak_ptr<ConsoleView>& weak_view) {
            return weak_view.expired();
        }), view_ptrs.end());
    }

    void add_view(std::shared_ptr<ConsoleView> view) {
        Log->trace("ViewNotifier::add_view");
        view_ptrs.emplace_back(view);
    }

private:
    // Data Members
    std::vector<std::weak_ptr<ConsoleView>> view_ptrs;
};


#endif // CONSOLEWIDGET_HPP
