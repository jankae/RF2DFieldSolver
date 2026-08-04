#include "savable.h"
#include "CustomWidgets/informationbox.h"

#include <QFileDialog>
#include <QFile>
#include <fstream>
#include <iomanip>

using namespace std;

bool Savable::openFromFile(QString filename)
{
    if(filename.isEmpty()) {
        return false;
    }
    ifstream file;
    file.open(filename.toStdString());
    if(!file.is_open()) {
        qWarning() << "Unable to open file:" << filename;
        return false;
    }
    nlohmann::json j;
    try {
        file >> j;
    } catch (exception &e) {
        InformationBox::ShowError("Error", "Failed to parse the setup file (" + QString(e.what()) + ")");
        qWarning() << "Parsing of setup file failed: " << e.what();
        file.close();
        return false;
    }
    file.close();
    fromJSON(j);
    return true;
}

bool Savable::openFromResource(QString resourcePath)
{
    // Qt resource paths (":/...") are not real filesystem paths, so they must be
    // read through QFile rather than std::ifstream.
    QFile file(resourcePath);
    if(!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to open resource:" << resourcePath;
        return false;
    }
    auto contents = file.readAll();
    file.close();
    nlohmann::json j;
    try {
        j = nlohmann::json::parse(contents.toStdString());
    } catch (exception &e) {
        InformationBox::ShowError("Error", "Failed to parse the example file (" + QString(e.what()) + ")");
        qWarning() << "Parsing of example file failed: " << e.what();
        return false;
    }
    fromJSON(j);
    return true;
}

bool Savable::openFromFileDialog(QString title, QString filetype)
{
    auto filename = QFileDialog::getOpenFileName(nullptr, title, "", filetype, nullptr, QFileDialog::DontUseNativeDialog);
    if(filename.isEmpty()) {
        // aborted selection
        return false;
    }
    ifstream file;
    file.open(filename.toStdString());
    if(!file.is_open()) {
        qWarning() << "Unable to open file:" << filename;
        return false;
    }
    nlohmann::json j;
    try {
        file >> j;
    } catch (exception &e) {
        InformationBox::ShowError("Error", "Failed to parse the setup file (" + QString(e.what()) + ")");
        qWarning() << "Parsing of setup file failed: " << e.what();
        file.close();
        return false;
    }
    file.close();
    fromJSON(j);
    return true;
}

bool Savable::saveToFileDialog(QString title, QString filetype, QString ending)
{
    auto filename = QFileDialog::getSaveFileName(nullptr, title, "", filetype, nullptr, QFileDialog::DontUseNativeDialog);
    if(filename.isEmpty()) {
        // aborted selection
        return false;
    }
    if(!ending.isEmpty()) {
        if(!filename.endsWith(ending)) {
            filename.append(ending);
        }
    }
    ofstream file;
    file.open(filename.toStdString());
    file << setw(4) << toJSON() << endl;
    file.close();
    return true;
}
