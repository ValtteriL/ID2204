
/*
 *  Author(s):
 *	Bharath Karthikeyan
 *	Guidance from: http://www.gecode.org/doc-latest/reference/classQueens.html
 *
 *	Running instructions:
 *	cl /DNDEBUG /EHsc /MD /wd4355 -I"%GECODEDIR%\include" -c -Foqueens.obj -Tpqueens.cpp
 *	cl /DNDEBUG /EHsc /MD /wd4355 -I"%GECODEDIR%\include" -Fequeens.exe queens.obj /link /LIBPATH:"%GECODEDIR%\lib"
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

class Queens : public Script {
public:
  /// Position of queens on boards
  IntVarArray q;
  /// Propagation to use for model
  enum {
    PROP_BINARY,  ///< Use only binary disequality constraints
    PROP_MIXED,   ///< Use single distinct and binary disequality constraints
    PROP_DISTINCT ///< Use three distinct constraints
  };
  /// The actual problem
  Queens(const SizeOptions& opt)
    : Script(opt), q(*this,opt.size(),0,opt.size()-1) {
    const int n = q.size();
    switch (opt.propagation()) {
    case PROP_BINARY:
      for (int i = 0; i<n; i++)
        for (int j = i+1; j<n; j++) {
          rel(*this, q[i] != q[j]);
          rel(*this, q[i]+i != q[j]+j);
          rel(*this, q[i]-i != q[j]-j);
        }
      break;
    case PROP_MIXED:
      for (int i = 0; i<n; i++)
        for (int j = i+1; j<n; j++) {
          rel(*this, q[i]+i != q[j]+j);
          rel(*this, q[i]-i != q[j]-j);
        }
      distinct(*this, q, opt.ipl());
      break;
    case PROP_DISTINCT:
      distinct(*this, IntArgs::create(n,0,1), q, opt.ipl());
      distinct(*this, IntArgs::create(n,0,-1), q, opt.ipl());
      distinct(*this, q, opt.ipl());
      break;
    }
    branch(*this, q, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
  }

  /// Constructor for cloning \a s
  Queens(Queens& s) : Script(s) {
    q.update(*this, s.q);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(void) {
    return new Queens(*this);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "queens\t";
    for (int i = 0; i < q.size(); i++) {
      os << q[i] << ", ";
      if ((i+1) % 10 == 0)
        os << std::endl << "\t";
    }
    os << std::endl;
  }
};

#if defined(GECODE_HAS_QT) && defined(GECODE_HAS_GIST)
/// Inspector showing queens on a chess board
class QueensInspector : public Gist::Inspector {
protected:
  /// The graphics scene displaying the board
  QGraphicsScene* scene;
  /// The window containing the graphics scene
  QMainWindow* mw;
  /// The size of a field on the board
  static const int unit = 20;
public:
  /// Constructor
  QueensInspector(void) : scene(NULL), mw(NULL) {}
  /// Inspect space \a s
  virtual void inspect(const Space& s) {
    const Queens& q = static_cast<const Queens&>(s);

    if (!scene)
      initialize();
    QList <QGraphicsItem*> itemList = scene->items();
    foreach (QGraphicsItem* i, scene->items()) {
      scene->removeItem(i);
      delete i;
    }

    for (int i=0; i<q.q.size(); i++) {
      for (int j=0; j<q.q.size(); j++) {
        scene->addRect(i*unit,j*unit,unit,unit);
      }
      QBrush b(q.q[i].assigned() ? Qt::black : Qt::red);
      QPen p(q.q[i].assigned() ? Qt::black : Qt::white);
      for (IntVarValues xv(q.q[i]); xv(); ++xv) {
        scene->addEllipse(QRectF(i*unit+unit/4,xv.val()*unit+unit/4,
                                 unit/2,unit/2), p, b);
      }
    }
    mw->show();
  }

  /// Set up main window
  void initialize(void) {
    mw = new QMainWindow();
    scene = new QGraphicsScene();
    QGraphicsView* view = new QGraphicsView(scene);
    view->setRenderHints(QPainter::Antialiasing);
    mw->setCentralWidget(view);
    mw->setAttribute(Qt::WA_QuitOnClose, false);
    mw->setAttribute(Qt::WA_DeleteOnClose, false);
    QAction* closeWindow = new QAction("Close window", mw);
    closeWindow->setShortcut(QKeySequence("Ctrl+W"));
    mw->connect(closeWindow, SIGNAL(triggered()),
                mw, SLOT(close()));
    mw->addAction(closeWindow);
  }

  /// Name of the inspector
  virtual std::string name(void) { return "Board"; }
  /// Finalize inspector
  virtual void finalize(void) {
    delete mw;
    mw = NULL;
  }
};

#endif /* GECODE_HAS_GIST */

/** \brief Main-function
 *  \relates Queens
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("Queens");
  opt.iterations(500);
  opt.size(8);
  opt.propagation(Queens::PROP_DISTINCT);
  opt.propagation(Queens::PROP_BINARY, "binary",
                      "only binary disequality constraints");
  opt.propagation(Queens::PROP_MIXED, "mixed",
                      "single distinct and binary disequality constraints");
  opt.propagation(Queens::PROP_DISTINCT, "distinct",
                      "three distinct constraints");

#if defined(GECODE_HAS_QT) && defined(GECODE_HAS_GIST)
  QueensInspector ki;
  opt.inspect.click(&ki);
#endif

  opt.parse(argc,argv);
  Script::run<Queens,DFS,SizeOptions>(opt);
  return 0;
}
