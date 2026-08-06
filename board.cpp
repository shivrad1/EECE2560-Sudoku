// Declarations and functions for project #4

#include <iostream>
#include <limits.h>
#include "d_matrix.h"
#include "d_except.h"
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

const int BoardSize = SquareSize * SquareSize;

const int MinValue = 1;
const int MaxValue = 9;

int numSolutions = 0;

int squareNumber(int i, int j)
// Return the square number of cell i,j (counting from left to right,
// top to bottom.  Note that i and j each go from 1 to BoardSize
{
   return SquareSize * ((i-1)/SquareSize) + (j-1)/SquareSize + 1;
}

class board
// Stores the entire Sudoku board
{
   public:
      board(int);
      void clear();
      void initialize(ifstream &fin);
      void print();
      bool isBlank(int, int);
      ValueType getCell(int, int);
      void setCell(int i, int j, ValueType val);
      void printConflicts();
      
   private:
      // Matrices indexed from 1 to BoardSize
      matrix<ValueType> value;

      // Conflict tracking matrices using Approach (2):
      matrix<bool> rowConflicts;
      matrix<bool> colConflicts;
      matrix<bool> sqConflicts;
};

board::board(int sqSize)
   : value(BoardSize+1, BoardSize+1),
     rowConflicts(BoardSize+1, MaxValue+1),
     colConflicts(BoardSize+1, MaxValue+1),
     sqConflicts(BoardSize+1, MaxValue+1)
// Board constructor
{
   clear();
}

void board::clear()
// Mark all possible values as legal for each board entry
{
   for (int i = 1; i <= BoardSize; i++)
   {
      for (int j = 1; j <= BoardSize; j++)
      {
         value[i][j] = Blank;
      }
   }

   // Reset conflict flags to false
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

void board::setCell(int i, int j, ValueType val)
// Sets cell i,j to val and updates conflict tracking matrices
{
   value[i][j] = val;
   int sq = squareNumber(i, j);

   if (val != Blank)
   {
      rowConflicts[i][val] = true;
      colConflicts[j][val] = true;
      sqConflicts[sq][val] = true;
   }
}

void board::initialize(ifstream &fin)
// Read a Sudoku board from the input file.
{
   char ch;

   clear();
   
   for (int i = 1; i <= BoardSize; i++)
   {
      for (int j = 1; j <= BoardSize; j++)
      {
         fin >> ch;

         // If the read char is not Blank
         if (ch != '.')
         {
            setCell(i, j, ch - '0');   // Convert char to int
         }
      }
   }
}

ValueType board::getCell(int i, int j)
{
   if (i >= 1 && i <= BoardSize && j >= 1 && j <= BoardSize)
      return value[i][j];
   else
      throw rangeError("bad value in getCell");
}

bool board::isBlank(int i, int j)
{
   if (i < 1 || i > BoardSize || j < 1 || j > BoardSize)
      throw rangeError("bad value in setCell");

   return (getCell(i,j) == Blank);
}

void board::print()
// Prints the current board.
{
   for (int i = 1; i <= BoardSize; i++)
   {
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
         if ((j-1) % SquareSize == 0)
            cout << "|";
         if (!isBlank(i,j))
            cout << " " << getCell(i,j) << " ";
         else
            cout << "   ";
      }
      cout << "|";
      cout << endl;
   }

   cout << " -";
   for (int j = 1; j <= BoardSize; j++)
      cout << "---";
   cout << "-";
   cout << endl;
}

void board::printConflicts()
// Displays which digits are present (in conflict) for each row, column, and square
{
   cout << "\nCONFLICTS TABLE:" << endl;
   
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

int main()
{
   ifstream fin;
   
   string fileName = "sudoku.txt";

   fin.open(fileName.c_str());
   if (!fin)
   {
      cerr << "Cannot open " << fileName << endl;
      exit(1);
   }

   try
   {
      board b1(SquareSize);

      while (fin && fin.peek() != 'Z')
      {
         b1.initialize(fin);
         b1.print();
         b1.printConflicts();
      }
   }
   catch (indexRangeError &ex)
   {
      cout << ex.what() << endl;
      exit(1);
   }
}