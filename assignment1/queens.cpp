/*
 * Binary n-queens
 * 
 *  Author(s):
 *	Bharath Karthikeyan
 *  Valtteri Lehtinen
 *	Template from: http://www.gecode.org/doc-latest/reference/classQueens.html
 *
 *	
 * What we can do about branching?
 * - Use different one!
 * 
 * Results with different branchers:
 * 
 * INT_VAR_SIZE_MAX(), INT_VAL_MAX()
 * runtime:      0.000 (0.603 ms)
 * solutions:    1
 * propagations: 1164
 * nodes:        45
 * failures:     21
 * restarts:     0
 * no-goods:     0
 * peak depth:   5  
 * 
 * INT_VAR_SIZE_MIN(), INT_VAL_MIN()
 * runtime:      0.001 (1.373 ms)
 * solutions:    1
 * propagations: 1123
 * nodes:        53
 * failures:     21
 * restarts:     0
 * no-goods:     0
 * peak depth:   16
 *
 * INT_VAR_RND(2), INT_VAL_RND(2)
 * runtime:      0.000 (0.426 ms)
 * solutions:    1
 * propagations: 246
 * nodes:        7
 * failures:     1
 * restarts:     0
 * no-goods:     0
 * peak depth:   5
 *
 * We see that INT_VAR_RND(2), INT_VAL_RND(2) was the fastest and INT_VAR_SIZE_MIN(), INT_VAL_MIN() was the slowest brancher for this problem
 *
 * Advantage of this binary model over "standard" (the one used on lecture) model is perhaps less values that variable can take 
 * Disadvantages are that this model doesn't capture the row constraint and is thus more expensive, it also has much more variables
 * 
 */

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#if defined(GECODE_HAS_QT) && defined(GECODE_HAS_GIST)
#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif
#endif

using namespace Gecode;

class Queens : public Script
{
public:
  /// Position of queens on boards
  IntVarArray q; // array of size n*n where each variable is 0 or 1

  /// The actual problem
  Queens(const SizeOptions &opt)
      : Script(opt), q(*this, opt.size() * opt.size(), 0, 1)
  {

    // ### definitions
    const int n = opt.size();
    Matrix<IntVarArray> m(q, n, n); // define matrix with variables from q of size n*n

    // ### constraints

    // only one binary can be 1 on each row and each column
    for (int i = 0; i < n; i++)
    {
      count(*this, m.row(i), 1, IRT_EQ, 1);
      count(*this, m.col(i), 1, IRT_EQ, 1);
    }

    // only one binary can be 1 on each diagonal
    // -> sum of each diagonal is less than equal to 1
    IntVarArgs buffervars;
    int start = 0;
    int end = 0;
    for (int i = 0; i < n; i++)
    {

      // top right
      for (int j = n - 1 - i; j < n; j++)
      {
        buffervars << m(i + j + 1 - n, j);
        end++;
      }
      count(*this, buffervars.slice(start, 1, end - start), 1, IRT_LQ, 1);
      start = end;

      // bottom right
      for (int j = n - 1; j >= i; j--)
      {
        buffervars << m(i - j + n - 1, j);
        end++;
      }
      count(*this, buffervars.slice(start, 1, end - start), 1, IRT_LQ, 1);
      start = end;

      // top left
      for (int j = 0; j <= i; j++)
      {
        buffervars << m(i - j, j);
        end++;
      }
      count(*this, buffervars.slice(start, 1, end - start), 1, IRT_LQ, 1);
      start = end;

      // bottom left
      for (int j = 0; j < n - i; j++)
      {
        buffervars << m(i + j, j);
        end++;
      }
      count(*this, buffervars.slice(start, 1, end - start), 1, IRT_LQ, 1);
      start = end;
    }

    // ### branch
    branch(*this, q, INT_VAR_RND(2), INT_VAL_RND(2));
  }

  /// Constructor for cloning \a s
  Queens(Queens &s) : Script(s)
  {
    q.update(*this, s.q);
  }

  /// Perform copying during cloning
  virtual Space *
  copy(void)
  {
    return new Queens(*this);
  }

  /// Print solution
  virtual void
  print(std::ostream &os) const
  {
    os << "queens\t";
    for (int i = 0; i < q.size(); i++)
    {
      os << q[i] << ", ";
      if ((i + 1) % 10 == 0)
        os << std::endl
           << "\t";
    }
    os << std::endl;
  }
};

#if defined(GECODE_HAS_QT) && defined(GECODE_HAS_GIST)
/// Inspector showing queens on a chess board
class QueensInspector : public Gist::Inspector
{
protected:
  /// The graphics scene displaying the board
  QGraphicsScene *scene;
  /// The window containing the graphics scene
  QMainWindow *mw;
  /// The size of a field on the board
  static const int unit = 20;

public:
  /// Constructor
  QueensInspector(void) : scene(NULL), mw(NULL) {}
  /// Inspect space \a s
  virtual void inspect(const Space &s)
  {
    const Queens &q = static_cast<const Queens &>(s);

    if (!scene)
      initialize();
    QList<QGraphicsItem *> itemList = scene->items();
    foreach (QGraphicsItem *i, scene->items())
    {
      scene->removeItem(i);
      delete i;
    }

    for (int i = 0; i < q.q.size(); i++)
    {
      for (int j = 0; j < q.q.size(); j++)
      {
        scene->addRect(i * unit, j * unit, unit, unit);
      }
      QBrush b(q.q[i].assigned() ? Qt::black : Qt::red);
      QPen p(q.q[i].assigned() ? Qt::black : Qt::white);
      for (IntVarValues xv(q.q[i]); xv(); ++xv)
      {
        scene->addEllipse(QRectF(i * unit + unit / 4, xv.val() * unit + unit / 4,
                                 unit / 2, unit / 2),
                          p, b);
      }
    }
    mw->show();
  }

  /// Set up main window
  void initialize(void)
  {
    mw = new QMainWindow();
    scene = new QGraphicsScene();
    QGraphicsView *view = new QGraphicsView(scene);
    view->setRenderHints(QPainter::Antialiasing);
    mw->setCentralWidget(view);
    mw->setAttribute(Qt::WA_QuitOnClose, false);
    mw->setAttribute(Qt::WA_DeleteOnClose, false);
    QAction *closeWindow = new QAction("Close window", mw);
    closeWindow->setShortcut(QKeySequence("Ctrl+W"));
    mw->connect(closeWindow, SIGNAL(triggered()),
                mw, SLOT(close()));
    mw->addAction(closeWindow);
  }

  /// Name of the inspector
  virtual std::string name(void) { return "Board"; }
  /// Finalize inspector
  virtual void finalize(void)
  {
    delete mw;
    mw = NULL;
  }
};

#endif /* GECODE_HAS_GIST */

/** \brief Main-function
 *  \relates Queens
 */
int main(int argc, char *argv[])
{
  SizeOptions opt("Queens");
  opt.iterations(500);
  opt.size(8);

#if defined(GECODE_HAS_QT) && defined(GECODE_HAS_GIST)
  QueensInspector ki;
  opt.inspect.click(&ki);
#endif

  opt.parse(argc, argv);
  Script::run<Queens, DFS, SizeOptions>(opt);
  return 0;
}
