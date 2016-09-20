/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2015 Marco Costalba, Joona Kiiski, Tord Romstad
  Copyright (C) 2015-2016 Marco Costalba, Joona Kiiski, Gary Linscott, Tord Romstad

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

#include "types.h"

Value PieceValue[PHASE_NB][PIECE_NB] = {
{ VALUE_ZERO, PawnValueMg, KnightValueMg, BishopValueMg, RookValueMg, QueenValueMg },
{ VALUE_ZERO, PawnValueEg, KnightValueEg, BishopValueEg, RookValueEg, QueenValueEg } };

namespace PSQT {

#define S(mg, eg) make_score(mg, eg)
#define V(value)  Value(value)

// Bonus[PieceType][Square / 2] contains Piece-Square scores. For each piece
// type on a given square a (middlegame, endgame) score pair is assigned. Table
// is defined for files A..D and white side: it is symmetric for black side and
// second half of the files.
Value Center   = V(216),
      InnerMid = V(201), InnerLong = V(197),
      OuterMid = V(175), Outer     = V(163), OuterLong = V(120),
      MidEdge  = V(122), NearEdge  = V(108), FarEdge   = V( 73), Corner = V(29);
Score Bonus[][RANK_NB][int(FILE_NB) / 2] = {
  { },
  { // Pawn
   { S(  0, 0), S(  0, 0), S(  0, 0), S( 0, 0) },
   { S(-16, 7), S(  1,-4), S(  7, 8), S( 3,-2) },
   { S(-23,-4), S( -7,-5), S( 19, 5), S(24, 4) },
   { S(-22, 3), S(-14, 3), S( 20,-8), S(35,-3) },
   { S(-11, 8), S(  0, 9), S(  3, 7), S(21,-6) },
   { S(-11, 8), S(-13,-5), S( -6, 2), S(-2, 4) },
   { S( -9, 3), S( 15,-9), S( -8, 1), S(-4,18) }
  },
  { // Knight
   { S(-143,-103), S(-96,-77), S(-80,-50), S(-73,-29) },
   { S( -83, -77), S(-43,-52), S(-21,-30), S(-10, -7) },
   { S( -71, -50), S(-22,-30), S(  0, -4), S(  9, 20) },
   { S( -25, -29), S( 18, -7), S( 43, 20), S( 47, 43) },
   { S( -26, -29), S( 16, -7), S( 38, 20), S( 50, 43) },
   { S( -11, -50), S( 37,-30), S( 56, -4), S( 71, 20) },
   { S( -62, -77), S(-17,-52), S(  5,-30), S( 14, -7) },
   { S(-195,-103), S(-66,-77), S(-42,-50), S(-29,-29) }
  },
  { // Bishop
   { S(-54,-66), S(-23,-42), S(-35,-40), S(-44,-32) },
   { S(-30,-42), S( 10,-19), S(  2,-16), S( -9, -9) },
   { S(-19,-40), S( 17,-16), S( 11,-10), S(  1, -5) },
   { S(-21,-32), S( 18, -9), S( 11, -5), S(  0,  6) },
   { S(-21,-32), S( 14, -9), S(  6, -5), S( -1,  6) },
   { S(-27,-40), S(  6,-16), S(  2,-10), S( -8, -5) },
   { S(-33,-42), S(  7,-19), S( -4,-16), S(-12, -9) },
   { S(-45,-66), S(-21,-42), S(-29,-40), S(-39,-32) }
  },
  { // Rook
   { S(-25, 0), S(-16, 0), S(-16, 0), S(-9, 0) },
   { S(-21, 0), S( -8, 0), S( -3, 0), S( 0, 0) },
   { S(-21, 0), S( -9, 0), S( -4, 0), S( 2, 0) },
   { S(-22, 0), S( -6, 0), S( -1, 0), S( 2, 0) },
   { S(-22, 0), S( -7, 0), S(  0, 0), S( 1, 0) },
   { S(-21, 0), S( -7, 0), S(  0, 0), S( 2, 0) },
   { S(-12, 0), S(  4, 0), S(  8, 0), S(12, 0) },
   { S(-23, 0), S(-15, 0), S(-11, 0), S(-5, 0) }
  },
  { // Queen
   { S( 0,-72), S(-3,-56), S(-4,-41), S(-1,-29) },
   { S(-4,-56), S( 6,-30), S( 9,-19), S( 8, -5) },
   { S(-2,-41), S( 6,-19), S( 9, -9), S( 9,  7) },
   { S(-1,-29), S( 8, -5), S(10,  7), S( 7, 20) },
   { S(-3,-29), S( 9, -5), S( 8,  7), S( 7, 20) },
   { S(-2,-41), S( 6,-19), S( 8, -9), S(10,  7) },
   { S(-2,-56), S( 7,-30), S( 7,-19), S( 6, -5) },
   { S(-1,-72), S(-4,-56), S(-1,-41), S( 0,-29) }
  },
  { // King
   { S(291, Corner),   S(344, FarEdge),   S(294, NearEdge),  S(219, MidEdge)  },
   { S(289, FarEdge),  S(329, OuterLong), S(263, Outer),     S(205, OuterMid) },
   { S(226, NearEdge), S(271, Outer),     S(202, InnerLong), S(136, InnerMid) },
   { S(204, MidEdge),  S(212, OuterMid),  S(175, InnerMid),  S(137, Center)   },
   { S(177, MidEdge),  S(205, OuterMid),  S(143, InnerMid),  S( 94, Center)   },
   { S(147, NearEdge), S(188, Outer),     S(113, InnerLong), S( 70, InnerMid) },
   { S(116, FarEdge),  S(158, OuterLong), S( 93, Outer),     S( 48, OuterMid) },
   { S( 94, Corner),   S(120, FarEdge),   S( 78, NearEdge),  S( 31, MidEdge)  }
  }
};

#undef S
#undef V

Score psq[PIECE_NB][SQUARE_NB];

// init() initializes piece-square tables: the white halves of the tables are
// copied from Bonus[] adding the piece value, then the black halves of the
// tables are initialized by flipping and changing the sign of the white scores.
void init() {

  for (Piece pc = W_PAWN; pc <= W_KING; ++pc)
  {
      PieceValue[MG][~pc] = PieceValue[MG][pc];
      PieceValue[EG][~pc] = PieceValue[EG][pc];

      Score v = make_score(PieceValue[MG][pc], PieceValue[EG][pc]);

      for (Square s = SQ_A1; s <= SQ_H8; ++s)
      {
          File f = std::min(file_of(s), FILE_H - file_of(s));
          psq[ pc][ s] = v + Bonus[pc][rank_of(s)][f];
          psq[~pc][~s] = -psq[pc][s];
      }
  }
}

void update_psqt() {

#define S(mg, eg) make_score(mg, eg)
Bonus[KING][RANK_1][FILE_A] = S(291, Corner);    Bonus[KING][RANK_1][FILE_B] = S(344, FarEdge);
Bonus[KING][RANK_1][FILE_C] = S(294, NearEdge);  Bonus[KING][RANK_1][FILE_D] = S(219, MidEdge);
Bonus[KING][RANK_2][FILE_A] = S(289, FarEdge);   Bonus[KING][RANK_2][FILE_B] = S(329, OuterLong);
Bonus[KING][RANK_2][FILE_C] = S(263, Outer);     Bonus[KING][RANK_2][FILE_D] = S(205, OuterMid);
Bonus[KING][RANK_3][FILE_A] = S(226, NearEdge);  Bonus[KING][RANK_3][FILE_B] = S(271, Outer);
Bonus[KING][RANK_3][FILE_C] = S(202, InnerLong); Bonus[KING][RANK_3][FILE_D] = S(136, InnerMid);
Bonus[KING][RANK_4][FILE_A] = S(204, MidEdge);   Bonus[KING][RANK_4][FILE_B] = S(212, OuterMid);
Bonus[KING][RANK_4][FILE_C] = S(175, InnerMid);  Bonus[KING][RANK_4][FILE_D] = S(137, Center);
Bonus[KING][RANK_5][FILE_A] = S(177, MidEdge);   Bonus[KING][RANK_5][FILE_B] = S(205, OuterMid);
Bonus[KING][RANK_5][FILE_C] = S(143, InnerMid);  Bonus[KING][RANK_5][FILE_D] = S( 94, Center);
Bonus[KING][RANK_6][FILE_A] = S(147, NearEdge);  Bonus[KING][RANK_6][FILE_B] = S(188, Outer);
Bonus[KING][RANK_6][FILE_C] = S(113, InnerLong); Bonus[KING][RANK_6][FILE_D] = S( 70, InnerMid);
Bonus[KING][RANK_7][FILE_A] = S(116, FarEdge);   Bonus[KING][RANK_7][FILE_B] = S(158, OuterLong);
Bonus[KING][RANK_7][FILE_C] = S( 93, Outer);     Bonus[KING][RANK_7][FILE_D] = S( 48, OuterMid);
Bonus[KING][RANK_8][FILE_A] = S( 94, Corner);    Bonus[KING][RANK_8][FILE_B] = S(120, FarEdge);
Bonus[KING][RANK_8][FILE_C] = S( 78, NearEdge);  Bonus[KING][RANK_8][FILE_D] = S( 31, MidEdge);
#undef S

init();

}

TUNE(Center, InnerMid, InnerLong, OuterMid, Outer, OuterLong, MidEdge, NearEdge, FarEdge, Corner, update_psqt);

} // namespace PSQT
