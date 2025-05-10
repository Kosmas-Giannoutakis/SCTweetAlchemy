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
class QMenu;       
class QMenuBar;    
QT_END_NAMESPACE

class SearchLineEdit;
class TweetRepository;
class FavoritesManager;
class FilterPanelWidget;
class TweetFilterEngine;
class TweetEditDialog; 
class NdefGenerator;   // Forward declare NdefGenerator

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // ... (all your existing slots like onTweetSelectionChanged, handleTweetsModified, onFileNewTweet, etc.)
    void onTweetSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void onSearchTextChanged(const QString &text);
    void onSearchNavigateKey(QKeyEvent *event);
    void focusSearchField();
    void toggleCurrentTweetFavorite();
    void onTweetItemDoubleClicked(QListWidgetItem *item);
    void applyAllFilters(); 
    void onTweetListContextMenuRequested(const QPoint &pos);


    void handleRepositoryLoadError(const QString& title, const QString& message);
    void handleTweetsLoaded(int count);
    void handleFavoritesChanged();
    void handleTweetsModified(); 

    void onFileNewTweet();
    void onFileSaveAllChanges();

    void onEditTweet();
    void onEditDeleteTweet();
    void onEditCopyCode();

    void onHelpAbout();


private:
    void setupUi();
    void setupMenuBar(); 
    void setupModelsAndManagers();
    void setupActions(); 
    void connectSignals();
    void updateActionStates(); 

    void displayTweetDetails(const TweetData* tweet);
    void populateTweetList(const QVector<const TweetData*>& tweetsToDisplay);
    void updateFavoriteIcon(QListWidgetItem* item, const QString& tweetId);
    QWidget* createRightPanel(); 
    void toggleFavoriteStatus(const QString& tweetId);

    // --- NEW METHOD DECLARATIONS for Ndef Panel ---
    QWidget* createNdefPanel();                     // <<< ADD THIS
    void displayNdefCode(const TweetData* tweet);   // <<< ADD THIS


    // --- UI Members ---
    QMenuBar *m_menuBar; 
    QMenu *m_fileMenu;
    QMenu *m_editMenu;
    QMenu *m_helpMenu;

    QAction *m_newTweetAction;
    QAction *m_saveAllAction;
    QAction *m_exitAction;
    QAction *m_editTweetAction;
    QAction *m_deleteTweetAction;
    QAction *m_copyCodeAction;
    QAction *m_toggleFavoriteAction;  
    QAction *m_focusSearchAction;     
    QAction *m_aboutAction;

    SearchLineEdit *m_searchLineEdit;
    QSplitter *m_mainSplitter;
    FilterPanelWidget *m_filterPanelWidget;
    QListWidget *m_tweetListWidget;
    
    // --- Renamed and NEW UI Panel Members ---
    QWidget *m_codeAndMetadataPanel;   // <<< DECLARE THIS (if you renamed m_rightPanel)
    QTextEdit *m_codeTextEdit;
    QTextEdit *m_metadataTextEdit;

    QWidget *m_ndefDisplayPanel;       // <<< DECLARE THIS
    QTextEdit *m_ndefCodeTextEdit;     // <<< DECLARE THIS


    // --- Managers and Services ---
    QSettings *m_settings;
    TweetRepository *m_tweetRepository;
    FavoritesManager *m_favoritesManager;
    TweetFilterEngine *m_tweetFilterEngine;
    NdefGenerator *m_ndefGenerator;     // Member for NdefGenerator

    QVector<const TweetData*> m_currentlyDisplayedTweets;
};

#endif // MAINWINDOW_H