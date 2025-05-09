#include "tweetrepository.h"
#include <QFile>            // For QFile
#include <QJsonDocument>    // For QJsonDocument
#include <QJsonObject>      // For QJsonObject
#include <QJsonArray>       // For QJsonArray
#include <QRegularExpression> 
#include <QDebug>           // For qInfo, qWarning, qCritical
#include <QSet>             
#include <QStandardPaths>   // For QStandardPaths
#include <QDir>             // For QDir
#include <QIODevice>        // For QIODevice::WriteOnly etc.

TweetRepository::TweetRepository(QObject *parent) 
    : QObject(parent), m_currentResourcePath(":/data/SCTweets.json") // Default load path
{
}

bool TweetRepository::loadTweets(const QString& filePathToLoad)
{
    // If filePathToLoad is empty, use the stored m_currentResourcePath (which defaults to resource)
    QString actualPath = filePathToLoad.isEmpty() ? m_currentResourcePath : filePathToLoad;
    
    // Update m_currentResourcePath only if loading from a new, non-resource path,
    // or if it was the initial load from resource.
    // This helps saveTweetsToResource know where to save if it's a user file.
    if (!actualPath.startsWith(":/") || m_currentResourcePath.startsWith(":/")) {
        m_currentResourcePath = actualPath;
    }

    QFile jsonFile(actualPath);
    qInfo() << "TweetRepository: Attempting to load tweets from:" << actualPath;

    if (!jsonFile.exists()) { // Check existence first for clearer error if it's a user file
        qWarning() << "TweetRepository: File does not exist -" << actualPath;
        if (!actualPath.startsWith(":/")) { // Only emit error for non-resource paths, resource path failure handled below
             emit loadError("Load Error", "Tweet file not found:\n" + actualPath);
        }
        return false;
    }

    if (!jsonFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "TweetRepository: Failed to open" << actualPath << ":" << jsonFile.errorString();
        emit loadError("Load Error", "Could not open tweet file:\n" + actualPath);
        return false;
    }

    QByteArray jsonData = jsonFile.readAll();
    jsonFile.close();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "TweetRepository: Failed to parse JSON from" << actualPath << ":" << parseError.errorString();
        emit loadError("JSON Error", "Failed to parse tweet file:\n" + actualPath + "\n" + parseError.errorString());
        return false;
    }
    if (!doc.isObject()) {
        qWarning() << "TweetRepository: JSON root is not an object in" << actualPath;
        emit loadError("JSON Error", "Tweet file root is not a valid JSON object:\n" + actualPath);
        return false;
    }

    QJsonObject rootObj = doc.object();
    m_tweets.clear(); // Clear existing tweets before loading new set

    for (auto it = rootObj.constBegin(); it != rootObj.constEnd(); ++it) {
        QString key = it.key();
        QJsonValue value = it.value();
        if (!value.isObject()) { 
            qWarning() << "TweetRepository: Item with key" << key << "is not an object. Skipping.";
            continue; 
        }
        QJsonObject tweetObj = value.toObject();
        if (!tweetObj.contains("original") || !tweetObj["original"].isString()) { 
            qWarning() << "TweetRepository: Item with key" << key << "is missing 'original' code. Skipping.";
            continue; 
        }

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
        
        extractUgens(td);
        m_tweets.append(td);
    }
    qInfo() << "TweetRepository: Loaded" << m_tweets.count() << "tweets from" << actualPath;
    emit tweetsLoaded(m_tweets.count());
    return true;
}

void TweetRepository::extractUgens(TweetData& tweetData) {
    QRegularExpression ugenMethodRegex(R"(\b([A-Z][a-zA-Z0-9]*)(?:\.(?:ar|kr|ir|new)\b|\())");
    QRegularExpression ugenFuncRegex(R"(\b(?:ar|kr|ir|new)\b\s*\(\s*([A-Z][a-zA-Z0-9]*)\b)");
    
    QSet<QString> ugensSet;
    QRegularExpressionMatchIterator i1 = ugenMethodRegex.globalMatch(tweetData.originalCode);
    while (i1.hasNext()) {
        ugensSet.insert(i1.next().captured(1));
    }
    QRegularExpressionMatchIterator i2 = ugenFuncRegex.globalMatch(tweetData.originalCode);
    while (i2.hasNext()) {
        ugensSet.insert(i2.next().captured(1));
    }
    tweetData.ugens = ugensSet.values();
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

QSet<QString> TweetRepository::getAllTweetIds() const
{
    QSet<QString> ids;
    for (const auto& tweet : m_tweets) {
        ids.insert(tweet.id);
    }
    return ids;
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

QString TweetRepository::getCurrentResourcePath() const
{
    return m_currentResourcePath;
}

// --- CRUD METHOD IMPLEMENTATIONS ---
bool TweetRepository::addTweet(const TweetData& newTweetData)
{
    for (const auto& tweet : m_tweets) {
        if (tweet.id == newTweetData.id) {
            qWarning() << "TweetRepository: Attempted to add tweet with duplicate ID:" << newTweetData.id;
            return false; 
        }
    }
    TweetData tweetToAdd = newTweetData; 
    extractUgens(tweetToAdd); 

    m_tweets.append(tweetToAdd);
    qInfo() << "TweetRepository: Added tweet:" << tweetToAdd.id;
    emit tweetsModified();
    return true;
}

bool TweetRepository::updateTweet(const TweetData& updatedTweetData)
{
    for (int i = 0; i < m_tweets.size(); ++i) {
        if (m_tweets[i].id == updatedTweetData.id) {
            TweetData tweetToUpdate = updatedTweetData; 
            extractUgens(tweetToUpdate); 

            m_tweets[i] = tweetToUpdate;
            qInfo() << "TweetRepository: Updated tweet:" << updatedTweetData.id;
            emit tweetsModified();
            return true;
        }
    }
    qWarning() << "TweetRepository: Attempted to update non-existent tweet ID:" << updatedTweetData.id;
    return false;
}

bool TweetRepository::deleteTweet(const QString& tweetId)
{
    for (int i = 0; i < m_tweets.size(); ++i) {
        if (m_tweets[i].id == tweetId) {
            m_tweets.remove(i);
            qInfo() << "TweetRepository: Deleted tweet:" << tweetId;
            emit tweetsModified();
            return true;
        }
    }
    qWarning() << "TweetRepository: Attempted to delete non-existent tweet ID:" << tweetId;
    return false;
}

bool TweetRepository::saveTweetsInternal(const QString& filePath) {
    QJsonObject rootObj;
    for (const auto& tweet : m_tweets) {
        QJsonObject tweetObj;
        tweetObj["original"] = tweet.originalCode;
        tweetObj["author"] = tweet.author;
        tweetObj["source_url"] = tweet.sourceUrl;
        tweetObj["description"] = tweet.description;
        tweetObj["publication_date"] = tweet.publicationDate;

        QJsonObject classificationObj;
        if (!tweet.sonicTags.isEmpty()) {
            classificationObj["sonic_characteristics"] = QJsonArray::fromStringList(tweet.sonicTags);
        }
        if (!tweet.techniqueTags.isEmpty()) {
            classificationObj["synthesis_techniques"] = QJsonArray::fromStringList(tweet.techniqueTags);
        }
        if (!classificationObj.isEmpty()){
             tweetObj["classification"] = classificationObj;
        }

        if (!tweet.genericTags.isEmpty()) {
            tweetObj["tags"] = QJsonArray::fromStringList(tweet.genericTags);
        }
        rootObj[tweet.id] = tweetObj;
    }

    QJsonDocument doc(rootObj);
    QFile jsonFile(filePath);

    if (filePath.startsWith(":/")) {
        qCritical() << "TweetRepository: CRITICAL - Cannot save tweets to a Qt Resource Path like" << filePath;
        emit loadError("Save Error", "Cannot save to read-only resource path.\nDeveloper: Fix save path logic.");
        return false;
    }

    if (!jsonFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "TweetRepository: Failed to open file for writing:" << filePath << jsonFile.errorString();
        emit loadError("Save Error", "Could not open file for saving tweets:\n" + filePath);
        return false;
    }

    jsonFile.write(doc.toJson(QJsonDocument::Indented));
    jsonFile.close();
    qInfo() << "TweetRepository: Successfully saved" << m_tweets.count() << "tweets to" << filePath;
    // emit tweetsModified(); // Not strictly necessary after save unless order changed or file system interaction is key
    return true;
}

bool TweetRepository::saveTweetsToResource(const QString& filePathToSaveTo) {
    QString savePath = filePathToSaveTo;

    if (savePath.isEmpty()) { // If no path provided, use the current one
        savePath = m_currentResourcePath;
    }

    // If current path is still a resource path or empty, determine a user-writable default
    if (savePath.isEmpty() || savePath.startsWith(":/")) {
        QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if (appDataPath.isEmpty()) { // Fallback if AppDataLocation is somehow empty
            appDataPath = QDir::homePath() + "/.SCTweetAlchemy"; // A common fallback pattern
            QDir fallbackDir(appDataPath);
            if (!fallbackDir.exists()) {
                fallbackDir.mkpath(".");
            }
        } else { // Ensure AppDataLocation path exists
             QDir appDataDir(appDataPath);
             if (!appDataDir.exists()) {
                appDataDir.mkpath(".");
             }
        }
        savePath = appDataPath + "/SCTweets_user.json";
        qInfo() << "TweetRepository: Original path was resource/empty. Resolved save path to user-writable:" << savePath;
        m_currentResourcePath = savePath; // IMPORTANT: Update current path to the user file
    }
    
    return saveTweetsInternal(savePath);
}