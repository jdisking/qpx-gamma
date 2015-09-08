/*******************************************************************************
 *
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 *
 * This software can be redistributed and/or modified freely provided that
 * any derivative works bear some notice that they are derived from it, and
 * any modified versions bear some notice that they have been modified.
 *
 * Author(s):
 *      Martin Shetty (NIST)
 *
 * Description:
 *      WidgetPlotCalib - 
 *
 ******************************************************************************/


#ifndef WIDGET_PLOT_CALIB_H
#define WIDGET_PLOT_CALIB_H

#include <QWidget>
#include "qsquarecustomplot.h"
#include "marker.h"
#include <set>

namespace Ui {
class WidgetPlotCalib;
}

class WidgetPlotCalib : public QWidget
{
  Q_OBJECT

public:
  explicit WidgetPlotCalib(QWidget *parent = 0);
  ~WidgetPlotCalib();
  void clearGraphs();

  void redraw();

  void setLabels(QString x, QString y);
  void setFloatingText(QString);

  std::set<double> get_selected_pts();
  void set_selected_pts(std::set<double>);

  void addPoints(const QVector<double>& x, const QVector<double>& y, AppearanceProfile style);
  void addFit(const QVector<double>& x, const QVector<double>& y, AppearanceProfile style);

signals:
  void clickedLeft(double);
  void clickedRight(double);
  void selection_changed();

private slots:
  void update_selection();

private:
  void setColorScheme(QColor fore, QColor back, QColor grid1, QColor grid2);

  Ui::WidgetPlotCalib *ui;

  QVector<double> x_fit, y_fit, x_pts, y_pts;
  std::set<double> selection_;

  AppearanceProfile style_pts, style_fit;

  QString floating_text_;
};

#endif // WIDGET_PLOST1D_H
