#pragma once

#include <QDialog>

namespace Ui {
class DirectoryDialog;
}

class DirectoryDialog : public QDialog {
    Q_OBJECT

public:
    explicit DirectoryDialog(QWidget *parent = nullptr);
    ~DirectoryDialog();

private:
    Ui::DirectoryDialog *ui;
};
