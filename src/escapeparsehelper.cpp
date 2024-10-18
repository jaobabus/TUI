#include <escapeparsehelper.hpp>

#include <iostream>




std::unique_ptr<escparse::BaseContainerRule> escparse::make_xterm_rules()
{
    using namespace escparse;

    auto after_csi = SkipChar('[');

    // Arrows
    after_csi
    .append(
        FinalRule<int>('A',
          [](int value) {
              std::cout << "Cursor up by: " << value << " rows." << std::endl;
          }
        ),
        FinalRule<int>('B',
            [](int value) {
                std::cout << "Cursor down by: " << value << " rows." << std::endl;
            }
        ),
        FinalRule<int>('C',
            [](int value) {
                std::cout << "Cursor forward by: " << value << " columns." << std::endl;
            }
        ),
        FinalRule<int>('D',
            [](int value) {
                std::cout << "Cursor backward by: " << value << " columns." << std::endl;
            }
        )
    );

    // All other
    after_csi
    .append(
        ParseInt()
        .append(
            // \e[<0|3>n Terminal status
            FinalRule<int>('n',
                [](int value) {
                    switch (value) {
                    case 0:
                        std::cout << "Terminal Status OK" << std::endl;
                        break;
                    case 3:
                        std::cout << "Terminal Status Malfunction" << std::endl;
                        break;
                    default:
                        throw std::runtime_error("Unknown terminal status");
                    }
                }
            ),
            // \e[<fn>~ Functional key
            FinalRule<int>('~',
                [](int value) {
                    auto fn_index = value - 11;
                    if (fn_index > 0 && fn_index < 12)
                        std::cout << "Key Fn" << fn_index + 1 << std::endl;
                    else
                        throw std::runtime_error("Unknown Fn key " + std::to_string(fn_index));
                }
            ),
            ParseInt()
            .append(
                SkipChar(';')
                .append(
                    ParseInt()
                    .append(
                        // \e[<row>;<col>R DSR
                        FinalRule<int, int>('R',
                            [](int row, int col) {
                                std::cout << "Cursor position at " << row << ", " << col << std::endl;
                            }
                        )
                    )
                )
            )
        )
    );

    auto csi = SkipChar('\e').append( std::move(after_csi) );

    return std::make_unique<SkipChar>(std::move(csi));
}
