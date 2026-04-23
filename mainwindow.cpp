#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "event.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QFileDialog>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QCoreApplication>
#include <QDir>

MainWindow::MainWindow(const QString& jsonFile, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , jsonFilePath(jsonFile)
    , saveValidBtn(nullptr)
    , saveInvalidBtn(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("Вывод данных");
    resize(900, 650);

    setupUI();
    loadAndProcessJson();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    auto *central = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    mainLayout->addWidget(new QLabel("<h2>Вывод данных</h2>"), 0, Qt::AlignCenter);

    auto *tablesLayout = new QHBoxLayout;
    tablesLayout->setSpacing(20);

    auto *validGroup = new QGroupBox("Корректные");
    auto *validLayout = new QVBoxLayout(validGroup);

    validTable = new QTableWidget(0, 5, this);
    validTable->setHorizontalHeaderLabels({"Название", "Описание", "Дата", "Стоимость", "Формат"});
    validTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    validTable->setAlternatingRowColors(true);
    validTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    validTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    validLayout->addWidget(validTable);
    tablesLayout->addWidget(validGroup, 1);

    auto *invalidGroup = new QGroupBox("Ошибки");
    auto *invalidLayout = new QVBoxLayout(invalidGroup);

    invalidTable = new QTableWidget(0, 6, this);
    invalidTable->setHorizontalHeaderLabels({"Название", "Описание", "Дата", "Стоимость", "Формат", "Ошибка"});
    invalidTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    invalidTable->setAlternatingRowColors(true);
    invalidTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    invalidTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    invalidTable->setStyleSheet("QTableWidget { background-color: #ffe6e6; } QHeaderView::section { background-color: #ffcccc; }");

    invalidLayout->addWidget(invalidTable);
    tablesLayout->addWidget(invalidGroup, 1);

    mainLayout->addLayout(tablesLayout);

    auto *btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(15);
    btnLayout->addStretch();

    QPushButton *loadBtn = new QPushButton("Загрузить .json", this);
    loadBtn->setFixedSize(140, 45);
    loadBtn->setStyleSheet("QPushButton { background-color: #9E9E9E; color: white; font-weight: bold; border-radius: 5px; }");
    connect(loadBtn, &QPushButton::clicked, this, &MainWindow::loadJson);
    btnLayout->addWidget(loadBtn);

    saveValidBtn = new QPushButton("Сохранить корректные", this);
    saveValidBtn->setFixedSize(160, 45);
    saveValidBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; border-radius: 5px; }");
    connect(saveValidBtn, &QPushButton::clicked, this, &MainWindow::saveValidEvents);
    btnLayout->addWidget(saveValidBtn);

    saveInvalidBtn = new QPushButton("Сохранить ошибки", this);
    saveInvalidBtn->setFixedSize(160, 45);
    saveInvalidBtn->setStyleSheet("QPushButton { background-color: #f44336; color: white; font-weight: bold; border-radius: 5px; }");
    connect(saveInvalidBtn, &QPushButton::clicked, this, &MainWindow::saveInvalidEvents);
    btnLayout->addWidget(saveInvalidBtn);

    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    setCentralWidget(central);
}

void MainWindow::loadJson()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Открыть JSON", "", "JSON (*.json)");
    if (fileName.isEmpty()) return;

    jsonFilePath = fileName;
    loadAndProcessJson();
}

void MainWindow::loadAndProcessJson()
{
    QFile file(jsonFilePath);
    if (!file.exists()) {
        QMessageBox::information(this, "Информация", "Файл JSON не найден.");
        return;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл");
        return;
    }

    validEvents.clear();
    invalidEvents.clear();

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    auto processObject = [&](const QJsonObject& obj) {
        Event e = Event::fromJson(obj);
        if (e.isValid()) {
            validEvents.append(e);
        } else {
            invalidEvents.append(e);
        }
    };

    if (doc.isArray()) {
        for (const QJsonValue& val : doc.array()) {
            processObject(val.toObject());
        }
    } else if (doc.isObject()) {
        processObject(doc.object());
    }

    fillTables();

    QMessageBox::information(this, "Готово",
                             QString("Загружено: %1\nКорректных: %2\nОшибок: %3")
                                 .arg(validEvents.size() + invalidEvents.size())
                                 .arg(validEvents.size())
                                 .arg(invalidEvents.size()));
}

void MainWindow::fillTables()
{
    validTable->setRowCount(validEvents.size());
    for (int i = 0; i < validEvents.size(); ++i) {
        const Event& e = validEvents[i];
        validTable->setItem(i, 0, new QTableWidgetItem(e.name));
        validTable->setItem(i, 1, new QTableWidgetItem(e.description));
        validTable->setItem(i, 2, new QTableWidgetItem(e.date.toString("yyyy-MM-dd")));
        validTable->setItem(i, 3, new QTableWidgetItem(QString::number(e.cost, 'f', 2)));
        validTable->setItem(i, 4, new QTableWidgetItem(e.format));
    }

    invalidTable->setRowCount(invalidEvents.size());
    for (int i = 0; i < invalidEvents.size(); ++i) {
        const Event& e = invalidEvents[i];
        QString errorText = getValidationError(e);

        invalidTable->setItem(i, 0, new QTableWidgetItem(e.name));
        invalidTable->setItem(i, 1, new QTableWidgetItem(e.description));
        invalidTable->setItem(i, 2, new QTableWidgetItem(e.date.isValid() ? e.date.toString("yyyy-MM-dd") : "Ошибка"));
        invalidTable->setItem(i, 3, new QTableWidgetItem(QString::number(e.cost)));
        invalidTable->setItem(i, 4, new QTableWidgetItem(e.format));
        invalidTable->setItem(i, 5, new QTableWidgetItem(errorText));
    }
}

QString MainWindow::getValidationError(const Event& e)
{
    QString errors;
    if (e.name.trimmed().isEmpty()) errors += "Пустое название; ";
    if (e.description.trimmed().isEmpty()) errors += "Пустое описание; ";
    if (!e.date.isValid()) errors += "Неверная дата; ";
    if (e.cost < 0) errors += "Отрицательная стоимость; ";

    QString f = e.format.toLower().trimmed();
    if (f != "онлайн" && f != "офлайн" && f != "гибридный")
        errors += "Неверный формат; ";

    return errors.isEmpty() ? "Неизвестная ошибка" : errors;
}

void MainWindow::saveValidEvents()
{
    if (validEvents.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Нет корректных объектов для сохранения");
        return;
    }

    QString appDir = QCoreApplication::applicationDirPath();
    QDir dir(appDir);
    if (!dir.exists("json")) {
        dir.mkdir("json");
    }

    QString fileName = appDir + "/json/valid_events.json";
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось создать файл");
        return;
    }

    QJsonArray arr;
    for (const Event& e : validEvents) {
        QJsonDocument doc = QJsonDocument::fromJson(e.toJson().toUtf8());
        arr.append(doc.object());
    }

    QJsonDocument finalDoc(arr);
    file.write(finalDoc.toJson(QJsonDocument::Indented));
    file.close();

    QMessageBox::information(this, "Успех",
                             QString("Корректные объекты сохранены в:\n%1\n\nВсего: %2").arg(fileName).arg(validEvents.size()));
}

void MainWindow::saveInvalidEvents()
{
    if (invalidEvents.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Нет объектов с ошибками для сохранения");
        return;
    }

    QString appDir = QCoreApplication::applicationDirPath();
    QDir dir(appDir);
    if (!dir.exists("json")) {
        dir.mkdir("json");
    }

    QString fileName = appDir + "/json/invalid_events.json";
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось создать файл");
        return;
    }

    QJsonArray arr;
    for (const Event& e : invalidEvents) {
        QJsonDocument doc = QJsonDocument::fromJson(e.toJson().toUtf8());
        arr.append(doc.object());
    }

    QJsonDocument finalDoc(arr);
    file.write(finalDoc.toJson(QJsonDocument::Indented));
    file.close();

    QMessageBox::information(this, "Успех",
                             QString("Объекты с ошибками сохранены в:\n%1\n\nВсего: %2").arg(fileName).arg(invalidEvents.size()));
}
