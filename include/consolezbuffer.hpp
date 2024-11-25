#pragma once

#include "consolewidget.hpp"




// Позволю себе костылизм
class ConsoleViewWidget : public ViewNotifier
{
public:
    // попа делает жим жим, но пока не будем требовать size от view, надеюсь это не аукнется
    ConsoleViewWidget(std::shared_ptr<ConsoleView>&& view = {nullptr}, Vector2u position = {0, 0})
      : view(std::move(view)), position(position), highlighted(false) {}

public:
    Vector2u get_position() const noexcept { return position; }
    void set_position(Vector2u pos) { position = pos; notify(); }
    const std::shared_ptr<ConsoleView>& get_view() const { return view; }
    void set_view(std::shared_ptr<ConsoleView>&& view) { this->view = std::move(view); notify(); }
    void draw(ConsoleArea* area) const;
    void set_highlighted(bool v = true) { highlighted = v; }

private:
    std::shared_ptr<ConsoleView> view;
    Vector2u position;
    bool highlighted;
};


class ConsoleZBuffer : public ConsoleView
{
public:
    ConsoleZBuffer(ConsoleZBuffer &&) = default;
    ConsoleZBuffer &operator=(ConsoleZBuffer &&) = default;

    // Constructor
    ConsoleZBuffer();
    // Destructor
    virtual ~ConsoleZBuffer();

public:
    // Add a view to the buffer
    void add_view(std::shared_ptr<ConsoleViewWidget>&& view);
    void notify(const ViewNotifier* notifier) override;

    // Draw method to draw all views
    void draw(ConsoleArea* area) const override;
    ConsoleViewWidget* get_widget_at(Vector2u position);
    ConsoleViewWidget* get_widget_at(std::string_view name);
    bool contains(Vector2u pos) const override;

    const std::vector<std::shared_ptr<ConsoleViewWidget>>& get_views() const;

private:
    std::vector<std::shared_ptr<ConsoleViewWidget>> views;

};
