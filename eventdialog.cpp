#include "eventdialog.h"
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDate>
#include <QCoreApplication>
#include <QDir>

EventDialog::EventDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Создание объектов");
    setFixedSize(550, 350);

    QString appDir = QCoreApplication::applicationDirPath();
    jsonFilePath = appDir + "/objects.json";

    setupUI();
}

void EventDialog::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    auto *title = new QLabel("<h2>Новый объект</h2>");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    auto *formWidget = new QWidget(this);
    auto *formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(10);
    formLayout->setLabelAlignment(Qt::AlignRight);

    nameEdit = new QLineEdit(this);
    formLayout->addRow("Название:", nameEdit);

    descEdit = new QTextEdit(this);
    descEdit->setMaximumHeight(50);
    formLayout->addRow("Описание:", descEdit);

    dateEdit = new QLineEdit(this);
    formLayout->addRow("Дата:", dateEdit);

    costEdit = new QLineEdit(this);
    formLayout->addRow("Стоимость:", costEdit);

    formatEdit = new QLineEdit(this);
    formLayout->addRow("Формат:", formatEdit);

    mainLayout->addWidget(formWidget);

    auto *btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(15);
    btnLayout->addStretch();

    QPushButton *txtBtn = new QPushButton("из .txt", this);
    txtBtn->setFixedSize(120, 50);
    txtBtn->setStyleSheet("QPushButton { background-color: #ffcccb; border-radius: 10px; font-weight: bold; font-size: 14px; }");
    connect(txtBtn, &QPushButton::clicked, this, &EventDialog::loadFromTxt);
    btnLayout->addWidget(txtBtn);

    btnLayout->addStretch();

    QPushButton *okBtn = new QPushButton("OK", this);
    okBtn->setFixedSize(120, 50);
    okBtn->setStyleSheet("QPushButton { background-color: #2196F3; color: white; font-size: 16px; font-weight: bold; border-radius: 10px; }");
    connect(okBtn, &QPushButton::clicked, this, &EventDialog::saveAndClose);
    btnLayout->addWidget(okBtn);

    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);
}

void EventDialog::loadFromTxt()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Загрузить из TXT", "", "TXT (*.txt)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл");
        return;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;

        QString error;
        Event temp;
        if (temp.fromTxt(line, error)) {
            nameEdit->setText(temp.name);
            descEdit->setPlainText(temp.description);
            dateEdit->setText(temp.date.toString("yyyy-MM-dd"));
            costEdit->setText(QString::number(temp.cost));
            formatEdit->setText(temp.format);
            break;
        } else {
            QMessageBox::warning(this, "Внимание", "Ошибка в TXT файле:\n" + error);
        }
    }
    file.close();
}

void EventDialog::saveAndClose()
{
    QDate date = QDate::fromString(dateEdit->text().trimmed(), "yyyy-MM-dd");
    if (!date.isValid()) {
        QMessageBox::warning(this, "Ошибка", "Неверный формат даты. Используйте: YYYY-MM-DD");
        return;
    }

    bool ok;
    double cost = costEdit->text().trimmed().toDouble(&ok);
    if (!ok || cost < 0) {
        QMessageBox::warning(this, "Ошибка", "Неверная стоимость. Введите число >= 0");
        return;
    }

    Event e(
        nameEdit->text().trimmed(),
        descEdit->toPlainText().trimmed(),
        date,
        cost,
        formatEdit->text().trimmed()
        );

    if (!e.isValid()) {
        QMessageBox::warning(this, "Ошибка валидации",
                             "Заполните все поля корректно.\n"
                             "Формат: онлайн / офлайн / гибридный");
        return;
    }

    QFile file(jsonFilePath);
    QJsonArray existingArray;

    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isArray()) {
            existingArray = doc.array();
        }
        file.close();
    }

    QJsonDocument newDoc = QJsonDocument::fromJson(e.toJson().toUtf8());
    existingArray.append(newDoc.object());

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QJsonDocument finalDoc(existingArray);
        file.write(finalDoc.toJson(QJsonDocument::Indented));
        file.close();

        QMessageBox::information(this, "Успех",
                                 QString("Объект сохранен!\nВсего объектов: %1").arg(existingArray.size()));
        accept();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить файл");
    }
}

QString EventDialog::getJsonFilePath() const
{
    return jsonFilePath;
}
