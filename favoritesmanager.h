#ifndef FAVORITESMANAGER_H
#define FAVORITESMANAGER_H

#include <QObject>
#include <QSet>
#include <QString>

class QSettings;

class FavoritesManager : public QObject
{
    Q_OBJECT
public:
    explicit FavoritesManager(QSettings* settings, QObject *parent = nullptr);

    void loadFavorites();
    void saveFavorites();

    bool isFavorite(const QString& tweetId) const;
    void addFavorite(const QString& tweetId);
    void removeFavorite(const QString& tweetId);
    const QSet<QString>& getFavoriteTweetIds() const;

signals:
    void favoritesChanged(); // Emitted when a favorite is added/removed

private:
    QSettings* m_settings;
    QSet<QString> m_favoriteTweetIds;
};

#endif // FAVORITESMANAGER_H