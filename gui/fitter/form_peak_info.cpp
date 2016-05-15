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
 *      Module and channel settings
 *
 ******************************************************************************/

#include "form_peak_info.h"
#include "ui_form_peak_info.h"
#include "custom_logger.h"

FormPeakInfo::FormPeakInfo(Hypermet &hm, QWidget *parent) :
  hm_(hm),
  ui(new Ui::FormPeakInfo)
{
  ui->setupUi(this);
  this->setFixedSize(this->size());

//  ui->labelCaliEnergy->setText(QString::fromStdString(hm_.cali_nrg_.to_string()));
//  ui->labelCaliFWHM->setText(QString::fromStdString(hm_.cali_fwhm_.to_string()));

  ui->labelCenter->setText(QString::fromStdString(hm_.center_.value.to_string()));
  ui->labelCenterPercent->setText(QString::fromStdString(hm_.center_.value.error_percent()));
  ui->labelAmplitude->setText(QString::fromStdString(hm_.height_.value.to_string()));
  ui->labelAmplitudePercent->setText(QString::fromStdString(hm_.height_.value.error_percent()));
  ui->labelWidth->setText(QString::fromStdString(hm_.width_.value.to_string()));
  ui->labelWidthPercent->setText(QString::fromStdString(hm_.width_.value.error_percent()));

  ui->labelStep->setText(QString::fromStdString(hm_.step_amplitude.value.to_string()));
  ui->labelStepPercent->setText(QString::fromStdString(hm_.step_amplitude.value.error_percent()));

  ui->labelTailH->setText(QString::fromStdString(hm_.tail_amplitude.value.to_string()));
  ui->labelTailHPercent->setText(QString::fromStdString(hm_.tail_amplitude.value.error_percent()));
  ui->labelTailS->setText(QString::fromStdString(hm_.tail_slope.value.to_string()));
  ui->labelTailSPercent->setText(QString::fromStdString(hm_.tail_slope.value.error_percent()));

  ui->labelLskewH->setText(QString::fromStdString(hm_.Lskew_amplitude.value.to_string()));
  ui->labelLskewHPercent->setText(QString::fromStdString(hm_.Lskew_amplitude.value.error_percent()));
  ui->labelLskewS->setText(QString::fromStdString(hm_.Lskew_slope.value.to_string()));
  ui->labelLskewSPercent->setText(QString::fromStdString(hm_.Lskew_slope.value.error_percent()));

  ui->labelRskewH->setText(QString::fromStdString(hm_.Rskew_amplitude.value.to_string()));
  ui->labelRskewHPercent->setText(QString::fromStdString(hm_.Rskew_amplitude.value.error_percent()));
  ui->labelRskewS->setText(QString::fromStdString(hm_.Rskew_slope.value.to_string()));
  ui->labelRskewSPercent->setText(QString::fromStdString(hm_.Rskew_slope.value.error_percent()));


  ui->doubleCenter->setValue(hm_.center_.value.value());
  ui->doubleCenterEpsilon->setValue(hm_.center_.value.uncertainty());
  ui->doubleAmplitude->setValue(hm_.height_.value.value());
  ui->doubleAmplitudeEpsilon->setValue(hm_.height_.value.uncertainty());
  ui->doubleWidth->setValue(hm_.width_.value.value());
  ui->doubleWidthEpsilon->setValue(hm_.width_.value.uncertainty());

  ui->checkStepEnable->setChecked(hm_.step_amplitude.enabled);
  ui->doubleMinStep->setValue(hm_.step_amplitude.lbound);
  ui->doubleMaxStep->setValue(hm_.step_amplitude.ubound);
  ui->doubleInitStep->setValue(hm_.step_amplitude.value.value());

  ui->checkTailEnable->setChecked(hm_.tail_amplitude.enabled);
  ui->doubleMinTailAmp->setValue(hm_.tail_amplitude.lbound);
  ui->doubleMaxTailAmp->setValue(hm_.tail_amplitude.ubound);
  ui->doubleInitTailAmp->setValue(hm_.tail_amplitude.value.value());
  ui->doubleMinTailSlope->setValue(hm_.tail_slope.lbound);
  ui->doubleMaxTailSlope->setValue(hm_.tail_slope.ubound);
  ui->doubleInitTailSlope->setValue(hm_.tail_slope.value.value());

  ui->checkEnableLskew->setChecked(hm_.Lskew_amplitude.enabled);
  ui->doubleMinLskewAmp->setValue(hm_.Lskew_amplitude.lbound);
  ui->doubleMaxLskewAmp->setValue(hm_.Lskew_amplitude.ubound);
  ui->doubleInitLskewAmp->setValue(hm_.Lskew_amplitude.value.value());
  ui->doubleMinLskewSlope->setValue(hm_.Lskew_slope.lbound);
  ui->doubleMaxLskewSlope->setValue(hm_.Lskew_slope.ubound);
  ui->doubleInitLskewSlope->setValue(hm_.Lskew_slope.value.value());

  ui->checkEnableRskew->setChecked(hm_.Rskew_amplitude.enabled);
  ui->doubleMinRskewAmp->setValue(hm_.Rskew_amplitude.lbound);
  ui->doubleMaxRskewAmp->setValue(hm_.Rskew_amplitude.ubound);
  ui->doubleInitRskewAmp->setValue(hm_.Rskew_amplitude.value.value());
  ui->doubleMinRskewSlope->setValue(hm_.Rskew_slope.lbound);
  ui->doubleMaxRskewSlope->setValue(hm_.Rskew_slope.ubound);
  ui->doubleInitRskewSlope->setValue(hm_.Rskew_slope.value.value());

//  ui->checkWidthCommon->setChecked(hm_.width_common);
//  ui->doubleMinWidthCommon->setValue(hm_.width_common_bounds.lbound);
//  ui->doubleMaxWidthCommon->setValue(hm_.width_common_bounds.ubound);
//  ui->doubleMinWidthVariable->setValue(hm_.width_variable_bounds.lbound);
//  ui->doubleMaxWidthVariable->setValue(hm_.width_variable_bounds.ubound);

//  ui->doubleLateralSlack->setValue(hm_.lateral_slack);
//  ui->spinFitterMaxIterations->setValue(hm_.fitter_max_iter);
}

void FormPeakInfo::on_buttonBox_accepted()
{
  hm_.center_.value.setValue(ui->doubleCenter->value());
  hm_.center_.value.setUncertainty(ui->doubleCenterEpsilon->value());

  hm_.height_.value.setValue(ui->doubleAmplitude->value());
  hm_.height_.value.setUncertainty(ui->doubleAmplitudeEpsilon->value());

  hm_.width_.value.setValue(ui->doubleWidth->value());
  hm_.width_.value.setUncertainty(ui->doubleWidthEpsilon->value());


  hm_.step_amplitude.enabled = ui->checkStepEnable->isChecked();
  if (hm_.step_amplitude.enabled) {
    hm_.step_amplitude.lbound = ui->doubleMinStep->value();
    hm_.step_amplitude.ubound = ui->doubleMaxStep->value();
    hm_.step_amplitude.value.setValue(ui->doubleInitStep->value());
  }

  hm_.tail_amplitude.enabled = ui->checkTailEnable->isChecked();
  if (hm_.tail_amplitude.enabled) {
    hm_.tail_amplitude.lbound = ui->doubleMinTailAmp->value();
    hm_.tail_amplitude.ubound = ui->doubleMaxTailAmp->value();
    hm_.tail_amplitude.value.setValue(ui->doubleInitTailAmp->value());
    hm_.tail_slope.lbound = ui->doubleMinTailSlope->value();
    hm_.tail_slope.ubound = ui->doubleMaxTailSlope->value();
    hm_.tail_slope.value.setValue(ui->doubleInitTailSlope->value());
  }

  hm_.Lskew_amplitude.enabled = ui->checkEnableLskew->isChecked();
  if (hm_.Lskew_amplitude.enabled) {
    hm_.Lskew_amplitude.lbound = ui->doubleMinLskewAmp->value();
    hm_.Lskew_amplitude.ubound = ui->doubleMaxLskewAmp->value();
    hm_.Lskew_amplitude.value.setValue(ui->doubleInitLskewAmp->value());
    hm_.Lskew_slope.lbound = ui->doubleMinLskewSlope->value();
    hm_.Lskew_slope.ubound = ui->doubleMaxLskewSlope->value();
    hm_.Lskew_slope.value.setValue(ui->doubleInitLskewSlope->value());
  }

  hm_.Rskew_amplitude.enabled = ui->checkEnableRskew->isChecked();
  if (hm_.Rskew_amplitude.enabled) {
    hm_.Rskew_amplitude.lbound = ui->doubleMinRskewAmp->value();
    hm_.Rskew_amplitude.ubound = ui->doubleMaxRskewAmp->value();
    hm_.Rskew_amplitude.value.setValue(ui->doubleInitRskewAmp->value());
    hm_.Rskew_slope.lbound = ui->doubleMinRskewSlope->value();
    hm_.Rskew_slope.ubound = ui->doubleMaxRskewSlope->value();
    hm_.Rskew_slope.value.setValue(ui->doubleInitRskewSlope->value());
  }

//  hm_.width_common = ui->checkWidthCommon->isChecked();
//  hm_.width_common_bounds.lbound = ui->doubleMinWidthCommon->value();
//  hm_.width_common_bounds.ubound = ui->doubleMaxWidthCommon->value();
//  hm_.width_variable_bounds.lbound = ui->doubleMinWidthVariable->value();
//  hm_.width_variable_bounds.ubound = ui->doubleMaxWidthVariable->value();

//  hm_.lateral_slack = ui->doubleLateralSlack->value();
//  hm_.fitter_max_iter = ui->spinFitterMaxIterations->value();

  accept();
}

void FormPeakInfo::enforce_bounds()
{

  ui->doubleInitStep->setMinimum(ui->doubleMinStep->value());
  ui->doubleMaxStep->setMinimum(ui->doubleMinStep->value());
  ui->doubleInitStep->setMaximum(ui->doubleMaxStep->value());
  ui->doubleMinStep->setMaximum(ui->doubleMaxStep->value());

  ui->doubleInitTailAmp->setMinimum(ui->doubleMinTailAmp->value());
  ui->doubleMaxTailAmp->setMinimum(ui->doubleMinTailAmp->value());
  ui->doubleInitTailAmp->setMaximum(ui->doubleMaxTailAmp->value());
  ui->doubleMinTailAmp->setMaximum(ui->doubleMaxTailAmp->value());

  ui->doubleInitTailSlope->setMinimum(ui->doubleMinTailSlope->value());
  ui->doubleMaxTailSlope->setMinimum(ui->doubleMinTailSlope->value());
  ui->doubleInitTailSlope->setMaximum(ui->doubleMaxTailSlope->value());
  ui->doubleMinTailSlope->setMaximum(ui->doubleMaxTailSlope->value());

  ui->doubleInitLskewAmp->setMinimum(ui->doubleMinLskewAmp->value());
  ui->doubleMaxLskewAmp->setMinimum(ui->doubleMinLskewAmp->value());
  ui->doubleInitLskewAmp->setMaximum(ui->doubleMaxLskewAmp->value());
  ui->doubleMinLskewAmp->setMaximum(ui->doubleMaxLskewAmp->value());

  ui->doubleInitLskewSlope->setMinimum(ui->doubleMinLskewSlope->value());
  ui->doubleMaxLskewSlope->setMinimum(ui->doubleMinLskewSlope->value());
  ui->doubleInitLskewSlope->setMaximum(ui->doubleMaxLskewSlope->value());
  ui->doubleMinLskewSlope->setMaximum(ui->doubleMaxLskewSlope->value());

  ui->doubleInitRskewAmp->setMinimum(ui->doubleMinRskewAmp->value());
  ui->doubleMaxRskewAmp->setMinimum(ui->doubleMinRskewAmp->value());
  ui->doubleInitRskewAmp->setMaximum(ui->doubleMaxRskewAmp->value());
  ui->doubleMinRskewAmp->setMaximum(ui->doubleMaxRskewAmp->value());

  ui->doubleInitRskewSlope->setMinimum(ui->doubleMinRskewSlope->value());
  ui->doubleMaxRskewSlope->setMinimum(ui->doubleMinRskewSlope->value());
  ui->doubleInitRskewSlope->setMaximum(ui->doubleMaxRskewSlope->value());
  ui->doubleMinRskewSlope->setMaximum(ui->doubleMaxRskewSlope->value());


}

FormPeakInfo::~FormPeakInfo()
{
  delete ui;
}

void FormPeakInfo::closeEvent(QCloseEvent *event) {
  event->accept();
}


void FormPeakInfo::on_buttonBox_rejected()
{
  reject();
}

void FormPeakInfo::on_doubleMinRskewSlope_valueChanged(double arg1)
{
  enforce_bounds();
}

void FormPeakInfo::on_doubleMaxRskewSlope_valueChanged(double arg1)
{
  enforce_bounds();
}

void FormPeakInfo::on_doubleMinRskewAmp_valueChanged(double arg1)
{
  enforce_bounds();
}

void FormPeakInfo::on_doubleMaxRskewAmp_valueChanged(double arg1)
{
  enforce_bounds();
}

void FormPeakInfo::on_doubleMinLskewSlope_valueChanged(double arg1)
{
  enforce_bounds();
}

void FormPeakInfo::on_doubleMaxLskewSlope_valueChanged(double arg1)
{
  enforce_bounds();
}

void FormPeakInfo::on_doubleMinLskewAmp_valueChanged(double arg1)
{
  enforce_bounds();
}

void FormPeakInfo::on_doubleMaxLskewAmp_valueChanged(double arg1)
{
  enforce_bounds();
}

void FormPeakInfo::on_doubleMinTailSlope_valueChanged(double arg1)
{
  enforce_bounds();
}

void FormPeakInfo::on_doubleMaxTailSlope_valueChanged(double arg1)
{
  enforce_bounds();
}

void FormPeakInfo::on_doubleMinTailAmp_valueChanged(double arg1)
{
  enforce_bounds();
}

void FormPeakInfo::on_doubleMaxTailAmp_valueChanged(double arg1)
{
  enforce_bounds();
}

void FormPeakInfo::on_doubleMinStep_valueChanged(double arg1)
{
  enforce_bounds();
}

void FormPeakInfo::on_doubleMaxStep_valueChanged(double arg1)
{
  enforce_bounds();
}
