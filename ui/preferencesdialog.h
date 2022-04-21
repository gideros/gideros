#pragma once
#include <QDialog>
#include <QTreeWidgetItem>
#include <QSettings>
#include <mdiarea.h>
#include <textedit.h>
#include "mainwindow.h"


namespace Ui {
    class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent = nullptr);
    ~PreferencesDialog();

    void setMdiArea(MdiArea* ma) { mdi_area = ma; }
    void updateEditors(const std::function< void(TextEdit*) > lambda);
    MainWindow* getMainWindow();

private:
    // get init values for controls from settings and setup lambda slot callbacks
    void setupEditorPrefs();

    Ui::PreferencesDialog* ui;
    std::map<std::string, int> sections;
    QSettings settings;
    MdiArea* mdi_area;
};
