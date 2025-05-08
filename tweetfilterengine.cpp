#include "tweetfilterengine.h"
#include <QDebug>

TweetFilterEngine::TweetFilterEngine() {}

QVector<const TweetData*> TweetFilterEngine::filterTweets(
    const QVector<TweetData>& allTweets,
    const FilterCriteria& criteria) const
{
    QVector<const TweetData*> filteredResults;

    // Pre-compile Regexes for checked UGens
    QList<QRegularExpression> ugenMethodCheckRegexes;
    QList<QRegularExpression> ugenFuncCheckRegexes;
    if (!criteria.checkedUgens.isEmpty()) {
        for(const QString& ugen : criteria.checkedUgens) {
            QString escapedUgen = QRegularExpression::escape(ugen);
            ugenMethodCheckRegexes.append(QRegularExpression(QString(R"(\b%1\b(?:\.(?:ar|kr|ir|new)\b|\())").arg(escapedUgen)));
            ugenFuncCheckRegexes.append(QRegularExpression(QString(R"(\b(?:ar|kr|ir|new)\b\s*\(\s*%1\b)").arg(escapedUgen)));
        }
    }

    qDebug() << "Filtering with criteria - Search:" << criteria.searchText
             << "FavsOnly:" << criteria.favoritesOnly
             << "Logic:" << (criteria.useAndLogic ? "AND" : "OR")
             << "Authors:" << criteria.checkedAuthors
             << "Sonics:" << criteria.checkedSonicTags
             << "Techniques:" << criteria.checkedTechniqueTags
             << "Ugens:" << criteria.checkedUgens;


    for (const auto& tweet : allTweets) {
        bool passesFilter = true;

        // 1. Global Search (Tweet ID/Name)
        if (!criteria.searchText.isEmpty() && !tweet.id.contains(criteria.searchText, Qt::CaseInsensitive)) {
            passesFilter = false;
        }

        // 2. Favorite Filter
        if (passesFilter && criteria.favoritesOnly) {
            if (!criteria.favoriteTweetIds || !criteria.favoriteTweetIds->contains(tweet.id)) {
                passesFilter = false;
            }
        }

        // Proceed to tag filters only if passed global filters
        if (passesFilter) {
            bool anyTagCheckboxesSelected = !criteria.checkedAuthors.isEmpty() ||
                                          !criteria.checkedSonicTags.isEmpty() ||
                                          !criteria.checkedTechniqueTags.isEmpty() ||
                                          !criteria.checkedUgens.isEmpty();

            if (anyTagCheckboxesSelected) {
                if (criteria.useAndLogic) { // AND Logic
                    // Check Authors
                    if (!criteria.checkedAuthors.isEmpty()) {
                        bool authorMatch = false; // Must match one of the checked authors if AND logic is per category
                                                  // If "Match All" means all *categories* must have a match, this is different.
                                                  // Assuming "Match All" means *all selected items within a category, and across categories*.
                        for (const QString& reqAuthor : criteria.checkedAuthors) {
                             if ((reqAuthor == "Unknown" && tweet.author.isEmpty()) || (tweet.author == reqAuthor)) {
                                 authorMatch = true; // This tweet matches at least one selected author.
                                 break;             // For AND logic on this category, this is enough.
                             }
                        }
                        // If "Match All" means tweet must match ALL selected authors:
                        // for (const QString& reqAuthor : criteria.checkedAuthors) {
                        //     if (!((reqAuthor == "Unknown" && tweet.author.isEmpty()) || (tweet.author == reqAuthor))) {
                        //         passesFilter = false; break;
                        //     }
                        // }
                        // Sticking to: tweet must contain ALL selected authors.
                        for (const QString& reqAuthor : criteria.checkedAuthors) {
                            bool currentAuthorMatch = (reqAuthor == "Unknown") ? tweet.author.isEmpty() : (tweet.author == reqAuthor);
                            // This interpretation of AND logic means if 'Author A' AND 'Author B' are checked,
                            // the tweet's author must be 'Author A' AND 'Author B', which is impossible for a single field.
                            // More likely interpretation for "Match All" (AND):
                            // Tweet must match (Author criteria) AND (Sonic criteria) AND (Technique criteria) AND (UGen criteria)
                            // Where (Author criteria) means: if authors are checked, tweet's author must be one of them.
                            // Let's assume the current UI means: if checkedAuthors is ["A", "B"], a tweet by "A" passes, a tweet by "B" passes.
                            // The "Match All" toggle then means it must pass the Author block AND the Sonic block, etc.

                            // Simpler: If authors are selected, the tweet's author MUST be in the selected list.
                            if (!criteria.checkedAuthors.contains(tweet.author) && !(tweet.author.isEmpty() && criteria.checkedAuthors.contains("Unknown"))) {
                                passesFilter = false; // If author not in list, AND logic fails here for author block.
                            }
                        }
                        if (!passesFilter) goto nextTweet; // Optimization
                    }


                    // Check Sonics
                    if (!criteria.checkedSonicTags.isEmpty()) {
                        for (const QString& reqSonic : criteria.checkedSonicTags) {
                            if (!tweet.sonicTags.contains(reqSonic, Qt::CaseInsensitive)) {
                                passesFilter = false; break;
                            }
                        }
                        if (!passesFilter) goto nextTweet;
                    }

                    // Check Techniques
                    if (!criteria.checkedTechniqueTags.isEmpty()) {
                        for (const QString& reqTechnique : criteria.checkedTechniqueTags) {
                            if (!tweet.techniqueTags.contains(reqTechnique, Qt::CaseInsensitive)) {
                                passesFilter = false; break;
                            }
                        }
                        if (!passesFilter) goto nextTweet;
                    }
                    
                    // Check UGens (Tweet must contain ALL checked UGens)
                    if (!criteria.checkedUgens.isEmpty()) {
                        for (int i = 0; i < criteria.checkedUgens.size(); ++i) {
                           if (i >= ugenMethodCheckRegexes.size() || i >= ugenFuncCheckRegexes.size()) {
                               qWarning() << "Regex list size mismatch for UGens!"; passesFilter = false; break;
                           }
                           bool ugenFoundInTweet = ugenMethodCheckRegexes[i].match(tweet.originalCode).hasMatch() ||
                                                 ugenFuncCheckRegexes[i].match(tweet.originalCode).hasMatch();
                           if (!ugenFoundInTweet) {
                               passesFilter = false; break;
                           }
                       }
                       if (!passesFilter) goto nextTweet;
                    }

                } else { // OR Logic (Tweet must match ANY checked item in ANY active category)
                    bool orMatchFound = false;
                    if (!criteria.checkedAuthors.isEmpty()) {
                        if (criteria.checkedAuthors.contains(tweet.author) || (tweet.author.isEmpty() && criteria.checkedAuthors.contains("Unknown"))) {
                            orMatchFound = true;
                        }
                    }
                    if (!orMatchFound && !criteria.checkedSonicTags.isEmpty()) {
                        for (const QString& reqSonic : criteria.checkedSonicTags) {
                            if (tweet.sonicTags.contains(reqSonic, Qt::CaseInsensitive)) {
                                orMatchFound = true; break;
                            }
                        }
                    }
                    if (!orMatchFound && !criteria.checkedTechniqueTags.isEmpty()) {
                        for (const QString& reqTechnique : criteria.checkedTechniqueTags) {
                            if (tweet.techniqueTags.contains(reqTechnique, Qt::CaseInsensitive)) {
                                orMatchFound = true; break;
                            }
                        }
                    }
                    if (!orMatchFound && !criteria.checkedUgens.isEmpty()) {
                        for (int i = 0; i < criteria.checkedUgens.size(); ++i) {
                            if (i >= ugenMethodCheckRegexes.size() || i >= ugenFuncCheckRegexes.size()) { qWarning() << "Regex list size mismatch for UGens!"; break; }
                            if (ugenMethodCheckRegexes[i].match(tweet.originalCode).hasMatch() || ugenFuncCheckRegexes[i].match(tweet.originalCode).hasMatch()) {
                                orMatchFound = true; break;
                            }
                        }
                    }
                    if (!orMatchFound) passesFilter = false;
                } // End OR Logic
            } // End if (anyTagCheckboxesSelected)
        } // End if (passes global filters)

      nextTweet:;
        if (passesFilter) {
            filteredResults.append(&tweet);
        }
    }
    return filteredResults;
}