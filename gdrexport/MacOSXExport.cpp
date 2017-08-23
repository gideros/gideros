/*
 * MacOSXExport.cpp
 *
 *  Created on: 22 janv. 2016
 *      Author: Nico
 */

#include "MacOSXExport.h"
#include <QProcess>
#include "Utilities.h"

void MacOSXExport::CodeSignMacOSX(ExportContext *ctx) {
	ctx->outputDir.cd(ctx->base + ".app");
	ctx->outputDir.cd("Contents");
	QString script = "";
	QProcess postProcess;
	QString cmd;
	QStringList dylibs;
	QString signingId = ctx->args["signingId"];
	if (signingId.isEmpty())
		signingId = ctx->properties.osx_signingId;
	QString installerId = ctx->args["installerId"];
	if (installerId.isEmpty())
		installerId = ctx->properties.osx_installerId;
	cmd = "iconutil -c icns -o "+ctx->outputDir.absoluteFilePath("Resources/desktop.icns")+" "+ctx->outputDir.absoluteFilePath("../../icon.iconset");
	script += cmd + "\n";
	Utilities::processOutput(cmd);

	if (ctx->outputDir.cd("Frameworks")) {
		QStringList frameworks = ctx->outputDir.entryList(
				QStringList() << "*.framework");
		for (int i = 0; i < frameworks.size(); ++i) {
			QString filename = ctx->outputDir.absoluteFilePath(frameworks[i]);
			cmd = "codesign -f -s \"" + signingId + "\" \"" + filename
					+ "/Versions/Current\"";
			script += cmd + "\n";
			Utilities::processOutput(cmd);
		}
		dylibs = ctx->outputDir.entryList(
				QStringList() << "*.dylib");
		for (int i = 0; i < dylibs.size(); ++i) {
			QString filename = ctx->outputDir.absoluteFilePath(dylibs[i]);
			cmd = "codesign -f -s \"" + signingId + "\" \"" + filename + "\"";
			script += cmd + "\n";
			Utilities::processOutput(cmd);
		}
		ctx->outputDir.cdUp();
	}

	if (ctx->outputDir.cd("Plugins")) {
		dylibs = ctx->outputDir.entryList(QStringList() << "*.dylib");
		for (int i = 0; i < dylibs.size(); ++i) {
			QString filename = ctx->outputDir.absoluteFilePath(dylibs[i]);
			cmd = "codesign -f -s \"" + signingId + "\" \"" + filename + "\"";
			script += cmd + "\n";
			Utilities::processOutput(cmd);
		}

		if (ctx->outputDir.cd("bearer")) {
			dylibs = ctx->outputDir.entryList(QStringList() << "*.dylib");
			for (int i = 0; i < dylibs.size(); ++i) {
				QString filename = ctx->outputDir.absoluteFilePath(dylibs[i]);
				cmd = "codesign -f -s \"" + signingId + "\" \"" + filename
						+ "\"";
				script += cmd + "\n";
				Utilities::processOutput(cmd);
			}
			ctx->outputDir.cdUp();
		}

		if (ctx->outputDir.cd("imageformats")) {
			dylibs = ctx->outputDir.entryList(QStringList() << "*.dylib");
			for (int i = 0; i < dylibs.size(); ++i) {
				QString filename = ctx->outputDir.absoluteFilePath(dylibs[i]);
				cmd = "codesign -f -s \"" + signingId + "\" \"" + filename
						+ "\"";
				script += cmd + "\n";
				Utilities::processOutput(cmd);
			}
			ctx->outputDir.cdUp();
		}

		if (ctx->outputDir.cd("platforms")) {
			dylibs = ctx->outputDir.entryList(QStringList() << "*.dylib");
			for (int i = 0; i < dylibs.size(); ++i) {
				QString filename = ctx->outputDir.absoluteFilePath(dylibs[i]);
				cmd = "codesign -f -s \"" + signingId + "\" \"" + filename
						+ "\"";
				script += cmd + "\n";
				Utilities::processOutput(cmd);
			}
			ctx->outputDir.cdUp();
		}

		if (ctx->outputDir.cd("printsupport")) {
			dylibs = ctx->outputDir.entryList(QStringList() << "*.dylib");
			for (int i = 0; i < dylibs.size(); ++i) {
				QString filename = ctx->outputDir.absoluteFilePath(dylibs[i]);
				cmd = "codesign -f -s \"" + signingId + "\" \"" + filename
						+ "\"";
				script += cmd + "\n";
				Utilities::processOutput(cmd);
			}
			ctx->outputDir.cdUp();
		}

		if (ctx->outputDir.cd("audio")) {
			dylibs = ctx->outputDir.entryList(QStringList() << "*.dylib");
			for (int i = 0; i < dylibs.size(); ++i) {
				QString filename = ctx->outputDir.absoluteFilePath(dylibs[i]);
				cmd = "codesign -f -s \"" + signingId + "\" \"" + filename
						+ "\"";
				script += cmd + "\n";
				Utilities::processOutput(cmd);
			}
			ctx->outputDir.cdUp();
		}

		if (ctx->outputDir.cd("mediaservice")) {
			dylibs = ctx->outputDir.entryList(QStringList() << "*.dylib");
			for (int i = 0; i < dylibs.size(); ++i) {
				QString filename = ctx->outputDir.absoluteFilePath(dylibs[i]);
				cmd = "codesign -f -s \"" + signingId + "\" \"" + filename
						+ "\"";
				script += cmd + "\n";
				Utilities::processOutput(cmd);
			}
			ctx->outputDir.cdUp();
		}

		ctx->outputDir.cdUp();
	}
	cmd = "codesign -f -s \"" + signingId + "\" \"" + ctx->outputDir.absoluteFilePath("Entitlements.plist")	+ "\"";
	script += cmd + "\n";
	Utilities::processOutput(cmd);

	ctx->outputDir.cdUp();
	ctx->outputDir.cdUp();
	cmd = "codesign -f -s \"" + signingId + "\" --entitlements \"/"
			+ ctx->outputDir.absolutePath() + "/Entitlements.plist\" \""
			+ ctx->outputDir.absoluteFilePath(ctx->base + ".app") + "\"";
	script += cmd + "\n";
	Utilities::processOutput(cmd);

	cmd = "productbuild --component \""
			+ ctx->outputDir.absoluteFilePath(ctx->base + ".app")
			+ "\" /Applications --sign \"" + installerId + "\" \""
			+ ctx->outputDir.absoluteFilePath(ctx->base + ".pkg") + "\"";
	script += cmd + "\n";
	Utilities::processOutput(cmd);

	QFile file(ctx->outputDir.absoluteFilePath("package.sh"));
	file.open(QIODevice::WriteOnly);
	file.write(script.toStdString().c_str(),
			qstrlen(script.toStdString().c_str()));
	file.close();

}
