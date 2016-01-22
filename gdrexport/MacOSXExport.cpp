/*
 * MacOSXExport.cpp
 *
 *  Created on: 22 janv. 2016
 *      Author: Nico
 */

#include "MacOSXExport.h"
#include <QProcess>
#include "Utilities.h"

void MacOSXExport::CodeSignMacOSX(ExportContext *ctx)
{
	ctx->outputDir.cdUp();
	ctx->outputDir.cd("Frameworks");
    QString script = "";
    QProcess postProcess;
    QString cmd;
    QStringList frameworks = ctx->outputDir.entryList(QStringList() << "*.framework");
    for(int i = 0; i < frameworks.size(); ++i){
        QString filename = ctx->outputDir.absoluteFilePath(frameworks[i]);
        cmd = "codesign -f -s \"3rd Party Mac Developer Application: "+ctx->args["organization"]+"\" \""+filename+"/Versions/Current\"";
        script += cmd+"\n";
        Utilities::processOutput(cmd);
    }
    QStringList dylibs = ctx->outputDir.entryList(QStringList() << "*.dylib");
    for(int i = 0; i < dylibs.size(); ++i){
        QString filename = ctx->outputDir.absoluteFilePath(dylibs[i]);
        cmd = "codesign -f -s \"3rd Party Mac Developer Application: "+ctx->args["organization"]+"\" \""+filename+"\"";
        script += cmd+"\n";
        Utilities::processOutput(cmd);
    }

    ctx->outputDir.cdUp();
    ctx->outputDir.cd("PlugIns");
    dylibs = ctx->outputDir.entryList(QStringList() << "*.dylib");
    for(int i = 0; i < dylibs.size(); ++i){
        QString filename = ctx->outputDir.absoluteFilePath(dylibs[i]);
        cmd = "codesign -f -s \"3rd Party Mac Developer Application: "+ctx->args["organization"]+"\" \""+filename+"\"";
        script += cmd+"\n";
        Utilities::processOutput(cmd);
    }

    ctx->outputDir.cd("bearer");
    dylibs = ctx->outputDir.entryList(QStringList() << "*.dylib");
    for(int i = 0; i < dylibs.size(); ++i){
        QString filename = ctx->outputDir.absoluteFilePath(dylibs[i]);
        cmd = "codesign -f -s \"3rd Party Mac Developer Application: "+ctx->args["organization"]+"\" \""+filename+"\"";
        script += cmd+"\n";
        Utilities::processOutput(cmd);
    }

    ctx->outputDir.cdUp();
    ctx->outputDir.cd("imageformats");
    dylibs = ctx->outputDir.entryList(QStringList() << "*.dylib");
    for(int i = 0; i < dylibs.size(); ++i){
        QString filename = ctx->outputDir.absoluteFilePath(dylibs[i]);
        cmd = "codesign -f -s \"3rd Party Mac Developer Application: "+ctx->args["organization"]+"\" \""+filename+"\"";
        script += cmd+"\n";
        Utilities::processOutput(cmd);
    }

    ctx->outputDir.cdUp();
    ctx->outputDir.cd("platforms");
    dylibs = ctx->outputDir.entryList(QStringList() << "*.dylib");
    for(int i = 0; i < dylibs.size(); ++i){
        QString filename = ctx->outputDir.absoluteFilePath(dylibs[i]);
        cmd = "codesign -f -s \"3rd Party Mac Developer Application: "+ctx->args["organization"]+"\" \""+filename+"\"";
        script += cmd+"\n";
        Utilities::processOutput(cmd);
    }

    ctx->outputDir.cdUp();
    ctx->outputDir.cd("printsupport");
    dylibs = ctx->outputDir.entryList(QStringList() << "*.dylib");
    for(int i = 0; i < dylibs.size(); ++i){
        QString filename = ctx->outputDir.absoluteFilePath(dylibs[i]);
        cmd = "codesign -f -s \"3rd Party Mac Developer Application: "+ctx->args["organization"]+"\" \""+filename+"\"";
        script += cmd+"\n";
        Utilities::processOutput(cmd);
    }

    ctx->outputDir.cdUp();
    ctx->outputDir.cd("audio");
    dylibs = ctx->outputDir.entryList(QStringList() << "*.dylib");
    for(int i = 0; i < dylibs.size(); ++i){
        QString filename = ctx->outputDir.absoluteFilePath(dylibs[i]);
        cmd = "codesign -f -s \"3rd Party Mac Developer Application: "+ctx->args["organization"]+"\" \""+filename+"\"";
        script += cmd+"\n";
        Utilities::processOutput(cmd);
    }

    ctx->outputDir.cdUp();
    ctx->outputDir.cd("mediaservice");
    dylibs = ctx->outputDir.entryList(QStringList() << "*.dylib");
    for(int i = 0; i < dylibs.size(); ++i){
        QString filename = ctx->outputDir.absoluteFilePath(dylibs[i]);
        cmd = "codesign -f -s \"3rd Party Mac Developer Application: "+ctx->args["organization"]+"\" \""+filename+"\"";
        script += cmd+"\n";
        Utilities::processOutput(cmd);
    }

    ctx->outputDir.cdUp();
    ctx->outputDir.cdUp();
    ctx->outputDir.cdUp();
    ctx->outputDir.cdUp();
    cmd = "codesign -f -s \"3rd Party Mac Developer Application: "+ctx->args["organization"]+"\" --entitlements \"/"+ctx->outputDir.absolutePath()+"/Entitlements.plist\" \""+ctx->outputDir.absoluteFilePath(ctx->base + ".app")+"\"";
    script += cmd+"\n";
    Utilities::processOutput(cmd);

    cmd = "productbuild --component \""+ctx->outputDir.absoluteFilePath(ctx->base + ".app")+"\" /Applications --sign \"3rd Party Mac Developer Installer: "+ctx->args["organization"]+"\" \""+ctx->outputDir.absoluteFilePath(ctx->base + ".pkg")+"\"";
    script += cmd+"\n";
    Utilities::processOutput(cmd);

    QFile file(ctx->outputDir.absoluteFilePath("package.sh"));
    file.open(QIODevice::WriteOnly);
    file.write(script.toStdString().c_str(), qstrlen(script.toStdString().c_str()));
    file.close();

}
