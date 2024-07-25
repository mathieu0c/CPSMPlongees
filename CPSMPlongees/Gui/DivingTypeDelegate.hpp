#pragma once

#include <QComboBox>
#include <QObject>
#include <QStyledItemDelegate>
#include <RawStructs.hpp>

namespace gui {

class DivingTypeDelegate : public QStyledItemDelegate {
  Q_OBJECT

 signals:
  void DivingTypeChanged(int diving_type_id) const;

 public:
  DivingTypeDelegate(std::map<int, cpsm::db::DivingType> diving_types, QObject *parent = nullptr)
      : QStyledItemDelegate(parent), m_db_diving_types{std::move(diving_types)} {}

  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const override {
    QComboBox *editor = new QComboBox(parent);
    // Assuming m_db_diving_types is accessible here or passed to the delegate
    for (const auto &type : m_db_diving_types) {
      editor->addItem(type.second.type_name, type.first);
    }

    return editor;
  }

  void setEditorData(QWidget *editor, const QModelIndex &index) const override {
    QComboBox *comboBox = dynamic_cast<QComboBox *>(editor);
    if (!comboBox) {
      return;
    }
    QString currentText = index.data(Qt::EditRole).toString();
    int idx = comboBox->findText(currentText);
    if (idx >= 0) {
      comboBox->setCurrentIndex(idx);
    }
  }

  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
    QComboBox *comboBox = static_cast<QComboBox *>(editor);
    if (!comboBox) {
      return;
    }
    model->setData(index, comboBox->currentData(), Qt::EditRole);

    emit DivingTypeChanged(comboBox->currentData().toInt());
  }

  void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const override {
    editor->setGeometry(option.rect);
  }

  void SetDivingTypes(const std::map<int, cpsm::db::DivingType> &diving_types) {
    m_db_diving_types = diving_types;
  }

 private:
  std::map<int, cpsm::db::DivingType> m_db_diving_types{};
};

}  // namespace gui
