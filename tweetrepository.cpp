#include "tweetrepository.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QDebug>
#include <QSet>

TweetRepository::TweetRepository(QObject *parent) : QObject(parent)
{
}

bool TweetRepository::loadTweets(const QString& resourcePath)
{
    QFile jsonFile(resourcePath);
    qInfo() << "Attempting to load tweets from resource:" << resourcePath;

    if (!jsonFile.exists() || !jsonFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open or find resource" << resourcePath;
        emit loadError("Load Error", "Could not open application resource:\n" + resourcePath);
        return false;
    }

    QByteArray jsonData = jsonFile.readAll();
    jsonFile.close();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse JSON:" << parseError.errorString();
        emit loadError("JSON Error", "Failed to parse SCTweets.json:\n" + parseError.errorString());
        return false;
    }
    if (!doc.isObject()) {
        qWarning() << "JSON root is not an object.";
        emit loadError("JSON Error", "SCTweets.json root is not a valid JSON object.");
        return false;
    }

    QJsonObject rootObj = doc.object();
    m_tweets.clear();

    for (auto it = rootObj.constBegin(); it != rootObj.constEnd(); ++it) {
        QString key = it.key();
        QJsonValue value = it.value();
        if (!value.isObject()) { continue; }
        QJsonObject tweetObj = value.toObject();
        if (!tweetObj.contains("original") || !tweetObj["original"].isString()) { continue; }

        TweetData td;
        td.id = key;
        td.originalCode = tweetObj["original"].toString();
        td.author = tweetObj.value("author").toString("Unknown");
        td.sourceUrl = tweetObj.value("source_url").toString("");
        td.description = tweetObj.value("description").toString("-");
        td.publicationDate = tweetObj.value("publication_date").toString("unknown");

        if (tweetObj.contains("classification") && tweetObj["classification"].isObject()) {
            QJsonObject classificationObj = tweetObj["classification"].toObject();
            if (classificationObj.contains("sonic_characteristics") && classificationObj["sonic_characteristics"].isArray()) {
                QJsonArray sonicArray = classificationObj["sonic_characteristics"].toArray();
                for (const QJsonValue &tagVal : sonicArray) {
                    if (tagVal.isString()) td.sonicTags.append(tagVal.toString());
                }
            }
            if (classificationObj.contains("synthesis_techniques") && classificationObj["synthesis_techniques"].isArray()) {
                QJsonArray techArray = classificationObj["synthesis_techniques"].toArray();
                for (const QJsonValue &tagVal : techArray) {
                    if (tagVal.isString()) td.techniqueTags.append(tagVal.toString());
                }
            }
        }

        if (tweetObj.contains("tags") && tweetObj["tags"].isArray()) {
            QJsonArray tagsArray = tweetObj["tags"].toArray();
            for (const QJsonValue &tagVal : tagsArray) { if (tagVal.isString()) td.genericTags.append(tagVal.toString()); }
        }
        
        extractUgens(td); // Extract UGens for this tweet
        m_tweets.append(td);
    }
    qInfo() << "Loaded" << m_tweets.count() << "tweets from JSON resource.";
    emit tweetsLoaded(m_tweets.count());
    return true;
}

void TweetRepository::extractUgens(TweetData& tweetData) {
    // Regexes for UGen Extraction
    QRegularExpression ugenMethodRegex(R"(\b([A-Z][a-zA-Z0-9]*)(?:\.(?:ar|kr|ir|new)\b|\())");
    QRegularExpression ugenFuncRegex(R"(\b(?:ar|kr|ir|new)\b\s*\(\s*([A-Z][a-zA-Z0-9]*)\b)");
    
    QSet<QString> ugensSet; // Use QSet to avoid duplicates per tweet
    QRegularExpressionMatchIterator i1 = ugenMethodRegex.globalMatch(tweetData.originalCode);
    while (i1.hasNext()) {
        ugensSet.insert(i1.next().captured(1));
    }
    QRegularExpressionMatchIterator i2 = ugenFuncRegex.globalMatch(tweetData.originalCode);
    while (i2.hasNext()) {
        ugensSet.insert(i2.next().captured(1));
    }
    tweetData.ugens = ugensSet.values(); // Convert QSet to QStringList
    tweetData.ugens.sort(Qt::CaseInsensitive);
}


const QVector<TweetData>& TweetRepository::getAllTweets() const {
    return m_tweets;
}

const TweetData* TweetRepository::findTweetById(const QString& id) const {
    for (const auto& tweet : m_tweets) {
        if (tweet.id == id) {
            return &tweet;
        }
    }
    return nullptr;
}

QSet<QString> TweetRepository::getAllUniqueAuthors() const {
    QSet<QString> authors;
    for (const auto& tweet : m_tweets) {
        authors.insert(tweet.author.isEmpty() ? "Unknown" : tweet.author);
    }
    return authors;
}

QSet<QString> TweetRepository::getAllUniqueSonicTags() const {
    QSet<QString> sonics;
    for (const auto& tweet : m_tweets) {
        for(const QString& tag : tweet.sonicTags) { sonics.insert(tag); }
    }
    return sonics;
}

QSet<QString> TweetRepository::getAllUniqueTechniqueTags() const {
    QSet<QString> techniques;
    for (const auto& tweet : m_tweets) {
        for(const QString& tag : tweet.techniqueTags) { techniques.insert(tag); }
    }
    return techniques;
}

QSet<QString> TweetRepository::getAllUniqueUgens() const {
    QSet<QString> ugens;
    for (const auto& tweet : m_tweets) {
        for(const QString& ugen : tweet.ugens) { ugens.insert(ugen); }
    }
    return ugens;
}