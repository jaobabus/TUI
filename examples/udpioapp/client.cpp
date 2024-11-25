#include "client.hpp"
#include <minijson/json.h>
#include <pwd.h>
#include <fstream>

#include "../editable_view/layout.hpp"
#include "appwidget.hpp"
#include "widgetregistry.hpp"




void Client::init()
{
    _sink.init();
    _loop.init();
    make_default();
}

static const char* json();

void Client::make_default()
{
    std::string file(json());
    _buffer = mini_json::parse<ConsoleZBuffer>(begin(file), end(file));
}

void Client::load_view(const std::filesystem::path& layout_path)
{
    if (exists(layout_path) and file_size(layout_path) > 16) {
        std::ifstream ifl(layout_path);
        std::string file(file_size(layout_path), '\0');
        ifl.read(file.data(), file.size());
        auto config = mini_json::parse<ConsoleZBuffer>(begin(file), end(file));
    }
}

void Client::save_view(const std::filesystem::path& layout_path)
{
    std::ofstream ofl(layout_path);
    mini_json::serialize(_buffer, ofl);
}

void Client::_run()
{
    while (not global_registry.find_widget<AppWidget>("app")->need_closed)
    {
        while (_loop.poll(_root));
        _buffer.draw(&_sink);
        _sink.update_size();
        _sink.draw();
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}

void Client::run()
{
    init();
    _run();
}

void Client::save()
{
    struct passwd *pw = getpwuid(getuid());
    const char* homedir = pw->pw_dir;
    auto config_dir = std::filesystem::path(homedir) / ".config" / "udpioapp";
    save_view(config_dir / "layout.json");
}


static const char* json() {
    return R"(
{
"type": "console-buffer",
"views": [
{
  "type": "console-view-widget",
  "position": { "x": 21, "y": 1 },
  "view" : {
    "type": "button-view",
    "label": "button-1-view",
    "widget": "bind-button",
    "textures": [
      {
        "text_map": {
          "size": {
            "x": 6,
            "y": 1
          },
          "data": [
            "[Bind]"
          ],
          "style": [
            {
              "insert_at": {
                "position": {
                  "x": 0,
                  "y": 0
                },
                "background": 0,
                "foreground": 15
              },
              "apply": null
            }
          ]
        }
      },
      {
        "text_map": {
          "size": {
            "x": 6,
            "y": 1
          },
          "data": [
            "[Bind]"
          ],
          "style": [
            {
              "insert_at": {
                "position": {
                  "x": 0,
                  "y": 0
                },
                "background": 240,
                "foreground": 15
              },
              "apply": null
            }
          ]
        }
      },
      {
        "text_map": {
          "size": {
            "x": 6,
            "y": 1
          },
          "data": [
            "[Bind]"
          ],
          "style": [
            {
              "insert_at": {
                "position": {
                  "x": 0,
                  "y": 0
                },
                "background": 64,
                "foreground": 15
              },
              "apply": null
            }
          ]
        }
      },
      null
    ]
  }
},
{
  "type": "console-view-widget",
  "position": { "x": 1, "y": 1 },
  "view" : {
    "widget": "address-area",
    "label": "address-area-view",
    "size": { "x": 20, "y": 1 }
  }
},
{
  "type": "console-view-widget",
  "position": { "x": 1, "y": 3 },
  "view" : {
    "widget": "packet-area",
    "label": "packet-area-view",
    "size": { "x": 26, "y": 12 }
  }
}
]
}
)";
}
