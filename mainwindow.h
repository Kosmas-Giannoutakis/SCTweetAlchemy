#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include "tweetdata.h"

QT_BEGIN_NAMESPACE
class QListWidget;
class QTextEdit;
class QSplitter;
class QAction;
class QSettings;
class QListWidgetItem;
class QKeyEvent;
QT_END_NAMESPACE

class SearchLineEdit;
class TweetRepository;
class FavoritesManager;
class FilterPanelWidget;
class TweetFilterEngine;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onTweetSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void onSearchTextChanged(const QString &text);
    void onSearchNavigateKey(QKeyEvent *event);
    void focusSearchField();
    void toggleCurrentTweetFavorite(); // Keeps existing Ctrl+D functionality
    void onTweetItemDoubleClicked(QListWidgetItem *item); // *** NEW SLOT ***

    void handleRepositoryLoadError(const QString& title, const QString& message);
    void handleTweetsLoaded(int count);
    void handleFavoritesChanged();
    void applyAllFilters();

private:
    void setupUi();
    void setupModelsAndManagers();
    void setupActions();
    void connectSignals();

    void displayTweetDetails(const TweetData* tweet);
    void populateTweetList(const QVector<const TweetData*>& tweetsToDisplay);
    void updateFavoriteIcon(QListWidgetItem* item, const QString& tweetId); // Will be modified
    QWidget* createRightPanel();
    void toggleFavoriteStatus(const QString& tweetId); // *** NEW HELPER METHOD ***


    SearchLineEdit *m_searchLineEdit;
    QSplitter *m_mainSplitter;
    FilterPanelWidget *m_filterPanelWidget;
    QListWidget *m_tweetListWidget;
    QWidget *m_rightPanel;
    QTextEdit *m_codeTextEdit;
    QTextEdit *m_metadataTextEdit;

    QAction *m_focusSearchAction;
    QAction *m_toggleFavoriteAction;

    QSettings *m_settings;
    TweetRepository *m_tweetRepository;
    FavoritesManager *m_favoritesManager;
    TweetFilterEngine *m_tweetFilterEngine;

    QVector<const TweetData*> m_currentlyDisplayedTweets;
};

#endif // MAINWINDOW_H