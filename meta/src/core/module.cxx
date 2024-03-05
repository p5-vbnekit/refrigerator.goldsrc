#include <stdexcept>
#include <exception>
#include <type_traits>

#include "module.hxx"


namespace p5::refrigerator::core::module {

this_::Loader::operator bool () const noexcept(true) {
    return static_cast<bool>(action_);
}

this_::Loader::Task this_::Loader::operator () () const noexcept(false) {
    if (! action_) return {};
    auto task_ = action_();
    if (! task_.linked()) return {};
    using State_ = ::std::decay_t<decltype(task_.state())>;
    switch (task_.state()) {
    default: break;
    case State_::Initial:
        if (! task_.start()) throw ::std::logic_error{
            "failed to start async module loader"
        };
        switch (task_.state()) {
        default: break;
        case State_::Started: return task_;
        case State_::Finished:
            task_.future().get(::std::rethrow_exception);
            return {};
        }
        break;
    case State_::Started: return task_;
    case State_::Finished:
        task_.future().get(::std::rethrow_exception);
        return {};
    }
    throw ::std::logic_error{"bad async core module loader state"};
}

this_::Loader::Loader() noexcept(true) = default;
this_::Loader::Loader(Loader &&) noexcept(true) = default;
this_::Loader::Loader(Loader const &) noexcept(false) = default;

this_::Loader & this_::Loader::operator = (Loader &&) noexcept(true) = default;
this_::Loader & this_::Loader::operator = (
    Loader const &
) noexcept(false) = default;

} // namespace p5::refrigerator::core::module
