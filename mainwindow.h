#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QVector>
#include <QStringList>
#include <QKeySequence>
#include <QAction>
#include <QSet>
#include <QList>

#include "searchlineedit.h"

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

// Data structure for holding tweet information (UPDATED)
struct TweetData {
    QString name;
    QString originalCode;
    QString author;
    QString sourceUrl;
    QString description;
    QString publicationDate; // <<< ADDED
    QStringList tags; // Keep original flat tags for raw display/misc use
    // --- NEW: Store categorized tags ---
    QStringList sonicTags;
    QStringList techniqueTags;
    // ----------------------------------
};

// Main application window class
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onTweetSelectionChanged();
    void onSearchTextChanged(const QString &searchText);
    void focusSearchField();
    void onSearchNavigateKey(QKeyEvent *event);
    void toggleFavorite();
    void applyFilters();
    void resetFilters();

private:
    void setupUi();
    void setupActions();
    void loadTweets(); // Logic updated to parse new JSON structure
    void populateFilterUI(); // Logic updated to use categorized tags
    void displayMetadata(const TweetData& tweet); // Logic updated to show new fields
    void displayCode(const TweetData* tweet);

    // Favorites management
    void loadFavorites();
    void saveFavorites();
    bool isFavorite(const QString& tweetName) const;

    // UI element pointers
    SearchLineEdit *searchLineEdit;
    QSplitter      *mainSplitter;
    QWidget        *filterPanel;
    QScrollArea    *filterScrollArea;
    QWidget        *filterScrollWidget;
    QVBoxLayout    *filterScrollLayout;
    QListWidget    *tweetListWidget;
    QWidget        *rightPanel;
    QTextEdit      *codeTextEdit;
    QTextEdit      *metadataTextEdit;

    // Checkbox lists (Names remain the same)
    QList<QCheckBox *> authorCheckboxes;
    QList<QCheckBox *> sonicCheckboxes;
    QList<QCheckBox *> techniqueCheckboxes;
    QList<QCheckBox *> ugenCheckboxes;

    // Control Buttons
    QCheckBox    *filterLogicToggle;
    QPushButton  *favoriteFilterButton;
    QPushButton  *resetFiltersButton;

    // Helper function for creating the right panel
    QWidget* createRightPanel();

    // Actions for shortcuts
    QAction *focusSearchAction;
    QAction *toggleFavoriteAction;

    // Data storage
    QVector<TweetData> tweets; // Holds instances of the updated struct
    QSet<QString>      favoriteTweetNames;
    QSettings         *settings;
};

#endif // MAINWINDOW_H