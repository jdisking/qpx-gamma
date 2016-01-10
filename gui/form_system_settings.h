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
 *      Module and channel settings UI
 *
 ******************************************************************************/

#ifndef FORM_SYSTEM_SETTINGS_H
#define FORM_SYSTEM_SETTINGS_H

#include <QWidget>
#include <QSettings>
#include <QCloseEvent>
#include <QTableView>
#include <QTreeView>
#include "detector.h"
#include "special_delegate.h"
#include "thread_runner.h"
#include "table_settings.h"
#include "tree_settings.h"

namespace Ui {
class FormSystemSettings;
}

class FormSystemSettings : public QWidget
{
  Q_OBJECT

public:
  explicit FormSystemSettings(ThreadRunner&, XMLableDB<Gamma::Detector>&, QSettings&, QWidget *parent = 0);
  Gamma::Setting get_tree() {return dev_settings_;}
  ~FormSystemSettings();

public slots:
  void refresh();
  void update(const Gamma::Setting &tree, const std::vector<Gamma::Detector> &channelsupdate, Qpx::DeviceStatus);

signals:
  void toggleIO(bool);
  void statusText(QString);
  void optimization_requested();
  void gain_matching_requested();
  void list_view_requested();

protected:
  void closeEvent(QCloseEvent*);

public slots:
  void updateDetDB();
  
private slots:
  void begin_editing();
  void on_pushSettingsRefresh_clicked();
  void on_pushOptimizeAll_clicked();

  void toggle_push(bool enable, Qpx::DeviceStatus status);
  void post_boot();

  void on_pushDetDB_clicked();

  void push_settings();
  void push_from_table(int chan, Gamma::Setting setting);
  void chose_detector(int chan, std::string name);

  void ask_binary_tree(Gamma::Setting, QModelIndex index);
  void ask_execute_tree(Gamma::Setting, QModelIndex index);
  void ask_binary_table(Gamma::Setting, QModelIndex index);
  void ask_execute_table(Gamma::Setting, QModelIndex index);
  void execute_command(Gamma::Setting);

  void on_checkShowRO_clicked();
  void on_bootButton_clicked();


  void on_pushOpenGainMatch_clicked();
  void on_pushOpenOptimize_clicked();

  void on_pushOpenListView_clicked();

private:
  Ui::FormSystemSettings *ui;

  Qpx::DeviceStatus current_status_;

  XMLableDB<Gamma::Detector>            &detectors_;
  QString data_directory_;
  QString settings_directory_;

  ThreadRunner        &runner_thread_;
  QSettings           &settings_;
  bool editing_;

  Gamma::Setting               dev_settings_;
  std::vector<Gamma::Detector> channels_;

  QTableView*         viewTableSettings;
  TableChanSettings   table_settings_model_;
  QpxSpecialDelegate  table_settings_delegate_;

  QTreeView*          viewTreeSettings;
  TreeSettings        tree_settings_model_;
  QpxSpecialDelegate  tree_delegate_;

  void loadSettings();
  void saveSettings();
  void chan_settings_to_det_DB();

};

#endif // FORM_SYSTEM_SETTINGS_H
