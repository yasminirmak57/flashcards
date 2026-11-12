#include "mainwindow.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTimer>
#include <QVBoxLayout>
#include <cstring>

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
    m_cardLabel->setMinimumSize(420, 200);
    m_cardLabel->setStyleSheet(
        "QLabel { font-size: 28px; background: white; color: #222;"
        " border: 2px solid #ccc; border-radius: 12px; padding: 16px; }");
    layout->addWidget(m_cardLabel);

    m_feedback = new QLabel;
    m_feedback->setAlignment(Qt::AlignCenter);
    m_feedback->setWordWrap(true);
    m_feedback->setMinimumHeight(48);
    m_feedback->setStyleSheet("font-size: 16px;");
    layout->addWidget(m_feedback);

    auto *row = new QHBoxLayout;
    m_input = new QLineEdit;
    m_input->setPlaceholderText("Type the English meaning and press Enter");
    m_input->setStyleSheet("font-size: 16px; padding: 6px;");
    m_checkBtn = new QPushButton("Check");
    m_checkBtn->setDefault(true);
    row->addWidget(m_input);
    row->addWidget(m_checkBtn);
    layout->addLayout(row);

    m_status = new QLabel;
    m_status->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_status);

    auto *row2 = new QHBoxLayout;
    auto *addBtn = new QPushButton("Add card");
    auto *importBtn = new QPushButton("Import deck");
    auto *restartBtn = new QPushButton("Restart");
    row2->addWidget(addBtn);
    row2->addWidget(importBtn);
    row2->addWidget(restartBtn);
    layout->addLayout(row2);

    // Enter in the text box and the Check button do the same thing:
    // check the guess, or (after a wrong answer) go to the next card.
    auto onEnter = [this] { m_showingAnswer ? next() : check(); };
    connect(m_input, &QLineEdit::returnPressed, this, onEnter);
    connect(m_checkBtn, &QPushButton::clicked, this, onEnter);
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::addCard);
    connect(importBtn, &QPushButton::clicked, this, &MainWindow::importDeck);
    connect(restartBtn, &QPushButton::clicked, this, &MainWindow::restart);

    setCentralWidget(central);
    setWindowTitle("Flashcards");
    showCard();
}

MainWindow::~MainWindow() { m_deck.save(m_path); }

// Lowercase, trim, drop "(...)" notes and a leading "to "/"the "/"a ".
static QString normalize(QString s) {
    s = s.toLower();
    s.remove(QRegularExpression("\\(.*?\\)"));
    s.remove(QRegularExpression("[.!?]"));
    s = s.simplified();
    for (const char *prefix : {"to ", "the ", "a ", "an "})
        if (s.startsWith(prefix))
            s = s.mid(int(strlen(prefix)));
    return s.trimmed();
}

// A guess is right if it matches any of the comma-separated meanings,
// e.g. "house" is right for "home, house".
bool MainWindow::isCorrect(const QString &guess, const QString &answer) {
    const QString g = normalize(guess);
    if (g.isEmpty())
        return false;
    if (g == normalize(answer))
        return true;
    for (const QString &part : answer.split(QRegularExpression("[,/;]")))
        if (g == normalize(part))
            return true;
    return false;
}

void MainWindow::showCard() {
    bool has = m_deck.hasCurrent();
    m_input->clear();
    m_input->setEnabled(has);
    m_checkBtn->setEnabled(has);
    m_checkBtn->setText("Check");

    if (has) {
        m_cardLabel->setText(m_deck.current().front);
        m_input->setFocus();
    } else {
        m_cardLabel->setText("Done! 🎉");
        m_feedback->setText("You know all the cards in this round.\nPress Restart to go again.");
        m_feedback->setStyleSheet("font-size: 16px; color: #2e7d32;");
    }
    m_status->setText(QString("Known: %1   ·   Left: %2   ·   Total: %3")
                          .arg(m_known).arg(m_deck.remaining()).arg(m_deck.total()));
}

void MainWindow::check() {
    if (!m_deck.hasCurrent())
        return;
    const QString answer = m_deck.current().back;

    if (isCorrect(m_input->text(), answer)) {
        m_deck.answer(true);     // into the "known" pile
        ++m_known;
        m_feedback->setText("✓ Correct!  (" + answer + ")");
        m_feedback->setStyleSheet("font-size: 16px; color: #2e7d32;");
        showCard();
    } else {
        // Show the right answer; Enter moves on and the card comes back later.
        m_showingAnswer = true;
        m_feedback->setText("✗ The answer is:  " + answer + "\nPress Enter to continue");
        m_feedback->setStyleSheet("font-size: 16px; color: #c62828;");
        m_input->setReadOnly(true);
        m_checkBtn->setText("Next");
    }
}

void MainWindow::next() {
    m_deck.answer(false);        // back of the queue, you'll see it again
    m_showingAnswer = false;
    m_input->setReadOnly(false);
    m_feedback->clear();
    showCard();
}

void MainWindow::restart() {
    m_deck.startSession();
    m_known = 0;
    m_showingAnswer = false;
    m_input->setReadOnly(false);
    m_feedback->clear();
    showCard();
}

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
        if (!m_showingAnswer)
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
    m_deck.save(m_path);
    restart();
}
