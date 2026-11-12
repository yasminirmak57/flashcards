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
#include <QProgressBar>
#include <QGraphicsDropShadowEffect>
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

    // ---------- look & feel ----------
    auto *central = new QWidget;
    central->setObjectName("root");
    central->setStyleSheet(R"(
        #root { background: #f4f5f7; }
        QLabel { color: #1d1d1f; }
        #groupLabel { font-size: 15px; font-weight: 600; color: #6e6e73; }
        #card { background: white; border-radius: 20px; }
        #hint { font-size: 12px; letter-spacing: 2px; color: #a1a1a6; }
        #word { font-size: 44px; font-weight: 600; color: #1d1d1f; }
        #status { font-size: 13px; color: #8e8e93; }
        QLineEdit {
            font-size: 18px; padding: 12px 16px; border: 2px solid #e1e3e8;
            border-radius: 12px; background: white; color: #1d1d1f;
        }
        QLineEdit:focus { border-color: #009246; }
        QPushButton {
            font-size: 14px; padding: 9px 16px; border-radius: 10px;
            background: white; color: #1d1d1f; border: 1px solid #e1e3e8;
        }
        QPushButton:hover { background: #f0f1f4; }
        QPushButton:disabled { color: #c7c7cc; }
        #primary {
            background: #009246; color: white; border: none; font-weight: 600;
            font-size: 16px; padding: 12px 22px;
        }
        #primary:hover { background: #007a3a; }
        #primary:disabled { background: #a8d5bb; color: white; }
        #nav { border: none; background: transparent; font-size: 18px; color: #009246; padding: 4px 10px; }
        #nav:disabled { color: #d1d1d6; }
        #flat { border: none; background: transparent; color: #6e6e73; font-size: 13px; }
        #flat:hover { color: #009246; }
        QProgressBar { background: #e5e6ea; border: none; border-radius: 4px; height: 8px; }
        QProgressBar::chunk { background: #009246; border-radius: 4px; }
    )");

    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(32, 24, 32, 20);
    layout->setSpacing(14);

    // Header: ◀  Group 3 of 116  ▶
    auto *header = new QHBoxLayout;
    m_prevBtn = new QPushButton("◀");
    m_nextBtn = new QPushButton("▶");
    m_prevBtn->setObjectName("nav");
    m_nextBtn->setObjectName("nav");
    m_groupLabel = new QLabel;
    m_groupLabel->setObjectName("groupLabel");
    m_groupLabel->setAlignment(Qt::AlignCenter);
    header->addWidget(m_prevBtn);
    header->addWidget(m_groupLabel, 1);
    header->addWidget(m_nextBtn);
    layout->addLayout(header);

    m_progress = new QProgressBar;
    m_progress->setRange(0, GroupSize);
    m_progress->setTextVisible(false);
    m_progress->setFixedHeight(8);
    layout->addWidget(m_progress);

    // The card
    auto *card = new QWidget;
    card->setObjectName("card");
    card->setMinimumSize(460, 230);
    auto *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(30);
    shadow->setOffset(0, 6);
    shadow->setColor(QColor(0, 0, 0, 30));
    card->setGraphicsEffect(shadow);
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(24, 22, 24, 22);
    auto *hint = new QLabel("ITALIAN");
    hint->setObjectName("hint");
    hint->setAlignment(Qt::AlignCenter);
    m_cardLabel = new QLabel;
    m_cardLabel->setObjectName("word");
    m_cardLabel->setAlignment(Qt::AlignCenter);
    m_cardLabel->setWordWrap(true);
    cardLayout->addWidget(hint);
    cardLayout->addStretch();
    cardLayout->addWidget(m_cardLabel);
    cardLayout->addStretch();
    layout->addWidget(card, 1);

    m_feedback = new QLabel;
    m_feedback->setAlignment(Qt::AlignCenter);
    m_feedback->setWordWrap(true);
    m_feedback->setMinimumHeight(52);
    layout->addWidget(m_feedback);

    auto *row = new QHBoxLayout;
    row->setSpacing(10);
    m_input = new QLineEdit;
    m_input->setPlaceholderText("Type the English meaning…");
    m_checkBtn = new QPushButton("Check");
    m_checkBtn->setObjectName("primary");
    m_checkBtn->setDefault(true);
    m_checkBtn->setCursor(Qt::PointingHandCursor);
    row->addWidget(m_input, 1);
    row->addWidget(m_checkBtn);
    layout->addLayout(row);

    m_status = new QLabel;
    m_status->setObjectName("status");
    m_status->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_status);

    auto *row2 = new QHBoxLayout;
    auto *addBtn = new QPushButton("＋ Add card");
    auto *importBtn = new QPushButton("Import deck");
    auto *restartBtn = new QPushButton("↻ Restart group");
    for (auto *b : {addBtn, importBtn, restartBtn}) {
        b->setObjectName("flat");
        b->setCursor(Qt::PointingHandCursor);
    }
    row2->addStretch();
    row2->addWidget(addBtn);
    row2->addWidget(importBtn);
    row2->addWidget(restartBtn);
    row2->addStretch();
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
    resize(560, 640);

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
    setFeedback(QString(), "empty");
    showCard();
}

void MainWindow::setFeedback(const QString &text, const QString &kind) {
    QString style = "font-size: 16px; border-radius: 12px; padding: 10px 14px;";
    if (kind == "ok")        style += "background: #e6f4ea; color: #1e7e34;";
    else if (kind == "bad")  style += "background: #fdecea; color: #c62828;";
    else if (kind == "info") style += "background: #e8f0fe; color: #1a56c4;";
    else                     style += "background: transparent;";
    m_feedback->setStyleSheet(style);
    m_feedback->setText(text);
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
        m_cardLabel->setText("🎉");
        setFeedback(m_group < groupCount() - 1
                        ? "You know all 10 words. Press Enter for the next group."
                        : "You finished the last group!", "info");
    }
    m_groupLabel->setText(QString("Group %1 of %2").arg(m_group + 1).arg(groupCount()));
    m_progress->setValue(m_known);
    m_status->setText(QString("Known %1  ·  Left %2").arg(m_known).arg(m_deck.remaining()));
}

void MainWindow::check() {
    if (!m_deck.hasCurrent())
        return;
    const QString answer = m_deck.current().back;
    const QString word = m_deck.current().front;

    if (isCorrect(m_input->text(), m_deck.current())) {
        m_deck.answer(true);     // into the "known" pile
        ++m_known;
        setFeedback("✓  Correct!   " + word + " = " + answer, "ok");
        showCard();
    } else {
        // Show the right answer; Enter moves on and the card comes back later.
        m_showingAnswer = true;
        setFeedback("✗  " + word + " = " + answer + "   ·   press Enter to continue", "bad");
        m_input->setReadOnly(true);
        m_checkBtn->setText("Next");
    }
}

void MainWindow::next() {
    m_deck.answer(false);        // back of the queue, you'll see it again
    m_showingAnswer = false;
    m_input->setReadOnly(false);
    setFeedback(QString(), "empty");
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
