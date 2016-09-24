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
Value Center   = V(  6),
      InnerMid = V( -5), InnerLong = V(-10),
      OuterMid = V( -9), Outer     = V(-16), OuterLong = V(-19),
      MidEdge  = V(-32), NearEdge  = V(-40), FarEdge   = V(-42), Corner = V(-66);
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
   { S(-143,-108), S(-96,-75), S(-80,-48), S(-73,-29) },
   { S( -83, -75), S(-43,-51), S(-21,-26), S(-10,  0) },
   { S( -71, -48), S(-22,-26), S(  0, -2), S(  9, 23) },
   { S( -25, -29), S( 18,  0), S( 43, 23), S( 47, 42) },
   { S( -26, -29), S( 16,  0), S( 38, 23), S( 50, 42) },
   { S( -11, -48), S( 37,-26), S( 56, -2), S( 71, 23) },
   { S( -62, -75), S(-17,-51), S(  5,-26), S( 14,  0) },
   { S(-195,-108), S(-66,-75), S(-42,-48), S(-29,-29) }
  },
  { // Bishop
   { S( -54, Corner),   S(-23, FarEdge),   S(-35, NearEdge),  S(-44, MidEdge) },
   { S( -30, FarEdge),  S( 10, OuterLong), S(  2, Outer),     S( -9, OuterMid) },
   { S( -19, NearEdge), S( 17, Outer),     S( 11, InnerLong), S(  1, InnerMid) },
   { S( -21, MidEdge),  S( 18, OuterMid),  S( 11, InnerMid),  S(  0, Center) },
   { S( -21, MidEdge),  S( 14, OuterMid),  S(  6, InnerMid),  S( -1, Center) },
   { S( -27, NearEdge), S(  6, Outer),     S(  2, InnerLong), S( -8, InnerMid) },
   { S( -33, FarEdge),  S(  7, OuterLong), S( -4, Outer),     S(-12, OuterMid) },
   { S( -45, Corner),   S(-21, FarEdge),   S(-29, NearEdge),  S(-39, MidEdge) }
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
   { S(291, 29), S(344, 71), S(294,113), S(219,119) },
   { S(289, 71), S(329,121), S(263,161), S(205,177) },
   { S(226,113), S(271,161), S(202,194), S(136,199) },
   { S(204,119), S(212,177), S(175,199), S(137,209) },
   { S(177,119), S(205,177), S(143,199), S( 94,209) },
   { S(147,113), S(188,161), S(113,194), S( 70,199) },
   { S(116, 71), S(158,121), S( 93,161), S( 48,177) },
   { S( 94, 29), S(120, 71), S( 78,113), S( 31,119) }
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
Bonus[BISHOP][RANK_1][FILE_A] = S(-54, Corner);    Bonus[BISHOP][RANK_1][FILE_B] = S(-23, FarEdge);
Bonus[BISHOP][RANK_1][FILE_C] = S(-35, NearEdge);  Bonus[BISHOP][RANK_1][FILE_D] = S(-44, MidEdge);
Bonus[BISHOP][RANK_2][FILE_A] = S(-30, FarEdge);   Bonus[BISHOP][RANK_2][FILE_B] = S( 10, OuterLong);
Bonus[BISHOP][RANK_2][FILE_C] = S(  2, Outer);     Bonus[BISHOP][RANK_2][FILE_D] = S( -9, OuterMid);
Bonus[BISHOP][RANK_3][FILE_A] = S(-19, NearEdge);  Bonus[BISHOP][RANK_3][FILE_B] = S( 17, Outer);
Bonus[BISHOP][RANK_3][FILE_C] = S( 11, InnerLong); Bonus[BISHOP][RANK_3][FILE_D] = S(  1, InnerMid);
Bonus[BISHOP][RANK_4][FILE_A] = S(-21, MidEdge);   Bonus[BISHOP][RANK_4][FILE_B] = S( 18, OuterMid);
Bonus[BISHOP][RANK_4][FILE_C] = S( 11, InnerMid);  Bonus[BISHOP][RANK_4][FILE_D] = S(  0, Center);
Bonus[BISHOP][RANK_5][FILE_A] = S(-21, MidEdge);   Bonus[BISHOP][RANK_5][FILE_B] = S( 14, OuterMid);
Bonus[BISHOP][RANK_5][FILE_C] = S(  6, InnerMid);  Bonus[BISHOP][RANK_5][FILE_D] = S( -1, Center);
Bonus[BISHOP][RANK_6][FILE_A] = S(-27, NearEdge);  Bonus[BISHOP][RANK_6][FILE_B] = S(  6, Outer);
Bonus[BISHOP][RANK_6][FILE_C] = S(  2, InnerLong); Bonus[BISHOP][RANK_6][FILE_D] = S( -8, InnerMid);
Bonus[BISHOP][RANK_7][FILE_A] = S(-33, FarEdge);   Bonus[BISHOP][RANK_7][FILE_B] = S(  7, OuterLong);
Bonus[BISHOP][RANK_7][FILE_C] = S( -4, Outer);     Bonus[BISHOP][RANK_7][FILE_D] = S(-12, OuterMid);
Bonus[BISHOP][RANK_8][FILE_A] = S(-45, Corner);    Bonus[BISHOP][RANK_8][FILE_B] = S(-21, FarEdge);
Bonus[BISHOP][RANK_8][FILE_C] = S(-29, NearEdge);  Bonus[BISHOP][RANK_8][FILE_D] = S(-39, MidEdge);
#undef S

init();

}

TUNE(
     SetRange(-30,42), Center  ,
     SetRange(-41,31), InnerMid, SetRange(-46,26), InnerLong,
     SetRange(-45,27), OuterMid, SetRange(-52,20), Outer    , SetRange(-55,17), OuterLong,
     SetRange(-68, 4), MidEdge , SetRange(-76,-4), NearEdge , SetRange(-78,-6), FarEdge  , SetRange(-102,-30), Corner,
     update_psqt);

} // namespace PSQT
