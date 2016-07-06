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
 *
 ******************************************************************************/

#include "form_plot1d.h"
#include "ui_form_plot1d.h"
#include "dialog_spectrum.h"
#include "custom_timer.h"
#include "form_manip1d.h"
#include "boost/algorithm/string.hpp"

using namespace Qpx;

FormPlot1D::FormPlot1D(QWidget *parent) :
  QWidget(parent),
  detectors_(nullptr),
  ui(new Ui::FormPlot1D)
{
  ui->setupUi(this);

  spectraSelector = new SelectorWidget();
  spectraSelector->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  spectraSelector->setMaximumWidth(280);
  ui->scrollArea->setWidget(spectraSelector);
  //ui->scrollArea->viewport()->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  //ui->scrollArea->viewport()->setMinimumWidth(283);

  moving.appearance.themes["light"] = QPen(Qt::darkGray, 2);
  moving.appearance.themes["dark"] = QPen(Qt::white, 2);

  markx.appearance.themes["light"] = QPen(Qt::darkRed, 1);
  markx.appearance.themes["dark"] = QPen(Qt::yellow, 1);
  marky = markx;

  connect(ui->mcaPlot, SIGNAL(clickedLeft(double)), this, SLOT(addMovingMarker(double)));
  connect(ui->mcaPlot, SIGNAL(clickedRight(double)), this, SLOT(removeMovingMarker(double)));

  connect(spectraSelector, SIGNAL(itemSelected(SelectorItem)), this, SLOT(spectrumDetails(SelectorItem)));
  connect(spectraSelector, SIGNAL(itemToggled(SelectorItem)), this, SLOT(spectrumLooksChanged(SelectorItem)));
  connect(spectraSelector, SIGNAL(itemDoubleclicked(SelectorItem)), this, SLOT(spectrumDoubleclicked(SelectorItem)));


  menuColors.addAction(QIcon(":/icons/show16.png"), "Show all", this, SLOT(showAll()));
  menuColors.addAction(QIcon(":/icons/hide16.png"), "Hide all", this, SLOT(hideAll()));
  menuColors.addAction(QIcon(":/icons/oxy/16/roll.png"), "Randomize all colors", this, SLOT(randAll()));
  ui->toolColors->setMenu(&menuColors);

  menuDelete.addAction(QIcon(":/icons/oxy/16/editdelete.png"), "Delete selected spectrum", this, SLOT(deleteSelected()));
  menuDelete.addAction(QIcon(":/icons/show16.png"), "Delete shown spectra", this, SLOT(deleteShown()));
  menuDelete.addAction(QIcon(":/icons/hide16.png"), "Delete hidden spectra", this, SLOT(deleteHidden()));
  ui->toolDelete->setMenu(&menuDelete);

  ui->toolEffCal->setMenu(&menuEffCal);
  connect(&menuEffCal, SIGNAL(triggered(QAction*)), this, SLOT(effCalRequested(QAction*)));

}

FormPlot1D::~FormPlot1D()
{
  delete ui;
}

void FormPlot1D::setDetDB(XMLableDB<Detector>& detDB) {
  detectors_ = &detDB;
}


void FormPlot1D::setSpectra(Project& new_set) {
  mySpectra = &new_set;
  updateUI();
}

void FormPlot1D::spectrumLooksChanged(SelectorItem item) {
  SinkPtr someSpectrum = mySpectra->get_sink(item.data.toLongLong());
  if (someSpectrum) {
    Setting vis = someSpectrum->metadata().attributes.branches.get(Setting("visible"));
    vis.value_int = item.visible;
    someSpectrum->set_option(vis);
  }
  mySpectra->activate();
}

void FormPlot1D::spectrumDoubleclicked(SelectorItem item)
{
  on_pushFullInfo_clicked();
}

void FormPlot1D::spectrumDetails(SelectorItem item)
{
  SelectorItem itm = spectraSelector->selected();

  SinkPtr someSpectrum = mySpectra->get_sink(itm.data.toLongLong());

  Metadata md;

  if (someSpectrum)
    ui->pushRescaleToThisMax->setEnabled(true);
  else
    ui->pushRescaleToThisMax->setEnabled(false);

  if (someSpectrum)
    md = someSpectrum->metadata();

  if (!someSpectrum) {
    ui->labelSpectrumInfo->setText("<html><head/><body><p>Left-click: see statistics here<br/>Right click: toggle visibility<br/>Double click: details / analysis</p></body></html>");
    ui->pushFullInfo->setEnabled(false);
    return;
  }

  std::string type = someSpectrum->type();
  double real = md.attributes.branches.get(Setting("real_time")).value_duration.total_milliseconds() * 0.001;
  double live = md.attributes.branches.get(Setting("live_time")).value_duration.total_milliseconds() * 0.001;

  double rate_total = 0;
  if (live > 0)
    rate_total = md.total_count.convert_to<double>() / live; // total count rate corrected for dead time

  double dead = 100;
  if (real > 0)
    dead = (real - live) * 100.0 / real;

  double rate_inst = md.attributes.branches.get(Setting("instant_rate")).value_dbl;

  Detector det = Detector();
  if (!md.detectors.empty())
    det = md.detectors[0];

  uint16_t bits = md.attributes.branches.get(Qpx::Setting("resolution")).value_int;

  QString detstr("Detector: ");
  detstr += QString::fromStdString(det.name_);
  if (det.energy_calibrations_.has_a(Calibration("Energy", bits)))
    detstr += " [ENRG]";
  else if (det.highest_res_calib().valid())
    detstr += " (enrg)";
  if (det.fwhm_calibration_.valid())
    detstr += " [FWHM]";
  if (det.efficiency_calibration_.valid())
    detstr += " [EFF]";

  QString infoText =
      "<nobr>" + itm.text + "(" + QString::fromStdString(type) + ", " + QString::number(bits) + "bits)</nobr><br/>"
      "<nobr>" + detstr + "</nobr><br/>"
      "<nobr>Count: " + QString::number(md.total_count.convert_to<double>()) + "</nobr><br/>"
      "<nobr>Rate (inst/total): " + QString::number(rate_inst) + "cps / " + QString::number(rate_total) + "cps</nobr><br/>"
      "<nobr>Live / real:  " + QString::number(live) + "s / " + QString::number(real) + "s</nobr><br/>"
      "<nobr>Dead:  " + QString::number(dead) + "%</nobr><br/>";

  ui->labelSpectrumInfo->setText(infoText);
  ui->pushFullInfo->setEnabled(true);
}

void FormPlot1D::reset_content() {
  moving.visible = false;
  markx.visible = false;
  marky.visible = false;
  ui->mcaPlot->reset_scales();
  ui->mcaPlot->clearGraphs();
  ui->mcaPlot->clearExtras();
  ui->mcaPlot->replot_markers();
  ui->mcaPlot->rescale();
  ui->mcaPlot->redraw();
}

void FormPlot1D::update_plot() {

  this->setCursor(Qt::WaitCursor);
//  CustomTimer guiside(true);

  std::map<double, double> minima, maxima;

  calib_ = Calibration();

  ui->mcaPlot->clearGraphs();
  for (auto &q: mySpectra->get_sinks(1)) {
    Metadata md;
    if (q.second)
      md = q.second->metadata();

    double livetime = md.attributes.branches.get(Setting("live_time")).value_duration.total_milliseconds() * 0.001;
    double rescale  = md.attributes.branches.get(Setting("rescale")).value_precise.convert_to<double>();
    uint16_t bits = md.attributes.branches.get(Qpx::Setting("resolution")).value_int;

    if (md.attributes.branches.get(Setting("visible")).value_int
        && (md.total_count > 0)) {

      QVector<double> x = QVector<double>::fromStdVector(q.second->axis_values(0));
      QVector<double> y(x.size());

      std::shared_ptr<EntryList> spectrum_data =
          std::move(q.second->data_range({{0, x.size()}}));

//      DBG << "Will plot " << md.name << "  size " << x.size() << "  sz " << spectrum_data->size();

      Detector detector = Detector();
      if (!md.detectors.empty())
        detector = md.detectors[0];
      Calibration temp_calib = detector.best_calib(bits);

      if (temp_calib.bits_ > calib_.bits_)
        calib_ = temp_calib;

      for (auto it : *spectrum_data) {
        double xx = x[it.first[0]];
        double yy = it.second.convert_to<double>() * rescale;
        if (ui->pushPerLive->isChecked() && (livetime > 0))
          yy = yy / livetime;
        y[it.first[0]] = yy;
        if (!minima.count(xx) || (minima[xx] > yy))
          minima[xx] = yy;
        if (!maxima.count(xx) || (maxima[xx] < yy))
          maxima[xx] = yy;
      }

      AppearanceProfile profile;
      profile.default_pen = QPen(QColor(QString::fromStdString(md.attributes.branches.get(Setting("appearance")).value_text)), 1);
      ui->mcaPlot->addGraph(x, y, profile, bits);

    }
  }

  ui->mcaPlot->use_calibrated(calib_.valid());
  ui->mcaPlot->setLabels(QString::fromStdString(calib_.units_), "count");
  ui->mcaPlot->setYBounds(minima, maxima);

  replot_markers();

  std::string new_label = boost::algorithm::trim_copy(mySpectra->identity());
  ui->mcaPlot->setTitle(QString::fromStdString(new_label));

  spectrumDetails(SelectorItem());

//  DBG << "<Plot1D> plotting took " << guiside.ms() << " ms";
  this->setCursor(Qt::ArrowCursor);
}

void FormPlot1D::on_pushFullInfo_clicked()
{  
  SinkPtr someSpectrum = mySpectra->get_sink(spectraSelector->selected().data.toLongLong());
  if (!someSpectrum)
    return;

  if (detectors_ == nullptr)
    return;

  DialogSpectrum* newSpecDia = new DialogSpectrum(*someSpectrum, *detectors_, true, this);
  connect(newSpecDia, SIGNAL(finished(bool)), this, SLOT(spectrumDetailsClosed(bool)));
  connect(newSpecDia, SIGNAL(delete_spectrum()), this, SLOT(spectrumDetailsDelete()));
  connect(newSpecDia, SIGNAL(analyse()), this, SLOT(analyse()));
  newSpecDia->exec();
}

void FormPlot1D::spectrumDetailsDelete()
{
  mySpectra->delete_sink(spectraSelector->selected().data.toLongLong());

  updateUI();
}

void FormPlot1D::updateUI()
{
  SelectorItem chosen = spectraSelector->selected();
  QVector<SelectorItem> items;
  QSet<QString> dets;

  for (auto &q : mySpectra->get_sinks(1)) {
    Metadata md;
    if (q.second != nullptr)
      md = q.second->metadata();

    if (!md.detectors.empty())
      dets.insert(QString::fromStdString(md.detectors.front().name_));

    SelectorItem new_spectrum;
    new_spectrum.text = QString::fromStdString(md.name);
    new_spectrum.data = QVariant::fromValue(q.first);
    new_spectrum.color = QColor(QString::fromStdString(md.attributes.branches.get(Setting("appearance")).value_text));
    new_spectrum.visible = md.attributes.branches.get(Setting("visible")).value_int;
    items.push_back(new_spectrum);
  }


  menuEffCal.clear();

  for (auto &q : dets)
    menuEffCal.addAction(q);

  spectraSelector->setItems(items);
  spectraSelector->setSelected(chosen.text);

  ui->scrollArea->updateGeometry();

  ui->toolColors->setEnabled(spectraSelector->items().size());
  ui->toolDelete->setEnabled(spectraSelector->items().size());
  ui->toolEffCal->setEnabled(menuEffCal.actions().size());

  ui->pushManip1D->setEnabled(spectraSelector->items().size());
  ui->pushRescaleReset->setEnabled(spectraSelector->items().size());


  mySpectra->activate();
}

void FormPlot1D::spectrumDetailsClosed(bool looks_changed) {
  updateUI();
}

void FormPlot1D::effCalRequested(QAction* choice) {
  QString det = choice->text();

  emit requestEffCal(choice->text());
}


void FormPlot1D::analyse()
{
  emit requestAnalysis(spectraSelector->selected().data.toLongLong());
}

void FormPlot1D::addMovingMarker(double x) {
  LINFO << "<Plot1D> marker at " << x;

  if (calib_.valid())
    moving.pos.set_energy(x, calib_);
  else
    moving.pos.set_bin(x, calib_.bits_, calib_);

  moving.visible = true;
  emit marker_set(moving.pos);
  replot_markers();
}

void FormPlot1D::removeMovingMarker(double x) {
  moving.visible = false;
  markx.visible = false;
  marky.visible = false;
  emit marker_set(Coord());
  replot_markers();
}

void FormPlot1D::set_markers2d(Coord x, Coord y) {

  markx.pos = x;
  markx.visible = !x.null();
  marky.pos = y;
  marky.visible = !y.null();

  if (!markx.visible && !marky.visible)
    moving.visible = false;

  replot_markers();
}

void FormPlot1D::replot_markers() {
  std::list<Marker1D> markers;

  markers.push_back(moving);
  markers.push_back(markx);
  markers.push_back(marky);

  ui->mcaPlot->set_markers(markers);
  ui->mcaPlot->replot_markers();
  ui->mcaPlot->redraw();
}


void FormPlot1D::showAll()
{
  spectraSelector->show_all();
  QVector<SelectorItem> items = spectraSelector->items();
  for (auto &q : items) {
    SinkPtr someSpectrum = mySpectra->get_sink(q.data.toLongLong());
    if (!someSpectrum)
      continue;
    Setting vis = someSpectrum->metadata().attributes.branches.get(Setting("visible"));
    vis.value_int = true;
    someSpectrum->set_option(vis);
  }
  mySpectra->activate();
}

void FormPlot1D::hideAll()
{
  spectraSelector->hide_all();
  QVector<SelectorItem> items = spectraSelector->items();
  for (auto &q : items) {
    SinkPtr someSpectrum = mySpectra->get_sink(q.data.toLongLong());
    if (!someSpectrum)
      continue;
    Setting vis = someSpectrum->metadata().attributes.branches.get(Setting("visible"));
    vis.value_int = false;
    someSpectrum->set_option(vis);
  }
  mySpectra->activate();
}

void FormPlot1D::randAll()
{
  QVector<SelectorItem> items = spectraSelector->items();
  for (auto &q : items) {
    SinkPtr someSpectrum = mySpectra->get_sink(q.data.toLongLong());
    if (!someSpectrum)
      continue;
    Setting app = someSpectrum->metadata().attributes.branches.get(Setting("appearance"));
    app.value_text = generateColor().name(QColor::HexArgb).toStdString();
    someSpectrum->set_option(app);
  }

  updateUI();
}

void FormPlot1D::set_scale_type(QString sct) {
  ui->mcaPlot->set_scale_type(sct);
}

void FormPlot1D::set_plot_style(QString stl) {
  ui->mcaPlot->set_plot_style(stl);
}

QString FormPlot1D::scale_type() {
  return ui->mcaPlot->scale_type();
}

QString FormPlot1D::plot_style() {
  return ui->mcaPlot->plot_style();
}

void FormPlot1D::on_pushPerLive_clicked()
{
  update_plot();
}

void FormPlot1D::on_pushRescaleToThisMax_clicked()
{
  SelectorItem itm = spectraSelector->selected();
  SinkPtr someSpectrum = mySpectra->get_sink(itm.data.toLongLong());

  if (!someSpectrum || !moving.visible)
    return;

  Metadata md = someSpectrum->metadata();


  double livetime = md.attributes.branches.get(Setting("live_time")).value_duration.total_milliseconds() * 0.001;
  uint16_t bits = md.attributes.branches.get(Qpx::Setting("resolution")).value_int;

  Calibration cal;
  if (!md.detectors.empty())
    cal = md.detectors[0].best_calib(bits);

  PreciseFloat max = someSpectrum->data({std::round(cal.inverse_transform(moving.pos.energy()))});

  if (ui->pushPerLive->isChecked() && (livetime != 0))
    max = max / livetime;

  if (max == 0)
    return;

  for (auto &q: mySpectra->get_sinks(1))
    if (q.second) {
      Metadata mdt = q.second->metadata();
      double lt = mdt.attributes.branches.get(Setting("live_time")).value_duration.total_milliseconds() * 0.001;
      uint16_t bits = mdt.attributes.branches.get(Qpx::Setting("resolution")).value_int;

      Calibration cal;
      if (!mdt.detectors.empty())
        cal = mdt.detectors[0].best_calib(bits);

      PreciseFloat mc = q.second->data({std::round(cal.inverse_transform(moving.pos.energy()))});

      if (ui->pushPerLive->isChecked() && (lt != 0))
        mc = mc / lt;

      Setting rescale = md.attributes.branches.get(Setting("rescale"));
      if (mc != 0)
        rescale.value_precise = PreciseFloat(max / mc);
      else
        rescale.value_precise = 0;
      q.second->set_option(rescale);
    }
  updateUI();
}

void FormPlot1D::on_pushRescaleReset_clicked()
{
  for (auto &q: mySpectra->get_sinks(1))
    if (q.second) {
      Setting rescale = q.second->metadata().attributes.branches.get(Setting("rescale"));
      rescale.value_precise = 1;
      q.second->set_option(rescale);
    }
  updateUI();
}

void FormPlot1D::on_pushManip1D_clicked()
{
  FormManip1D* newDialog = new FormManip1D(*mySpectra, this);
  newDialog->exec();
}

void FormPlot1D::deleteSelected() {
  mySpectra->delete_sink(spectraSelector->selected().data.toLongLong());
  updateUI();
}

void FormPlot1D::deleteShown() {
  QVector<SelectorItem> items = spectraSelector->items();
  for (auto &q : items)
    if (q.visible)
      mySpectra->delete_sink(q.data.toLongLong());
  updateUI();
}

void FormPlot1D::deleteHidden() {
  QVector<SelectorItem> items = spectraSelector->items();
  for (auto &q : items)
    if (!q.visible)
      mySpectra->delete_sink(q.data.toLongLong());
  updateUI();
}
