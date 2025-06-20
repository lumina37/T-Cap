#include <expected>
#include <utility>

#include <X11/Xlib.h>

#include "tcap/utils/error.hpp"

#ifndef _TCAP_LIB_HEADER_ONLY
#    include "tcap/screen/x11/display.hpp"
#endif

namespace tcap::x11 {

DisplayBox::DisplayBox(Display* display) noexcept : display_(display) {}

DisplayBox::DisplayBox(DisplayBox&& rhs) noexcept : display_(std::exchange(rhs.display_, nullptr)) {}

DisplayBox& DisplayBox::operator=(DisplayBox&& rhs) noexcept {
    display_ = std::exchange(rhs.display_, nullptr);
    return *this;
}

DisplayBox::~DisplayBox() noexcept {
    if (display_ == nullptr) return;
    XCloseDisplay(display_);
    display_ = nullptr;
}

std::expected<DisplayBox, Error> DisplayBox::create() noexcept {
    Display* display = XOpenDisplay(nullptr);
    if (display == nullptr) {
        return std::unexpected{Error{ECate::eX11, 0}};
    }

    return DisplayBox{display};
}

}  // namespace tcap::x11
