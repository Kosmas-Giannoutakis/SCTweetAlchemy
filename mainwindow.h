#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QVector>
#include <QStringList>
#include <QKeySequence>
#include <QAction>
#include <QSet>
#include <QMap> // For storing buttons

#include "searchlineedit.h"

// Forward declarations
class QListWidget; // Back to QListWidget
class QTextEdit;
class QPushButton; // For filter buttons
class QGroupBox;   // For grouping filters
class QButtonGroup;// For managing exclusive filters
class QVBoxLayout;
class QHBoxLayout;
class QScrollArea; // To make filter area scrollable
class QWidget;     // Generic widget
class QSplitter;
class QSettings;
class QKeyEvent;   // Keep for search navigate key

// Struct remains the same
struct TweetData {
    QString name;
    QString originalCode;
    QString author;
    QString sourceUrl;
    QString description;
    QStringList tags;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onTweetSelectionChanged(); // Slot for ListWidget selection
    void onSearchTextChanged(const QString &searchText);
    void focusSearchField();
    void onSearchNavigateKey(QKeyEvent *event); // Keep for now
    void toggleFavorite();
    void applyFilters();        // Slot triggered by filter button changes or clear
    void clearAllFilters();     // Slot for the "Clear Filters" button

private:
    void setupUi();
    void setupActions();
    void loadTweets();
    void populateFilterUI();     // New function to create filter buttons
    void displayMetadata(const TweetData& tweet);
    void displayCode(const TweetData* tweet);

    // Favorites management
    void loadFavorites();
    void saveFavorites();
    bool isFavorite(const QString& tweetName) const;

    // Helper function to create filter group boxes
    QWidget* createFilterGroup(const QString& title, const QSet<QString>& items, QButtonGroup*& buttonGroup, QMap<QString, QPushButton*>& buttonMap);

    // UI elements
    SearchLineEdit *searchLineEdit;
    QListWidget    *tweetListWidget;   // Use QListWidget again
    QTextEdit      *codeTextEdit;
    QTextEdit      *metadataTextEdit;

    // --- Filter UI Elements ---
    QWidget     *filterWidget;       // Main container for filter groups
    QPushButton *clearFiltersButton;
    // Group boxes (optional references if needed later)
    // QGroupBox *authorGroupBox;
    // QGroupBox *ugenGroupBox;
    // ... etc ...
    // Button Groups (to manage exclusivity)
    QButtonGroup *authorButtonGroup;
    QButtonGroup *ugenButtonGroup;
    QButtonGroup *sonicButtonGroup;
    QButtonGroup *complexityButtonGroup;
    QButtonGroup *favoritesButtonGroup; // For optional favorite filter
    // Maps to easily find buttons later (optional)
    QMap<QString, QPushButton*> authorButtons;
    QMap<QString, QPushButton*> ugenButtons;
    QMap<QString, QPushButton*> sonicButtons;
    QMap<QString, QPushButton*> complexityButtons;
    QPushButton* favoriteFilterButton; // Single button for favorite filter


    // Layout helpers
    QWidget* createMainContentPanel(); // Panel with List + Code/Meta
    QWidget* createRightPanel();       // Panel with Code + Meta

    // Actions
    QAction *focusSearchAction;
    QAction *toggleFavoriteAction;

    // Data storage
    QVector<TweetData> tweets;
    QSet<QString>      favoriteTweetNames;
    QSettings         *settings;
};

#endif // MAINWINDOW_H