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
  { VALUE_ZERO, PawnValueEg, KnightValueEg, BishopValueEg, RookValueEg, QueenValueEg }
};

namespace PSQT {

#define S(mg, eg) make_score(mg, eg)

// Bonus[PieceType][Square / 2] contains Piece-Square scores. For each piece
// type on a given square a (middlegame, endgame) score pair is assigned. Table
// is defined for files A..D and white side: it is symmetric for black side and
// second half of the files.
const Score Bonus[][RANK_NB][int(FILE_NB) / 2] = {
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
   { S(-54,-66), S(-23,-45), S(-35,-43), S(-44,-34) },
   { S(-30,-45), S( 10,-18), S(  2,-15), S( -9, -6) },
   { S(-19,-43), S( 17,-15), S( 11,-17), S(  1, -6) },
   { S(-21,-34), S( 18, -6), S( 11, -6), S(  0,  5) },
   { S(-21,-34), S( 14, -6), S(  6, -6), S( -1,  5) },
   { S(-27,-43), S(  6,-15), S(  2,-17), S( -8, -6) },
   { S(-33,-45), S(  7,-18), S( -4,-15), S(-12, -6) },
   { S(-45,-66), S(-21,-45), S(-29,-43), S(-39,-34) }
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
   { S( 0,-72), S(-3,-59), S(-4,-35), S(-1,-36) },
   { S(-4,-59), S( 6,-31), S( 9,-21), S( 8, -1) },
   { S(-2,-35), S( 6,-21), S( 9,-12), S( 9,  3) },
   { S(-1,-36), S( 8, -1), S(10,  3), S( 7, 21) },
   { S(-3,-36), S( 9, -1), S( 8,  3), S( 7, 21) },
   { S(-2,-35), S( 6,-21), S( 8,-12), S(10,  3) },
   { S(-2,-59), S( 7,-31), S( 7,-21), S( 6, -1) },
   { S(-1,-72), S(-4,-59), S(-1,-35), S( 0,-36) }
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

} // namespace PSQT
