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
    // on our StackedLayout, for 'turning' pages on tree click (only Editor for now)
    sections = { {"Editor", 0}, {"Keybindings", 1} };
    setupEditorPrefs();

    ui->stackedWidget->setCurrentIndex(0);
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::updateEditors(const std::function<void (TextEdit*)> lambda) {
    if (mdi_area != nullptr)
        for (auto window : mdi_area->subWindowList()) {
            TextEdit* te = qobject_cast<TextEdit*>(window);
            if (te) lambda(te);
        }
}

void PreferencesDialog::setupEditorPrefs()
{
    QSettings settings;

    connect(ui->preferencesTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
            this, SLOT(listSelected(QTreeWidgetItem*, int)));

    // use QOverload to specify signature of overloaded method we want to use
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
                this->updateEditors([new_value](TextEdit* te) { te->setTabWidth(new_value);});
            }
    );


    ui->tabsVsSpacesComboBox->setCurrentIndex(settings.value(Keys::Prefs::tabsVsSpaces, 0).toInt());

    connect(ui->tabsVsSpacesComboBox, QOverload<int>::of(&QComboBox::activated), this,
            [this](int index) {
                this->settings.setValue(Keys::Prefs::tabsVsSpaces, index);
                this->updateEditors([index](TextEdit* te) { te->setUseTabs(index ? false : true);});
            }
    );

    // 0 - hide, 1 - show
    ui->showIndentGuideComboBox->setCurrentIndex(settings.value(Keys::Prefs::indentGuides, 1).toInt());

    connect(ui->showIndentGuideComboBox, QOverload<int>::of(&QComboBox::activated), this,
            [this](int index) {
                this->settings.setValue(Keys::Prefs::indentGuides, index);
                this->updateEditors([index](TextEdit* te) { te->setIndentGuide(index);});
            }
    );

    // 0 - hide, 1 - show
    ui->lineNumberingComboBox->setCurrentIndex(settings.value(Keys::Prefs::showLineNumbers, 1).toInt());

    connect(ui->lineNumberingComboBox, QOverload<int>::of(&QComboBox::activated), this,
            [this](int show) {
                this->settings.setValue(Keys::Prefs::showLineNumbers, show);
                this->updateEditors([show](TextEdit* te) { te->setShowLineNumbers(show);});
            }
    );

    // 0 - no, 1 - yes, use
    ui->backspaceUnindentsComboBox->setCurrentIndex(settings.value(Keys::Prefs::backspaceUnindents, 1).toInt());

    connect(ui->backspaceUnindentsComboBox, QOverload<int>::of(&QComboBox::activated), this,
            [this](int use) {
                this->settings.setValue(Keys::Prefs::backspaceUnindents, use);
                this->updateEditors([use](TextEdit* te) { te->setBackspaceUnindents(use);});
            }
    );

    // 0 - 3, follows QsciScintilla::WhitespaceVisibility::
    ui->whitespaceVisibilityComboBox->setCurrentIndex(settings.value(Keys::Prefs::whitespaceVisibility, 0).toInt());

    connect(ui->whitespaceVisibilityComboBox, QOverload<int>::of(&QComboBox::activated), this,
            [this](int mode) {
                this->settings.setValue(Keys::Prefs::whitespaceVisibility, mode);
                this->updateEditors([mode](TextEdit* te) { te->setWhitespaceVisibility(mode);});
            }
    );
}
