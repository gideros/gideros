#include "exportprojectdialog.h"
#include "ui_exportprojectdialog.h"
#include "projectproperties.h"
//#include "pluginschooser.h"
#include "propertyeditingtable.h"

#include <QDir>
#include <QFile>

ExportProjectDialog::ExportProjectDialog(ProjectProperties* properties, QDir projectDir, bool licensed, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportProjectDialog)
{
    osxCat["Business"] = "public.app-category.business";
    osxCat["Developer Tools"] = "public.app-category.developer-tools";
    osxCat["Education"] = "public.app-category.education";
    osxCat["Entertainment"] = "public.app-category.entertainment";
    osxCat["Finance"] = "public.app-category.finance";
    osxCat["Games"] = "public.app-category.games";
    osxCat["Graphics & Design"] = "public.app-category.graphics-design";
    osxCat["Healthcare & Fitness"] = "public.app-category.healthcare-fitness";
    osxCat["Lifestyle"] = "public.app-category.lifestyle";
    osxCat["Medical"] = "public.app-category.medical";
    osxCat["Music"] = "public.app-category.music";
    osxCat["News"] = "public.app-category.news";
    osxCat["Photography"] = "public.app-category.photography";
    osxCat["Productivity"] = "public.app-category.productivity";
    osxCat["Reference"] = "public.app-category.reference";
    osxCat["Social Networking"] = "public.app-category.social-networking";
    osxCat["Sports"] = "public.app-category.sports";
    osxCat["Travel"] = "public.app-category.travel";
    osxCat["Utilities"] = "public.app-category.utilities";
    osxCat["Video"] = "public.app-category.video";
    osxCat["Weather"] = "public.app-category.weather";
    osxCat["Action Games"] = "public.app-category.action-games";
    osxCat["Adventure Games"] = "public.app-category.adventure-games";
    osxCat["Arcade Games"] = "public.app-category.arcade-games";
    osxCat["Board Games"] = "public.app-category.board-games";
    osxCat["Card Games"] = "public.app-category.card-games";
    osxCat["Casino Games"] = "public.app-category.casino-games";
    osxCat["Dice Games"] = "public.app-category.dice-games";
    osxCat["Educational Games"] = "public.app-category.educational-games";
    osxCat["Family Games"] = "public.app-category.family-games";
    osxCat["Kids Games"] = "public.app-category.kids-games";
    osxCat["Music Games"] = "public.app-category.music-games";
    osxCat["Puzzle Games"] = "public.app-category.puzzle-games";
    osxCat["Racing Games"] = "public.app-category.racing-games";
    osxCat["Role Playing Games"] = "public.app-category.role-playing-games";
    osxCat["Simulation Games"] = "public.app-category.simulation-games";
    osxCat["Sports Games"] = "public.app-category.sports-games";
    osxCat["Strategy Games"] = "public.app-category.strategy-games";
    osxCat["Trivia Games"] = "public.app-category.trivia-games";
    osxCat["Word Games"] = "public.app-category.word-games";

    exportTypes << "Apple" << "Desktop"
            << "WinRT" << "GApp" << "Win32" << "Linux" << "Html5";

    qtPlatforms << "Windows" << "Mac" << "Linux";

	properties_ = properties;

	ui->setupUi(this);

    QMap<QString, QString>::iterator i;
    for (i = osxCat.begin(); i != osxCat.end(); ++i){
        ui->osx_category->addItem(i.key(), i.value());
        ui->macos_category->addItem(i.key(), i.value());
    }

	connect(ui->architecture, SIGNAL(currentIndexChanged(int)), ui->architectureTab, SLOT(setCurrentIndex(int)));
	connect(ui->architectureTab, SIGNAL(currentChanged(int)), ui->architecture, SLOT(setCurrentIndex(int)));
	connect(ui->html5_fbinstant, SIGNAL(stateChanged(int)), this, SLOT(actionHtml5FbInstant(int)));
	//connect(ui->plugins_choose, SIGNAL(clicked()), this, SLOT(onSelectPlugins()));

    ui->exportMode->setCurrentIndex(properties_->exportMode);
    ui->ios_bundle->setText(properties_->ios_bundle);
    ui->atv_bundle->setText(properties_->atv_bundle);
    ui->macos_bundle->setText(properties_->macos_bundle);
    ui->macos_category->setCurrentIndex(properties_->macos_category);
    ui->qtexp_org->setText(properties->qtexp_org);
    ui->qtexp_domain->setText(properties->qtexp_domain);
    ui->qtexp_platform->setCurrentIndex(properties->qtexp_platform);
    ui->osx_bundle->setText(properties_->osx_bundle);
    ui->osx_signingid->setText(properties_->osx_signingId);
    ui->osx_installerid->setText(properties_->osx_installerId);
    ui->osx_category->setCurrentIndex(properties_->osx_category);
    ui->winrt_org->setText(properties->winrt_org);
    ui->winrt_package->setText(properties->winrt_package);
    ui->html5_host->setText(properties->html5_host);
    ui->html5_crash->setText(properties->html5_crash);
    //ui->html5_mem->setText(QString::number(properties->html5_mem));
    ui->html5_pack->setChecked(properties_->html5_pack);
    ui->html5_symbols->setChecked(properties_->html5_symbols);
    ui->html5_fbinstant->setChecked(properties_->html5_fbinstant);
    ui->html5_pwa->setChecked(properties_->html5_pwa);
    ui->html5_fbload->setValue(properties_->html5_fbload);
    actionHtml5FbInstant(0);
    //plugins=properties->plugins;

    if (licensed)
    {
        ui->encryptCode->setEnabled(true);
        ui->encryptCode->setChecked(properties_->encryptCode);
        ui->encryptAssets->setEnabled(true);
        ui->encryptAssets->setChecked(properties_->encryptAssets);
    }
    else
    {
        ui->encryptCode->setEnabled(false);
        ui->encryptCode->setChecked(false);
        ui->encryptAssets->setEnabled(false);
        ui->encryptAssets->setChecked(false);
    }

    //XML based templates
    xmlTabCount=0;
    QDir sourceDir(TEMPLATES_PATH);
   	QStringList filters;
    filters << "*.gexport";
    sourceDir.setNameFilters(filters);
    QStringList files = sourceDir.entryList(QDir::Files | QDir::Hidden);
    for(int i = 0; i < files.count(); i++)
    {
	       QDomDocument doc("export");
	       QFile file(sourceDir.absoluteFilePath(files[i]));
           if (!file.open(QIODevice::ReadOnly))
                continue;
            if (!doc.setContent(&file))
            {
                file.close();
                continue;
            }
            file.close();

            QDomElement exporter = doc.documentElement();
            QString exname=exporter.attribute("name");
            exportTypes.prepend(exname);
            QMap<QString,QString> props;
        	for (QSet<ProjectProperties::Export>::const_iterator it=properties_->exports.begin();it!=properties_->exports.end(); it++)
        		if ((*it).name==exname)
        			props=(*it).properties;

            PropertyEditingTable *table=new PropertyEditingTable(projectDir);
            QString exlabel=exporter.attribute("label");
            if (exlabel.isEmpty())
            	exlabel=exname;
            ui->architectureTab->insertTab(0,table,exlabel);
            ui->architecture->insertItem(0,exlabel);
            table->fill(exporter,props);
            xmlTabCount++;
	}

	ui->architecture->setCurrentIndex(properties_->architecture);

	connect(this, SIGNAL(accepted()), this, SLOT(onAccepted()));
}

ExportProjectDialog::~ExportProjectDialog()
{
    delete ui;
}

void ExportProjectDialog::actionHtml5FbInstant(int)
{
	bool en=ui->html5_fbinstant->isChecked();
	ui->html5_host->setEnabled(!en);
	ui->html5_fbload->setEnabled(en);
}

QString ExportProjectDialog::exportType() const
{
    return exportTypes[ui->architecture->currentIndex()];
}

QString ExportProjectDialog::ios_bundle() const
{
    return ui->ios_bundle->text();
}

QString ExportProjectDialog::atv_bundle() const
{
    return ui->atv_bundle->text();
}

QString ExportProjectDialog::macos_bundle() const
{
    return ui->macos_bundle->text();
}

QString ExportProjectDialog::macos_category() const
{
    return osxCat[ui->macos_category->currentText()];
}

QString ExportProjectDialog::qtexp_platform() const
{
    return qtPlatforms[ui->qtexp_platform->currentIndex()];
}

QString ExportProjectDialog::qtexp_org() const
{
    return ui->qtexp_org->text();
}

QString ExportProjectDialog::qtexp_domain() const
{
    return ui->qtexp_domain->text();
}

QString ExportProjectDialog::osx_bundle() const
{
    return ui->osx_bundle->text();
}

QString ExportProjectDialog::osx_signingId() const
{
    return ui->osx_signingid->text();
}

QString ExportProjectDialog::osx_installerId() const
{
    return ui->osx_installerid->text();
}

QString ExportProjectDialog::osx_category() const
{
    return osxCat[ui->osx_category->currentText()];
}

QString ExportProjectDialog::winrt_org() const
{
    return ui->winrt_org->text();
}

QString ExportProjectDialog::winrt_package() const
{
    return ui->winrt_package->text();
}

QString ExportProjectDialog::html5_host() const
{
    return ui->html5_host->text();
}

int ExportProjectDialog::exportMode() const
{
	return ui->exportMode->currentIndex();
}

bool ExportProjectDialog::encryptCode() const
{
    return ui->encryptCode->isChecked();
}

bool ExportProjectDialog::encryptAssets() const
{
    return ui->encryptAssets->isChecked();
}

void ExportProjectDialog::onAccepted()
{
	properties_->architecture = ui->architecture->currentIndex();
	properties_->exportMode = ui->exportMode->currentIndex();
    properties_->ios_bundle = ui->ios_bundle->text();
    properties_->atv_bundle = ui->atv_bundle->text();
    properties_->macos_bundle = ui->macos_bundle->text();
    properties_->macos_category = ui->macos_category->currentIndex();
    properties_->qtexp_platform = ui->qtexp_platform->currentIndex();
    properties_->qtexp_org = ui->qtexp_org->text();
    properties_->qtexp_domain = ui->qtexp_domain->text();
    properties_->osx_bundle = ui->osx_bundle->text();
    properties_->osx_signingId = ui->osx_signingid->text();
    properties_->osx_installerId = ui->osx_installerid->text();
    properties_->osx_category = ui->osx_category->currentIndex();
    properties_->winrt_org = ui->winrt_org->text();
    properties_->winrt_package = ui->winrt_package->text();
    properties_->encryptCode = ui->encryptCode->isChecked();
    properties_->encryptAssets = ui->encryptAssets->isChecked();
    properties_->html5_host = ui->html5_host->text();
    properties_->html5_crash = ui->html5_crash->text();
    properties_->html5_mem = 256; //ui->html5_mem->text().toInt();
    properties_->html5_pack = ui->html5_pack->isChecked();
    properties_->html5_symbols = ui->html5_symbols->isChecked();
    properties_->html5_fbinstant = ui->html5_fbinstant->isChecked();
    properties_->html5_pwa = ui->html5_pwa->isChecked();
    properties_->html5_fbload = ui->html5_fbload->value();
    //properties_->plugins=plugins;

    for (int tab=0;tab<xmlTabCount;tab++)
    {
    	QString exname=exportTypes[tab];
    	PropertyEditingTable *table=(PropertyEditingTable *)ui->architectureTab->widget(tab);
     	for (QSet<ProjectProperties::Export>::iterator it=properties_->exports.begin();it!=properties_->exports.end();)
        		if ((*it).name==exname)
        			it=properties_->exports.erase(it);
        		else
        			it++;
     	ProjectProperties::Export exp;
     	exp.name=exname;
     	exp.properties=table->extract();
     	properties_->exports.insert(exp);
    }
}

/*
void ExportProjectDialog::onSelectPlugins()
{
    PluginsChooser dialog(plugins, this);
	if (dialog.exec() == QDialog::Accepted)
	{
		plugins=dialog.selection();
	}
}
*/
