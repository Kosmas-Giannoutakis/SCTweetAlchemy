#include "mainwindow.h"
#include "searchlineedit.h"

#include <QtWidgets> // Includes common Qt Widget classes
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QDebug>
#include <QFontDatabase>
#include <QSplitter>
#include <QMenuBar>
#include <QKeyEvent>
// #include <QScrollArea> // No longer needed after layout change
#include <QPushButton>
// #include <QGroupBox> // No longer needed for filters
#include <QButtonGroup>
#include <QSet>
#include <QSettings>
#include <QVariant>
#include <algorithm>
#include <QComboBox>
#include <QLabel>
#include <QRegularExpression> // Needed for UGen extraction


// --- Constructor ---
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , searchLineEdit(nullptr)
    , tweetListWidget(nullptr)
    , codeTextEdit(nullptr)
    , metadataTextEdit(nullptr)
    , filterWidget(nullptr)
    , authorComboBox(nullptr)
    , ugenComboBox(nullptr)
    , sonicComboBox(nullptr)
    , complexityComboBox(nullptr)
    , favoriteFilterButton(nullptr)
    , favoritesButtonGroup(nullptr)
    , resetFiltersButton(nullptr) // Initialize reset button pointer
    , authorLabel(nullptr)
    , ugenLabel(nullptr)
    , sonicLabel(nullptr)
    , complexityLabel(nullptr)
    , favoriteLabel(nullptr)
    , focusSearchAction(nullptr)
    , toggleFavoriteAction(nullptr)
{
    // --- Settings Initialization ---
    QCoreApplication::setOrganizationName("Kosmas"); // Example org name
    QCoreApplication::setApplicationName("SCTweetAlchemy"); // Example app name
    settings = new QSettings(this); // Parented to MainWindow for auto-deletion

    // --- Setup Steps ---
    setupUi();        // Create the UI layout
    setupActions();   // Create Ctrl+F, Ctrl+D actions
    loadFavorites();  // Load saved favorites from settings
    loadTweets();     // Load tweet data from JSON resource
    populateFilterUI(); // Create filter controls based on loaded tweets

    // --- Connect Signals to Slots ---
    if (tweetListWidget) {
        connect(tweetListWidget, &QListWidget::currentItemChanged,
                this, &MainWindow::onTweetSelectionChanged);
    }
    if (searchLineEdit) {
        connect(searchLineEdit, &QLineEdit::textChanged,
                this, &MainWindow::onSearchTextChanged);
        connect(searchLineEdit, &SearchLineEdit::navigationKeyPressed,
                this, &MainWindow::onSearchNavigateKey);
    }
    // Connections for ComboBoxes, Favorite button, and Reset button are done in populateFilterUI

    // --- Initial State ---
    applyFilters(); // Populate the list based on default filters

    if (tweetListWidget && tweetListWidget->count() > 0) {
       tweetListWidget->setCurrentRow(0); // Select the first item if list is not empty
    } else {
        // Handle empty list case (no tweets loaded or none match default filters)
        if(codeTextEdit) codeTextEdit->setPlaceholderText("No tweets found or match filters.");
        if(metadataTextEdit) metadataTextEdit->setPlaceholderText("");
         onTweetSelectionChanged(); // Explicitly clear panes if list is empty initially
    }

    // Give initial focus to the list widget for keyboard navigation
    if(tweetListWidget) {
        tweetListWidget->setFocus();
    }
}

// --- Destructor ---
MainWindow::~MainWindow()
{
    // No explicit deletion needed for widgets/objects parented to 'this' (MainWindow)
    // QSettings is also parented.
}

// --- Action Setup ---
void MainWindow::setupActions()
{
    // --- Focus Search Action (Ctrl+F) ---
    if (searchLineEdit) {
        focusSearchAction = new QAction("Focus Search", this);
        focusSearchAction->setShortcut(QKeySequence::Find); // Standard Ctrl+F shortcut
        focusSearchAction->setToolTip("Focus the search field (Ctrl+F)");
        connect(focusSearchAction, &QAction::triggered, this, &MainWindow::focusSearchField);
        this->addAction(focusSearchAction); // Add action globally to the window
    } else {
        qWarning() << "Search Line Edit is null, cannot create focus search action.";
    }

    // --- Toggle Favorite Action (Ctrl+D) ---
    toggleFavoriteAction = new QAction("Toggle Favorite", this);
    toggleFavoriteAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D)); // Custom shortcut Ctrl+D
    toggleFavoriteAction->setToolTip("Mark/Unmark selected tweet in list as favorite (Ctrl+D)");
    connect(toggleFavoriteAction, &QAction::triggered, this, &MainWindow::toggleFavorite);
    this->addAction(toggleFavoriteAction); // Add action globally to the window

    /* Optional: Add actions to Menus if you have a MenuBar
    QMenu *editMenu = menuBar()->addMenu("&Edit");
    if(focusSearchAction) editMenu->addAction(focusSearchAction);

    QMenu *tweetMenu = menuBar()->addMenu("&Tweet");
    tweetMenu->addAction(toggleFavoriteAction);
    */
}

// --- Slot to Focus Search Field ---
void MainWindow::focusSearchField()
{
    if (searchLineEdit) {
        searchLineEdit->setFocus(); // Set keyboard focus
        searchLineEdit->selectAll(); // Optional: Select existing text for easy replacement
    }
}

// --- Helper to Create Right Panel (Code + Metadata) ---
QWidget* MainWindow::createRightPanel()
{
    // --- Create Code Panel with Title ---
    QWidget* codePanel = new QWidget(this);
    QVBoxLayout* codeLayout = new QVBoxLayout(codePanel);
    codeLayout->setContentsMargins(0, 2, 0, 0); // Adjust margins
    codeLayout->setSpacing(3);                 // Adjust spacing

    QLabel* codeTitleLabel = new QLabel("Code", codePanel);
    QFont titleFont = codeTitleLabel->font();
    titleFont.setBold(true);
    codeTitleLabel->setFont(titleFont);

    codeTextEdit = new QTextEdit(this); // Initialize member variable
    codeTextEdit->setReadOnly(true);
    codeTextEdit->setLineWrapMode(QTextEdit::WidgetWidth); // Enable line wrapping
    codeTextEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont)); // Use monospace font
    codeTextEdit->setObjectName("codeTextEdit");

    codeLayout->addWidget(codeTitleLabel);
    codeLayout->addWidget(codeTextEdit);

    // --- Create Metadata Panel with Title ---
    QWidget* metadataPanel = new QWidget(this);
    QVBoxLayout* metadataLayout = new QVBoxLayout(metadataPanel);
    metadataLayout->setContentsMargins(0, 2, 0, 0); // Adjust margins
    metadataLayout->setSpacing(3);                 // Adjust spacing

    QLabel* metadataTitleLabel = new QLabel("Metadata", metadataPanel);
    metadataTitleLabel->setFont(titleFont); // Reuse bold font

    metadataTextEdit = new QTextEdit(this); // Initialize member variable
    metadataTextEdit->setReadOnly(true);
    metadataTextEdit->setObjectName("metadataTextEdit");
    // metadataTextEdit->setLineWrapMode(QTextEdit::WidgetWidth); // Optional wrapping for metadata

    metadataLayout->addWidget(metadataTitleLabel);
    metadataLayout->addWidget(metadataTextEdit);

    // --- Use a Splitter for the Panels ---
    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    splitter->addWidget(codePanel);     // Add panel containing title and text
    splitter->addWidget(metadataPanel); // Add panel containing title and text

    // --- Set Initial Splitter Sizes ---
    splitter->setStretchFactor(0, 4); // Give code panel more initial space
    splitter->setStretchFactor(1, 1);

    return splitter; // Return the containing splitter
}

// --- Setup Overall UI Layout ---
void MainWindow::setupUi()
{
    // --- Top Search Bar (Global) ---
    searchLineEdit = new SearchLineEdit(this);
    searchLineEdit->setPlaceholderText("Search Tweets (Global)...");
    searchLineEdit->setClearButtonEnabled(true); // Show clear button when text exists

    // --- Filter Controls Container ---
    filterWidget = new QWidget(this);
    QHBoxLayout *filterLayout = new QHBoxLayout(filterWidget);
    filterLayout->setContentsMargins(2, 2, 2, 2);
    filterLayout->setSpacing(5);
    // Filter controls (labels, combos, buttons) are added in populateFilterUI

    // --- Tweet List ---
    tweetListWidget = new QListWidget(this);
    tweetListWidget->setObjectName("tweetListWidget");

    // --- Left Panel (Filters + List) ---
    QWidget *leftPanel = new QWidget(this);
    QVBoxLayout *leftPanelLayout = new QVBoxLayout(leftPanel);
    leftPanelLayout->setContentsMargins(0, 0, 0, 0);
    leftPanelLayout->setSpacing(5);
    leftPanelLayout->addWidget(filterWidget);      // Filters at top of left panel
    leftPanelLayout->addWidget(tweetListWidget);   // List below filters

    // --- Right Panel (Code + Metadata) ---
    QWidget *rightPanel = createRightPanel(); // Create panel with titles

    // --- Main Content Splitter ---
    QSplitter *mainContentSplitter = new QSplitter(Qt::Horizontal, this);
    mainContentSplitter->addWidget(leftPanel);     // Left side (Filters + List)
    mainContentSplitter->addWidget(rightPanel);    // Right side (Code + Metadata)
    mainContentSplitter->setStretchFactor(0, 1); // Set initial proportions (adjust as needed)
    mainContentSplitter->setStretchFactor(1, 3);

    // --- Overall Window Layout ---
    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(searchLineEdit);       // Global search at top
    centralLayout->addWidget(mainContentSplitter); // Main content area below

    // --- Set Central Widget ---
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(centralLayout);
    setCentralWidget(centralWidget);

    // --- Window Properties ---
    setWindowTitle("SCTweetAlchemy Paster");
    resize(1100, 850); // Set larger initial window size
}

// --- Load Tweets from JSON Resource ---
void MainWindow::loadTweets()
{
    QString resourcePath = ":/data/SCTweets.json"; // Path defined in resources.qrc alias
    QFile jsonFile(resourcePath);
    qInfo() << "Attempting to load tweets from resource:" << resourcePath;

    if (!jsonFile.exists()) {
        qWarning() << "Resource" << resourcePath << "not found.";
        QMessageBox::critical(this, "Load Error", "Could not find SCTweets.json in application resources.\n" + resourcePath + "\nEnsure resources.qrc is compiled correctly.");
        return;
    }
    if (!jsonFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open resource" << resourcePath << ":" << jsonFile.errorString();
        QMessageBox::warning(this, "Load Error", "Could not open resource SCTweets.json:\n" + jsonFile.errorString());
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
    tweets.clear(); // Clear any previous data

    // Iterate through the JSON object and populate the tweets vector
    for (auto it = rootObj.constBegin(); it != rootObj.constEnd(); ++it) {
        QString key = it.key(); // Tweet name (e.g., "bass_glitch")
        QJsonValue value = it.value();
        if (!value.isObject()) { continue; } // Skip if not a valid tweet object

        QJsonObject tweetObj = value.toObject();
        // Basic validation: Ensure at least the 'original' code exists
        if (!tweetObj.contains("original") || !tweetObj["original"].isString()) { continue; }

        TweetData td;
        td.name = key;
        td.originalCode = tweetObj["original"].toString();
        td.author = tweetObj.value("author").toString("Unknown");
        td.sourceUrl = tweetObj.value("source_url").toString("");
        td.description = tweetObj.value("description").toString("-");

        // Parse tags array
        if (tweetObj.contains("tags") && tweetObj["tags"].isArray()) {
            QJsonArray tagsArray = tweetObj["tags"].toArray();
            for (const QJsonValue &tagVal : tagsArray) {
                if (tagVal.isString()) {
                    td.tags.append(tagVal.toString());
                }
            }
        }
        tweets.append(td);
    }
    qInfo() << "Loaded" << tweets.count() << "tweets from JSON resource.";
}

// --- Populate Filter UI Controls ---
void MainWindow::populateFilterUI()
{
    if (!filterWidget || tweets.isEmpty()) return; // Safety checks

    // Get the layout of the filter container widget
    QHBoxLayout *layout = qobject_cast<QHBoxLayout*>(filterWidget->layout());
    if (!layout) { // Should exist from setupUi, but create if missing
        layout = new QHBoxLayout(filterWidget);
        qWarning() << "Filter widget layout was missing, created new one.";
    }

    // --- Clear previous filter UI elements ---
    // Delete direct child widgets of filterWidget to remove old controls
    qDeleteAll(filterWidget->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly));
    // Reset pointers to UI elements (Qt handles deletion via parentage)
    authorComboBox = nullptr;
    ugenComboBox = nullptr;
    sonicComboBox = nullptr;
    complexityComboBox = nullptr;
    favoriteFilterButton = nullptr;
    favoritesButtonGroup = nullptr;
    resetFiltersButton = nullptr;
    authorLabel = nullptr;
    ugenLabel = nullptr;
    sonicLabel = nullptr;
    complexityLabel = nullptr;
    favoriteLabel = nullptr;

    // --- Prepare Sets for Unique Filter Items ---
    QSet<QString> authors;
    QSet<QString> ugens; // Will be populated from code analysis
    QSet<QString> sonics; // Populated from JSON tags
    QSet<QString> complexities; // Populated from JSON tags

    // --- Regular Expressions for UGen Extraction ---
    // Pattern 1: UgenName.method or UgenName(
    QRegularExpression ugenMethodRegex(R"(\b([A-Z][a-zA-Z0-9]*)(?:\.(?:ar|kr|ir|new)\b|\())");
    // Pattern 2: method(UgenName, ...) or method(UgenName)
    QRegularExpression ugenFuncRegex(R"(\b(?:ar|kr|ir|new)\b\s*\(\s*([A-Z][a-zA-Z0-9]*)\b)");

    // --- Iterate Through Tweets to Extract Filter Items ---
    for (const auto& tweet : tweets) {
        // Add author to set
        authors.insert(tweet.author.isEmpty() ? "Unknown" : tweet.author);

        // Extract UGens from code using both regex patterns
        QRegularExpressionMatchIterator i1 = ugenMethodRegex.globalMatch(tweet.originalCode);
        while (i1.hasNext()) {
            QRegularExpressionMatch match = i1.next();
            QString potentialUgen = match.captured(1);
            if (!potentialUgen.isEmpty()) ugens.insert(potentialUgen);
        }
        QRegularExpressionMatchIterator i2 = ugenFuncRegex.globalMatch(tweet.originalCode);
        while (i2.hasNext()) {
            QRegularExpressionMatch match = i2.next();
            QString potentialUgen = match.captured(1);
            if (!potentialUgen.isEmpty()) ugens.insert(potentialUgen);
        }

        // Classify JSON tags into Sonic and Complexity/Tag categories
        for (const QString& tag : tweet.tags) {
            QString lowerTag = tag.toLower();
            // Define classification rules (adjust as needed)
            if (lowerTag == "drone" || lowerTag == "ambient" || lowerTag == "noise" || lowerTag == "glitch" || lowerTag == "percussive" || lowerTag == "melodic" || lowerTag == "texture" || lowerTag == "rhythm" || lowerTag == "bass" || lowerTag == "lead" || lowerTag == "pad" || lowerTag == "sfx" || lowerTag == "sequence" || lowerTag == "animal" || lowerTag == "space" || lowerTag == "harmonic" || lowerTag == "atonal") {
                sonics.insert(tag);
            }
            else if (lowerTag == "simple" || lowerTag == "complex" || lowerTag == "one-liner" || lowerTag == "generative" || lowerTag == "feedback" || lowerTag == "filter" || lowerTag == "delay" || lowerTag == "reverb" || lowerTag == "random" || lowerTag == "pattern" || lowerTag == "buffer" || lowerTag == "chaos" || lowerTag == "pitchshift" || lowerTag == "granular") {
                 complexities.insert(tag); // Consider renaming this category label later if needed
            }
             // Don't add tags to 'ugens' set here; it's populated from code only.
        }
    }

    // --- Helper Lambda to Create Label + ComboBox Pairs ---
    auto createFilterCombo = [&](const QString& labelText, const QSet<QString>& items, QComboBox*& comboBoxPtr, QLabel*& labelPtr) {
        if (items.isEmpty()) { // Don't create if no items found for this category
            comboBoxPtr = nullptr;
            labelPtr = nullptr;
            return;
        }
        // Create label and combo box, parented to filterWidget
        labelPtr = new QLabel(labelText + ":", filterWidget);
        comboBoxPtr = new QComboBox(filterWidget);
        comboBoxPtr->setObjectName("FilterCombo_" + labelText.simplified().replace(" ", "_")); // For styling/testing

        // Sort items for display
        QStringList sortedItems = items.values();
        sortedItems.sort(Qt::CaseInsensitive);

        // Add default "All" item and the sorted list
        comboBoxPtr->addItem("All"); // Index 0 is "All"
        comboBoxPtr->addItems(sortedItems);

        // Connect signal to update filters when selection changes
        connect(comboBoxPtr, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &MainWindow::applyFilters);

        // Add label and combo box to the filter layout
        layout->addWidget(labelPtr);
        layout->addWidget(comboBoxPtr);
    };

    // --- Create Filter Controls using the Lambda ---
    createFilterCombo("Author", authors, authorComboBox, authorLabel);
    createFilterCombo("UGen", ugens, ugenComboBox, ugenLabel);
    createFilterCombo("Sonic", sonics, sonicComboBox, sonicLabel);
    createFilterCombo("Tag", complexities, complexityComboBox, complexityLabel); // "Tag" label for complexity category

    // --- Create Favorite Filter Button ---
    favoriteLabel = new QLabel("Show:", filterWidget);
    favoriteFilterButton = new QPushButton("Favorites Only", filterWidget);
    favoriteFilterButton->setCheckable(true); // Make it a toggle button
    favoriteFilterButton->setObjectName("FilterButton_Favorite");
    // Use a button group for easier state management (optional for single button)
    favoritesButtonGroup = new QButtonGroup(filterWidget);
    favoritesButtonGroup->setExclusive(false); // Allow unchecking
    favoritesButtonGroup->addButton(favoriteFilterButton);
    // Connect toggle signal to update filters
    connect(favoriteFilterButton, &QPushButton::toggled, this, &MainWindow::applyFilters);

    // --- Create Reset Button ---
    resetFiltersButton = new QPushButton("Reset Filters", filterWidget);
    resetFiltersButton->setObjectName("ResetFiltersButton");
    resetFiltersButton->setToolTip("Reset all dropdown filters and favorite toggle");
    // Connect click signal to the reset slot
    connect(resetFiltersButton, &QPushButton::clicked, this, &MainWindow::resetFilters);

    // --- Add Buttons to Layout ---
    layout->addStretch(1); // Add stretchable space to push buttons to the right
    layout->addWidget(favoriteLabel);
    layout->addWidget(favoriteFilterButton);
    layout->addWidget(resetFiltersButton);


    qInfo() << "Populated filter UI. Found" << ugens.count() << "unique potential UGens.";
}

// --- Apply Filters Based on Current Control States ---
void MainWindow::applyFilters()
{
    if (!tweetListWidget) return; // Safety check

    // Store current selection to try and restore it after repopulating
    QString previousSelectionText = tweetListWidget->currentItem() ? tweetListWidget->currentItem()->text() : "";

    tweetListWidget->clear(); // Clear the list before applying filters

    // --- Get Active Filter Values ---
    QString authorFilter = (authorComboBox && authorComboBox->currentIndex() > 0)
                               ? authorComboBox->currentText() // Get selected text if not "All"
                               : QString(); // Empty string means no filter applied for this category
    QString ugenFilter = (ugenComboBox && ugenComboBox->currentIndex() > 0)
                             ? ugenComboBox->currentText()
                             : QString();
    QString sonicFilter = (sonicComboBox && sonicComboBox->currentIndex() > 0)
                              ? sonicComboBox->currentText()
                              : QString();
    QString tagFilter = (complexityComboBox && complexityComboBox->currentIndex() > 0)
                             ? complexityComboBox->currentText()
                             : QString();
    bool favoritesOnly = (favoriteFilterButton && favoriteFilterButton->isChecked());
    QString searchText = searchLineEdit ? searchLineEdit->text() : QString();

    // --- Pre-compile Regexes for UGen Check (if needed) ---
    QRegularExpression ugenMethodCheckRegex;
    QRegularExpression ugenFuncCheckRegex;
    if (!ugenFilter.isEmpty()) {
        QString escapedUgen = QRegularExpression::escape(ugenFilter); // Handle special chars in UGen names
        ugenMethodCheckRegex.setPattern(QString(R"(\b%1\b(?:\.(?:ar|kr|ir|new)\b|\())").arg(escapedUgen));
        ugenFuncCheckRegex.setPattern(QString(R"(\b(?:ar|kr|ir|new)\b\s*\(\s*%1\b)").arg(escapedUgen));
    }

    // --- Log Current Filters (for debugging) ---
    qDebug() << "Applying filters:"
             << "Search:" << searchText
             << "Author:" << (authorFilter.isEmpty() ? "All" : authorFilter)
             << "UGen:" << (ugenFilter.isEmpty() ? "All" : ugenFilter)
             << "Sonic:" << (sonicFilter.isEmpty() ? "All" : sonicFilter)
             << "Tag:" << (tagFilter.isEmpty() ? "All" : tagFilter)
             << "FavsOnly:" << favoritesOnly;

    // --- Iterate Through All Tweets and Check Filters ---
    QStringList namesToAdd; // List to hold names of tweets that pass filters
    for (const auto& tweet : tweets) {
        bool passesFilter = true; // Assume passes initially

        // Apply Global Search Filter
        if (passesFilter && !searchText.isEmpty() && !tweet.name.contains(searchText, Qt::CaseInsensitive)) {
             passesFilter = false;
        }
        // Apply Author Filter
        if (passesFilter && !authorFilter.isEmpty()) {
             if (authorFilter == "Unknown" && !tweet.author.isEmpty()) passesFilter = false;
             else if (authorFilter != "Unknown" && tweet.author != authorFilter) passesFilter = false;
        }
        // Apply UGen Filter (check code using pre-compiled regexes)
        if (passesFilter && !ugenFilter.isEmpty()) {
            bool foundMethodSyntax = ugenMethodCheckRegex.match(tweet.originalCode).hasMatch();
            bool foundFuncSyntax = ugenFuncCheckRegex.match(tweet.originalCode).hasMatch();
            if (!foundMethodSyntax && !foundFuncSyntax) { // Must match at least one syntax
                passesFilter = false;
            }
        }
        // Apply Sonic Filter (check JSON tags)
        if (passesFilter && !sonicFilter.isEmpty() && !tweet.tags.contains(sonicFilter, Qt::CaseInsensitive)) {
            passesFilter = false;
        }
         // Apply Tag Filter (check JSON tags)
        if (passesFilter && !tagFilter.isEmpty() && !tweet.tags.contains(tagFilter, Qt::CaseInsensitive)) {
            passesFilter = false;
        }
        // Apply Favorite Filter
        if (passesFilter && favoritesOnly && !isFavorite(tweet.name)) {
            passesFilter = false;
        }

        // If tweet passed all active filters, add its name to the list
        if (passesFilter) {
            namesToAdd.append(tweet.name);
        }
    }

    // --- Populate List Widget with Filtered Results ---
    namesToAdd.sort(Qt::CaseInsensitive); // Sort names alphabetically
    QListWidgetItem* itemToSelect = nullptr; // Pointer to restore selection
    for (const QString &name : namesToAdd) {
        QListWidgetItem* newItem = new QListWidgetItem(name, tweetListWidget); // Create item, parented to list
        // Add favorite icon if applicable
        if (isFavorite(name)) {
             newItem->setIcon(QIcon::fromTheme("emblem-favorite", QIcon(":/icons/favorite.png"))); // Use theme icon or fallback
        }
        // Check if this is the item that was previously selected
        if (name == previousSelectionText) {
            itemToSelect = newItem;
        }
    }

    // --- Restore Selection or Select First/None ---
    if (itemToSelect) {
        tweetListWidget->setCurrentItem(itemToSelect); // Restore previous selection
    } else if (tweetListWidget->count() > 0) {
        tweetListWidget->setCurrentRow(0); // Select first item if list not empty
    } else {
         displayCode(nullptr); // Clear code/meta panes if list is now empty
    }

     qInfo() << "Filters applied, list count:" << tweetListWidget->count();
}

// --- Slot for Arrow Key Navigation from Search Field ---
void MainWindow::onSearchNavigateKey(QKeyEvent *event)
{
    // If Down or Up pressed in search field, focus the list widget
    if (tweetListWidget && tweetListWidget->count() > 0) {
        tweetListWidget->setFocus(); // Give focus to the list
        // Select first item if nothing is currently selected in the list
        if (!tweetListWidget->currentItem()) {
            tweetListWidget->setCurrentRow(0);
        }
        // QListWidget handles subsequent Up/Down navigation internally
    }
}

// --- Slot for Search Text Changes ---
void MainWindow::onSearchTextChanged(const QString &searchText)
{
    // Re-apply all filters whenever the global search text changes
    applyFilters();
}

// --- Display Metadata in the Text Edit ---
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
    metadataString += QLatin1Char('\n');

    metadataTextEdit->setText(metadataString);
}

// --- Display Code (or clear) in the Text Edit ---
void MainWindow::displayCode(const TweetData* tweet) {
    if (!codeTextEdit || !metadataTextEdit) return; // Safety checks

    if (tweet) { // If a valid tweet pointer is passed
        codeTextEdit->setText(tweet->originalCode); // Display code
        displayMetadata(*tweet); // Display metadata (dereference pointer)
    } else { // If null pointer (e.g., no selection, empty list)
        codeTextEdit->clear(); // Clear code area
        codeTextEdit->setPlaceholderText("Select a Tweet or adjust filters."); // Show placeholder text
        metadataTextEdit->clear(); // Clear metadata area
        metadataTextEdit->setPlaceholderText("Select a Tweet to view its metadata."); // Show placeholder text
    }
}

// --- Load Favorites from Settings ---
void MainWindow::loadFavorites() {
    if(!settings) return;
    // Retrieve "favorites" value, defaulting to empty list if not found
    QStringList favList = settings->value("favorites", QStringList()).toStringList();
    // Construct the QSet from the loaded list
    favoriteTweetNames = QSet<QString>(favList.begin(), favList.end());
    qInfo() << "Loaded" << favoriteTweetNames.count() << "favorites.";
}

// --- Save Favorites to Settings ---
void MainWindow::saveFavorites() {
    if(!settings) return;
    // Convert the QSet back to a QStringList for storing
    settings->setValue("favorites", QStringList(favoriteTweetNames.begin(), favoriteTweetNames.end()));
    qInfo() << "Saved" << favoriteTweetNames.count() << "favorites.";
    settings->sync(); // Ensure data is written to persistent storage
}

// --- Check if a Tweet is a Favorite ---
bool MainWindow::isFavorite(const QString& tweetName) const {
    return favoriteTweetNames.contains(tweetName);
}

// --- Toggle Favorite Status of Selected Tweet ---
void MainWindow::toggleFavorite() {
    if (!tweetListWidget) return; // Safety check
    QListWidgetItem* currentItem = tweetListWidget->currentItem(); // Get selected list item
    if (!currentItem) {
         qWarning() << "Cannot toggle favorite: No item selected in list.";
         return; // Do nothing if no item is selected
    }

    QString tweetName = currentItem->text(); // Get the name from the list item

    // Find the corresponding TweetData object (needed for metadata refresh)
    const TweetData* currentTweetData = nullptr;
     for (const auto& tweet : tweets) {
        if (tweet.name == tweetName) {
            currentTweetData = &tweet;
            break;
        }
    }
     if (!currentTweetData) {
          qWarning() << "Cannot toggle favorite: Could not find data for" << tweetName;
          return; // Should not happen if list populated correctly
     }

    // Add/Remove from the favorites set and update icon
    bool wasFavorite = isFavorite(tweetName);
    if (wasFavorite) {
        favoriteTweetNames.remove(tweetName);
        currentItem->setIcon(QIcon()); // Remove icon
        qInfo() << "Removed favorite:" << tweetName;
    } else {
        favoriteTweetNames.insert(tweetName);
        // Set icon (using theme icon with fallback)
        currentItem->setIcon(QIcon::fromTheme("emblem-favorite", QIcon(":/icons/favorite.png")));
        qInfo() << "Added favorite:" << tweetName;
    }

    saveFavorites(); // Persist the change

    // Refresh the metadata display to show updated favorite status
    displayMetadata(*currentTweetData);

    // If the "Favorites Only" filter is active and we *removed* a favorite,
    // we need to re-apply filters to potentially remove the item from the list.
    if (wasFavorite && favoriteFilterButton && favoriteFilterButton->isChecked()) {
        applyFilters();
    }
    // No need to refresh list if adding a favorite while Favs Only is active
}

// --- Slot for List Selection Changes ---
void MainWindow::onTweetSelectionChanged()
{
    // Ensure widgets exist
    if (!tweetListWidget || !codeTextEdit || !metadataTextEdit) {
         // Clear displays if widgets are missing (shouldn't happen in normal operation)
         if(codeTextEdit) codeTextEdit->clear();
         if(metadataTextEdit) metadataTextEdit->clear();
         return;
    }

    QListWidgetItem *currentItem = tweetListWidget->currentItem(); // Get currently selected item
    if (!currentItem) {
        displayCode(nullptr); // No selection, clear code/meta panes
        return;
    }

    QString selectedName = currentItem->text(); // Get name of selected tweet
    const TweetData* foundTweet = nullptr;

    // Find the corresponding TweetData in our master list
    for (const auto& tweet : tweets) {
        if (tweet.name == selectedName) {
            foundTweet = &tweet;
            break;
        }
    }

    // Display the found tweet's code/meta (or clear if somehow not found)
    displayCode(foundTweet);
}

// --- Reset Filters Slot ---
void MainWindow::resetFilters()
{
    qInfo() << "Resetting filters...";

    // Block signals to prevent multiple applyFilters calls
    bool authorBlocked = authorComboBox ? authorComboBox->signalsBlocked() : false;
    bool ugenBlocked = ugenComboBox ? ugenComboBox->signalsBlocked() : false;
    bool sonicBlocked = sonicComboBox ? sonicComboBox->signalsBlocked() : false;
    bool complexityBlocked = complexityComboBox ? complexityComboBox->signalsBlocked() : false;
    bool favBlocked = favoriteFilterButton ? favoriteFilterButton->signalsBlocked() : false;

    if (authorComboBox) authorComboBox->blockSignals(true);
    if (ugenComboBox) ugenComboBox->blockSignals(true);
    if (sonicComboBox) sonicComboBox->blockSignals(true);
    if (complexityComboBox) complexityComboBox->blockSignals(true);
    if (favoriteFilterButton) favoriteFilterButton->blockSignals(true);

    // Set ComboBoxes to index 0 ("All")
    if (authorComboBox) authorComboBox->setCurrentIndex(0);
    if (ugenComboBox) ugenComboBox->setCurrentIndex(0);
    if (sonicComboBox) sonicComboBox->setCurrentIndex(0);
    if (complexityComboBox) complexityComboBox->setCurrentIndex(0);

    // Uncheck Favorite button
    if (favoriteFilterButton) favoriteFilterButton->setChecked(false);

    // Unblock signals
    if (authorComboBox) authorComboBox->blockSignals(authorBlocked);
    if (ugenComboBox) ugenComboBox->blockSignals(ugenBlocked);
    if (sonicComboBox) sonicComboBox->blockSignals(sonicBlocked);
    if (complexityComboBox) complexityComboBox->blockSignals(complexityBlocked);
    if (favoriteFilterButton) favoriteFilterButton->blockSignals(favBlocked);

    // Apply the cleared filters manually once
    applyFilters();
}