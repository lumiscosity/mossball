#include "directorydialog.h"
#include "ui_directorydialog.h"

DirectoryDialog::DirectoryDialog(QWidget *parent) : QDialog(parent), ui(new Ui::DirectoryDialog) {
    ui->setupUi(this);
}

DirectoryDialog::~DirectoryDialog() {
    delete ui;
}
