
#include "consolearea.hpp"
#include "coprintf.hpp"
#include "spdlog/sinks/sink.h"
#include <spdlog/pattern_formatter.h>


extern int button_example_run();
extern int editable_example_run();
extern int udpio_example_run();



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




int main()
{
    auto s = std::make_shared<CoPrintfSink>();
    Log->sinks().push_back(s);
    s->set_formatter(std::make_unique<spdlog::pattern_formatter>("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v"));
    Log->set_level(spdlog::level::trace);

    udpio_example_run();
    return 0;
}
