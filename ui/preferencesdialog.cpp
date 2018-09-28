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

    // match the string we get back from our sections TreeWidget to page numbers
    // on our StackedLayout, for 'turning' pages on tree click
    sections = { {"Editor", 0}, {"Keybindings", 1} };
    setupEditorPrefs();

    ui->stackedWidget->setCurrentIndex(0);
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::setupEditorPrefs()
{
    QSettings settings;

    connect(ui->preferencesTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
            this, SLOT(listSelected(QTreeWidgetItem*, int)));

    connect(ui->preferencesTreeWidget, QOverload<QTreeWidgetItem*, int>::of(&QTreeWidget::itemClicked), this,
            [this](QTreeWidgetItem* w_item, int item) {
                (void) item;
                auto map_entry = sections.find(w_item->data(0, 0).toString().toStdString());
                if (map_entry != sections.end())
                    ui->stackedWidget->setCurrentIndex(map_entry->second);
            }
    );

    ui->tabSizeSpinBox->setValue(settings.value(Keys::Prefs::tabSize, 4).toInt());

    connect(ui->tabSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int new_value) {
                this->settings.setValue(Keys::Prefs::tabSize, new_value);
                if (this->current_editor != nullptr)
                    this->current_editor->setTabWidth(new_value);
            }
    );

    ui->tabsVsSpacesComboBox->setCurrentIndex(settings.value(Keys::Prefs::tabsVsSpaces, 0).toInt());

    connect(ui->tabsVsSpacesComboBox, QOverload<int>::of(&QComboBox::activated), this,
            [this](int index) {
                this->settings.setValue(Keys::Prefs::tabsVsSpaces, index);
                if (this->current_editor != nullptr)
                    this->current_editor->setUseTabs(index ? false : true);
            }
    );
}
