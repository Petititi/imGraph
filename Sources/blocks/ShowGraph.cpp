
#include <vector>

#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>
#include <qwt/qwt_plot_grid.h>
#include <qwt/qwt_symbol.h>
#include <qwt/qwt_legend.h>


#include "Block.h"
#include "view/GraphViewer.h"
#include "view/GuiReceiver.h"
#include "ParamValidator.h"
#include "view/Window.h"

using namespace charliesoft;
using std::vector;
using std::string;
using cv::Mat;

namespace charliesoft
{
  BLOCK_BEGIN_INSTANTIATION(ShowGraph);
  //You can add methods, re implement needed functions...
  GraphViewer *graphWindow;
  BLOCK_END_INSTANTIATION(ShowGraph, AlgoType::output, BLOCK__SHOWGRAPH_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(ShowGraph);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(true, Matrix, "BLOCK__SHOWGRAPH_IN_VALUES", "BLOCK__SHOWGRAPH_IN_VALUES_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(ShowGraph);
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(ShowGraph);
  END_BLOCK_PARAMS();

  ShowGraph::ShowGraph() :Block("BLOCK__SHOWGRAPH_NAME"){
    _myInputs["BLOCK__SHOWGRAPH_IN_VALUES"].addValidator({ new ValNeeded() });
    graphWindow = NULL;
  };

  bool ShowGraph::run(bool oneShot){
    cv::Mat in = _myInputs["BLOCK__SHOWGRAPH_IN_VALUES"].get<cv::Mat>(true);

    if (graphWindow==NULL)
      graphWindow = createGraphView("ceci est un test...");

    return true;
  };
};