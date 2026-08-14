# EECE2560-Sudoku

# Project #3 – Sudoku Solver

## Description

This project implements a Sudoku solver in C++ using recursive backtracking. The program reads Sudoku boards from `sudoku.txt`, stores each board in a matrix, solves the puzzle, prints the completed board, and reports the number of recursive calls required.

## Board Representation

The Sudoku board is represented using a 9x9 matrix. Blank cells are represented by `-1`.

Three additional conflict tables are maintained:

* `rowConflicts` – tracks values already used in each row
* `colConflicts` – tracks values already used in each column
* `sqConflicts` – tracks values already used in each 3x3 square

These tables allow the program to quickly determine whether a value can legally be placed in a cell.

## Part A

The first portion of the project implements the basic Sudoku board operations.

* `initialize()` reads a Sudoku puzzle from the input file.
* `print()` displays the Sudoku board.
* `setCell()` places a value into a cell and updates the conflict tables.
* `clearCell()` removes a value and updates the conflict tables.
* `isBlank()` determines whether a cell is empty.
* `isSolved()` determines whether the Sudoku board has been completely solved.
* `printConflicts()` displays the current row, column, and square conflicts.

## Part B

The second portion implements the recursive Sudoku solver.

### Finding a Blank Cell

`findBlank()` locates a blank cell that can be filled by the solver.

### Checking Legal Values

`isLegal()` determines whether a value can be placed in a particular cell without violating Sudoku rules. Instead of searching the entire row, column, and 3x3 square each time, the function checks the three conflict tables.

This makes each legality check efficient because only three conflict-table values need to be checked.

### Recursive Backtracking

`solve()` uses recursive backtracking to solve the puzzle.

For each selected blank cell, the solver:

1. Tries values from 1 through 9.
2. Uses `isLegal()` to determine whether the value is valid.
3. Places a legal value using `setCell()`.
4. Recursively attempts to solve the remaining board.
5. If the attempt fails, removes the value using `clearCell()` and tries the next possible value.
6. Returns true once the complete Sudoku board has been solved.

## Recursive Call Optimization

The program tracks the number of calls to `solve()` using the `recursiveCall` variable. This provides a way to measure the efficiency of the recursive backtracking algorithm.

The original implementation, which selected blank cells in row-major order, required approximately **4.09529 × 10^6 recursive calls per board on average**.

An improved blank-cell selection method reduced this to approximately **22,666.8 recursive calls per board on average**.

This represents a significant reduction in unnecessary recursive searching. Selecting more constrained cells earlier allows invalid branches to be discovered sooner, reducing the amount of backtracking required.

## Input

The program reads puzzles from:

`sudoku.txt`

Each Sudoku puzzle contains digits `1-9` for filled cells and `.` for blank cells.

## Output

For each Sudoku puzzle, the program displays:

* The original board
* Conflict information
* Whether the original board is solved
* The solved board
* Whether the resulting board is solved
* Number of recursive calls

After all puzzles have been processed, the program also displays:

* Total number of recursive calls
* Average number of recursive calls per board

## Files / Requirements

The program requires:

* Sudoku solver source file
* `sudoku.txt`
* `d_matrix.h`
* `d_except.h`

## Summary

The project demonstrates recursive backtracking applied to Sudoku. Conflict tables improve the efficiency of legality checks, while improved blank-cell selection greatly reduces the amount of recursive backtracking needed to solve the provided puzzles.
