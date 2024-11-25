#include "editable.hpp"

#include <fstream>
#include <thread>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <pwd.h>

#include "coprintf.hpp"
#include "layout.hpp"
#include "widgetregistry.hpp"


using namespace editable;

Vector2u ConsoleSink::update_size()
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    auto now = size;
    size = {w.ws_col, w.ws_row};
    if (size != now)
        console.resize(size);
    return now;
}

#include <spdlog/sinks/sink.h>
#include <spdlog/pattern_formatter.h>

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

void ConsoleSink::init() {
    update_size();
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

void EditableViewApp::init()
{
    make_default();
    {
        struct passwd *pw = getpwuid(getuid());
        const char* homedir = pw->pw_dir;
        auto config_dir = std::filesystem::path(homedir) / ".config" / "editable_view";
        if (not exists(config_dir))
            create_directories(config_dir);
        load_view(config_dir / "layout.json");
        save_view(config_dir / "layout.json");
    }
    _sink.init();
    _loop.init();
}


std::map<std::string, std::unique_ptr<ViewNotifier>>& widgets()
{
    static std::map<std::string, std::unique_ptr<ViewNotifier>> widgets;
    return widgets;
}


class MyButton : public Button
{
public:
    using Button::Button;

public:
    void press() { Log->info("Button {} press", label); Button::press(); }
    void release() { Log->info("Button {} release", label); Button::release(); }
    void click(conevent::MouseButton btn) { Log->info("Button {} click by {}", label, (int)btn); }

};

void EditableViewApp::make_default()
{
    global_registry.register_widget(std::make_shared<MyButton>("button-1"));
    auto btn1 = std::make_shared<ButtonView>(global_registry.find_widget<Button>("button-1"), std::string{"button-1-view"});
    _buffer.add_view(std::make_shared<ConsoleViewWidget>(btn1, Vector2u{0, 0}));

    btn1->load_texture(std::make_shared<TextMap>(TextMap::create_from("[Btn " + std::to_string(1) + "]")), ButtonView::Normal);
    auto tex = std::make_shared<TextMap>(TextMap::create_from("[Btn " + std::to_string(1) + "]"));
    for (size_t x = 0; x < tex->size.x; x++) {
        auto ch = tex->get_char({(unsigned)x, 0});
        ch.meta.background = 240;
        tex->set_char({(unsigned)x, 0}, std::move(ch));
    }
    btn1->load_texture(tex, ButtonView::Hovered);
    tex = std::make_shared<TextMap>(TextMap::create_from("[Btn " + std::to_string(1) + "]"));
    for (size_t x = 0; x < tex->size.x; x++) {
        auto ch = tex->get_char({(unsigned)x, 0});
        ch.meta.background = 64;
        tex->set_char({(unsigned)x, 0}, std::move(ch));
    }
    btn1->load_texture(tex, ButtonView::Pressed);
}

void EditableViewApp::load_view(const std::filesystem::path& layout_path)
{
    if (exists(layout_path) and file_size(layout_path) > 16) {
        std::ifstream ifl(layout_path);
        std::string file(file_size(layout_path), '\0');
        ifl.read(file.data(), file.size());
        auto config = mini_json::parse<ConsoleZBuffer>(begin(file), end(file));
    }
}

void EditableViewApp::save_view(const std::filesystem::path& layout_path)
{
    std::ofstream ofl(layout_path);
    mini_json::serialize(_buffer, ofl);
}

void EditableViewApp::_run()
{
    while (true)
    {
        while (_loop.poll(_root));
        _buffer.draw(&_sink);
        _sink.draw();
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}

void EditableViewApp::run()
{
    init();
    _run();
}

void EditableViewApp::save()
{
    struct passwd *pw = getpwuid(getuid());
    const char* homedir = pw->pw_dir;
    auto config_dir = std::filesystem::path(homedir) / ".config" / "editable_view";
    save_view(config_dir / "layout.json");
}



ViewNotifier* resolve_widget(std::string_view name)
{
    auto it = widgets().find(std::string{name});
    if (it == widgets().end())
        return nullptr;
    return it->second.get();
}



extern int editable_example_run();
int editable_example_run()
{
    EditableViewApp app{};
    app.run();
    return 0;
}
