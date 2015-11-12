/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2015 Marco Costalba, Joona Kiiski, Tord Romstad

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TIMEMAN_H_INCLUDED
#define TIMEMAN_H_INCLUDED

#include "misc.h"
#include "search.h"
#include "thread.h"

/// The TimeManagement class computes the optimal time to think depending on
/// the maximum available time, the game move number and other parameters.

class TimeManagement {
public:
  void init(Search::LimitsType& limits, Color us, int ply);
  void pv_instability(bool easy, int easyPlayed, double bestMoveChanges) {
      double easyOff_1 = .01 * double(easyPercentOff1), easyOff_2 = .01 * double(easyPercentOff2);
      unstablePvFactor = 1.0 + bestMoveChanges - easyOff_1 * easyOff_2 * double(easy) / (easyOff_2 + (easyOff_1 - easyOff_2) * easyPlayed); }
  int available() {
      return int(optimumTime * unstablePvFactor * (0.01 * timeFactorPercent)); }
  int maximum() const { return maximumTime; }
  int elapsed() const { return int(Search::Limits.npmsec ? Threads.nodes_searched() : now() - startTime); }

  int64_t availableNodes; // When in 'nodes as time' mode

private:
  TimePoint startTime;
  int optimumTime;
  int maximumTime;
  double unstablePvFactor;

  int easyPercentOff1 = 87, easyPercentOff2 = 65;
  TUNE(SetRange(80, 94), easyPercentOff1, SetRange(50,79), easyPercentOff2);

  int timeFactorPercent = 80;
  TUNE(SetRange(70, 120), timeFactorPercent);
};

extern TimeManagement Time;

#endif // #ifndef TIMEMAN_H_INCLUDED
