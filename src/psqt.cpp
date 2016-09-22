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
Value Center   = V( 43),
      InnerMid = V( 20), InnerLong = V( -4),
      OuterMid = V( -7), Outer     = V(-30), OuterLong = V(-52),
      MidEdge  = V(-29), NearEdge  = V(-50), FarEdge   = V(-77), Corner = V(-103);
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
   { S(-143, Corner),   S(-96, FarEdge),   S(-80, NearEdge),  S(-73, MidEdge) },
   { S( -83, FarEdge),  S(-43, OuterLong), S(-21, Outer),     S(-10, OuterMid) },
   { S( -71, NearEdge), S(-22, Outer),     S(  0, InnerLong), S(  9, InnerMid) },
   { S( -25, MidEdge),  S( 18, OuterMid),  S( 43, InnerMid),  S( 47, Center) },
   { S( -26, MidEdge),  S( 16, OuterMid),  S( 38, InnerMid),  S( 50, Center) },
   { S( -11, NearEdge), S( 37, Outer),     S( 56, InnerLong), S( 71, InnerMid) },
   { S( -62, FarEdge),  S(-17, OuterLong), S(  5, Outer),     S( 14, OuterMid) },
   { S(-195, Corner),   S(-66, FarEdge),   S(-42, NearEdge),  S(-29, MidEdge) }
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
   { S(-25, 0), S(-16, 0), S(-16,-1), S(-9, 1) },
   { S(-21, 0), S( -8, 1), S( -3, 1), S( 0,-1) },
   { S(-21,-1), S( -9, 1), S( -4, 3), S( 2, 0) },
   { S(-22, 1), S( -6,-1), S( -1, 0), S( 2,-1) },
   { S(-22, 1), S( -7,-1), S(  0, 0), S( 1,-1) },
   { S(-21,-1), S( -7, 1), S(  0, 3), S( 2, 0) },
   { S(-12, 0), S(  4, 1), S(  8, 1), S(12,-1) },
   { S(-23, 0), S(-15, 0), S(-11,-1), S(-5, 1) }
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
Bonus[KNIGHT][RANK_1][FILE_A] = S(-143, Corner);    Bonus[KNIGHT][RANK_1][FILE_B] = S(-96, FarEdge);
Bonus[KNIGHT][RANK_1][FILE_C] = S( -80, NearEdge);  Bonus[KNIGHT][RANK_1][FILE_D] = S(-73, MidEdge);
Bonus[KNIGHT][RANK_2][FILE_A] = S( -83, FarEdge);   Bonus[KNIGHT][RANK_2][FILE_B] = S(-43, OuterLong);
Bonus[KNIGHT][RANK_2][FILE_C] = S( -21, Outer);     Bonus[KNIGHT][RANK_2][FILE_D] = S( 10, OuterMid);
Bonus[KNIGHT][RANK_3][FILE_A] = S( -71, NearEdge);  Bonus[KNIGHT][RANK_3][FILE_B] = S(-22, Outer);
Bonus[KNIGHT][RANK_3][FILE_C] = S(   0, InnerLong); Bonus[KNIGHT][RANK_3][FILE_D] = S(  9, InnerMid);
Bonus[KNIGHT][RANK_4][FILE_A] = S( -25, MidEdge);   Bonus[KNIGHT][RANK_4][FILE_B] = S( 18, OuterMid);
Bonus[KNIGHT][RANK_4][FILE_C] = S(  43, InnerMid);  Bonus[KNIGHT][RANK_4][FILE_D] = S( 47, Center);
Bonus[KNIGHT][RANK_5][FILE_A] = S( -26, MidEdge);   Bonus[KNIGHT][RANK_5][FILE_B] = S( 16, OuterMid);
Bonus[KNIGHT][RANK_5][FILE_C] = S(  38, InnerMid);  Bonus[KNIGHT][RANK_5][FILE_D] = S( 50, Center);
Bonus[KNIGHT][RANK_6][FILE_A] = S( -11, NearEdge);  Bonus[KNIGHT][RANK_6][FILE_B] = S( 37, Outer);
Bonus[KNIGHT][RANK_6][FILE_C] = S(  56, InnerLong); Bonus[KNIGHT][RANK_6][FILE_D] = S( 71, InnerMid);
Bonus[KNIGHT][RANK_7][FILE_A] = S( -62, FarEdge);   Bonus[KNIGHT][RANK_7][FILE_B] = S(-17, OuterLong);
Bonus[KNIGHT][RANK_7][FILE_C] = S(   5, Outer);     Bonus[KNIGHT][RANK_7][FILE_D] = S( 14, OuterMid);
Bonus[KNIGHT][RANK_8][FILE_A] = S(-195, Corner);    Bonus[KNIGHT][RANK_8][FILE_B] = S(-66, FarEdge);
Bonus[KNIGHT][RANK_8][FILE_C] = S( -42, NearEdge);  Bonus[KNIGHT][RANK_8][FILE_D] = S(-29, MidEdge);
#undef S

init();

}

TUNE(
     SetRange( -57, 143), Center,
     SetRange( -80, 120), InnerMid, SetRange(-104, 96), InnerLong,
     SetRange(-107,  93), OuterMid, SetRange(-130, 70), Outer,     SetRange(-152, 48), OuterLong,
     SetRange(-129,  71), MidEdge,  SetRange(-150, 50), NearEdge,  SetRange(-177, 23), FarEdge,   SetRange(-203, -3), Corner,
     update_psqt);

} // namespace PSQT
