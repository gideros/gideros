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

private:
    // get init values for controls from settings and setup lambda slot callbacks
    void setupEditorPrefs();

    Ui::PreferencesDialog* ui;
    std::map<std::string, int> sections;
    QSettings settings;
    TextEdit* current_editor;
};
