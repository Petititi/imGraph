#ifndef _PARAMETERS_DOCK_HEADER_
#define _PARAMETERS_DOCK_HEADER_

#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503)
#endif
#include <QGraphicsView>
#include <QResizeEvent>
#include <QDialog >
#include <QPainter>
#include <QRect>
#include <QString>
#include <QDialog>
#include <QLabel>
#include <QWidget>
#include <QLayout>
#include <QMainWindow>
#include <QComboBox>
#include <QDial>
#include <QCheckBox>
#include <QPainterPath>
#include <QGroupBox>
#include <QBasicTimer>

#include <boost/bimap.hpp>

#ifdef _WIN32
#pragma warning(pop)
#endif

#include "Block.h"

#include <map>
#include "Window.h"
#include "Configuration.h"
#include "Connectors.h"

namespace charliesoft
{

  class ConditionConfigurator :public QDialog
  {
    Q_OBJECT;
    VertexRepresentation* _vertex;

    QPushButton* _OKbutton;
    QPushButton* _Cancelbutton;
    QPushButton* _Deletebutton;
    QComboBox* _condition_left;
    QComboBox* _condition_type;
    QComboBox* _condition_right;

    QLineEdit* __valueleft;
    QLineEdit* __valueright;

    QGridLayout* _comboBoxLayout;
  public:
    ConditionConfigurator(VertexRepresentation* vertex);
    public slots:
    void accept_button();
    void reject_button();
    void delete_button();
    void updateLeft(int);
    void updateRight(int);

  signals:
    void askSynchro();
  };

  class ParamsConfigurator :public QDialog
  {
    Q_OBJECT;

    QLabel* _statLabel;
    QBasicTimer _timer;

    std::map<QObject*, QLineEdit*> openFiles_;
    std::map<ParamRepresentation*, cv::Mat> _paramMatrix;
    std::map<ParamRepresentation*, cv::Scalar> _paramColor;

    std::map<QGroupBox*, ParamRepresentation*> _inputGroup;
    std::map<QWidget*, std::vector<ParamRepresentation*>> _subparamGroup;
    std::map<ParamRepresentation*, std::vector<QWidget*>> _anyTypeValues;
    std::map<QGroupBox*, ParamRepresentation*> _outputGroup;

    std::map<QCheckBox*, ParamRepresentation* > _inputModificator12;
    std::map<ParamRepresentation*, QCheckBox*> _inputModificator21;
    std::map<ParamRepresentation*, QObject* > _inputValue12;
    std::map<QObject*, ParamRepresentation*> _inputValue21;

    std::vector< LinkConnexionRepresentation*>& _in_param;
    std::map<std::string, LinkConnexionRepresentation*>& _sub_param;
    std::vector< LinkConnexionRepresentation*>& _out_param;

    std::map<ParamRepresentation*, QWidget*> subWidget_;

    VertexRepresentation* _vertex;

    QTabWidget* tabWidget_;
    std::vector<QVBoxLayout *> tabs_content_;

    QPushButton* _switchSynchro;

    void addParamOut(ParamRepresentation  *p);
    void addParamIn(ParamRepresentation  *p, ParamRepresentation* parent = NULL);
    bool updateParamModel(ParamRepresentation* param, bool withAlert = true);

    void addParam(ParamDefinition* param, bool input);

    void updateInfoTab();
  public:
    ParamsConfigurator(VertexRepresentation* vertex);

    void timerEvent(QTimerEvent * ev);
    cv::Scalar getColor(ParamRepresentation* rep) const{
      auto iter = _paramColor.find(rep);
      return iter != _paramColor.end() ? iter->second : cv::Scalar();
    };
    void setColor(ParamRepresentation* rep, cv::Scalar val){
      _paramColor[rep] = val;
    };
    cv::Mat getMatrix(ParamRepresentation* rep) const{
      auto iter = _paramMatrix.find(rep);
      return iter != _paramMatrix.end() ? iter->second : cv::Mat();
    };
    void setMatrix(ParamRepresentation* rep, cv::Mat val){
      _paramMatrix[rep] = val;
    };
  signals:
    void changeVisibility(bool isVisible);
    void askSynchro();

    public slots:
    void changeTab(int);
    void changeSynchro();
    void textChanged();
    void switchEnable(int);
    void switchParamUse(bool);
    void openFile();
    void configCondition();
    void matrixEditor();
    void colorEditor();
    void subParamChange(int);

    void addNewParamIn();
    void addNewParamOut();
  };


  class AnyTypeWidget : public QWidget
  {
    Q_OBJECT;

    std::vector<QWidget*> _widgets;
    ParamsConfigurator* _configurator;
    ParamRepresentation* _paramRep;
    QComboBox* _combo;
    QVBoxLayout* _vbox;
    int _oldIndex;

  public:
    AnyTypeWidget(ParamsConfigurator* configurator, ParamRepresentation* param);

    ParamValue getValue();
    public slots:
    void anyTypeChange(int);
  };
}


#endif