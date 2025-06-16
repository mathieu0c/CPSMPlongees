#pragma once

#include <QAbstractSpinBox>
#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <type_traits>

/* Label that goes invisible when empty text */
class StatusLabel : public QLabel {
  Q_OBJECT
 public:
  StatusLabel(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
  StatusLabel(const QString& text, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

  void setText(const QString& text);
};

template <typename T, typename ParentT>
  requires requires(T obj, ParentT parent) {
    obj.editingFinished();
    parent.FocusNext();
  }
void ConnectEditingFinished(ParentT* parent, QObject* target) {
  qInfo() << "Testing cast for" << target->metaObject()->className() << "to" << T::staticMetaObject.className();
  auto* child{qobject_cast<T*>(target)};
  if (!child) {
    return;
  }

  qInfo() << "Connected: <" << child->objectName() << ">";
  QObject::connect(child, &T::editingFinished, parent, &ParentT::FocusNext);
}

template <typename ParentT>
  requires std::is_base_of_v<QObject, ParentT>
void SetupFocusOnEditingFinished(ParentT* parent, QObject* sub_parent = nullptr) {
  for (auto* raw_child : (sub_parent ? sub_parent->children() : parent->children())) {
    qInfo() << "Checking child:" << raw_child->metaObject()->className() << raw_child->objectName();
    ConnectEditingFinished<QAbstractSpinBox, ParentT>(parent, raw_child);
    ConnectEditingFinished<QLineEdit, ParentT>(parent, raw_child);
    if (!raw_child->children().empty()) {
      SetupFocusOnEditingFinished(parent, raw_child);
    }
  }
}
