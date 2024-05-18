#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <Constants.hpp>
#include <QMainWindow>
#include <QPushButton>
#include <UpdateHandler.hpp>
#include <functional>
#include <unordered_map>

#include <Models/DiversViewModel.hpp>

#include "Models/table_model_test.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

 private:
  enum DiverTabPages : int32_t {
    kBrowseDivers = 0,
    kEditDiver = 1,
  };

 signals:
  void DBLoaded();

 public:
  const QColor kBaseBlueColor{25, 180, 255};

  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

 private slots:
  void EditDiver(const cpsm::DiverWithDiveCount &diver);
  void OnDiverEdited(std::optional<std::tuple<db::Diver, db::DiverAddress>> edit_opt);

  void on_action_check_updates_triggered();

  void on_pb_editDiver_clicked();

  void on_pb_deleteDiver_clicked();

 private:
 private:
  Ui::MainWindow *ui;

  const QString c_appdata_folder;
  const QString c_config_file;

  updt::UpdateHandler *m_updateHandler;

  //    sot::KeyboardProfile m_config{};

  cpsm::DiversViewModel m_test_model;
  // MyModel m_test_model;
};
#endif  // MAINWINDOW_HPP
