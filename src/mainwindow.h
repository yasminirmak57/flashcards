#pragma once
#include "deck.h"
#include <QMainWindow>

class QLabel;
class QPushButton;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void flip();
    void gotIt();
    void missed();
    void addCard();
    void restart();
    void importDeck();

private:
    void showCard();

    Deck m_deck;
    QString m_path;
    bool m_showingBack = false;

    QLabel *m_cardLabel;
    QLabel *m_status;
    QPushButton *m_flipBtn;
    QPushButton *m_gotBtn;
    QPushButton *m_missBtn;
};
