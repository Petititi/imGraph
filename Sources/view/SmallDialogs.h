#ifndef _SMALL_DIALOGS_HEADER_
#define _SMALL_DIALOGS_HEADER_

#ifdef _WIN32
#pragma warning(disable:4503)
#pragma warning(push)
#pragma warning(disable:4996 4251 4275 4800 4503)
#endif
#include <QGraphicsLineItem>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QHBoxLayout>
#include <QDialog>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "Block.h"

namespace charliesoft
{

  class CreateParamWindow : public QDialog
  {
    Q_OBJECT
  public:
    CreateParamWindow();
    ~CreateParamWindow();

    ParamDefinition getParamDef() const;
  protected:
    QPushButton* _OK;
    QPushButton* _cancel;

    //type, name, helper, initialValue
    QComboBox* _type;
    QLineEdit* _name;
    QLineEdit* _helper;

    QLineEdit* _initialValue_text;
    QPushButton* _initialValue_button_matrix;
    QPushButton* _initialValue_button_color;

    QHBoxLayout* _initValLayout;
    mutable ParamValue _initVal;
    cv::Mat _initMat;
    cv::Scalar _initColor;

    public slots:
    void ok_exit();
    void cancel_exit();
    void matrixEditor();
    void colorEditor();

    void typeChange(int);
  };
}


#endif