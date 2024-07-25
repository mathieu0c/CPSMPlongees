#pragma once

#include <QWidget>
#include <RawStructs.hpp>

#include <Gui/DivingTypeDelegate.hpp>
#include <Models/DiveEditMembers.hpp>

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

  void OnDiveMembersChanged();
  void RefreshDiveMembers();

  void MoveSelectedDiversFromNonMembersToMembers();
  void MoveSelectedDiversFromMembersToNonMembers();

 private:
  Ui::DiveEdit *ui;

  cpsm::db::DiveAndDivers m_dive{};
  cpsm::db::DiveAndDivers m_original_dive{};

  cpsm::DiveEditNonMembers m_non_members_model{};
  cpsm::DiveEditMembers m_members_model{};
  std::set<int> m_previous_dive_members{};

  DivingTypeDelegate *m_diving_type_delegate{nullptr};
};

}  // namespace gui
