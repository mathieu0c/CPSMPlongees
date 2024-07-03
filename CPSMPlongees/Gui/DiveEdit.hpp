#pragma once

#include <QWidget>
#include <RawStructs.hpp>

namespace gui {

namespace Ui {
class DiveEdit;
}

class DiveEdit : public QWidget {
  Q_OBJECT

 signals:
  void DiveEdited(std::optional<cpsm::db::DiveAndDivers> edit_opt);

 public:
  explicit DiveEdit(QWidget *parent = nullptr);
  ~DiveEdit();

  void RefreshFromDB();
  bool SetDive(const cpsm::db::Dive &dive);

 private:
  bool WasEdited() const;

  void UpdateUiFromDive();
  void SetDivingSiteComboboxFromSiteId(int site_id);

 private slots:
  void OnOk();
  void OnCancelled();

 private:
  Ui::DiveEdit *ui;

  cpsm::db::DiveAndDivers m_dive{};
  cpsm::db::DiveAndDivers m_original_dive{};
};

}  // namespace gui
