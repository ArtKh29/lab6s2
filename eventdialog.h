#ifndef EVENTDIALOG_H
#define EVENTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include "event.h"

class EventDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EventDialog(QWidget *parent = nullptr);
    QString getJsonFilePath() const;

private slots:
    void loadFromTxt();
    void saveAndClose();

private:
    void setupUI();

    QLineEdit *nameEdit;
    QTextEdit *descEdit;
    QLineEdit *dateEdit;
    QLineEdit *costEdit;
    QLineEdit *formatEdit;

    QString jsonFilePath;
};

#endif // EVENTDIALOG_H
