#ifndef TWEETREPOSITORY_H
#define TWEETREPOSITORY_H

#include "tweetdata.h"
#include <QVector>
#include <QString>
#include <QObject>
#include <QSet> // For getAllTweetIds

class TweetRepository : public QObject
{
    Q_OBJECT
public:
    explicit TweetRepository(QObject *parent = nullptr);

    bool loadTweets(const QString& resourcePath = ":/data/SCTweets.json");
    const QVector<TweetData>& getAllTweets() const;
    const TweetData* findTweetById(const QString& id) const;
    QSet<QString> getAllTweetIds() const; // For uniqueness checks

    // For populating filters
    QSet<QString> getAllUniqueAuthors() const;
    QSet<QString> getAllUniqueSonicTags() const;
    QSet<QString> getAllUniqueTechniqueTags() const;
    QSet<QString> getAllUniqueUgens() const;
    QString getCurrentResourcePath() const;

    // --- NEW METHODS FOR CRUD ---
    bool addTweet(const TweetData& newTweet);
    bool updateTweet(const TweetData& updatedTweet); // Assumes ID in updatedTweet matches existing
    bool deleteTweet(const QString& tweetId);
    bool saveTweetsToResource(const QString& resourcePath = ":/data/SCTweets.json"); // Or to a user file path

signals:
    void loadError(const QString& title, const QString& message);
    void tweetsLoaded(int count);
    void tweetsModified(); // *** NEW SIGNAL *** emitted after add, update, delete, save

private:
    void extractUgens(TweetData& tweetData);
    bool saveTweetsInternal(const QString& filePath); // Helper for saving

    QVector<TweetData> m_tweets;
    QString m_currentResourcePath; // Store the path used for loading/saving
    friend class MainWindow;
};

#endif // TWEETREPOSITORY_H