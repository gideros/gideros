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

MainWindow* PreferencesDialog::getMainWindow()
{
    foreach (QWidget *w, qApp->topLevelWidgets())
        if (MainWindow* mainWin = qobject_cast<MainWindow*>(w))
            return mainWin;
    return nullptr;
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
    ui->showIndentGuideCheckBox->setChecked(settings.value(Keys::Prefs::indentGuides, 1).toBool());
    
    connect(ui->showIndentGuideCheckBox, QOverload<int>::of(&QCheckBox::stateChanged), this,
            [this](int state) {
                this->settings.setValue(Keys::Prefs::indentGuides, state);
                this->updateEditors([state](TextEdit* te) { te->setIndentGuide(state);});
            }
    );

    // 0 - hide, 1 - show
    ui->lineNumberingCheckBox->setChecked(settings.value(Keys::Prefs::showLineNumbers, 1).toBool());

    connect(ui->lineNumberingCheckBox, QOverload<int>::of(&QCheckBox::stateChanged), this,
            [this](int state) {
                this->settings.setValue(Keys::Prefs::showLineNumbers, state);
                this->updateEditors([state](TextEdit* te) { te->setShowLineNumbers(state);});
            }
    );

    // 0 - no, 1 - yes, use
    ui->backspaceUnindentsCheckBox->setChecked(settings.value(Keys::Prefs::backspaceUnindents, 1).toBool());

    connect(ui->backspaceUnindentsCheckBox, QOverload<int>::of(&QCheckBox::stateChanged), this,
            [this](int state) {
                this->settings.setValue(Keys::Prefs::backspaceUnindents, state);
                this->updateEditors([state](TextEdit* te) { te->setBackspaceUnindents(state);});
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
	
	// 0 - no, 1 - yes, use
	
	ui->compactFoldCheckBox->setChecked(settings.value(Keys::Prefs::foldCompact, 1).toBool());
	
	connect(ui->compactFoldCheckBox, QOverload<int>::of(&QCheckBox::stateChanged), this,
            [this](int state) {
                this->settings.setValue(Keys::Prefs::foldCompact, state);
                this->updateEditors([state](TextEdit* te) { te->setCompactFolding(state); });
            }
    );
    
    ui->wordHighlighterCheckBox->setChecked(settings.value(Keys::Prefs::wordHighlighter, 1).toBool());
    
    connect(ui->wordHighlighterCheckBox, QOverload<int>::of(&QCheckBox::stateChanged), this,
            [this](int state) {
                this->settings.setValue(Keys::Prefs::wordHighlighter, state);
                this->ui->wordHighlighterSimpleCheckBox->setEnabled(state);
                this->updateEditors([state](TextEdit* te) { te->wordHighlighter()->setEnabled(state); });
            }
    );
    
    ui->wordHighlighterSimpleCheckBox->setEnabled(ui->wordHighlighterCheckBox->isChecked());
    ui->wordHighlighterSimpleCheckBox->setChecked(settings.value(Keys::Prefs::wordHighlighterSimple, 1).toBool());
    
    connect(ui->wordHighlighterSimpleCheckBox, QOverload<int>::of(&QCheckBox::stateChanged), this,
            [this](int state) {
                this->settings.setValue(Keys::Prefs::wordHighlighterSimple, state);
                this->updateEditors([state](TextEdit* te) { te->wordHighlighter()->setSimpleMode(state); });
            }
    );
    
	// append toolbar to menu checkbox
    ui->appendToolbarCheckBox->setChecked(settings.value("toggleToolBar", 0).toBool());
    connect(ui->appendToolbarCheckBox, QOverload<int>::of(&QCheckBox::stateChanged), this,
            [this](int state) {
				auto mw = getMainWindow();
				mw->toggleToolBar(state);
            }
    );
}
