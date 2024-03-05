#pragma once

#include "namespace_.hxx"


namespace p5::refrigerator::features {
namespace report {

namespace parent_ = this_;
namespace this_ = parent_::report;

struct Type final {};

} // namespace report

using Report = report::Type;

} // namespace p5::refrigerator::features
