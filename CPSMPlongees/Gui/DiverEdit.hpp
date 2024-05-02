#pragma once

#include <QWidget>
#include <RawStructs.hpp>

namespace gui {

namespace Ui {
class DiverEdit;
}

class DiverEdit : public QWidget {
  Q_OBJECT

 signals:
  void DiverEdited(std::optional<std::tuple<db::Diver, db::DiverAddress>> edit_opt);

 public:
  explicit DiverEdit(QWidget *parent = nullptr);
  ~DiverEdit();

  bool SetDiver(const db::Diver &diver, int dive_count);

 public slots:
  void OnDatabaseLoaded();

 private:
  bool AllGearChecked() const;

 private slots:
  void SetAllGearChecked(bool checked);

  void OnPaymentValueChanged(int new_val);
  void UpdateUiSold();

  void OnOk();
  void OnCancelled();

  void on_buttonBox_accepted(); /* NOLINT */
  void on_buttonBox_rejected(); /* NOLINT */

 private:
  bool SetDiverAddressFromId(int address_id);
  void UpdateUiFromDiver();
  void UpdateAddressUi();
  void SetLevelComboboxFromLevelId(int level_id);

 private:
  Ui::DiverEdit *ui;

  db::Diver m_diver{};
  int32_t m_diver_original_paid_dives_count{};
  int32_t m_dive_count{};
  db::DiverAddress m_address{};
  db::DiverAddress m_original_address{};

  bool m_inhibit_all_gear_checkbox_change{false}; /* Avoid spam when check the all gear checkbox */
};

}  // namespace gui
