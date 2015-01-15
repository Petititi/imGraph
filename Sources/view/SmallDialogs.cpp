
#include "SmallDialogs.h"

#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4244 4251 4275 4800 4503)
#endif
#include <QLabel>
#include <QVBoxLayout>
#include "MatrixViewer.h"
//#include "..\src\gui\dialogs\qcolordialog.h"
#include <QColorDialog>
#ifdef _WIN32
#pragma warning(pop)
#endif

using namespace std;

namespace charliesoft
{

  CreateParamWindow::CreateParamWindow()
  {
    _QT("CREATE_PARAM_TITLE");

    _OK = new QPushButton(_QT("BUTTON_OK"));
    _OK->setDefault(true);
    _cancel = new QPushButton(_QT("BUTTON_CANCEL"));

    _type = new QComboBox();
    _type->addItem(_QT("TYPE_DATAS_BOOL"));
    _type->addItem(_QT("TYPE_DATAS_INT"));
    _type->addItem(_QT("TYPE_DATAS_FLOAT"));
    _type->addItem(_QT("TYPE_DATAS_COLOR"));
    _type->addItem(_QT("TYPE_DATAS_MATRIX"));
    _type->addItem(_QT("TYPE_DATAS_STRING"));
    _type->addItem(_QT("TYPE_DATAS_FILE"));


    _name = new QLineEdit();
    _helper = new QLineEdit();

    _initialValue_text = new QLineEdit();
    _initialValue_button_color = new QPushButton();
    _initialValue_button_matrix = new QPushButton();

    //create the layout:
    QVBoxLayout *vbox = new QVBoxLayout();
    setLayout(vbox);
    QWidget *tmp = new QWidget();
    QHBoxLayout *hbox = new QHBoxLayout();
    tmp->setLayout(hbox);
    hbox->addWidget(new QLabel(_QT("CREATE_PARAM_TYPE")));
    hbox->addWidget(_type);
    vbox->addWidget(tmp);

    tmp = new QWidget();
    hbox = new QHBoxLayout();
    tmp->setLayout(hbox);
    hbox->addWidget(new QLabel(_QT("CREATE_PARAM_NAME")));
    hbox->addWidget(_name);
    vbox->addWidget(tmp);

    vbox->addWidget(new QLabel(_QT("CREATE_PARAM_NAME_HELP")));
    vbox->addWidget(_helper);

    tmp = new QWidget();
    _initValLayout = new QHBoxLayout();
    tmp->setLayout(_initValLayout);
    _initValLayout->addWidget(new QLabel(_QT("CREATE_PARAM_INIT_VAL")));
    _initValLayout->addWidget(_initialValue_text);
    vbox->addWidget(tmp);

    tmp = new QWidget();
    hbox = new QHBoxLayout();
    tmp->setLayout(hbox);
    hbox->addWidget(_OK);
    hbox->addWidget(_cancel);
    vbox->addWidget(tmp);

    connect(_initialValue_button_color, SIGNAL(clicked()), this, SLOT(colorEditor()));
    connect(_initialValue_button_matrix, SIGNAL(clicked()), this, SLOT(matrixEditor()));
    connect(_OK, SIGNAL(clicked()), this, SLOT(ok_exit()));
    connect(_cancel, SIGNAL(clicked()), this, SLOT(cancel_exit()));

    connect(_type, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChange(int)));
  }
  CreateParamWindow::~CreateParamWindow()
  {

  };

  void CreateParamWindow::typeChange(int newIndex)
  {
    if (newIndex == 4)//matrix type:
    {
      _initValLayout->removeWidget(_initialValue_text);
      _initValLayout->removeWidget(_initialValue_button_color);
      _initValLayout->addWidget(_initialValue_button_matrix);
    }
    else
    {
      if (newIndex == 3)//color type:
      {
        _initValLayout->removeWidget(_initialValue_text);
        _initValLayout->removeWidget(_initialValue_button_matrix);
        _initValLayout->addWidget(_initialValue_button_color);
      }
      else
      {
        _initValLayout->removeWidget(_initialValue_button_matrix);
        _initValLayout->removeWidget(_initialValue_button_color);
        _initValLayout->addWidget(_initialValue_text);
      }
    }
  };

  void CreateParamWindow::matrixEditor()
  {
    MatrixViewer* win = createWindow(_STR("BUTTON_MATRIX"), _WINDOW_MATRIX_CREATION_MODE);
    win->setParent(this, Qt::Tool);
    if (win->exec() == QDialog::Accepted)//now try to get matrix:
      _initMat = win->getMatrix();
    delete win;
  }

  void CreateParamWindow::colorEditor()
  {
    QColor tmpColor = QColorDialog::getColor(
      QColor(), this);
    if (tmpColor.isValid())
    {
      _initColor[0] = tmpColor.blue();
      _initColor[1] = tmpColor.green();
      _initColor[2] = tmpColor.red();
      _initColor[3] = tmpColor.alpha();
    }
  }

  void CreateParamWindow::ok_exit()
  {
    accept();
  };
  void CreateParamWindow::cancel_exit()
  {
    reject();
  };

  ParamDefinition CreateParamWindow::getParamDef() const
  {
    string name = _name->text().toStdString();
    string help = _helper->text().toStdString();
    ParamType type;
    switch (_type->currentIndex())
    {
    case 1:
      type = Int;
      _initVal = _initialValue_text->text().toInt(0);
      break;
    case 2:
      type = Float;
      _initVal = _initialValue_text->text().toFloat(0);
      break;
    case 3:
      type = Color;
      _initVal = _initColor;
      break;
    case 4:
      type = Matrix;
      _initVal = _initMat;
      break;
    case 5:
      type = String;
      _initVal = _initialValue_text->text().toStdString();
      break;
    case 6:
      type = FilePath;
      _initVal = _initialValue_text->text().toStdString();
      break;
    default:
      type = Boolean;
      _initVal = _initialValue_text->text().toInt(0) != 0;
      break;
    }
    return ParamDefinition(true, type, name, help, _initVal);
  }
}