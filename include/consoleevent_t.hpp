#pragma once

#include "utils.hpp"
#include "kbevent.hpp"




namespace conevent
{



// ----------------------- Common events ------------------------


enum class CtrlStatus : uint8_t
{
    None = 0,
    Control = 1 << 0,
    Shift = 1 << 1,
    Meta = 1 << 2,
    Alt = Meta,
    MB1 = 1 << 3,
    MB2 = 1 << 4,
    MB3 = 1 << 5,
    MB4 = 1 << 6,
    MB5 = 1 << 7,
};


constexpr inline
CtrlStatus operator|(CtrlStatus a, CtrlStatus b)
{ return CtrlStatus((uint8_t)a | (uint8_t)b); }

constexpr inline
CtrlStatus operator&(CtrlStatus a, CtrlStatus b)
{ return CtrlStatus((uint8_t)a & (uint8_t)b); }


// ----------------------- ------------- ------------------------



// ------------------------ Mouse events ------------------------


enum class MouseButton : uint8_t
{
    Unspecified,
    LeftButton,
    MiddleButton,
    RightButton,
    MB1 = LeftButton, MB2, MB3, MB4, MB5, MB6, MB7
};


struct MouseButtonEvent
{
    Vector2u position;
    MouseButton button = MouseButton::Unspecified;
    bool is_down;
    CtrlStatus status;
    uint64_t bam;
};


struct MouseMoveEvent
{
    Vector2u previous;
    Vector2u position;
    CtrlStatus status;
};


struct FocusEvent
{
    bool is_gain;
};


struct MouseEnterExitEvent
{
    bool is_enter;
    Vector2u position;
    CtrlStatus status;
};


// ------------------------ ------------ ------------------------



// ---------------------- Keyboard events -----------------------


struct KeyboardEvent
{
    Key key;
    int count = 1;
    uint64_t bam[2];
};


// ---------------------- --------------- -----------------------



// ------------------------ XTerm events ------------------------


struct XTermStatusEvent
{
    int status;
    uint64_t bam[2];
};


struct XTermDSRPositionEvent
{
    Vector2u position;
    uint64_t bam[2];
};


// ------------------------ ------------ ------------------------



}
