#pragma once
#include <QDialog>
#include <QTreeWidgetItem>
#include <QSettings>
#include <textedit.h>


namespace Ui {
    class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent = nullptr);
    ~PreferencesDialog();

    void setTextEdit(TextEdit* text_edit) { current_editor = text_edit; }

public slots:
    void listSelected(QTreeWidgetItem* w_item, int item);

private:
    Ui::PreferencesDialog* ui;
    QSettings settings;
    TextEdit* current_editor;
};
