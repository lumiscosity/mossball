/********************************************************************************
** Form generated from reading UI file 'changelogwidget.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHANGELOGWIDGET_H
#define UI_CHANGELOGWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ChangelogWidget
{
public:
    QVBoxLayout *verticalLayout;
    QSplitter *splitter;
    QTreeView *treeView;
    QPlainTextEdit *plainTextEdit;
    QDialogButtonBox *buttonBox;

    void setupUi(QWidget *ChangelogWidget)
    {
        if (ChangelogWidget->objectName().isEmpty())
            ChangelogWidget->setObjectName("ChangelogWidget");
        ChangelogWidget->resize(800, 600);
        verticalLayout = new QVBoxLayout(ChangelogWidget);
        verticalLayout->setObjectName("verticalLayout");
        splitter = new QSplitter(ChangelogWidget);
        splitter->setObjectName("splitter");
        splitter->setOrientation(Qt::Orientation::Horizontal);
        treeView = new QTreeView(splitter);
        treeView->setObjectName("treeView");
        splitter->addWidget(treeView);
        plainTextEdit = new QPlainTextEdit(splitter);
        plainTextEdit->setObjectName("plainTextEdit");
        splitter->addWidget(plainTextEdit);

        verticalLayout->addWidget(splitter);

        buttonBox = new QDialogButtonBox(ChangelogWidget);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Orientation::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::StandardButton::Cancel|QDialogButtonBox::StandardButton::Ok);
        buttonBox->setCenterButtons(false);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(ChangelogWidget);

        QMetaObject::connectSlotsByName(ChangelogWidget);
    } // setupUi

    void retranslateUi(QWidget *ChangelogWidget)
    {
        ChangelogWidget->setWindowTitle(QCoreApplication::translate("ChangelogWidget", "Patch Creator", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ChangelogWidget: public Ui_ChangelogWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHANGELOGWIDGET_H
