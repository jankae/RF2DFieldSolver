#ifndef EXAMPLEBROWSERDIALOG_H
#define EXAMPLEBROWSERDIALOG_H

#include <QDialog>
#include <QList>
#include <QString>

namespace Ui {
class ExampleBrowserDialog;
}

// Lets the user browse the built-in example projects: a list of names on the
// left, the explanatory cross-section diagram of the selected one on the right,
// and a "Load Example" button. On accept, selectedResourcePath() returns the
// Qt-resource path of the chosen example project file.
class ExampleBrowserDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ExampleBrowserDialog(QWidget *parent = nullptr);
    ~ExampleBrowserDialog();

    QString selectedResourcePath() const { return chosenPath; }

private:
    struct Example {
        QString name;
        QString imagePath;     // ":/images/..."
        QString projectPath;   // ":/examples/....RF2Dproj"
    };
    void updateImage();

    Ui::ExampleBrowserDialog *ui;
    QList<Example> examples;
    QString chosenPath;
};

#endif // EXAMPLEBROWSERDIALOG_H
