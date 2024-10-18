
#include "consoleevent.hpp"




std::unique_ptr<escparse::BaseContainerRule> XTermConsoleEventSender::make_xterm_rules()
{
    using namespace escparse;

    auto after_csi = SkipChar('[');

    auto arrows = ParseChar([](char chr) { return 'A' <= chr and chr <= 'D'; }, "<A-D>");
    arrows(this, &XTermConsoleEventSender::xterm_arrow);

    auto term_status = SkipChar('n')(this, &XTermConsoleEventSender::xterm_status, "xterm-status");

    auto fn_key = SkipChar('~')(this, &XTermConsoleEventSender::xterm_fn_key, "fn-key");

    auto third_arg = ParseInt();
    third_arg.append({
                         // \e[<btn>;<x>;<y>M
                         SkipChar('M')(this, &XTermConsoleEventSender::xterm_btn_urxvt_M, "btn-urxvt-M"),
                         // \e[<btn>;<x>;<y>m
                         SkipChar('m')(this, &XTermConsoleEventSender::xterm_btn_sgr_m, "btn-sgr-m"),
                     });

    auto second_arg = ParseInt();
    second_arg.append({
                          // \e[<x>;<y>R
                          SkipChar('R')(this, &XTermConsoleEventSender::xterm_dsr, "btn-dsr-R"),
                      });
    second_arg.append(SkipChar(';').append(third_arg));

    // \e[<a1>...
    auto first_arg = ParseInt();
    first_arg.append({
                         // \e[<a1><A-D>
                         std::move(arrows),
                         // \e[<a1>n
                         std::move(term_status),
                         // \e[<fn>~
                         std::move(fn_key),
                     });
    first_arg.append(SkipChar(';').append(std::move(second_arg)));

    auto sgrs = ParseInt();
    sgrs.append({
                    // \e[<<btn>;<x>;<y>M
                    SkipChar('M')(this, &XTermConsoleEventSender::xterm_btn_sgr_M, "btn-sgr-<M"),
                    // \e[<<btn>;<x>;<y>m
                    SkipChar('m')(this, &XTermConsoleEventSender::xterm_btn_sgr_m, "btn-sgr-<m")
                });

    after_csi.append({
                         // \e[...
                         std::move(first_arg),
                         (SkipChar('<'), ParseInt(), SkipChar(';'), ParseInt(), SkipChar(';'), std::move(sgrs)),
                     });

    auto csi = SkipChar('\e').append(std::move(after_csi));

    Log->debug("Parse tree:\n{}", dump(&csi));
    return std::make_unique<SkipChar>(std::move(csi));
}
