#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <Constants.hpp>
#include <QMainWindow>
#include <QPushButton>
#include <UpdateHandler.hpp>

#include <Models/DiversViewModel.hpp>

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

  enum MainTabs : int32_t {
    kDivers = 0,
    kDives = 1,
  };

 signals:
  void DBLoaded();

 public:
  const QColor kBaseBlueColor{25, 180, 255};

  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

 private slots:
  void EditDiver(const cpsm::DiverWithDiveCount &diver);
  void OnDiverEdited(std::optional<std::tuple<cpsm::db::Diver, cpsm::db::DiverAddress>> edit_opt);

  void on_action_check_updates_triggered();

  void on_pb_editDiver_clicked();

  void on_pb_deleteDiver_clicked();

  void on_pb_newDiver_clicked();

 private:
 private:
  Ui::MainWindow *ui;

  const QString c_appdata_folder;
  const QString c_config_file;

  updt::UpdateHandler *m_updateHandler;
};
#endif  // MAINWINDOW_HPP
