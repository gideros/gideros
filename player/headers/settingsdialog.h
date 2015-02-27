#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include "ui_settingsdialog.h"
#include "mainwindow.h"

class SettingsDialog : public QDialog{
	Q_OBJECT

    public:
        SettingsDialog(MainWindow *mainWindow, QWidget *parent = 0);
        ~SettingsDialog();

    private:
        Ui::SettingsDialogClass ui;
        MainWindow *mainWindow_;

        void setupUiItems();
        void loadValues();
        void saveValues();
        void updateMainStatus();

    private slots:
        void ok();
        void cancel();
};

#endif // SETTINGSDIALOG_H
