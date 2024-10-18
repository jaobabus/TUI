#include "consoleevent.hpp"
#include "consolezbuffer.hpp"

#include <queue>
#include <spdlog/pattern_formatter.h>
#include <termios.h>
#include <thread>
#include <memory>
#include <unistd.h>
#include <vector>
#include <consolewidget.hpp>
#include <textmap.hpp>
#include <coprintf.hpp>


class ConsoleSink : public ConsoleArea
{
public:
    ConsoleSink(Vector2u size) : ConsoleArea(size), console(size) {}

    void init() {
        termios tios;
        tcgetattr(fileno(stdin), &tios);
        tios.c_lflag &= ~ECHO & ~ICANON;
        tcsetattr(fileno(stdin), 0, &tios);
        fwrite(std::string(24, '\n').data(), 24, 1, stdout);
        fwrite("\e[24A", 5, 1, stdout);
        fwrite("\e7", 2, 1, stdout);
        printf("\e[6n\n");
        char buffer[30];
        size_t i = 0;
        for(char ch = 0; ch != 'R'; i++) {
            auto ret = read(STDIN_FILENO, &ch, 1);
            if (ret <= 0)
                throw std::runtime_error("Error read cursor position");
            buffer[i] = ch;
        }
        buffer[i] = '\0';
        Vector2u& pos = home_position;
        if (sscanf(buffer, "\e[%u;%uR", &pos.y, &pos.x) != 2)
            throw std::runtime_error("Error parse position: " + std::string(buffer));
        pos.x -= 1, pos.y -= 1;
        Log->info("Console pos at ({}, {})", pos.x, pos.y);
    }

    Vector2u homepos() const {
        return home_position;
    }

    void set_char(Vector2u pos, CharUnit&& ch) override {
        if (pos < size) {
            console.set_char(pos, std::move(ch));
        }
    }

    const CharUnit& get_char(Vector2u pos) const override {
        if (pos < size) {
            return console.get_char(pos);
        }
        return CharUnit::none;
    }

    void draw() {
        // Восстанавливаем положение для цикла
        fwrite("\e8", 2, 1, stdout);
        fwrite("\e7", 2, 1, stdout);
        auto w = console.print();
        fwrite(w.data(), w.size(), 1, stdout);
        fflush(stdout);
    }

private:
    STDIOTextArea console;
    Vector2u home_position;
};


class TextArea : public ConsoleView
{
public:
    TextArea(std::string&& label, Vector2u size)
        : ConsoleView(std::move(label)), text_map(size) {}

    void draw(ConsoleArea* area) const override {
        // Log->trace("TextArea::draw");
        text_map.draw(area);
    }

    void add_text(const std::string& text) {
        Log->trace("TextArea::add_text '{}'", text);
        for (char c : text) {
            if (cursor_pos.x >= text_map.size.x) {
                cursor_pos.x = 0;
                cursor_pos.y++;
            }
            if (cursor_pos.y >= text_map.size.y) {
                scroll_up();
                cursor_pos.y = text_map.size.y - 1;
            }
            text_map.set_char(cursor_pos, CharUnit{static_cast<wchar_t>(c)});
            cursor_pos.x++;
        }
    }

    bool contains(Vector2u pos) const override {
        return pos < text_map.size;
    }

private:
    void scroll_up() {
        Log->trace("TextArea::scroll_up");
        for (unsigned y = 1; y < text_map.size.y; ++y) {
            for (unsigned x = 0; x < text_map.size.x; ++x) {
                text_map.set_char({x, y - 1}, CharUnit{text_map.get_char({x, y})});
            }
        }
        // Очищаем последнюю строку после прокрутки.
        for (unsigned x = 0; x < text_map.size.x; ++x) {
            text_map.set_char({x, text_map.size.y - 1}, CharUnit{L' '});
        }
    }

private:
    TextMap text_map;
    Vector2u cursor_pos{0, 0};
};

class Button : public ViewNotifier
{
public:
    // Constructor
    Button(std::string label) : label(std::move(label)), pressed(false) {
        Log->trace("Button({})::created", this->label);
    }

    // Destructor
    virtual ~Button() {
        Log->trace("Button({})::destroyed", label);
    }

    // Methods
    virtual void press() {
        Log->trace("Button({})::press", label);
        pressed = true;
        notify();
    }

    virtual void release() {
        Log->trace("Button({})::release", label);
        pressed = false;
        notify();
    }

    virtual void click(conevent::MouseButton btn) {
        Log->trace("Button({})::click with {}", label, (int)btn);
    }

    bool is_pressed() const {
        return pressed;
    }

private:
    // Data Members
    std::string label;
    bool pressed;
};

class ButtonView : public ConsoleView
{
public:
    enum State
    {
        Normal,
        Hovered,
        Pressed,
        Disabled
    };

public:
    // Constructor
    ButtonView(std::shared_ptr<Button>&& butt, std::string&& label)
        : ConsoleView(std::move(label)), button(std::move(butt)), current_texture(nullptr) {
        Log->trace("ButtonView({})::created", this->label);
        textures.resize(4);
    }

    // Destructor
    virtual ~ButtonView() {
        Log->trace("ButtonView({})::destroyed", label);
    }

    // Methods
    void notify(const ViewNotifier* notifier) override {
        // Log->trace("ButtonView({})::notify", label);
        // Redraw or update view based on the button state
    }

    void load_texture(std::shared_ptr<TextMap> texture, State state) {
        Log->trace("ButtonView({})::load_texture", label);
        textures[state] = std::move(texture);
        if (state == Normal)
            current_texture = textures[Normal].get();
    }

    void draw(ConsoleArea* area) const override {
        // Log->trace("ButtonView({})::draw", label);
        if (current_texture) {
            current_texture->draw(area);
        } else {
            ::draw(area, {0, 0}, "<no tex:" + label + ">");
        }
    }

    bool contains(Vector2u pos) const override {
        return current_texture and pos < current_texture->size;
    }

public:
    void on_event(const conevent::MouseEnterExitEvent& event) override {
        if (event.is_enter) {
            current_texture = textures[Hovered].get();
            if (event.is_enter)
                button->press();
        }
        else {
            current_texture = textures[Normal].get();
            if (not button->is_pressed())
                button->release();
        }
    }
    void on_event(const conevent::MouseButtonEvent& event) override {
        if (event.is_down) {
            current_texture = textures[Pressed].get();
            button->press();
        }
        else {
            current_texture = textures[(mouse_inside ? Hovered : Normal)].get();
            button->release();
            if (mouse_inside)
                button->click(event.button);
        }
    }

private:
    // Data Members
    std::shared_ptr<Button> button;
    TextMap* current_texture;
    std::vector<std::shared_ptr<TextMap>> textures;
};


class EventLoop
{
public:
    class EventReceiver : public ExtendedConsoleEventReceiver
    {
    public:
        EventReceiver(std::mutex& mutex, std::queue<AnyConsoleEvent>& queue)
            : mutex(mutex), queue(queue) {}

    public:
        void on_any_event(AnyConsoleEvent&& ev) override
        {
            std::lock_guard lg(mutex);
            queue.push(std::move(ev));
        }

    private:
        std::mutex& mutex;
        std::queue<AnyConsoleEvent>& queue;

    };

public:
    EventLoop()
        : receiver(mutex, queue), sender(receiver), should_exit(false) {}
    ~EventLoop() {
        should_exit = true;
        thread.join();
    }

private:
    void loop() {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        timeval tv;
        tv.tv_usec = 0;
        tv.tv_sec = 1;
        char buffer[128];
        auto recl = select(1, &fds, nullptr, nullptr, &tv);
        switch(recl)
        {
            case 0: break;
            case -1: break;
            default:
                if (auto s = read(STDIN_FILENO, buffer, sizeof(buffer)); s > 0) {
                    for (size_t i = 0; i < s; i++)
                        sender.on_char(buffer[i]);
                }
        }
    }

public:
    bool poll(ConsoleEventReceiver& recv) {
        AnyConsoleEvent event;
        {
            std::lock_guard lg(mutex);
            if (queue.empty())
                return false;
            event = std::move(queue.front());
            queue.pop();
        }
        std::visit([&](auto&& event) { recv.on_event(event); }, event);
        return true;
    }

    void init() {
        sender.init();
    }

    void run_thread() {
        thread = std::thread([this]{ while (not should_exit) loop(); });
    }

private:
    std::mutex mutex;
    std::queue<AnyConsoleEvent> queue;
    EventReceiver receiver;
    XTermConsoleEventSender sender;
    std::thread thread;
    std::atomic<bool> should_exit;

};


class ConsoleApp : public ConsoleEventReceiver
{
public:
    ConsoleApp()
        : console(Vector2u{80, 24}) {}

public:
    void construct()
    {
        Log->info("Construct App");
        for (int i = 0; i < 3; ++i) {
            auto text_area = std::make_shared<TextArea>("text " + std::to_string(i), Vector2u{30, 1});
            text_area->add_text("Text line " + std::to_string(i + 1));
            auto text_widget = std::make_shared<ConsoleViewWidget>(text_area, Vector2u{10, static_cast<unsigned>(i)});
            zbuffer.add_view(std::move(text_widget));
        }

        for (int i = 0; i < 3; ++i) {
            auto button = std::make_shared<Button>("Button " + std::to_string(i + 1));
            auto button_view = std::make_shared<ButtonView>(std::shared_ptr{button}, "Button " + std::to_string(i + 1));
            button->add_view(button_view);
            button_view->load_texture(std::make_shared<TextMap>(TextMap::create_from("[Btn " + std::to_string(i) + "]")), ButtonView::Normal);
            auto tex = std::make_shared<TextMap>(TextMap::create_from("[Btn " + std::to_string(i) + "]"));
            for (size_t x = 0; x < tex->size.x; x++) {
                auto ch = tex->get_char({(unsigned)x, 0});
                ch.meta.background = 240;
                tex->set_char({(unsigned)x, 0}, std::move(ch));
            }
            button_view->load_texture(tex, ButtonView::Hovered);
            tex = std::make_shared<TextMap>(TextMap::create_from("[Btn " + std::to_string(i) + "]"));
            for (size_t x = 0; x < tex->size.x; x++) {
                auto ch = tex->get_char({(unsigned)x, 0});
                ch.meta.background = 64;
                tex->set_char({(unsigned)x, 0}, std::move(ch));
            }
            button_view->load_texture(tex, ButtonView::Pressed);
            auto button_widget = std::make_shared<ConsoleViewWidget>(button_view, Vector2u{0, static_cast<unsigned>(i)});
            zbuffer.add_view(std::move(button_widget));
        }

        auto text_area = std::make_shared<TextArea>("maintext", Vector2u{60, 8});
        text_area->add_text("Initial text in the text area\nSecond line");
        auto text_area_widget = std::make_shared<ConsoleViewWidget>(text_area, Vector2u{0, 5});
        zbuffer.add_view(std::move(text_area_widget));
    }

    void init()
    {
        Log->info("Init App");
        event_loop.init();
        console.init();
        null_position = console.homepos();
        event_loop.run_thread();
        for (auto& view : zbuffer.get_views())
            view->set_highlighted(false);
    }

public:
    void run()
    {
        Log->info("Run App");
        while (true) {
            while (event_loop.poll(*this));
            zbuffer.draw(&console);
            console.draw();
            std::this_thread::sleep_for(std::chrono::milliseconds(33));
        }
    }

private:
    void on_event(const conevent::MouseMoveEvent& event) override
    {
        auto from = event.previous - null_position;
        auto to = event.position - null_position;
        Log->trace("mouse_move ({}, {}) ({} {}), status: {}", from.x, from.y, to.x, to.y, static_cast<int>(event.status));
        auto widget = zbuffer.get_widget_at(to);
        if (last_mouse_view) {
            if (last_mouse_view != widget) {
                last_mouse_view->get_view()->on_event(conevent::MouseEnterExitEvent{false, to});
                last_mouse_view = nullptr;
            }
        }
        if (widget && last_mouse_view != widget) {
            last_mouse_view = widget;
            last_mouse_view->get_view()->on_event(conevent::MouseEnterExitEvent{true, to});
        }
    }

    void on_event(const conevent::MouseButtonEvent& event) override
    {
        auto at = event.position - null_position;
        auto widget = zbuffer.get_widget_at(at);
        if (event.is_down) {
            if (widget) {
                last_pressed_view = widget;
                last_pressed_view->get_view()->on_event(event);
            }
        } else {
            if (last_pressed_view) {
                last_pressed_view->get_view()->on_event(event);
            }
            last_pressed_view = nullptr;
        }
    }

private:
    ConsoleSink console;
    ConsoleZBuffer zbuffer;
    EventLoop event_loop;
    Vector2u null_position;

private:
    ConsoleViewWidget* last_mouse_view = nullptr;
    ConsoleViewWidget* last_pressed_view = nullptr;

};


#include "spdlog/sinks/sink.h"

class CoPrintfSink : public spdlog::sinks::sink
{
public:
    CoPrintfSink() {}

public:
    void log(const spdlog::details::log_msg& msg) {
        spdlog::memory_buf_t s;
        if (formatter)
            formatter->format(msg, s);
        else
            throw std::runtime_error("No formatter");
        s.append(std::string(1, '\0'));
        coprintf("%s", s.data());
    }
    void flush() {}
    void set_pattern(const std::string &pattern) { throw std::runtime_error("Illegal operation"); }
    void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) { formatter = std::move(sink_formatter); }

private:
    std::string pattern;
    std::unique_ptr<spdlog::formatter> formatter;

};


extern int button_example_run();
int button_example_run()
{
    auto s = std::make_shared<CoPrintfSink>();
    Log->sinks().push_back(s);
    s->set_formatter(std::make_unique<spdlog::pattern_formatter>("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v"));
    Log->set_level(spdlog::level::trace);
    ConsoleApp app;
    app.construct();
    app.init();
    app.run();
    return 0;
}
