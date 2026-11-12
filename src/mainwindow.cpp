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
#include <QSettings>
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
    Deck builtIn;
    builtIn.load(":/data/italian_1000.json");
    if (!m_deck.load(m_path) || m_deck.total() == 0)
        m_deck = builtIn;
    else
        m_deck.mergeSynonyms(builtIn);   // saved decks from older versions get the new synonyms

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
    m_prevBtn = new QPushButton("◀ Previous group");
    m_nextBtn = new QPushButton("Next group ▶");
    row2->addWidget(m_prevBtn);
    row2->addWidget(m_nextBtn);
    layout->addLayout(row2);

    row2 = new QHBoxLayout;
    auto *addBtn = new QPushButton("Add card");
    auto *importBtn = new QPushButton("Import deck");
    auto *restartBtn = new QPushButton("Restart");
    row2->addWidget(addBtn);
    row2->addWidget(importBtn);
    row2->addWidget(restartBtn);
    layout->addLayout(row2);

    // Enter in the text box and the Check button do the same thing:
    // check the guess, or (after a wrong answer) go to the next card.
    auto onEnter = [this] {
        if (m_showingAnswer) next();
        else if (!m_deck.hasCurrent()) nextGroup();   // group finished
        else check();
    };
    connect(m_input, &QLineEdit::returnPressed, this, onEnter);
    connect(m_checkBtn, &QPushButton::clicked, this, onEnter);
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::addCard);
    connect(importBtn, &QPushButton::clicked, this, &MainWindow::importDeck);
    connect(restartBtn, &QPushButton::clicked, this, &MainWindow::restart);
    connect(m_prevBtn, &QPushButton::clicked, this, &MainWindow::prevGroup);
    connect(m_nextBtn, &QPushButton::clicked, this, &MainWindow::nextGroup);

    setCentralWidget(central);
    setWindowTitle("Flashcards");

    // Continue from the group you were on last time.
    startGroup(QSettings().value("group", 0).toInt());
}

int MainWindow::groupCount() const {
    return (m_deck.total() + GroupSize - 1) / GroupSize;
}

void MainWindow::startGroup(int g) {
    m_group = qBound(0, g, qMax(0, groupCount() - 1));
    QSettings().setValue("group", m_group);
    m_deck.startSession(m_group * GroupSize, GroupSize);
    m_known = 0;
    m_showingAnswer = false;
    m_input->setReadOnly(false);
    m_feedback->clear();
    showCard();
}

void MainWindow::prevGroup() { startGroup(m_group - 1); }
void MainWindow::nextGroup() { startGroup(m_group + 1); }

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

// A guess is right if it matches any of the comma-separated meanings
// ("house" for "home, house") or one of the card's synonyms ("dad" for "father").
bool MainWindow::isCorrect(const QString &guess, const Card &card) {
    const QString g = normalize(guess);
    if (g.isEmpty())
        return false;
    QStringList accepted = card.back.split(QRegularExpression("[,/;]"));
    accepted << card.back << card.alt;
    for (const QString &a : accepted)
        if (g == normalize(a))
            return true;
    return false;
}

void MainWindow::showCard() {
    bool has = m_deck.hasCurrent();
    m_input->clear();
    m_input->setEnabled(has);
    m_checkBtn->setEnabled(has || m_group < groupCount() - 1);
    m_checkBtn->setText(has ? "Check" : "Next group");
    m_prevBtn->setEnabled(m_group > 0);
    m_nextBtn->setEnabled(m_group < groupCount() - 1);

    if (has) {
        m_cardLabel->setText(m_deck.current().front);
        m_input->setFocus();
    } else {
        m_cardLabel->setText("Group done! 🎉");
        m_feedback->setText(m_group < groupCount() - 1
                                ? "You know all 10 words.\nPress Enter for the next group."
                                : "You finished the last group!");
        m_feedback->setStyleSheet("font-size: 16px; color: #2e7d32;");
    }
    m_status->setText(QString("Group %1 of %2   ·   Known: %3   ·   Left: %4")
                          .arg(m_group + 1).arg(groupCount()).arg(m_known).arg(m_deck.remaining()));
}

void MainWindow::check() {
    if (!m_deck.hasCurrent())
        return;
    const QString answer = m_deck.current().back;

    if (isCorrect(m_input->text(), m_deck.current())) {
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

void MainWindow::restart() { startGroup(m_group); }

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
            showCard();   // new cards land in the last group
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
    startGroup(0);
}
