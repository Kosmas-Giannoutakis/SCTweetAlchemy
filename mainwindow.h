#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>

// Qt Widget Includes needed for member declarations
#include <QTextEdit>
#include <QSplitter>
#include <QListWidget>
#include <QMenuBar>  // For QMenuBar and QMenu
#include <QMenu>
#include <QAction>
#include <QComboBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox> // Specifically for m_ndefFadeTimeSpinBox
#include <QLabel>

// Project-specific includes
#include "tweetdata.h"
#include "ndefgenerator.h" // For NdefFormattingOptions struct

// Forward declarations for classes defined in .cpp or other headers
// (if not fully defined by includes above)
QT_BEGIN_NAMESPACE
class QSettings;
class QListWidgetItem;
class QKeyEvent;
QT_END_NAMESPACE

class SearchLineEdit;
class TweetRepository;
class FavoritesManager;
class FilterPanelWidget;
class TweetFilterEngine;
class TweetEditDialog; 
// NdefGenerator is included above

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // UI Interaction Slots
    void onTweetSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void onSearchTextChanged(const QString &text);
    void onSearchNavigateKey(QKeyEvent *event);
    void focusSearchField();
    void toggleCurrentTweetFavorite();
    void onTweetItemDoubleClicked(QListWidgetItem *item);
    void onTweetListContextMenuRequested(const QPoint &pos);
    
    // Core Logic Slots
    void applyAllFilters(); 
    void onNdefFormattingOptionsChanged();

    // Slots for Manager/Repository Signals
    void handleRepositoryLoadError(const QString& title, const QString& message);
    void handleTweetsLoaded(int count);
    void handleFavoritesChanged();
    void handleTweetsModified(); 

    // Slots for Menu Actions
    void onFileNewTweet();
    void onFileSaveAllChanges();
    // onFileExit is connected directly to qApp->quit()

    void onEditTweet();
    void onEditDeleteTweet();
    void onEditCopyCode();
    // onEditToggleFavorite uses toggleCurrentTweetFavorite

    void onHelpAbout();


private:
    // Setup Methods
    void setupUi();
    void setupMenuBar(); 
    void setupModelsAndManagers();
    void setupActions(); 
    void connectSignals();
    void updateActionStates(); 

    // Helper Methods
    void displayTweetDetails(const TweetData* tweet);
    void populateTweetList(const QVector<const TweetData*>& tweetsToDisplay);
    void updateFavoriteIcon(QListWidgetItem* item, const QString& tweetId);
    QWidget* createRightPanel(); 
    QWidget* createNdefPanel();  
    void toggleFavoriteStatus(const QString& tweetId);
    void displayNdefCode(const TweetData* tweet); 
    void updateNdefEnhancementOptionsUI(); 


    // --- UI Members ---
    // Menu Bar
    QMenuBar *m_menuBar; 
    QMenu *m_fileMenu;
    QMenu *m_editMenu;
    QMenu *m_helpMenu;

    // Actions (some are global shortcuts, some primarily menu)
    QAction *m_newTweetAction;
    QAction *m_saveAllAction;
    QAction *m_exitAction;
    QAction *m_editTweetAction;
    QAction *m_deleteTweetAction;
    QAction *m_copyCodeAction;
    QAction *m_toggleFavoriteAction;  
    QAction *m_focusSearchAction;     
    QAction *m_aboutAction;

    // Main Layout Widgets
    SearchLineEdit *m_searchLineEdit;
    QSplitter *m_mainSplitter;
    FilterPanelWidget *m_filterPanelWidget;
    QListWidget *m_tweetListWidget;
    
    // Code and Metadata Panel
    QWidget *m_codeAndMetadataPanel; 
    QTextEdit *m_codeTextEdit;
    QTextEdit *m_metadataTextEdit;

    // Ndef Panel UI
    QWidget *m_ndefDisplayPanel;     
    QTextEdit *m_ndefCodeTextEdit;   
    QComboBox *m_ndefStyleComboBox;         
    QGroupBox *m_ndefEnhancementsGroup;    
    QCheckBox *m_ndefAddReshapingCheckBox;    
    QCheckBox *m_ndefAddSplayAzCheckBox;    
    QLabel    *m_ndefSplayChannelsLabel;    
    QSpinBox  *m_ndefSplayChannelsSpinBox;  
    QCheckBox *m_ndefSetFadeTimeCheckBox;     
    QLabel    *m_ndefFadeTimeLabel;           
    QDoubleSpinBox *m_ndefFadeTimeSpinBox;    


    // --- Managers and Services ---
    QSettings *m_settings;
    TweetRepository *m_tweetRepository;
    FavoritesManager *m_favoritesManager;
    TweetFilterEngine *m_tweetFilterEngine;
    NdefGenerator *m_ndefGenerator;     

    // --- State for Ndef Formatting Options ---
    NdefFormattingOptions m_currentNdefOptions; 

    QVector<const TweetData*> m_currentlyDisplayedTweets;
};

#endif // MAINWINDOW_H