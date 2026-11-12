#pragma once
#include "deck.h"
#include <QMainWindow>

class QLabel;
class QLineEdit;
class QPushButton;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void check();          // compare typed guess with the answer
    void next();           // move on after a wrong answer
    void addCard();
    void restart();          // start the current group again
    void prevGroup();
    void nextGroup();
    void importDeck();

private:
    void showCard();
    void startGroup(int g);
    int groupCount() const;
    static bool isCorrect(const QString &guess, const Card &card);

    Deck m_deck;
    QString m_path;
    bool m_showingAnswer = false;   // true while a wrong answer is displayed
    int m_known = 0;                // cards answered right in this group
    int m_group = 0;                // current group (0-based)
    static constexpr int GroupSize = 10;

    QLabel *m_cardLabel;
    QLabel *m_feedback;
    QLabel *m_status;
    QLineEdit *m_input;
    QPushButton *m_checkBtn;
    QPushButton *m_prevBtn;
    QPushButton *m_nextBtn;
};
