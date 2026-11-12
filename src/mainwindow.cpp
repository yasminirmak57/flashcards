#include "mainwindow.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStandardPaths>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // Cards are saved in the user's app data folder.
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    m_path = dir + "/cards.json";

    // First launch: start with the built-in Italian deck.
    if (!m_deck.load(m_path) || m_deck.total() == 0)
        m_deck.load(":/data/italian_1000.json");
    m_deck.startSession();

    auto *central = new QWidget;
    auto *layout = new QVBoxLayout(central);

    m_cardLabel = new QLabel;
    m_cardLabel->setAlignment(Qt::AlignCenter);
    m_cardLabel->setWordWrap(true);
    m_cardLabel->setMinimumSize(400, 220);
    m_cardLabel->setStyleSheet(
        "QLabel { font-size: 20px; background: white; color: #222;"
        " border: 2px solid #ccc; border-radius: 12px; padding: 16px; }");
    layout->addWidget(m_cardLabel);

    m_status = new QLabel;
    m_status->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_status);

    auto *row = new QHBoxLayout;
    m_flipBtn = new QPushButton("Flip");
    m_missBtn = new QPushButton("Missed");
    m_gotBtn  = new QPushButton("Got it");
    row->addWidget(m_missBtn);
    row->addWidget(m_flipBtn);
    row->addWidget(m_gotBtn);
    layout->addLayout(row);

    auto *row2 = new QHBoxLayout;
    auto *addBtn = new QPushButton("Add card");
    auto *restartBtn = new QPushButton("Restart");
    auto *importBtn = new QPushButton("Import deck");
    row2->addWidget(addBtn);
    row2->addWidget(importBtn);
    row2->addWidget(restartBtn);
    layout->addLayout(row2);

    connect(m_flipBtn, &QPushButton::clicked, this, &MainWindow::flip);
    connect(m_gotBtn,  &QPushButton::clicked, this, &MainWindow::gotIt);
    connect(m_missBtn, &QPushButton::clicked, this, &MainWindow::missed);
    connect(addBtn,    &QPushButton::clicked, this, &MainWindow::addCard);
    connect(restartBtn,&QPushButton::clicked, this, &MainWindow::restart);
    connect(importBtn, &QPushButton::clicked, this, &MainWindow::importDeck);

    setCentralWidget(central);
    setWindowTitle("Flashcards");
    showCard();
}

MainWindow::~MainWindow() { m_deck.save(m_path); }

void MainWindow::showCard() {
    bool has = m_deck.hasCurrent();
    if (has) {
        const Card &c = m_deck.current();
        m_cardLabel->setText(m_showingBack ? c.back : c.front);
        m_status->setText(QString("%1 left of %2 · %3")
                              .arg(m_deck.remaining()).arg(m_deck.total())
                              .arg(m_showingBack ? "answer" : "question"));
    } else {
        m_cardLabel->setText("Done! 🎉\nPress Restart to go again.");
        m_status->setText(QString("%1 cards").arg(m_deck.total()));
    }
    m_flipBtn->setEnabled(has);
    m_gotBtn->setEnabled(has && m_showingBack);   // must see answer before grading
    m_missBtn->setEnabled(has && m_showingBack);
}

void MainWindow::flip() { m_showingBack = !m_showingBack; showCard(); }

void MainWindow::gotIt()  { m_deck.answer(true);  m_showingBack = false; showCard(); }
void MainWindow::missed() { m_deck.answer(false); m_showingBack = false; showCard(); }

void MainWindow::restart() { m_deck.startSession(); m_showingBack = false; showCard(); }

void MainWindow::addCard() {
    QDialog dlg(this);
    dlg.setWindowTitle("New card");
    auto *form = new QFormLayout(&dlg);
    auto *front = new QLineEdit;
    auto *back = new QLineEdit;
    form->addRow("Front:", front);
    form->addRow("Back:", back);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted && !front->text().isEmpty() && !back->text().isEmpty()) {
        m_deck.addCard(front->text(), back->text());
        m_deck.save(m_path);
        showCard();
    }
}

void MainWindow::importDeck() {
    QString file = QFileDialog::getOpenFileName(this, "Import deck", QString(), "Deck (*.json)");
    if (file.isEmpty())
        return;
    Deck d;
    if (!d.load(file) || d.total() == 0) {
        QMessageBox::warning(this, "Import failed", "That file isn't a valid deck.");
        return;
    }
    m_deck = d;
    m_deck.startSession();
    m_deck.save(m_path);
    m_showingBack = false;
    showCard();
}
