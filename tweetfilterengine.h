#ifndef TWEETFILTERENGINE_H
#define TWEETFILTERENGINE_H

#include "tweetdata.h" // For TweetData
#include <QVector>
#include <QStringList>
#include <QSet> // For passing favorite IDs
#include <QRegularExpression>


struct FilterCriteria {
    QString searchText;
    bool favoritesOnly;
    const QSet<QString>* favoriteTweetIds; // Pointer to the set from FavoritesManager
    bool useAndLogic;
    QStringList checkedAuthors;
    QStringList checkedSonicTags;
    QStringList checkedTechniqueTags;
    QStringList checkedUgens;
};

class TweetFilterEngine
{
public:
    TweetFilterEngine();

    QVector<const TweetData*> filterTweets(
        const QVector<TweetData>& allTweets,
        const FilterCriteria& criteria
    ) const;
};

#endif // TWEETFILTERENGINE_H