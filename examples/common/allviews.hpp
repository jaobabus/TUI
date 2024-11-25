#pragma once

#include "widgets/textarea.hpp"
#include "widgets/button.hpp"


using AllViewsVariant =
    std::variant<std::shared_ptr<ButtonView>, std::shared_ptr<TextAreaView>>;
