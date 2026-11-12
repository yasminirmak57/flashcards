#pragma once
#include "card.h"
#include <QString>
#include <QVector>

// Holds all cards and the order they're studied in this session.
class Deck {
public:
    bool load(const QString &path);
    bool save(const QString &path) const;

    void addCard(const QString &front, const QString &back);
    void startSession();              // shuffle all cards into the queue
    bool hasCurrent() const;
    Card &current();
    void answer(bool gotIt);          // missed cards go to the back of the queue

    int total() const { return m_cards.size(); }
    int remaining() const { return m_queue.size(); }

private:
    QVector<Card> m_cards;
    QVector<int> m_queue;             // indices into m_cards
};
