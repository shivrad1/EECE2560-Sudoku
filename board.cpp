//Code by Hayden Trent, Alex Viatchenko-Karpinski, Shiv Radhakrishnan.
//Project #3 (Sudoku, Part b)
// Reads Sudoku boards from a file, solves each with recursive backtracking,
// prints the solution, and reports the number of recursive calls used.
// Uses the "improved conflict counts" approach: three boolean tables track
// which digits are already used in each row, column, and 3x3 square.

#include <iostream>
#include <limits.h>
#include "d_matrix.h"     // matrix<T> template (provided course header)
#include "d_except.h"     // exception classes (rangeError, indexRangeError, ...)
#include <list>
#include <fstream>
#include <vector>

using namespace std;

typedef int ValueType; // The type of the value in a cell
const int Blank = -1;  // Indicates that a cell is blank

const int SquareSize = 3;  //  The number of cells in a small square
                           //  (usually 3).  The board has
                           //  SquareSize^2 rows and SquareSize^2
                           //  columns.

const int BoardSize = SquareSize * SquareSize;   // 9: full board is 9x9

const int MinValue = 1;    // smallest legal digit
const int MaxValue = 9;    // largest legal digit

int numSolutions = 0;      // (available for counting solutions)

// Return the square number (1..9) of cell (i, j), counting squares left to
// right, top to bottom. i and j each run from 1 to BoardSize.
// (i-1)/SquareSize and (j-1)/SquareSize map the row/column into bands 0..2;
// the formula flattens that 3x3 grid of squares into a single 1-based index.
int squareNumber(int i, int j)
{
   return SquareSize * ((i-1)/SquareSize) + (j-1)/SquareSize + 1;
}

// Stores the entire Sudoku board plus the conflict-tracking tables.
class board
{
   public:
      board(int);                              // constructor
      void clear();                            // reset whole board to blank
      void initialize(ifstream &fin);          // (a1) load a board from file
      void print();                            // (a2) print the board
      bool isBlank(int, int);                  // is cell (i,j) empty?
      ValueType getCell(int, int);             // read cell (i,j)
      void setCell(int i, int j, ValueType val); // (a3) place a digit + update conflicts
      void printConflicts();                   // (a2) print the conflict tables
      void clearCell(int i, int j);            // (a4) empty a cell + update conflicts
      bool isSolved();                         // (a5) is the board completely solved?
      bool findBlank(int &row, int &col);      // (b1) find the next blank cell to fill
      bool isLegal(int i, int j, ValueType val); // (b2) may val go in cell (i,j)?
      bool solve();                            // (b3) recursive backtracking solver
      int getRecursiveCall();                  // read the recursive-call counter

   private:
      // Matrices indexed from 1 to BoardSize (index 0 is unused).
      matrix<ValueType> value;      // the digit (or Blank) in each cell

      // Conflict tracking matrices, Approach (2):
      // rowConflicts[i][d] == true means digit d is already placed in row i.
      // colConflicts[j][d] and sqConflicts[s][d] are the same for columns/squares.
      matrix<bool> rowConflicts;
      matrix<bool> colConflicts;
      matrix<bool> sqConflicts;
      int recursiveCall;            // counts calls to solve() for the current board
};

// Constructor: size all four matrices (index range 1..BoardSize, so dimension
// BoardSize+1 / MaxValue+1), then clear the board to a blank starting state.
board::board(int sqSize)
   : value(BoardSize+1, BoardSize+1),
     rowConflicts(BoardSize+1, MaxValue+1),
     colConflicts(BoardSize+1, MaxValue+1),
     sqConflicts(BoardSize+1, MaxValue+1)
{
   clear();
}

// Reset the whole board: every cell blank, every conflict flag false, and the
// recursive-call counter back to zero. Runs at the start of every board (via
// initialize), so each board's count starts fresh.
void board::clear()
{
   recursiveCall = 0;              // reset the per-board counter, once per board
   // Blank out every cell.
   for (int i = 1; i <= BoardSize; i++)
   {
      for (int j = 1; j <= BoardSize; j++)
      {
         value[i][j] = Blank;
      }
   }

   // Reset every conflict flag to false. All three tables share the same
   // dimensions here (9x9), so one loop clears all of them.
   for (int i = 1; i <= BoardSize; i++)
   {
      for (int d = 1; d <= MaxValue; d++)
      {
         rowConflicts[i][d] = false;
         colConflicts[i][d] = false;
         sqConflicts[i][d] = false;
      }
   }
}

// (a3) Place digit 'val' in cell (i, j) and mark it used in the row, column,
// and square conflict tables. This is the inverse of clearCell.
void board::setCell(int i, int j, ValueType val)
{
   value[i][j] = val;              // write the digit into the cell
   int sq = squareNumber(i, j);    // which 3x3 square this cell belongs to

   if (val != Blank)               // only track real digits, not blanks
   {
      rowConflicts[i][val] = true; // digit now used in this row
      colConflicts[j][val] = true; // ...this column
      sqConflicts[sq][val] = true; // ...this square
   }
}

// (a4) Empty cell (i, j) and un-mark its digit in all three conflict tables.
// Inverse of setCell: read the old digit first, blank the cell, then clear
// that digit's three flags. Being an exact inverse is what lets the solver
// undo a move and leave the tables exactly as they were before.
void board::clearCell(int i, int j)
{
   ValueType old = value[i][j];    // capture the current digit BEFORE erasing it
   value[i][j] = Blank;            // blank the cell
   int sq = squareNumber(i,j);     // which square the cell is in

   if (old != Blank)               // only clear flags if a digit was actually here
   {
      rowConflicts[i][old] = false; // digit no longer used in this row
      colConflicts[j][old] = false; // ...this column
      sqConflicts[sq][old] = false; // ...this square
   }
}

// (a1) Read one Sudoku board from the input file into this object.
// The board is given as 81 characters: digits 1-9, or '.' for a blank.
void board::initialize(ifstream &fin)
{
   char ch;

   clear();                        // start from a fully blank board

   for (int i = 1; i <= BoardSize; i++)
   {
      for (int j = 1; j <= BoardSize; j++)
      {
         fin >> ch;                // read one character (>> skips whitespace)

         // Anything other than '.' is a given digit; place it (which also
         // updates the conflict tables via setCell).
         if (ch != '.')
         {
            setCell(i, j, ch - '0');   // convert the digit character to an int
         }
      }
   }
}

// Read the digit in cell (i, j), with bounds checking.
ValueType board::getCell(int i, int j)
{
   if (i >= 1 && i <= BoardSize && j >= 1 && j <= BoardSize)
      return value[i][j];
   else
      throw rangeError("bad value in getCell");
}

// Return true if cell (i, j) is blank. Bounds-checked.
bool board::isBlank(int i, int j)
{
   if (i < 1 || i > BoardSize || j < 1 || j > BoardSize)
      throw rangeError("bad value in setCell");

   return (getCell(i,j) == Blank);
}

// (a2) Print the board as a grid, drawing separator lines between the 3x3
// squares. Blank cells are shown as empty space.
void board::print()
{
   for (int i = 1; i <= BoardSize; i++)
   {
      // Draw a horizontal divider above each band of 3 rows.
      if ((i-1) % SquareSize == 0)
      {
         cout << " -";
         for (int j = 1; j <= BoardSize; j++)
            cout << "---";
         cout << "-";
         cout << endl;
      }
      for (int j = 1; j <= BoardSize; j++)
      {
         // Draw a vertical divider at the start of each band of 3 columns.
         if ((j-1) % SquareSize == 0)
            cout << "|";
         if (!isBlank(i,j))
            cout << " " << getCell(i,j) << " ";  // print the digit
         else
            cout << "   ";                        // blank cell
      }
      cout << "|";
      cout << endl;
   }

   // Bottom border.
   cout << " -";
   for (int j = 1; j <= BoardSize; j++)
      cout << "---";
   cout << "-";
   cout << endl;
}

// (a2) Print the conflict tables: for each row, column, and square, list which
// digits are currently placed (i.e. which conflict flags are true).
void board::printConflicts()
{
   cout << "\nCONFLICTS TABLE:" << endl;

   // Digits present in each row.
   cout << "Rows (placed digits):" << endl;
   for (int i = 1; i <= BoardSize; i++)
   {
      cout << "Row " << i << ": ";
      for (int d = 1; d <= MaxValue; d++)
      {
         if (rowConflicts[i][d])
            cout << d << " ";
      }
      cout << endl;
   }

   // Digits present in each column.
   cout << "\nColumns (placed digits):" << endl;
   for (int j = 1; j <= BoardSize; j++)
   {
      cout << "Col " << j << ": ";
      for (int d = 1; d <= MaxValue; d++)
      {
         if (colConflicts[j][d])
            cout << d << " ";
      }
      cout << endl;
   }

   // Digits present in each 3x3 square.
   cout << "\nSquares (placed digits):" << endl;
   for (int sq = 1; sq <= BoardSize; sq++)
   {
      cout << "Square " << sq << ": ";
      for (int d = 1; d <= MaxValue; d++)
      {
         if (sqConflicts[sq][d])
            cout << d << " ";
      }
      cout << endl;
   }
   cout << "-----------------------\n" << endl;
}

// (a5) Return true if the board is completely and correctly solved, and print
// the result. Solved means: no blank cells, and every digit 1-9 present in
// every row, column, and square (every conflict flag true).
bool board::isSolved()
{
   bool solved = true;

   // First test: no cell may be blank. Stop early on the first blank found
   // (the "&& solved" in the loop conditions short-circuits the scan).
   for (int i = 1; i <= BoardSize && solved; i++)
   {
      for (int j = 1; j <= BoardSize && solved; j++)
      {
         if (value[i][j] == Blank)
            solved = false;
      }
   }

   // Second test: every row, column, and square must contain all 9 digits,
   // i.e. every conflict flag must be true. Any false flag means not solved.
   for (int k = 1; k <= BoardSize && solved; k++)
   {
      for (int d = MinValue; d <= MaxValue && solved; d++)
      {
         if (!rowConflicts[k][d] || !colConflicts[k][d] || !sqConflicts[k][d])
            solved = false;
      }
   }

   // Print based on the single computed result, so the message and the
   // returned value can never disagree.
   if (solved)
      cout << "The board is solved." << endl;
   else
      cout << "The board is not solved." << endl;

   return solved;
}

// Return the number of recursive solve() calls used on the most recent board.
int board::getRecursiveCall() {
   return recursiveCall;
}

// (b1) Find the next blank cell to fill. Scans in row-major order and returns
// the first blank found, setting row/col to its location. Returns true if a
// blank exists, or false if the board is full (which means it is solved).
bool board::findBlank(int &row, int &col) {

   for (int i = 1; i <= BoardSize; i++)
   {
      for (int j = 1; j <= BoardSize; j++)
      {
         if (isBlank(i,j)) {
            row = i; col = j;      // hand the blank's coordinates back to the caller
            return true;
         }
      }
   }
   return false;                   // no blank left: board is full
}

// (b2) Legality check: return true if digit 'val' may be placed in cell (i, j)
// without conflict, i.e. it is not already used in that row, column, or square.
// Thanks to the conflict tables this is O(1) (three flag reads) rather than a
// scan of 27 cells.
bool board::isLegal(int i, int j, ValueType val)
{
   int sq = squareNumber(i, j);
   return !rowConflicts[i][val] && !colConflicts[j][val] && !sqConflicts[sq][val];
}

// (b3) Recursive backtracking solver. Returns true if the board can be solved
// from its current state, false if this branch is a dead end. Every call
// increments recursiveCall so the number of recursive iterations can be
// reported (the metric the assignment asks us to minimize).
bool board::solve() {
   recursiveCall++;               // count this call (initial call and every recursion)
   int row; int col;
   // Base case: no blank cell left means the board is full -> solved.
   if (!findBlank(row, col)) {
      return true;
   }
   else {
      // Try every legal digit in the chosen blank cell.
      for (int val = MinValue; val <=MaxValue; val++ ){
         if (isLegal(row, col, val)) {

         setCell(row,col,val);          // place the digit (updates conflicts)
         if (solve()) return true;      // recurse; success propagates straight up
         clearCell(row, col);           // undo (backtrack) and try the next digit
         }
      }
      return false;              // no digit worked: report failure so caller backtracks
   }

}

int main()
{
   ifstream fin;

   string fileName = "sudoku.txt";   // input file: one board per line

   // Open the input file; abort if it can't be read.
   fin.open(fileName.c_str());
   if (!fin)
   {
      cerr << "Cannot open " << fileName << endl;
      exit(1);
   }

   // Totals tracked across all boards, used to report the average at the end.
   int totalCalls = 0; int boardCount = 0;
   try
   {
      board b1(SquareSize);          // reusable board object

      // Process each board until end of file or the sentinel 'Z'.
      while (fin && fin.peek() != 'Z')
      {
         b1.initialize(fin);         // (a1) load the next board (also resets counter)
         b1.print();                 // (a2) show the board
         b1.printConflicts();        // (a2) show the conflict tables
         b1.isSolved();              // (a5) report solved / not solved (before solving)
         b1.solve();                 // (b3) solve via recursive backtracking
         b1.isSolved();              // (a5) report solved / not solved (after solving)
         b1.print();                 // print the solved board
         int calls = b1.getRecursiveCall();
         cout << "Recursive calls: " << calls << endl;
         totalCalls += calls;        // accumulate for the average
         boardCount++;
      }
      cout << "Total recursive calls: " << totalCalls << endl;
      // Cast to double so the average is a real number, not truncated integer division.
      cout << "Average recursive calls per board: " << (double)totalCalls / boardCount << endl;
   }
   catch (indexRangeError &ex)
   {
      cout << ex.what() << endl;
      exit(1);
   }
}
