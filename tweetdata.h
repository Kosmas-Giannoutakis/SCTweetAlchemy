#ifndef TWEETDATA_H
#define TWEETDATA_H

#include <QString>
#include <QStringList>

struct TweetData {
    QString id; // Unique identifier (original JSON key)
    QString originalCode;
    QString author;
    QString sourceUrl;
    QString description;
    QString publicationDate;
    QStringList sonicTags;
    QStringList techniqueTags;
    QStringList genericTags; // Original flat 'tags' array
    QStringList ugens;       // Extracted UGens from the code
};

#endif // TWEETDATA_H