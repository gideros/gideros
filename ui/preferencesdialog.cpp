#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include <QDebug>
#include <QSettings>
#include <QComboBox>
#include "settingskeys.h"

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);

    QSettings settings;

    connect(ui->preferencesTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
            this, SLOT(listSelected(QTreeWidgetItem*, int)));

    ui->tabSizeSpinBox->setValue(settings.value(Keys::Prefs::tabSize, 4).toInt());
    connect(ui->tabSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int new_value) {
                this->settings.setValue(Keys::Prefs::tabSize, new_value);
                if (this->current_editor != nullptr)
                    this->current_editor->setTabWidth(new_value);
                qDebug() << "tabSizeSpinBox changed to: " << new_value; }
    );

    ui->tabsVsSpacesComboBox->setCurrentIndex(settings.value(Keys::Prefs::tabsVsSpaces, 0).toInt());
    connect(ui->tabsVsSpacesComboBox, QOverload<int>::of(&QComboBox::activated), this,
            [this](int index) {
                this->settings.setValue(Keys::Prefs::tabsVsSpaces, index);
                if (this->current_editor != nullptr)
                    this->current_editor->setUseTabs(index ? false : true);
                qDebug() << "tabsVsSpaces index changed to: " << index; }
    );
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}


void PreferencesDialog::listSelected(QTreeWidgetItem* w_item, int item)
{
    (void)item;
    qDebug() << "Clicked preferences section: " << w_item->data(0,item);
}

