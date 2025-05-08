#include "favoritesmanager.h"
#include <QSettings>
#include <QStringList>
#include <QDebug>

FavoritesManager::FavoritesManager(QSettings* settings, QObject *parent)
    : QObject(parent), m_settings(settings)
{
    Q_ASSERT(m_settings != nullptr);
    loadFavorites();
}

void FavoritesManager::loadFavorites()
{
    if (!m_settings) return;
    QStringList favList = m_settings->value("favorites", QStringList()).toStringList();
    m_favoriteTweetIds = QSet<QString>(favList.begin(), favList.end());
    qInfo() << "Loaded" << m_favoriteTweetIds.count() << "favorites from settings.";
}

void FavoritesManager::saveFavorites()
{
    if (!m_settings) return;
    m_settings->setValue("favorites", QStringList(m_favoriteTweetIds.begin(), m_favoriteTweetIds.end()));
    qInfo() << "Saved" << m_favoriteTweetIds.count() << "favorites to settings.";
    m_settings->sync(); // Ensure data is written
}

bool FavoritesManager::isFavorite(const QString& tweetId) const
{
    return m_favoriteTweetIds.contains(tweetId);
}

void FavoritesManager::addFavorite(const QString& tweetId)
{
    if (!m_favoriteTweetIds.contains(tweetId)) {
        m_favoriteTweetIds.insert(tweetId);
        saveFavorites();
        emit favoritesChanged();
    }
}

void FavoritesManager::removeFavorite(const QString& tweetId)
{
    if (m_favoriteTweetIds.contains(tweetId)) {
        m_favoriteTweetIds.remove(tweetId);
        saveFavorites();
        emit favoritesChanged();
    }
}

const QSet<QString>& FavoritesManager::getFavoriteTweetIds() const
{
    return m_favoriteTweetIds;
}