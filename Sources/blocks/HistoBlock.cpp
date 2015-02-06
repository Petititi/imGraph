
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
  BLOCK_BEGIN_INSTANTIATION(HistogramBlock);
  //You can add methods, re implement needed functions...
  Mat outHisto;
public:
  virtual void init();
  BLOCK_END_INSTANTIATION(HistogramBlock, AlgoType::imgProcess, BLOCK__HISTOGRAM_NAME);

  BEGIN_BLOCK_INPUT_PARAMS(HistogramBlock);
  //Add parameters, with following parameters:
  //default visibility, type of parameter, name (key of internationalizor), helper...
  ADD_PARAMETER(true, Matrix, "BLOCK__HISTOGRAM_IN_IMAGE", "BLOCK__HISTOGRAM_IN_IMAGE_HELP");
  ADD_PARAMETER_FULL(false, Int, "BLOCK__HISTOGRAM_IN_BINS", "BLOCK__HISTOGRAM_IN_BINS_HELP", 256);
  ADD_PARAMETER_FULL(false, Boolean, "BLOCK__HISTOGRAM_IN_ACCUMULATE", "BLOCK__HISTOGRAM_IN_ACCUMULATE_HELP", false);
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_OUTPUT_PARAMS(HistogramBlock);
  ADD_PARAMETER(true, Matrix, "BLOCK__HISTOGRAM_OUT_HISTO", "BLOCK__HISTOGRAM_OUT_HISTO_HELP");
  END_BLOCK_PARAMS();

  BEGIN_BLOCK_SUBPARAMS_DEF(HistogramBlock);
  END_BLOCK_PARAMS();

  HistogramBlock::HistogramBlock() :Block("BLOCK__HISTOGRAM_NAME", true){
    _myInputs["BLOCK__HISTOGRAM_IN_IMAGE"].addValidator({ new ValNeeded() });
  };

  void HistogramBlock::init()
  {
    outHisto = Mat();
  }


  bool HistogramBlock::run(bool oneShot){
    cv::Mat in = _myInputs["BLOCK__HISTOGRAM_IN_IMAGE"].get<cv::Mat>();

    /// Establish the number of bins
    int histSize = _myInputs["BLOCK__HISTOGRAM_IN_BINS"].get<int>();

    bool accu = false;
    if (!_myInputs["BLOCK__HISTOGRAM_IN_ACCUMULATE"].isDefaultValue())
      accu = _myInputs["BLOCK__HISTOGRAM_IN_ACCUMULATE"].get<bool>();

    vector<Mat> bgr_planes;
    split(in, bgr_planes);

    vector<Mat> histoLayers(in.channels());
    if (!outHisto.empty())
      split(outHisto, histoLayers);

    /// Set the ranges ( for B,G,R) )
    float range[] = { 0, histSize };
    const float* histRange = { range };

    for (int i = 0; i < in.channels(); i++)
      calcHist(&bgr_planes[i], 1, 0, Mat(), histoLayers[i], 1, &histSize, &histRange, true, accu);

    //merge layer to the output
    merge(histoLayers, outHisto);

    _myOutputs["BLOCK__HISTOGRAM_OUT_HISTO"] = outHisto;

    return true;
  };
};