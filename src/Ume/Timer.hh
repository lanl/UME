/*
  Copyright (c) 2023, Triad National Security, LLC. All rights reserved.

  This is open source software; you can redistribute it and/or modify it under
  the terms of the BSD-3 License. If software is modified to produce derivative
  works, such modified software should be clearly marked, so as not to confuse
  it with the version available from LANL. Full text of the BSD-3 License can be
  found in the LICENSE.md file, and the full assertion of copyright in the
  NOTICE.md file.
*/

/*!
  \file Ume/Timer.hh
*/

#ifndef UME_TIMER_HH
#define UME_TIMER_HH 1

#include <cassert>
#include <chrono>
#include <ostream>

namespace Ume {

//! A simple elapsed-time class
class Timer {
  typedef std::chrono::system_clock CLOCK_T;

public:
  void start() {
    assert(!running);
    running = true;
    start_tp = CLOCK_T::now();
  }
  void stop() {
    std::chrono::time_point<CLOCK_T> stop_tp = CLOCK_T::now();
    assert(running);
    running = false;
    accum += stop_tp - start_tp;
  }
  double seconds() const { return accum.count(); }
  void clear() {
    running = false;
    accum = std::chrono::duration<double>(0.0);
  }

private:
  bool running = false;
  std::chrono::duration<double> accum = std::chrono::duration<double>(0.0);
  std::chrono::time_point<CLOCK_T> start_tp;
};

inline std::ostream &operator<<(std::ostream &os, const Timer &t) {
  return os << t.seconds() << 's';
}

} // namespace Ume
#endif
