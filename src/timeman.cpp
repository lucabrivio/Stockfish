/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2014 Marco Costalba, Joona Kiiski, Tord Romstad

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

#include <algorithm>
#include <cfloat>

#include "search.h"
#include "timeman.h"
#include "uci.h"

namespace {

  enum TimeType { OptimumTime, MaxTime };

  const int MoveHorizon   = 50;   // Plan time management at most this many moves ahead
  const double MaxRatio   = 7.0;  // When in trouble, we can step over reserved time with this ratio
  const double StealRatio = 0.33; // However we must not steal time from remaining moves over this ratio

  template<TimeType T>
  int remaining(int myTime, int movesToGo, int slowMover)
  {
    const double TMaxRatio   = (T == OptimumTime ? 1 : MaxRatio);
    const double TStealRatio = (T == OptimumTime ? 0 : StealRatio);

    double moveImportance = double(slowMover) / 100;
    double otherMovesImportance = double(movesToGo - 1) * (1.00 - 0.25 * Search::RootPos.game_phase() / PHASE_MIDGAME);

    double ratio1 = (TMaxRatio * moveImportance) / (TMaxRatio * moveImportance + otherMovesImportance);
    double ratio2 = (moveImportance + TStealRatio * otherMovesImportance) / (moveImportance + otherMovesImportance);

    return int(myTime * std::min(ratio1, ratio2)); // Intel C++ asks an explicit cast
  }

} // namespace


/// init() is called at the beginning of the search and calculates the allowed
/// thinking time out of the time control and current game phase. We support four
/// different kinds of time controls, passed in 'limits':
///
///  inc == 0 && movestogo == 0 means: x basetime  [sudden death!]
///  inc == 0 && movestogo != 0 means: x moves in y minutes
///  inc >  0 && movestogo == 0 means: x basetime + z increment
///  inc >  0 && movestogo != 0 means: x moves in y minutes + z increment

void TimeManager::init(const Search::LimitsType& limits, Color us)
{
  int minThinkingTime = Options["Minimum Thinking Time"];
  int moveOverhead    = Options["Move Overhead"];
  int slowMover       = Options["Slow Mover"];

  // Initialize unstablePvFactor to 1 and search times to maximum values
  unstablePvFactor = 1;
  optimumSearchTime = maximumSearchTime = std::max(limits.time[us], minThinkingTime);

  const int MaxMTG = limits.movestogo ? std::min(limits.movestogo, MoveHorizon) : MoveHorizon;

  // We calculate optimum time usage for different hypothetical "moves to go"-values
  // and choose the minimum of calculated search time values. Usually the greatest
  // hypMTG gives the minimum values.
  for (int hypMTG = 1; hypMTG <= MaxMTG; ++hypMTG)
  {
      // Calculate thinking time for hypothetical "moves to go"-value
      int hypMyTime =  limits.time[us]
                     + limits.inc[us] * (hypMTG - 1)
                     - moveOverhead * (2 + std::min(hypMTG, 40));

      hypMyTime = std::max(hypMyTime, 0);

      int t1 = minThinkingTime + remaining<OptimumTime>(hypMyTime, hypMTG, slowMover);
      int t2 = minThinkingTime + remaining<MaxTime    >(hypMyTime, hypMTG, slowMover);

      optimumSearchTime = std::min(t1, optimumSearchTime);
      maximumSearchTime = std::min(t2, maximumSearchTime);
  }

  if (Options["Ponder"])
      optimumSearchTime += optimumSearchTime / 4;

  optimumSearchTime = std::min(optimumSearchTime, maximumSearchTime);
}
