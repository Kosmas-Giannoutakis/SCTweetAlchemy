#include "mainwindow.h"
#include "searchlineedit.h" // Include custom line edit

#include <QtWidgets> // Includes common Qt Widget classes
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>        // To help find the JSON file
#include <QMessageBox> // For showing errors
#include <QDebug>      // For printing debug messages (optional)
#include <QFontDatabase> // For fixed-width font
#include <QSplitter> // Include QSplitter
#include <QMenuBar> // Include for adding actions (optional but good practice)
#include <QKeyEvent> // Include QKeyEvent
#include <QScrollArea> // Include ScrollArea
#include <QPushButton> // Include PushButton
#include <QGroupBox>   // Include GroupBox
#include <QButtonGroup>// Include ButtonGroup
#include <QSet>        // Include Set
#include <QSettings>   // Include Settings
#include <QVariant>    // Include Variant
#include <algorithm>   // Needed for std::sort

// Define custom role (if not already defined elsewhere)
// const int TweetNameRole = Qt::UserRole + 1; // If needed later for list items

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , searchLineEdit(nullptr)
    , tweetListWidget(nullptr) // Initialize ListWidget pointer
    , codeTextEdit(nullptr)
    , metadataTextEdit(nullptr)
    , filterWidget(nullptr)    // Initialize filter container
    , clearFiltersButton(nullptr)
    , authorButtonGroup(nullptr) // Initialize button groups
    , ugenButtonGroup(nullptr)
    , sonicButtonGroup(nullptr)
    , complexityButtonGroup(nullptr)
    , favoritesButtonGroup(nullptr)
    , favoriteFilterButton(nullptr)
    , focusSearchAction(nullptr)
    , toggleFavoriteAction(nullptr)
{
    // --- Settings for Favorites ---
    QCoreApplication::setOrganizationName("YourOrgName"); // Set your org name
    QCoreApplication::setApplicationName("SCTweetPaster_CPP");
    settings = new QSettings(this); // Use standard INI format (or native)
    // -----------------------------

    setupUi();        // Create the overall UI structure
    setupActions();   // Create actions and shortcuts
    loadFavorites();  // Load favorites before loading tweets
    loadTweets();     // Load the tweet data
    populateFilterUI(); // Create filter buttons AFTER loading tweets

    // --- Connect Signals ---
    if (tweetListWidget) {
        connect(tweetListWidget, &QListWidget::currentItemChanged,
                this, &MainWindow::onTweetSelectionChanged);
    }
    if (searchLineEdit) {
        // Connect standard text changed signal
        connect(searchLineEdit, &QLineEdit::textChanged, // Use base class signal name
                this, &MainWindow::onSearchTextChanged);

        // Connect custom navigation signal
        connect(searchLineEdit, &SearchLineEdit::navigationKeyPressed, // Use custom signal name
                this, &MainWindow::onSearchNavigateKey);
    }
     if (clearFiltersButton) {
        connect(clearFiltersButton, &QPushButton::clicked, this, &MainWindow::clearAllFilters);
    }
    // Filter button connections are done in populateFilterUI / createFilterGroup

    applyFilters(); // Initial population of the list widget based on default filter state

    // Select first item if list not empty after initial filter
    if (tweetListWidget && tweetListWidget->count() > 0) {
       tweetListWidget->setCurrentRow(0);
       // Manually trigger selection changed for the first item if needed
       // onTweetSelectionChanged(); // applyFilters should trigger selection logic indirectly if row 0 exists
    } else {
        // Handle case where no tweets loaded or match initial filter
        if(codeTextEdit) codeTextEdit->setPlaceholderText("No tweets found or match filters.");
        if(metadataTextEdit) metadataTextEdit->setPlaceholderText("");
         // Call explicitly to clear panes if list is empty initially
         onTweetSelectionChanged();
    }

    // Give initial focus to the list
    if(tweetListWidget) {
        tweetListWidget->setFocus();
    }
}

MainWindow::~MainWindow()
{
    // No explicit deletion needed for widgets with 'this' as parent
    // QSettings is also parented to 'this'
}

// --- Action Setup ---
void MainWindow::setupActions()
{
    // --- Focus Search Action ---
    if (searchLineEdit) {
        focusSearchAction = new QAction("Focus Search", this);
        focusSearchAction->setShortcut(QKeySequence::Find);
        focusSearchAction->setToolTip("Focus the search field (Ctrl+F)");
        connect(focusSearchAction, &QAction::triggered, this, &MainWindow::focusSearchField); // Connect to slot
        this->addAction(focusSearchAction);
    } else {
        qWarning() << "Search Line Edit is null, cannot create focus search action.";
    }

    // --- Toggle Favorite Action ---
    toggleFavoriteAction = new QAction("Toggle Favorite", this);
    toggleFavoriteAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    toggleFavoriteAction->setToolTip("Mark/Unmark selected tweet in list as favorite (Ctrl+D)");
    connect(toggleFavoriteAction, &QAction::triggered, this, &MainWindow::toggleFavorite); // Connect to slot
    this->addAction(toggleFavoriteAction);

    /* Optional: Add actions to Menus
    QMenu *editMenu = menuBar()->addMenu("&Edit");
    if(focusSearchAction) editMenu->addAction(focusSearchAction);

    QMenu *tweetMenu = menuBar()->addMenu("&Tweet");
    tweetMenu->addAction(toggleFavoriteAction);
    */
}

// --- FOCUS SEARCH SLOT ---
void MainWindow::focusSearchField()
{
    if (searchLineEdit) {
        searchLineEdit->setFocus(); // Set keyboard focus
        searchLineEdit->selectAll(); // Optional: Select existing text
    }
}

// Helper to create the left panel is no longer needed (combined in setupUi)

// Helper to create the right panel (Code + Metadata)
QWidget* MainWindow::createRightPanel()
{
    // --- Create Widgets for the right side ---
    codeTextEdit = new QTextEdit(this);
    codeTextEdit->setReadOnly(true);
    codeTextEdit->setLineWrapMode(QTextEdit::NoWrap);
    codeTextEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    codeTextEdit->setObjectName("codeTextEdit");

    metadataTextEdit = new QTextEdit(this);
    metadataTextEdit->setReadOnly(true);
    metadataTextEdit->setObjectName("metadataTextEdit");

    // --- Use a Splitter for Resizing ---
    QSplitter *splitter = new QSplitter(Qt::Vertical, this); // Split vertically
    splitter->addWidget(codeTextEdit);
    splitter->addWidget(metadataTextEdit);

    // --- Set Initial Sizes (optional) ---
    splitter->setStretchFactor(0, 3); // Index 0 (code) gets 3 parts
    splitter->setStretchFactor(1, 1); // Index 1 (metadata) gets 1 part

    return splitter; // Return the containing widget
}

// --- Setup the overall UI structure ---
void MainWindow::setupUi()
{
    // --- Top Search Bar ---
    searchLineEdit = new SearchLineEdit(this); // Use custom SearchLineEdit
    searchLineEdit->setPlaceholderText("Search Tweets (Global)...");
    searchLineEdit->setClearButtonEnabled(true);

    // --- Filter Area ---
    filterWidget = new QWidget(this); // Container for filter groups
    QVBoxLayout *filterLayout = new QVBoxLayout(filterWidget); // Layout for the container
    filterLayout->setContentsMargins(5, 5, 5, 5);
    filterLayout->setSpacing(10);
    // We will populate this layout in populateFilterUI

    // Make the filter area scrollable in case of many buttons
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true); // Allow contained widget to resize
    scrollArea->setWidget(filterWidget);
    scrollArea->setFixedHeight(150); // Give it a fixed initial height (adjust as needed)

    // --- Main Content Area (List + Right Panel) ---
    tweetListWidget = new QListWidget(this); // Create ListWidget
    tweetListWidget->setObjectName("tweetListWidget");

    QWidget *rightPanel = createRightPanel(); // Create Code + Metadata panel

    QSplitter *mainContentSplitter = new QSplitter(Qt::Horizontal, this); // Split List and RightPanel
    mainContentSplitter->addWidget(tweetListWidget);
    mainContentSplitter->addWidget(rightPanel);
    mainContentSplitter->setStretchFactor(0, 1); // List gets 1 part
    mainContentSplitter->setStretchFactor(1, 3); // Right panel gets 3 parts


    // --- Overall Layout ---
    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(searchLineEdit);   // Search at top
    centralLayout->addWidget(scrollArea);       // Filters below search
    centralLayout->addWidget(mainContentSplitter); // List/Code/Meta below filters

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(centralLayout);
    setCentralWidget(centralWidget);

    // --- Window Properties ---
    setWindowTitle("SCTweet Paster (C++/Qt Filter Buttons)");
    resize(850, 750); // Increased height slightly
}

// --- Load tweet data from JSON ---
void MainWindow::loadTweets()
{
    QDir appDir(QApplication::applicationDirPath());
    if (!appDir.cdUp()) {
         qWarning() << "Could not navigate up from application directory:" << QApplication::applicationDirPath();
         QMessageBox::critical(this, "Load Error", "Could not navigate to parent directory to find SCTweets.json.");
         return;
    }
    QString jsonPath = appDir.filePath("SCTweets.json");
    QFile jsonFile(jsonPath);
    qInfo() << "Attempting to load tweets from:" << jsonPath;

    if (!jsonFile.exists()) {
        qWarning() << "SCTweets.json not found at expected location.";
        QMessageBox::warning(this, "Load Error", "Could not find SCTweets.json in parent directory:\n" + jsonPath);
        return;
    }
    if (!jsonFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open SCTweets.json:" << jsonFile.errorString();
        QMessageBox::warning(this, "Load Error", "Could not open SCTweets.json:\n" + jsonFile.errorString());
        return;
    }

    QByteArray jsonData = jsonFile.readAll();
    jsonFile.close();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse JSON:" << parseError.errorString();
        QMessageBox::critical(this, "JSON Error", "Failed to parse SCTweets.json:\n" + parseError.errorString());
        return;
    }
    if (!doc.isObject()) {
        qWarning() << "JSON root is not an object.";
        QMessageBox::critical(this, "JSON Error", "SCTweets.json root is not a valid JSON object.");
        return;
    }

    QJsonObject rootObj = doc.object();
    tweets.clear(); // Clear previous data

    for (auto it = rootObj.constBegin(); it != rootObj.constEnd(); ++it) {
        QString key = it.key();
        QJsonValue value = it.value();
        if (!value.isObject()) { continue; }
        QJsonObject tweetObj = value.toObject();
        if (!tweetObj.contains("original") || !tweetObj["original"].isString()) { continue; }

        TweetData td;
        td.name = key;
        td.originalCode = tweetObj["original"].toString();
        td.author = tweetObj.value("author").toString("Unknown");
        td.sourceUrl = tweetObj.value("source_url").toString("");
        td.description = tweetObj.value("description").toString("-");
        if (tweetObj.contains("tags") && tweetObj["tags"].isArray()) {
            QJsonArray tagsArray = tweetObj["tags"].toArray();
            for (const QJsonValue &tagVal : tagsArray) {
                if (tagVal.isString()) { td.tags.append(tagVal.toString()); }
            }
        }
        tweets.append(td);
    }
    qInfo() << "Loaded" << tweets.count() << "tweets from JSON.";
}

// --- Populate the Filter UI Dynamically ---
void MainWindow::populateFilterUI()
{
    if (!filterWidget || tweets.isEmpty()) return;

    // Layout for the filterWidget container
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(filterWidget->layout());
    if (!layout) { // Should have been set in setupUi
        layout = new QVBoxLayout(filterWidget); // Create if missing
        qWarning() << "Filter widget layout was missing, created new one.";
    }

    // --- Clear previous filter UI elements ---
    // (This is basic, proper cleanup might be needed if repopulating often)
    qDeleteAll(filterWidget->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly));
    authorButtons.clear();
    ugenButtons.clear();
    sonicButtons.clear();
    complexityButtons.clear();
    // Reset pointers, Qt should handle deletion via parentage
    authorButtonGroup = nullptr;
    ugenButtonGroup = nullptr;
    sonicButtonGroup = nullptr;
    complexityButtonGroup = nullptr;
    favoritesButtonGroup = nullptr;
    favoriteFilterButton = nullptr;
    clearFiltersButton = nullptr;


    // --- Extract Unique Items for Filters ---
    QSet<QString> authors;
    QSet<QString> ugens;
    QSet<QString> sonics;
    QSet<QString> complexities;

    for (const auto& tweet : tweets) {
        authors.insert(tweet.author.isEmpty() ? "Unknown" : tweet.author);
        for (const QString& tag : tweet.tags) {
            QString lowerTag = tag.toLower();
            // --- Classify tags (adjust these rules based on your actual tags) ---
            if (lowerTag.contains("osc") || lowerTag.contains("gen") || lowerTag.startsWith("sin") || lowerTag.startsWith("saw") || lowerTag.startsWith("pulse") || lowerTag.startsWith("fm") || lowerTag.startsWith("grain")) {
                ugens.insert(tag);
            } else if (lowerTag == "drone" || lowerTag == "ambient" || lowerTag == "noise" || lowerTag == "glitch" || lowerTag == "percussive" || lowerTag == "melodic") {
                sonics.insert(tag);
            } else if (lowerTag == "simple" || lowerTag == "complex" || lowerTag == "one-liner") {
                complexities.insert(tag);
            }
        }
    }

    // --- Create Filter Groups ---
    // Pass button groups and maps by reference to be initialized
    QWidget* authorGroupWidget = createFilterGroup("Author", authors, authorButtonGroup, authorButtons);
    QWidget* ugenGroupWidget = createFilterGroup("Primary UGen", ugens, ugenButtonGroup, ugenButtons);
    QWidget* sonicGroupWidget = createFilterGroup("Sonic Characteristic", sonics, sonicButtonGroup, sonicButtons);
    QWidget* complexityGroupWidget = createFilterGroup("Complexity", complexities, complexityButtonGroup, complexityButtons);

    // --- Create Favorite Filter ---
    QGroupBox* favGroupBox = new QGroupBox("Show");
    favGroupBox->setObjectName("FavGroup"); // Give object name
    QHBoxLayout* favLayout = new QHBoxLayout(favGroupBox);
    favoriteFilterButton = new QPushButton("Favorites Only");
    favoriteFilterButton->setCheckable(true);
    favoriteFilterButton->setObjectName("FilterButton_Favorite"); // Unique ID
    // Button group for favorite is optional, could just check isChecked()
    favoritesButtonGroup = new QButtonGroup(favGroupBox); // Parented
    favoritesButtonGroup->setExclusive(false); // Allow unchecking
    favoritesButtonGroup->addButton(favoriteFilterButton);
    connect(favoriteFilterButton, &QPushButton::toggled, this, &MainWindow::applyFilters);
    favLayout->addWidget(favoriteFilterButton);
    favLayout->addStretch();

    // --- Add Groups to the main filter layout ---
    // Use horizontal layouts to arrange groups side-by-side if desired
    QHBoxLayout* groupLayout1 = new QHBoxLayout();
    if(authorGroupWidget) groupLayout1->addWidget(authorGroupWidget);
    if(ugenGroupWidget) groupLayout1->addWidget(ugenGroupWidget);
    layout->addLayout(groupLayout1);

    QHBoxLayout* groupLayout2 = new QHBoxLayout();
    if(sonicGroupWidget) groupLayout2->addWidget(sonicGroupWidget);
    if(complexityGroupWidget) groupLayout2->addWidget(complexityGroupWidget);
    layout->addLayout(groupLayout2);

    layout->addWidget(favGroupBox); // Add favorite group box

    // --- Clear All Button ---
    clearFiltersButton = new QPushButton("Clear All Filters");
    // Connection done in constructor
    layout->addWidget(clearFiltersButton, 0, Qt::AlignRight); // Align button to right

    layout->addStretch(); // Push filter groups to the top

    qInfo() << "Populated filter UI.";
}

// Helper function to create a group box with checkable buttons
QWidget* MainWindow::createFilterGroup(const QString& title, const QSet<QString>& items, QButtonGroup*& buttonGroup, QMap<QString, QPushButton*>& buttonMap)
{
    if (items.isEmpty()) {
        buttonGroup = nullptr; // Ensure button group is null if no items
        return nullptr; // Don't create group if no items
    }

    QGroupBox *groupBox = new QGroupBox(title);
    groupBox->setObjectName("Group_" + title.simplified().replace(" ", "_")); // Set object name
    QVBoxLayout *vLayout = new QVBoxLayout(); // Main layout for the group

    buttonGroup = new QButtonGroup(groupBox); // Parent is groupbox
    buttonGroup->setExclusive(true); // Only one selection per group

    QStringList sortedItems = items.values(); // Convert set to list
    sortedItems.sort(Qt::CaseInsensitive); // Sort for display

    // Use a widget with a flow layout ideally, but QHBoxLayout is simpler for now
    QWidget* buttonContainer = new QWidget();
    QHBoxLayout* buttonLayout = new QHBoxLayout(buttonContainer); // Simple horizontal layout
    buttonLayout->setSpacing(5);
    buttonLayout->setContentsMargins(2,2,2,2);
    // Consider QGridLayout or a custom FlowLayout for better wrapping

    for (const QString& item : sortedItems) {
        QPushButton *button = new QPushButton(item);
        button->setCheckable(true);
        // Sanitize item text for object name
        QString safeItemName = item;
        safeItemName.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");
        button->setObjectName("FilterButton_" + title.simplified().replace(" ", "_") + "_" + safeItemName);
        buttonLayout->addWidget(button);
        buttonGroup->addButton(button); // Add to group for exclusivity
        buttonMap.insert(item, button); // Store reference if needed
        // Connect toggle signal to applyFilters slot
        connect(button, &QPushButton::toggled, this, &MainWindow::applyFilters);
    }
    buttonLayout->addStretch(); // Push buttons to the left

    vLayout->addWidget(buttonContainer); // Add the container with buttons
    groupBox->setLayout(vLayout); // Set layout for the group box

    return groupBox;
}


// --- Slot to clear all active filters ---
void MainWindow::clearAllFilters()
{
    // Block signals while changing button states to avoid multiple filter applications
    bool authorBlocked = authorButtonGroup ? authorButtonGroup->signalsBlocked() : false;
    bool ugenBlocked = ugenButtonGroup ? ugenButtonGroup->signalsBlocked() : false;
    bool sonicBlocked = sonicButtonGroup ? sonicButtonGroup->signalsBlocked() : false;
    bool complexityBlocked = complexityButtonGroup ? complexityButtonGroup->signalsBlocked() : false;
    bool favBlocked = favoriteFilterButton ? favoriteFilterButton->signalsBlocked() : false;

    if(authorButtonGroup) authorButtonGroup->blockSignals(true);
    if(ugenButtonGroup) ugenButtonGroup->blockSignals(true);
    if(sonicButtonGroup) sonicButtonGroup->blockSignals(true);
    if(complexityButtonGroup) complexityButtonGroup->blockSignals(true);
    if(favoriteFilterButton) favoriteFilterButton->blockSignals(true);

    // Uncheck the currently checked button in each exclusive group (if any)
    if (authorButtonGroup && authorButtonGroup->checkedButton()) {
        // Set exclusive must be false to allow unchecking the only checked button
        authorButtonGroup->setExclusive(false);
        authorButtonGroup->checkedButton()->setChecked(false);
        authorButtonGroup->setExclusive(true);
    }
    if (ugenButtonGroup && ugenButtonGroup->checkedButton()) {
        ugenButtonGroup->setExclusive(false);
        ugenButtonGroup->checkedButton()->setChecked(false);
        ugenButtonGroup->setExclusive(true);
    }
     if (sonicButtonGroup && sonicButtonGroup->checkedButton()) {
        sonicButtonGroup->setExclusive(false);
        sonicButtonGroup->checkedButton()->setChecked(false);
        sonicButtonGroup->setExclusive(true);
    }
    if (complexityButtonGroup && complexityButtonGroup->checkedButton()) {
        complexityButtonGroup->setExclusive(false);
        complexityButtonGroup->checkedButton()->setChecked(false);
        complexityButtonGroup->setExclusive(true);
    }
    // Uncheck favorite button
    if (favoriteFilterButton && favoriteFilterButton->isChecked()) {
        favoriteFilterButton->setChecked(false);
    }

    // Unblock signals
     if(authorButtonGroup) authorButtonGroup->blockSignals(authorBlocked);
     if(ugenButtonGroup) ugenButtonGroup->blockSignals(ugenBlocked);
     if(sonicButtonGroup) sonicButtonGroup->blockSignals(sonicBlocked);
     if(complexityButtonGroup) complexityButtonGroup->blockSignals(complexityBlocked);
     if(favoriteFilterButton) favoriteFilterButton->blockSignals(favBlocked);


    // Now apply the (cleared) filters
    applyFilters();
    if(searchLineEdit) searchLineEdit->clear(); // Also clear search text
}

// --- Apply All Active Filters ---
void MainWindow::applyFilters()
{
    if (!tweetListWidget) return;

    // Store current selection text to try and restore it
    QString previousSelectionText = tweetListWidget->currentItem() ? tweetListWidget->currentItem()->text() : "";

    tweetListWidget->clear(); // Clear the list to repopulate

    // --- Get Active Filter Values ---
    QString authorFilter = (authorButtonGroup && authorButtonGroup->checkedButton())
                                ? authorButtonGroup->checkedButton()->text()
                                : QString(); // Empty if no filter
    QString ugenFilter = (ugenButtonGroup && ugenButtonGroup->checkedButton())
                                ? ugenButtonGroup->checkedButton()->text()
                                : QString();
    QString sonicFilter = (sonicButtonGroup && sonicButtonGroup->checkedButton())
                                ? sonicButtonGroup->checkedButton()->text()
                                : QString();
    QString complexityFilter = (complexityButtonGroup && complexityButtonGroup->checkedButton())
                                ? complexityButtonGroup->checkedButton()->text()
                                : QString();
    bool favoritesOnly = (favoriteFilterButton && favoriteFilterButton->isChecked());

    QString searchText = searchLineEdit ? searchLineEdit->text() : QString();

    qDebug() << "Applying filters:"
             << "Search:" << searchText
             << "Author:" << authorFilter
             << "UGen:" << ugenFilter
             << "Sonic:" << sonicFilter
             << "Complexity:" << complexityFilter
             << "FavsOnly:" << favoritesOnly;


    // --- Iterate through ALL tweets and apply filters ---
    QStringList namesToAdd;
    for (const auto& tweet : tweets) {
        bool passesFilter = true;

        // Apply global search filter first (searches tweet name only for now)
        if (!searchText.isEmpty() && !tweet.name.contains(searchText, Qt::CaseInsensitive)) {
             passesFilter = false;
        }

        // Apply Author filter
        if (passesFilter && !authorFilter.isEmpty()) {
             if (authorFilter == "Unknown") { // Special case for Unknown
                 if (!tweet.author.isEmpty()) passesFilter = false;
             } else {
                 if (tweet.author != authorFilter) passesFilter = false;
             }
        }
        // Apply UGen filter (check tags)
        if (passesFilter && !ugenFilter.isEmpty() && !tweet.tags.contains(ugenFilter, Qt::CaseInsensitive)) {
            // Maybe allow partial match or smarter tag check? For now, exact match (case insensitive)
            passesFilter = false;
        }
        // Apply Sonic filter (check tags)
        if (passesFilter && !sonicFilter.isEmpty() && !tweet.tags.contains(sonicFilter, Qt::CaseInsensitive)) {
            passesFilter = false;
        }
         // Apply Complexity filter (check tags)
        if (passesFilter && !complexityFilter.isEmpty() && !tweet.tags.contains(complexityFilter, Qt::CaseInsensitive)) {
            passesFilter = false;
        }
        // Apply Favorite filter
        if (passesFilter && favoritesOnly && !isFavorite(tweet.name)) {
            passesFilter = false;
        }

        // If it passes all filters, add its name
        if (passesFilter) {
            namesToAdd.append(tweet.name);
        }
    }

    // --- Populate List Widget ---
    namesToAdd.sort(Qt::CaseInsensitive);
    QListWidgetItem* itemToSelect = nullptr;
    for (const QString &name : namesToAdd) {
        QListWidgetItem* newItem = new QListWidgetItem(name, tweetListWidget); // Parent ensures deletion
        // Set favorite icon on list item?
        if (isFavorite(name)) {
             newItem->setIcon(QIcon::fromTheme("emblem-favorite", QIcon(":/icons/favorite.png")));
        }
        if (name == previousSelectionText) {
            itemToSelect = newItem; // Found item to re-select
        }
    }

    // Try to restore selection or select first
    if (itemToSelect) {
        tweetListWidget->setCurrentItem(itemToSelect);
    } else if (tweetListWidget->count() > 0) {
        tweetListWidget->setCurrentRow(0);
    } else {
         // If list becomes empty, explicitly clear displays
         displayCode(nullptr);
    }

     // Trigger selection changed if an item is selected now, or handle empty case
     // onTweetSelectionChanged(); // Might cause issues if called too early? Let selection signal handle it.

     // Update status bar or similar if needed
     // statusBar()->showMessage(QString("Showing %1 tweets").arg(tweetListWidget->count()));

     qInfo() << "Filters applied, list count:" << tweetListWidget->count();
}

// --- Implementation of the slot handling Up/Down keys from search field ---
void MainWindow::onSearchNavigateKey(QKeyEvent *event)
{
    // Simpler version for ListWidget: just focus the list
    if (tweetListWidget && tweetListWidget->count() > 0) {
        tweetListWidget->setFocus(); // Give focus to the list

        // Select first item if nothing is selected
        if (!tweetListWidget->currentItem()) {
            tweetListWidget->setCurrentRow(0);
        }
        // QListWidget handles subsequent Up/Down navigation itself.
    }
}

void MainWindow::onSearchTextChanged(const QString &searchText) // Check signature here
{
    // When the global search text changes, we just re-apply all filters.
    // The applyFilters function now includes the search text check.
    applyFilters();
}

// --- Display metadata (added favorite status) ---
void MainWindow::displayMetadata(const TweetData& tweet)
{
    if (!metadataTextEdit) return; // Safety check

    QString metadataString;
    metadataString += "Author: " + tweet.author + "\n";
    metadataString += "Source: " + (!tweet.sourceUrl.isEmpty() ? tweet.sourceUrl : QStringLiteral("N/A")) + "\n";
    metadataString += "Description: " + tweet.description + "\n";
    metadataString += "Tags: " + (tweet.tags.isEmpty() ? QStringLiteral("None") : tweet.tags.join(", ")) + "\n";
    metadataString += QStringLiteral("Favorite: ");
    metadataString += (isFavorite(tweet.name) ? QStringLiteral("Yes") : QStringLiteral("No"));
    metadataString += QLatin1Char('\n'); // Append newline efficiently

    metadataTextEdit->setText(metadataString);
}


// --- Display code or clear panes ---
void MainWindow::displayCode(const TweetData* tweet) {
    if (!codeTextEdit || !metadataTextEdit) return;

    if (tweet) { // tweet is a pointer, check if it's not null
        // --- CORRECTED LINE ---
        codeTextEdit->setText(tweet->originalCode); // Use -> for pointers
        // --- END CORRECTION ---
        displayMetadata(*tweet); // Pass the dereferenced object to displayMetadata
    } else {
        // Clear if null pointer
        codeTextEdit->clear();
        codeTextEdit->setPlaceholderText("Select a Tweet or clear filters.");
        metadataTextEdit->clear();
        metadataTextEdit->setPlaceholderText("Select a Tweet to view its metadata.");
    }
}


// --- Favorites Management ---
void MainWindow::loadFavorites() {
    if(!settings) return;
    // Get the value as a QStringList
    QStringList favList = settings->value("favorites", QStringList()).toStringList();
    // Construct the QSet from the QStringList
    favoriteTweetNames = QSet<QString>(favList.begin(), favList.end());
    qInfo() << "Loaded" << favoriteTweetNames.count() << "favorites.";
}

void MainWindow::saveFavorites() {
    if(!settings) return;
    // Convert QSet to QStringList for saving
    settings->setValue("favorites", QStringList(favoriteTweetNames.begin(), favoriteTweetNames.end()));
    qInfo() << "Saved" << favoriteTweetNames.count() << "favorites.";
    settings->sync(); // Ensure data is written
}

bool MainWindow::isFavorite(const QString& tweetName) const {
    return favoriteTweetNames.contains(tweetName);
}

// --- Toggle Favorite Status ---
void MainWindow::toggleFavorite() {
    // Get selected item from ListWidget
    if (!tweetListWidget) return;
    QListWidgetItem* currentItem = tweetListWidget->currentItem();
    if (!currentItem) {
         qWarning() << "Cannot toggle favorite: No item selected in list.";
         return; // No selection
    }

    QString tweetName = currentItem->text(); // Get name from list item

    const TweetData* currentTweetData = nullptr;
     for (const auto& tweet : tweets) {
        if (tweet.name == tweetName) {
            currentTweetData = &tweet;
            break;
        }
    }
     if (!currentTweetData) {
          qWarning() << "Cannot toggle favorite: Could not find data for" << tweetName;
          return; // Should not happen
     }


    // Add or remove from the set
    bool wasFavorite = isFavorite(tweetName);
    if (wasFavorite) {
        favoriteTweetNames.remove(tweetName);
        currentItem->setIcon(QIcon()); // Remove icon
        qInfo() << "Removed favorite:" << tweetName;
    } else {
        favoriteTweetNames.insert(tweetName);
        // Set icon
        currentItem->setIcon(QIcon::fromTheme("emblem-favorite", QIcon(":/icons/favorite.png")));
        qInfo() << "Added favorite:" << tweetName;
    }

    saveFavorites();

    // Refresh metadata display for the current item
    displayMetadata(*currentTweetData);

    // Refresh list if "Favorites Only" filter is active (to potentially remove item)
    if (wasFavorite && favoriteFilterButton && favoriteFilterButton->isChecked()) {
        applyFilters();
    }
}


// --- Slot called when list selection changes ---
void MainWindow::onTweetSelectionChanged()
{
    // Ensure all necessary widgets exist
    if (!tweetListWidget || !codeTextEdit || !metadataTextEdit) {
         if(codeTextEdit) codeTextEdit->clear();
         if(metadataTextEdit) metadataTextEdit->clear();
         return;
    }

    // Get the currently selected item (might be null if list is empty/cleared)
    QListWidgetItem *currentItem = tweetListWidget->currentItem();
    if (!currentItem) {
        displayCode(nullptr); // Clear panes if no selection
        return;
    }

    QString selectedName = currentItem->text();
    const TweetData* foundTweet = nullptr;

    // Find the corresponding tweet data in our master vector
    for (const auto& tweet : tweets) {
        if (tweet.name == selectedName) {
            foundTweet = &tweet;
            break;
        }
    }

    displayCode(foundTweet); // Display code/meta or clear if tweet data somehow not found
}