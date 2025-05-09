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
class QMenu;       // For menu bar
class QMenuBar;    // For menu bar
QT_END_NAMESPACE

class SearchLineEdit;
class TweetRepository;
class FavoritesManager;
class FilterPanelWidget;
class TweetFilterEngine;
class TweetEditDialog; // Forward declare our new dialog

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Existing slots
    void onTweetSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void onSearchTextChanged(const QString &text);
    void onSearchNavigateKey(QKeyEvent *event);
    void focusSearchField();
    void toggleCurrentTweetFavorite();
    void onTweetItemDoubleClicked(QListWidgetItem *item);
    void onTweetListContextMenuRequested(const QPoint &pos);
    void applyAllFilters();

    // Slots for manager signals
    void handleRepositoryLoadError(const QString& title, const QString& message);
    void handleTweetsLoaded(int count);
    void handleFavoritesChanged();
    void handleTweetsModified(); // *** NEW SLOT for repository modifications ***

    // --- Slots for Menu Actions ---
    void onFileNewTweet();
    void onFileSaveAllChanges();
    // void onFileExit(); // QApplication::quit can be connected directly

    void onEditTweet();
    void onEditDeleteTweet();
    void onEditCopyCode();
    // void onEditToggleFavorite(); // Already covered by toggleCurrentTweetFavorite

    // void onToolsCopyAsNdef(); // For later

    void onHelpAbout();


private:
    void setupUi();
    void setupMenuBar(); // *** NEW METHOD for menu bar ***
    void setupModelsAndManagers();
    void setupActions(); // Will now primarily set up non-menu QActions if any remain
    void connectSignals();
    void updateActionStates(); // *** NEW METHOD to enable/disable actions ***

    void displayTweetDetails(const TweetData* tweet);
    void populateTweetList(const QVector<const TweetData*>& tweetsToDisplay);
    void updateFavoriteIcon(QListWidgetItem* item, const QString& tweetId);
    QWidget* createRightPanel();
    void toggleFavoriteStatus(const QString& tweetId);


    // --- UI Members ---
    QMenuBar *m_menuBar; // Menu bar itself
    QMenu *m_fileMenu;
    QMenu *m_editMenu;
    // QMenu *m_toolsMenu; // For later
    QMenu *m_helpMenu;

    // Actions for menu items
    QAction *m_newTweetAction;
    QAction *m_saveAllAction;
    QAction *m_exitAction;

    QAction *m_editTweetAction;
    QAction *m_deleteTweetAction;
    QAction *m_copyCodeAction;
    QAction *m_toggleFavoriteAction;
    QAction *m_focusSearchAction;
    // m_toggleFavoriteAction is already declared for Ctrl+D, will add to menu
    // m_focusSearchAction is already declared for Ctrl+F

    QAction *m_aboutAction;


    SearchLineEdit *m_searchLineEdit;
    QSplitter *m_mainSplitter;
    FilterPanelWidget *m_filterPanelWidget;
    QListWidget *m_tweetListWidget;
    QWidget *m_rightPanel;
    QTextEdit *m_codeTextEdit;
    QTextEdit *m_metadataTextEdit;

    // QAction *m_focusSearchAction; // Already member
    // QAction *m_toggleFavoriteAction; // Already member

    QSettings *m_settings;
    TweetRepository *m_tweetRepository;
    FavoritesManager *m_favoritesManager;
    TweetFilterEngine *m_tweetFilterEngine;
    // No need for CodeFormatter yet for this phase

    QVector<const TweetData*> m_currentlyDisplayedTweets;
};

#endif // MAINWINDOW_H