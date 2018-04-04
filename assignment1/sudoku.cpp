/*
 *  Authors:
 *    Valtteri Lehtinen (with a lil help from http://www.gecode.org/doc-latest/reference/examples_2sudoku_8cpp_source.html)
 *
 *  to compile:
 *  	g++ -L /usr/local/lib -lgecodesearch -lgecodeint -lgecodekernel -lgecodesupport -lgecodedriver -lgecodeminimodel -lgecodegist -o sudoku sudoku.cpp A1.cpp
 *  to run: 
 *  	./sudoku
 *  	./sudoku -ipl <insert wanted ipl here>
 *
 *  By giving different values to ipl (integer propagation level) using the first sudoku, we get following results:
 *  ipl,    no. of propagations,  nodes, peak depth
 *  DEF     161                    13     10
 *  VAL     161                    13     10
 *  BND     313                    9      7
 *  DOM     169                    1      0
 *
 */

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include "A1.cpp" // include the example sudokus

using namespace Gecode;

class Sudoku : public Script {
protected:
  const int n = 3; // matrix is 3*3 * 3*3
public:
  // Branching variants
  enum {
    BRANCH_NONE,        
    BRANCH_SIZE,        
    BRANCH_SIZE_DEGREE, 
    BRANCH_SIZE_AFC,    
    BRANCH_AFC          
  };

  Sudoku(const SizeOptions& opt): Script(opt),n(3) {}

  Sudoku(Sudoku& s) : Script(s), n(s.n) {}

};


class SudokuInt : virtual public Sudoku {
protected:
  IntVarArray l;
public:

  SudokuInt(const SizeOptions& opt) : Sudoku(opt), l(*this, n*n*n*n, 1, n*n) {
    const int nn = n*n;
    Matrix<IntVarArray> m(l, nn, nn); // define matrix

    // Constraints for rows and columns
     for (int i=0; i<nn; i++) {
       distinct(*this, m.row(i), opt.ipl());
       distinct(*this, m.col(i), opt.ipl());
     }
 
     // Constraints for squares
     for (int i=0; i<nn; i+=n) {
       for (int j=0; j<nn; j+=n) {
         distinct(*this, m.slice(i, i+n, j, j+n), opt.ipl());
       }
     }
 
     // Fill-in predefined fields
     for (int i=0; i<nn; i++)
       for (int j=0; j<nn; j++)
         if (int v = examples[opt.size()][i][j])
           rel(*this, m(i,j), IRT_EQ, v );


    // post branching
    if (opt.branching() == BRANCH_NONE) {
       branch(*this, l, INT_VAR_NONE(), INT_VAL_SPLIT_MIN());
     } else if (opt.branching() == BRANCH_SIZE) {
       branch(*this, l, INT_VAR_SIZE_MIN(), INT_VAL_SPLIT_MIN());
     } else if (opt.branching() == BRANCH_SIZE_DEGREE) {
       branch(*this, l, INT_VAR_DEGREE_SIZE_MAX(), INT_VAL_SPLIT_MIN());
     } else if (opt.branching() == BRANCH_SIZE_AFC) {
       branch(*this, l, INT_VAR_AFC_SIZE_MAX(opt.decay()), INT_VAL_SPLIT_MIN());
     } else if (opt.branching() == BRANCH_AFC) {
       branch(*this, l, INT_VAR_AFC_MAX(opt.decay()), INT_VAL_SPLIT_MIN());
     }
   }
  
  

  // search support
  SudokuInt(SudokuInt& s) : Sudoku(s) {
    l.update(*this, s.l);
  }
  virtual Space* copy(void) {
    return new SudokuInt(*this);
  }
  // print solution
  void print(std::ostream& os) const {
     os << "  ";
     for (int i = 0; i<n*n*n*n; i++) {
       if (l[i].assigned()) {
         if (l[i].val()<10)
           os << l[i] << " ";
         else
           os << (char)(l[i].val()+'A'-10) << " ";
       }
       else
         os << ". ";
       if((i+1)%(n*n) == 0)
         os << std::endl << "  ";
     }
     os << std::endl;
  }
};

// main function
int main(int argc, char* argv[]) {
    // commandline options
    SizeOptions opt("SUDOKU");
    opt.size(0);
    opt.ipl(IPL_DOM); // use DOM ipl by default
    opt.solutions(1); // show only first solution by default
    opt.branching(SudokuInt::BRANCH_SIZE_AFC);
    opt.branching(SudokuInt::BRANCH_NONE, "none", "none");
    opt.branching(SudokuInt::BRANCH_SIZE, "size", "min size");
    opt.branching(SudokuInt::BRANCH_SIZE_DEGREE, "sizedeg", "min size over degree");
    opt.branching(SudokuInt::BRANCH_SIZE_AFC, "sizeafc", "min size over afc");
    opt.branching(SudokuInt::BRANCH_AFC, "afc", "maximum afc");
    opt.parse(argc,argv);
    // run script
    Script::run<SudokuInt,DFS,SizeOptions>(opt);
    return 0;
}
