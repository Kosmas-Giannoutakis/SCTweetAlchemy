#ifndef TWEETREPOSITORY_H
#define TWEETREPOSITORY_H

#include "tweetdata.h"
#include <QVector>
#include <QString>
#include <QObject> // For Q_OBJECT if signals/slots needed later

class TweetRepository : public QObject
{
    Q_OBJECT
public:
    explicit TweetRepository(QObject *parent = nullptr);

    bool loadTweets(const QString& resourcePath = ":/data/SCTweets.json");
    const QVector<TweetData>& getAllTweets() const;
    const TweetData* findTweetById(const QString& id) const;

    // For populating filters
    QSet<QString> getAllUniqueAuthors() const;
    QSet<QString> getAllUniqueSonicTags() const;
    QSet<QString> getAllUniqueTechniqueTags() const;
    QSet<QString> getAllUniqueUgens() const;

signals:
    void loadError(const QString& title, const QString& message);
    void tweetsLoaded(int count);

private:
    void extractUgens(TweetData& tweetData);

    QVector<TweetData> m_tweets;
};

#endif // TWEETREPOSITORY_H