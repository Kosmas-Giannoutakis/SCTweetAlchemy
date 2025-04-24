#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QVector>
#include <QStringList>
#include <QKeySequence>
#include <QAction>
#include <QSet>
#include <QList> // Include QList

#include "searchlineedit.h" // Include custom search line edit

// Forward declarations
class QListWidget;
class QTextEdit;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QWidget;
class QSplitter;
class QSettings;
class QKeyEvent;
class QLabel;
class QScrollArea;
class QGroupBox;
class QCheckBox;

// Data structure for holding tweet information
struct TweetData {
    QString name;
    QString originalCode;
    QString author;
    QString sourceUrl;
    QString description;
    QStringList tags; // Keep original flat tags for now, classification happens in C++
};

// Main application window class
class MainWindow : public QMainWindow
{
    Q_OBJECT // Macro required for classes with signals and slots

public:
    // Constructor and Destructor
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Slots for handling UI interactions and events
    void onTweetSelectionChanged(); // Called when the selected item in the list changes
    void onSearchTextChanged(const QString &searchText); // Called when text in search bar changes
    void focusSearchField(); // Called by the Ctrl+F action
    void onSearchNavigateKey(QKeyEvent *event); // Called when Up/Down arrow pressed in search bar
    void toggleFavorite(); // Called by the Ctrl+D action or potentially a button
    void applyFilters(); // Called when any filter control changes to update the list
    void resetFilters(); // Called by the Reset Filters button

private:
    // Private helper functions for setup and logic
    void setupUi(); // Sets up the main user interface layout and widgets
    void setupActions(); // Creates QAction objects for shortcuts (Find, Favorite)
    void loadTweets(); // Loads tweet data from the JSON resource file
    void populateFilterUI(); // Dynamically creates filter controls (checkboxes, buttons)
    void displayMetadata(const TweetData& tweet); // Displays metadata for the selected tweet
    void displayCode(const TweetData* tweet); // Displays code for the selected tweet (or clears if null)

    // Favorites management functions
    void loadFavorites(); // Loads favorite tweet names from QSettings
    void saveFavorites(); // Saves favorite tweet names to QSettings
    bool isFavorite(const QString& tweetName) const; // Checks if a tweet name is in the favorites set

    // UI element pointers
    SearchLineEdit *searchLineEdit; // Top global search
    QSplitter      *mainSplitter;      // Main Horizontal Splitter (Filters | List | Code/Meta)
    QWidget        *filterPanel;       // Leftmost panel holding the filter scroll area
    QScrollArea    *filterScrollArea;  // Scroll area for filters
    QWidget        *filterScrollWidget; // Widget inside the scroll area
    QVBoxLayout    *filterScrollLayout; // Layout for the scroll widget
    QListWidget    *tweetListWidget; // Middle panel: List of tweets
    QWidget        *rightPanel;        // Rightmost panel (Code/Meta - created by helper)
    QTextEdit      *codeTextEdit;      // Belongs to rightPanel
    QTextEdit      *metadataTextEdit;  // Belongs to rightPanel

    // Keep track of all created checkboxes for easy access
    QList<QCheckBox *> authorCheckboxes;
    QList<QCheckBox *> sonicCheckboxes;
    QList<QCheckBox *> techniqueCheckboxes;
    QList<QCheckBox *> ugenCheckboxes;

    // Control Buttons
    QCheckBox    *filterLogicToggle;  // Toggle for AND/OR logic
    QPushButton  *favoriteFilterButton;
    QPushButton  *resetFiltersButton;

    // Helper function for creating the right panel
    QWidget* createRightPanel();

    // Actions for shortcuts
    QAction *focusSearchAction; // Action for Ctrl+F
    QAction *toggleFavoriteAction; // Action for Ctrl+D

    // Data storage
    QVector<TweetData> tweets; // Master list of all loaded tweet data
    QSet<QString>      favoriteTweetNames; // Set of names marked as favorite
    QSettings         *settings; // For persistent storage of favorites
};

#endif // MAINWINDOW_H