#include "DiveEdit.hpp"

#include <Constants.hpp>
#include <QMessageBox>

#include "Utils.hpp"
#include "ui_DiveEdit.h"

namespace gui {

DiveEdit::DiveEdit(QWidget *parent)
    : QWidget(parent), ui(new Ui::DiveEdit), m_diving_type_delegate{new DivingTypeDelegate{{}, parent}} {
  ui->setupUi(this);

  connect(&m_members_model,
          &cpsm::DiveEditMembers::DivingTypesUpdated,
          this,
          [this](const std::map<int, cpsm::db::DivingType> &diving_types) {
            m_diving_type_delegate->SetDivingTypes(diving_types);
          });

  connect(ui->pb_time_preset_morning, &QPushButton::clicked, this, [this]() { ui->te_diveTime->setTime(QTime(9, 0)); });
  connect(
      ui->pb_time_preset_afternoon, &QPushButton::clicked, this, [this]() { ui->te_diveTime->setTime(QTime(15, 0)); });
  connect(ui->pb_time_preset_sunset, &QPushButton::clicked, this, [this]() { ui->te_diveTime->setTime(QTime(20, 0)); });
  connect(ui->pb_time_preset_night, &QPushButton::clicked, this, [this]() { ui->te_diveTime->setTime(QTime(23, 0)); });

  connect(ui->te_diveTime, &QTimeEdit::timeChanged, this, [this](const QTime &time) {
    ui->te_diveTime->setStyleSheet(
        QString{"background-color: %0"}.arg(cpsm::BackgroundColorFromTime(time).name(QColor::HexRgb)));
  });

  connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DiveEdit::OnOk);
  connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &DiveEdit::OnCancelled);

  ui->diverSearch_global->SetModel(&m_non_members_model);
  ui->diverSearch_dive->SetModel(&m_members_model);
  ui->diverSearch_dive->SetItemDelegateForColumn(cpsm::DiveEditMembers::kDivingType, m_diving_type_delegate);

  connect(&m_members_model, &cpsm::DiveEditMembers::DiveMembersChanged, this, &DiveEdit::OnDiveMembersChanged);
  connect(
      &m_members_model, &cpsm::DiveEditMembers::SetSelectedRows, ui->diverSearch_dive, &DiverSearch::SetSelectedRows);

  connect(
      ui->pb_non_members_to_members, &QPushButton::clicked, this, &DiveEdit::MoveSelectedDiversFromNonMembersToMembers);
  connect(ui->diverSearch_global,
          &DiverSearch::DoubleClickOnDiver,
          this,
          &DiveEdit::MoveSelectedDiversFromNonMembersToMembers);
  connect(
      ui->pb_members_to_non_members, &QPushButton::clicked, this, &DiveEdit::MoveSelectedDiversFromMembersToNonMembers);
  connect(ui->diverSearch_dive,
          &DiverSearch::DoubleClickOnDiver,
          this,
          &DiveEdit::MoveSelectedDiversFromMembersToNonMembers);

  /* Dive edit connections */
  connect(ui->de_diveDate, &QDateEdit::dateChanged, this, [this](const QDate &date) {
    m_dive.dive.datetime.setDate(date);
  });
  connect(ui->te_diveTime, &QTimeEdit::timeChanged, this, [this](const QTime &time) {
    m_dive.dive.datetime.setTime(time);
  });
  connect(ui->cb_diveSite, &QComboBox::currentIndexChanged, this, [this](int index) {
    m_dive.dive.diving_site_id = ui->cb_diveSite->itemData(index).toInt();
  });
}

DiveEdit::~DiveEdit() {
  delete ui;
}

bool DiveEdit::SetDive(const cpsm::db::Dive &dive) {
  bool success{true};

  m_dive.dive = dive;
  m_original_dive.dive = dive;

  m_members_model.SetDiveId(dive.dive_id);
  m_members_model.LoadFromDB();
  m_non_members_model.LoadFromDB();

  UpdateUiFromDive();
  return success;
}

void DiveEdit::RefreshFromDB() {
  auto level_list{db::readLFromDB<cpsm::db::DivingSite>(
      db::Def(), cpsm::db::ExtractDivingSite, "SELECT * FROM %0", {cpsm::db::DivingSite::db_table}, {})};
  std::sort(level_list.begin(), level_list.end(), [](const auto &lhs, const auto &rhs) {
    return lhs.site_name < rhs.site_name;
  });
  if (!level_list.empty()) {
    ui->cb_diveSite->clear();
  }
  for (const auto &e : level_list) {
    ui->cb_diveSite->addItem(e.site_name, e.diving_site_id);
  }
}

bool DiveEdit::WasEdited() const {
  return m_dive != m_original_dive;
}

void DiveEdit::UpdateUiFromDive() {
  ui->de_diveDate->setDate(m_dive.dive.datetime.date());
  ui->te_diveTime->setTime(m_dive.dive.datetime.time());

  if (m_dive.dive.diving_site_id <= 0) {
    if (ui->cb_diveSite->count() > 0) {
      m_dive.dive.diving_site_id = ui->cb_diveSite->itemData(0).toInt();
    }
  }

  SetDivingSiteComboboxFromSiteId(m_dive.dive.diving_site_id);
}

void DiveEdit::SetDivingSiteComboboxFromSiteId(int site_id) {
  const auto kFindResults{ui->cb_diveSite->findData(site_id)};
  if (kFindResults != -1) {
    ui->cb_diveSite->setCurrentIndex(kFindResults);
  } else {
    SPDLOG_WARN("Failed to find site id: <{}> in diver edit combobox", site_id);
  }
}

void DiveEdit::OnOk() {
  RefreshDiveMembers();
  emit DiveEdited({m_dive});
  m_original_dive = m_dive;
}

void DiveEdit::OnCancelled() {
  if (!WasEdited()) {
    emit DiveEdited({});
    return;
  }

  auto ans{QMessageBox::question(
      this,
      tr("Confirmation"),
      tr("Toute modification sera définitivement perdue.\nSouhaitez-vous abandonner toutes les modifications ?"),
      QMessageBox::Yes | QMessageBox::No)};
  if (ans == QMessageBox::Yes) {
    emit DiveEdited({});
  }
}

void DiveEdit::OnDiveMembersChanged() {
  const auto kMembersDiversIds{m_members_model.GetMembersDiversIds()};
  if (m_previous_dive_members !=
      kMembersDiversIds) { /* Only change (reapply) the filter if the divers ids were changed */
    m_non_members_model.SetHideDiversIdsFilter(kMembersDiversIds);
  }
  m_previous_dive_members = kMembersDiversIds;
}

void DiveEdit::RefreshDiveMembers() {
  m_dive.members.clear();
  m_dive.members.reserve(m_members_model.GetDivingMembers().size());
  for (const auto &[diver_id, dive_member] : m_members_model.GetDivingMembers()) {
    std::ignore = diver_id;
    m_dive.members.emplace_back(dive_member);
  }
}

void DiveEdit::MoveSelectedDiversFromNonMembersToMembers() {
  const auto kSelectedDivers{ui->diverSearch_global->GetSelectedDivers()};
  if (kSelectedDivers.empty()) {
    return;
  }

  const auto kAdded{m_members_model.AddDivers(kSelectedDivers, true)};
}

void DiveEdit::MoveSelectedDiversFromMembersToNonMembers() {
  const auto kSelectedDivers{ui->diverSearch_dive->GetSelectedDivers()};
  if (kSelectedDivers.empty()) {
    return;
  }

  const auto kRemoved{m_members_model.RemoveDivers(kSelectedDivers)};
}

}  // namespace gui
