// Pegasus Frontend
// Copyright (C) 2017  Mátyás Mustoha
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.


#include "Api.h"
#include "FrontendLayer.h"
#include "Model.h"
#include "ProcessLauncher.h"
#include "QuitStatus.h"
#include "ScriptRunner.h"
#include "SystemCommands.h"

#include <QCommandLineParser>
#include <QGamepadManager>
#include <QGuiApplication>
#include <QQmlContext>
#include <QSettings>


void runGamepadConfigScripts();
void runQuitScripts();

int main(int argc, char *argv[])
{
    QCoreApplication::addLibraryPath("lib/plugins");
    QCoreApplication::addLibraryPath("lib");

    QGuiApplication app(argc, argv);
    app.setApplicationName("pegasus-frontend");
    app.setApplicationVersion(GIT_REVISION);
    app.setOrganizationName("pegasus-frontend");
    app.setOrganizationDomain("pegasus-frontend.org");
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QCommandLineParser argparser;
    argparser.setApplicationDescription("\n" + QObject::tr(
        "A cross platform, customizable graphical frontend for launching emulators\n"
        "and managing your game collection."));
    argparser.addHelpOption();
    argparser.addVersionOption();
    argparser.process(app);


    static constexpr auto API_URI = "Pegasus.Model";
    const QString error_msg = QObject::tr("Sorry, you cannot create this type in QML.");
    qmlRegisterUncreatableType<Model::Platform>(API_URI, 0, 2, "Platform", error_msg);
    qmlRegisterUncreatableType<Model::Game>(API_URI, 0, 2, "Game", error_msg);
    qmlRegisterUncreatableType<Model::GameAssets>(API_URI, 0, 2, "GameAssets", error_msg);
    qmlRegisterUncreatableType<ApiParts::Language>(API_URI, 0, 3, "Language", error_msg);


    ApiObject api;
    FrontendLayer frontend(&api);
    ProcessLauncher launcher;

    // the following communication is required because process handling
    // and destroying/rebuilding the frontend stack are asynchronous tasks;
    // see the relevant classes

    QObject::connect(&api, &ApiObject::prepareLaunch,
                     &frontend, &FrontendLayer::teardown);

    QObject::connect(&frontend, &FrontendLayer::teardownComplete,
                     &api, &ApiObject::onReadyToLaunch);

    QObject::connect(&api, &ApiObject::executeLaunch,
                     &launcher, &ProcessLauncher::launchGame);

    QObject::connect(&launcher, &ProcessLauncher::processFinished,
                     &api, &ApiObject::onGameFinished);

    QObject::connect(&api, &ApiObject::restoreAfterGame,
                     &frontend, &FrontendLayer::rebuild);


    // run the gamepad configuration change scripts
    QObject::connect(QGamepadManager::instance(), &QGamepadManager::axisConfigured,
                     runGamepadConfigScripts);
    QObject::connect(QGamepadManager::instance(), &QGamepadManager::buttonConfigured,
                     runGamepadConfigScripts);


    // run the quit/reboot/shutdown scripts on exit;
    // on some platforms, app.exec() may not return so aboutToQuit()
    // is used for calling these methods
    QObject::connect(&app, &QCoreApplication::aboutToQuit,
                     runQuitScripts);

    return app.exec();
}

void runGamepadConfigScripts()
{
    using ScriptEvent = ScriptRunner::EventType;

    ScriptRunner::findAndRunScripts(ScriptEvent::CONFIG_CHANGED);
    ScriptRunner::findAndRunScripts(ScriptEvent::CONTROLS_CHANGED);
}

void runQuitScripts()
{
    using ScriptEvent = ScriptRunner::EventType;

    ScriptRunner::findAndRunScripts(ScriptEvent::QUIT);
    switch (QuitStatus::status) {
        case QuitStatus::Type::REBOOT:
            ScriptRunner::findAndRunScripts(ScriptEvent::REBOOT);
            break;
        case QuitStatus::Type::SHUTDOWN:
            ScriptRunner::findAndRunScripts(ScriptEvent::SHUTDOWN);
            break;
        default:
            break;
    }

    qInfo().noquote() << QObject::tr("Closing Pegasus, goodbye!");

    switch (QuitStatus::status) {
        case QuitStatus::Type::REBOOT:
            SystemCommands::reboot();
            break;
        case QuitStatus::Type::SHUTDOWN:
            SystemCommands::shutdown();
            break;
        default:
            break;
    }
}
