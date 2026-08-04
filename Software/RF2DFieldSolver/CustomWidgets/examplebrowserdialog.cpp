#include "examplebrowserdialog.h"
#include "ui_examplebrowserdialog.h"

#include <QListWidget>
#include <QPushButton>
#include <QPixmap>

ExampleBrowserDialog::ExampleBrowserDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::ExampleBrowserDialog)
{
    ui->setupUi(this);

    // The built-in examples. Image paths reuse the original scenario diagrams.
    examples = {
        { "Microstrip",                        ":/images/microstrip.png",                       ":/examples/microstrip.RF2Dproj" },
        { "Coplanar Microstrip",               ":/images/coplanar_microstrip.png",              ":/examples/coplanar_microstrip.RF2Dproj" },
        { "Differential Microstrip",           ":/images/microstrip_differential.png",          ":/examples/differential_microstrip.RF2Dproj" },
        { "Coplanar Differential Microstrip",  ":/images/coplanar_microstrip_differential.png", ":/examples/coplanar_differential_microstrip.RF2Dproj" },
        { "Stripline",                         ":/images/stripline.png",                        ":/examples/stripline.RF2Dproj" },
        { "Coplanar Stripline",                ":/images/coplanar_stripline.png",               ":/examples/coplanar_stripline.RF2Dproj" },
        { "Differential Stripline",            ":/images/stripline_differential.png",           ":/examples/differential_stripline.RF2Dproj" },
        { "Coplanar Differential Stripline",   ":/images/coplanar_stripline_differential.png",  ":/examples/coplanar_differential_stripline.RF2Dproj" },
    };

    for(auto &e : examples) {
        ui->list->addItem(e.name);
    }

    connect(ui->list, &QListWidget::currentRowChanged, this, [=](){ updateImage(); });
    connect(ui->list, &QListWidget::itemDoubleClicked, this, [=](){
        if(ui->list->currentRow() >= 0) {
            chosenPath = examples[ui->list->currentRow()].projectPath;
            accept();
        }
    });
    connect(ui->load, &QPushButton::clicked, this, [=](){
        int row = ui->list->currentRow();
        if(row >= 0 && row < examples.size()) {
            chosenPath = examples[row].projectPath;
            accept();
        }
    });
    connect(ui->cancel, &QPushButton::clicked, this, &QDialog::reject);

    ui->list->setCurrentRow(0);
    updateImage();
}

ExampleBrowserDialog::~ExampleBrowserDialog()
{
    delete ui;
}

void ExampleBrowserDialog::updateImage()
{
    int row = ui->list->currentRow();
    if(row < 0 || row >= examples.size()) {
        ui->image->clear();
        return;
    }
    QPixmap pix(examples[row].imagePath);
    if(pix.isNull()) {
        ui->image->setText("(no image)");
        return;
    }
    // fit the diagram into the label while keeping its aspect ratio
    ui->image->setPixmap(pix.scaled(ui->image->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
