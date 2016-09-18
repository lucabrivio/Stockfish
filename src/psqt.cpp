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
   { S(-143,-103), S(-96,-77), S(-80,-50), S(-73,-29) },
   { S( -83, -77), S(-43,-52), S(-21,-30), S(-10, -7) },
   { S( -71, -50), S(-22,-30), S(  0, -8), S(  9, 16) },
   { S( -25, -29), S( 18, -7), S( 43, 16), S( 47, 39) },
   { S( -26, -29), S( 16, -7), S( 38, 16), S( 50, 39) },
   { S( -11, -50), S( 37,-30), S( 56, -8), S( 71, 16) },
   { S( -62, -77), S(-17,-52), S(  5,-30), S( 14, -7) },
   { S(-195,-103), S(-66,-77), S(-42,-50), S(-29,-29) }
  },
  { // Bishop
   { S(-54,-66), S(-23,-42), S(-35,-40), S(-44,-32) },
   { S(-30,-42), S( 10,-19), S(  2,-16), S( -9, -9) },
   { S(-19,-40), S( 17,-16), S( 11,-11), S(  1, -6) },
   { S(-21,-32), S( 18, -9), S( 11, -6), S(  0,  5) },
   { S(-21,-32), S( 14, -9), S(  6, -6), S( -1,  5) },
   { S(-27,-40), S(  6,-16), S(  2,-11), S( -8, -6) },
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
   { S(291, 28), S(344, 76), S(294,103), S(219,112) },
   { S(289, 70), S(329,119), S(263,170), S(205,159) },
   { S(226,109), S(271,164), S(202,195), S(136,191) },
   { S(204,131), S(212,194), S(175,194), S(137,204) },
   { S(177,132), S(205,187), S(143,224), S( 94,227) },
   { S(147,118), S(188,178), S(113,199), S( 70,197) },
   { S(116, 72), S(158,121), S( 93,142), S( 48,161) },
   { S( 94, 30), S(120, 76), S( 78,101), S( 31,111) }
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
