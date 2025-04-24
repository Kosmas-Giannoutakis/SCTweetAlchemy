#include "mainwindow.h"
#include "searchlineedit.h"

#include <QtWidgets>
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
#include <QScrollArea>
#include <QGroupBox>
#include <QCheckBox>
#include <QLabel>
#include <QRegularExpression>
#include <QSet> // Include QSet


// --- Constructor ---
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , searchLineEdit(nullptr)
    , mainSplitter(nullptr)
    , filterPanel(nullptr)
    , filterScrollArea(nullptr)
    , filterScrollWidget(nullptr)
    , filterScrollLayout(nullptr)
    , tweetListWidget(nullptr)
    , rightPanel(nullptr)
    , codeTextEdit(nullptr)
    , metadataTextEdit(nullptr)
    , filterLogicToggle(nullptr)
    , favoriteFilterButton(nullptr)
    , resetFiltersButton(nullptr)
    , focusSearchAction(nullptr)
    , toggleFavoriteAction(nullptr)
{
    // Clear checkbox lists initially
    authorCheckboxes.clear();
    sonicCheckboxes.clear();
    techniqueCheckboxes.clear();
    ugenCheckboxes.clear();

    // Settings Initialization
    QCoreApplication::setOrganizationName("Kosmas");
    QCoreApplication::setApplicationName("SCTweetAlchemy");
    settings = new QSettings(this);

    // Setup Steps
    setupUi();
    setupActions();
    loadFavorites();
    loadTweets(); // Reads new JSON structure
    populateFilterUI(); // Uses new data structure

    // Connect Signals
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
    // Connections for checkboxes, favorite button, reset button, logic toggle are done in populateFilterUI

    // Initial State
    applyFilters();

    if (tweetListWidget && tweetListWidget->count() > 0) {
       tweetListWidget->setCurrentRow(0);
    } else {
        if(codeTextEdit) codeTextEdit->setPlaceholderText("No tweets found or match filters.");
        if(metadataTextEdit) metadataTextEdit->setPlaceholderText("");
         onTweetSelectionChanged();
    }

    if(tweetListWidget) {
        tweetListWidget->setFocus();
    }
}

// --- Destructor ---
MainWindow::~MainWindow() { }

// --- Action Setup ---
void MainWindow::setupActions()
{
    if (searchLineEdit) {
        focusSearchAction = new QAction("Focus Search", this);
        focusSearchAction->setShortcut(QKeySequence::Find);
        focusSearchAction->setToolTip("Focus the search field (Ctrl+F)");
        connect(focusSearchAction, &QAction::triggered, this, &MainWindow::focusSearchField);
        this->addAction(focusSearchAction);
    } else {
        qWarning() << "Search Line Edit is null, cannot create focus search action.";
    }

    toggleFavoriteAction = new QAction("Toggle Favorite", this);
    toggleFavoriteAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    toggleFavoriteAction->setToolTip("Mark/Unmark selected tweet in list as favorite (Ctrl+D)");
    connect(toggleFavoriteAction, &QAction::triggered, this, &MainWindow::toggleFavorite);
    this->addAction(toggleFavoriteAction);
}

// --- Slot to Focus Search Field ---
void MainWindow::focusSearchField()
{
    if (searchLineEdit) {
        searchLineEdit->setFocus();
        searchLineEdit->selectAll();
    }
}

// --- Helper to Create Right Panel ---
QWidget* MainWindow::createRightPanel()
{
    QWidget* codePanel = new QWidget(this);
    QVBoxLayout* codeLayout = new QVBoxLayout(codePanel);
    codeLayout->setContentsMargins(0, 2, 0, 0); codeLayout->setSpacing(3);
    QLabel* codeTitleLabel = new QLabel("Code", codePanel);
    QFont titleFont = codeTitleLabel->font(); titleFont.setBold(true);
    codeTitleLabel->setFont(titleFont);
    codeTextEdit = new QTextEdit(this);
    codeTextEdit->setReadOnly(true); codeTextEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    codeTextEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    codeTextEdit->setObjectName("codeTextEdit");
    codeLayout->addWidget(codeTitleLabel); codeLayout->addWidget(codeTextEdit);

    QWidget* metadataPanel = new QWidget(this);
    QVBoxLayout* metadataLayout = new QVBoxLayout(metadataPanel);
    metadataLayout->setContentsMargins(0, 2, 0, 0); metadataLayout->setSpacing(3);
    QLabel* metadataTitleLabel = new QLabel("Metadata", metadataPanel);
    metadataTitleLabel->setFont(titleFont);
    metadataTextEdit = new QTextEdit(this);
    metadataTextEdit->setReadOnly(true); metadataTextEdit->setObjectName("metadataTextEdit");
    metadataLayout->addWidget(metadataTitleLabel); metadataLayout->addWidget(metadataTextEdit);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    splitter->addWidget(codePanel); splitter->addWidget(metadataPanel);
    splitter->setStretchFactor(0, 4); splitter->setStretchFactor(1, 1);
    return splitter;
}

// --- Setup Overall UI Layout (Adjusted Splitter Ratios 5:1:5) ---
void MainWindow::setupUi()
{
    searchLineEdit = new SearchLineEdit(this);
    searchLineEdit->setPlaceholderText("Search Tweets (Global)...");
    searchLineEdit->setClearButtonEnabled(true);

    // Column 1: Filter Panel
    filterPanel = new QWidget(this);
    QVBoxLayout* filterPanelLayout = new QVBoxLayout(filterPanel);
    filterPanelLayout->setContentsMargins(0,0,0,0); filterPanelLayout->setSpacing(0);
    filterScrollArea = new QScrollArea(filterPanel);
    filterScrollArea->setWidgetResizable(true); filterScrollArea->setFrameShape(QFrame::NoFrame);
    filterScrollWidget = new QWidget();
    filterScrollLayout = new QVBoxLayout(filterScrollWidget);
    filterScrollLayout->setContentsMargins(5, 5, 5, 5); filterScrollLayout->setSpacing(6);
    // Stretch will be added last in populateFilterUI
    filterScrollArea->setWidget(filterScrollWidget);
    filterPanelLayout->addWidget(filterScrollArea);

    // Column 2: Tweet List
    tweetListWidget = new QListWidget(this);
    tweetListWidget->setObjectName("tweetListWidget");

    // Column 3: Code/Metadata Panel
    rightPanel = createRightPanel();

    // Main Horizontal Splitter
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->setObjectName("mainSplitter");
    mainSplitter->addWidget(filterPanel);      // Col 1
    mainSplitter->addWidget(tweetListWidget);  // Col 2
    mainSplitter->addWidget(rightPanel);       // Col 3

    // Set Initial Splitter Sizes (Adjusted Ratios: 5:1:5)
    mainSplitter->setStretchFactor(0, 5); // Filter column stretch << WIDER
    mainSplitter->setStretchFactor(1, 1); // List column stretch   << NARROWER
    mainSplitter->setStretchFactor(2, 5); // Code/Meta column stretch

    // Overall Window Layout
    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(searchLineEdit);
    centralLayout->addWidget(mainSplitter);
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(centralLayout);
    setCentralWidget(centralWidget);

    setWindowTitle("SCTweetAlchemy Paster");
    resize(1600, 850); // Keep overall window size
}

// --- Load Tweets (Reads new JSON structure with DEBUGGING) ---
void MainWindow::loadTweets()
{
    QString resourcePath = ":/data/SCTweets.json";
    QFile jsonFile(resourcePath);
    qInfo() << "Attempting to load tweets from resource:" << resourcePath;

    if (!jsonFile.exists() || !jsonFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open or find resource" << resourcePath;
        QMessageBox::critical(this, "Load Error", "Could not open application resource:\n" + resourcePath);
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
    tweets.clear();

    for (auto it = rootObj.constBegin(); it != rootObj.constEnd(); ++it) {
        QString key = it.key();
        QJsonValue value = it.value();
        if (!value.isObject()) { continue; }
        QJsonObject tweetObj = value.toObject();
        if (!tweetObj.contains("original") || !tweetObj["original"].isString()) { continue; }

        TweetData td; // Create new data struct instance
        td.name = key;
        td.originalCode = tweetObj["original"].toString();
        td.author = tweetObj.value("author").toString("Unknown");
        td.sourceUrl = tweetObj.value("source_url").toString("");
        td.description = tweetObj.value("description").toString("-");
        td.publicationDate = tweetObj.value("publication_date").toString("unknown");

        // --- Read Categorized Tags from "classification" object ---
        if (tweetObj.contains("classification") && tweetObj["classification"].isObject()) {
            QJsonObject classificationObj = tweetObj["classification"].toObject();
            // qDebug() << "Tweet [" << key << "]: Found 'classification' object."; // DEBUG

            // Read Sonic Characteristics into td.sonicTags
            if (classificationObj.contains("sonic_characteristics") && classificationObj["sonic_characteristics"].isArray()) {
                QJsonArray sonicArray = classificationObj["sonic_characteristics"].toArray();
                for (const QJsonValue &tagVal : sonicArray) {
                    if (tagVal.isString()) td.sonicTags.append(tagVal.toString());
                }
                 if(td.sonicTags.isEmpty()){
                     // qDebug() << "Tweet [" << key << "]: 'sonic_characteristics' array was empty."; // DEBUG
                 } else {
                    // qDebug() << "Tweet [" << key << "]: Loaded sonic tags:" << td.sonicTags; // DEBUG
                 }
            } else {
                 qDebug() << "Tweet [" << key << "]: Missing or invalid 'sonic_characteristics' array."; // DEBUG
            }

            // Read Synthesis Techniques into td.techniqueTags
            if (classificationObj.contains("synthesis_techniques") && classificationObj["synthesis_techniques"].isArray()) {
                QJsonArray techArray = classificationObj["synthesis_techniques"].toArray();
                for (const QJsonValue &tagVal : techArray) {
                    if (tagVal.isString()) td.techniqueTags.append(tagVal.toString());
                }
                 if(td.techniqueTags.isEmpty()){
                     // qDebug() << "Tweet [" << key << "]: 'synthesis_techniques' array was empty."; // DEBUG
                 } else {
                    // qDebug() << "Tweet [" << key << "]: Loaded technique tags:" << td.techniqueTags; // DEBUG
                 }
            } else {
                 qDebug() << "Tweet [" << key << "]: Missing or invalid 'synthesis_techniques' array."; // DEBUG
            }
        } else {
            qDebug() << "Tweet [" << key << "]: Missing 'classification' object."; // DEBUG
        }


        // Read original flat tags (optional, for display/fallback)
        if (tweetObj.contains("tags") && tweetObj["tags"].isArray()) {
            QJsonArray tagsArray = tweetObj["tags"].toArray();
            for (const QJsonValue &tagVal : tagsArray) { if (tagVal.isString()) td.tags.append(tagVal.toString()); }
        }

        tweets.append(td); // Add the populated struct to the main vector
    }
    qInfo() << "Loaded" << tweets.count() << "tweets from JSON resource.";
}


// --- Populate Filter UI Controls (Using Categorized Tags) ---
void MainWindow::populateFilterUI()
{
    if (!filterScrollLayout || tweets.isEmpty()) { /* ... checks ... */ return; }

    // Clear Previous Controls...
    qDeleteAll(filterScrollWidget->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly));
    authorCheckboxes.clear(); sonicCheckboxes.clear(); techniqueCheckboxes.clear(); ugenCheckboxes.clear();
    filterLogicToggle = nullptr; favoriteFilterButton = nullptr; resetFiltersButton = nullptr;

    // --- Prepare Sets for Unique Filter Items ---
    QSet<QString> authors;
    QSet<QString> sonics;       // Populated from td.sonicTags
    QSet<QString> techniques;   // Populated from td.techniqueTags
    QSet<QString> ugens;        // Populated from code analysis

    // Regexes for UGen Extraction...
    QRegularExpression ugenMethodRegex(R"(\b([A-Z][a-zA-Z0-9]*)(?:\.(?:ar|kr|ir|new)\b|\())");
    QRegularExpression ugenFuncRegex(R"(\b(?:ar|kr|ir|new)\b\s*\(\s*([A-Z][a-zA-Z0-9]*)\b)");

    // --- Iterate Through Tweets to Extract Filter Items ---
    for (const auto& tweet : tweets) { // Iterate through loaded TweetData vector
        authors.insert(tweet.author.isEmpty() ? "Unknown" : tweet.author);

        // Extract UGens from code (remains the same)
        QRegularExpressionMatchIterator i1 = ugenMethodRegex.globalMatch(tweet.originalCode); while (i1.hasNext()) { ugens.insert(i1.next().captured(1)); }
        QRegularExpressionMatchIterator i2 = ugenFuncRegex.globalMatch(tweet.originalCode); while (i2.hasNext()) { ugens.insert(i2.next().captured(1)); }

        // --- Collect categorized tags directly from the struct ---
        for(const QString& tag : tweet.sonicTags) { sonics.insert(tag); }
        for(const QString& tag : tweet.techniqueTags) { techniques.insert(tag); }
    }

    // --- Create Control Buttons (Favorite, Reset, Logic Toggle) FIRST ---
    QWidget* buttonWidget = new QWidget();
    QHBoxLayout* buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->setContentsMargins(0, 0, 0, 4); buttonLayout->setSpacing(6);
    filterLogicToggle = new QCheckBox("Match All", buttonWidget); // RENAMED
    filterLogicToggle->setObjectName("FilterLogicToggle");
    filterLogicToggle->setToolTip("Check to show tweets matching ALL selected criteria.\nUncheck to show tweets matching ANY selected criterion.");
    filterLogicToggle->setChecked(true); connect(filterLogicToggle, &QCheckBox::checkStateChanged, this, &MainWindow::applyFilters);
    favoriteFilterButton = new QPushButton("Favorites", buttonWidget); // RENAMED
    favoriteFilterButton->setCheckable(true); connect(favoriteFilterButton, &QPushButton::toggled, this, &MainWindow::applyFilters);
    resetFiltersButton = new QPushButton("Reset", buttonWidget); // RENAMED
    resetFiltersButton->setToolTip("Reset all filter checkboxes and toggles"); connect(resetFiltersButton, &QPushButton::clicked, this, &MainWindow::resetFilters);
    buttonLayout->addWidget(filterLogicToggle); buttonLayout->addStretch(1);
    buttonLayout->addWidget(favoriteFilterButton); buttonLayout->addWidget(resetFiltersButton);
    filterScrollLayout->insertWidget(0, buttonWidget); // Insert buttons at top

    // --- Helper Lambda to Create Checkbox Groups ---
    auto createCheckboxGroup = [&](const QString& title, const QSet<QString>& items, QList<QCheckBox*>& checkboxList) {
        if (items.isEmpty()) { qDebug() << "No items found for filter group:" << title; return; } // Debug if empty
        QGroupBox *groupBox = new QGroupBox(title);
        QVBoxLayout *groupLayout = new QVBoxLayout(groupBox);
        groupLayout->setContentsMargins(4, 4, 4, 4); groupLayout->setSpacing(4);
        QStringList sortedItems = items.values(); sortedItems.sort(Qt::CaseInsensitive);
        for (const QString& item : sortedItems) {
            QCheckBox *checkbox = new QCheckBox(item);
            checkbox->setObjectName("FilterCheck_" + title.simplified().replace(" ", "_") + "_" + item);
            connect(checkbox, &QCheckBox::checkStateChanged, this, &MainWindow::applyFilters); // Use fixed signal
            groupLayout->addWidget(checkbox); checkboxList.append(checkbox);
        }
        groupLayout->addStretch(1);
        filterScrollLayout->addWidget(groupBox); // Add groupbox below buttons
    };

    // --- Create Checkbox Groups (Specific Order) ---
    createCheckboxGroup("Author", authors, authorCheckboxes);
    createCheckboxGroup("Sonic Characteristic", sonics, sonicCheckboxes);       // Uses collected sonicTags
    createCheckboxGroup("Synthesis Technique", techniques, techniqueCheckboxes); // Uses collected techniqueTags
    createCheckboxGroup("UGen", ugens, ugenCheckboxes);                            // Uses collected ugens

    // --- Add Stretch at the very bottom ---
    QLayoutItem *lastItem = filterScrollLayout->itemAt(filterScrollLayout->count() - 1);
    if (!lastItem || !lastItem->spacerItem()) { filterScrollLayout->addStretch(1); }

    qDebug() << "Populated Filters - Authors:" << authors.size() << "Sonics:" << sonics.size() << "Techniques:" << techniques.size() << "UGens:" << ugens.size();
    qInfo() << "Populated filter UI with checkboxes.";
}

// --- Apply Filters Based on Checkbox States (Reads Categorized Tags) ---
void MainWindow::applyFilters()
{
    if (!tweetListWidget) return;

    QString previousSelectionText = tweetListWidget->currentItem() ? tweetListWidget->currentItem()->text() : "";
    tweetListWidget->clear();

    // Get Checked Items
    auto getChecked = [](const QList<QCheckBox*>& list) -> QStringList {
        QStringList checked;
        for (const QCheckBox* checkbox : list) { if (checkbox && checkbox->isChecked()) { checked.append(checkbox->text()); } }
        return checked;
    };
    QStringList checkedAuthors = getChecked(authorCheckboxes);
    QStringList checkedSonics = getChecked(sonicCheckboxes);
    QStringList checkedTechniques = getChecked(techniqueCheckboxes);
    QStringList checkedUgens = getChecked(ugenCheckboxes);

    // Get Filter Logic State
    bool useAndLogic = filterLogicToggle ? filterLogicToggle->isChecked() : true;

    // Get Other Filters
    bool favoritesOnly = favoriteFilterButton ? favoriteFilterButton->isChecked() : false;
    QString searchText = searchLineEdit ? searchLineEdit->text() : QString();

    // Pre-compile Regexes for checked UGens
    QList<QRegularExpression> ugenMethodCheckRegexes; QList<QRegularExpression> ugenFuncCheckRegexes;
    if (!checkedUgens.isEmpty()) {
        for(const QString& ugen : checkedUgens) {
            QString escapedUgen = QRegularExpression::escape(ugen);
            ugenMethodCheckRegexes.append(QRegularExpression(QString(R"(\b%1\b(?:\.(?:ar|kr|ir|new)\b|\())").arg(escapedUgen)));
            ugenFuncCheckRegexes.append(QRegularExpression(QString(R"(\b(?:ar|kr|ir|new)\b\s*\(\s*%1\b)").arg(escapedUgen)));
        }
    }

    qDebug() << "Applying filters [" << (useAndLogic ? "AND" : "OR") << " logic] - Authors:" << checkedAuthors // ... debug output ...
             << "Sonics:" << checkedSonics << "Techniques:" << checkedTechniques << "UGens:" << checkedUgens
             << "FavsOnly:" << favoritesOnly << "Search:" << searchText;

    // Iterate Through Tweets and Check Filters
    QStringList namesToAdd;
    for (const auto& tweet : tweets) { // tweet is the updated TweetData struct
        bool passesFilter = true;

        // 1. Global Search...
        if (!searchText.isEmpty() && !tweet.name.contains(searchText, Qt::CaseInsensitive)) passesFilter = false;
        // 2. Favorite Filter...
        if (passesFilter && favoritesOnly && !isFavorite(tweet.name)) passesFilter = false;

        // Proceed to tag filters only if passed global filters
        if (passesFilter) {
            bool anyCheckboxesSelected = !checkedAuthors.isEmpty() || !checkedSonics.isEmpty() || !checkedTechniques.isEmpty() || !checkedUgens.isEmpty();

            if (anyCheckboxesSelected) {
                if (useAndLogic) {
                    // --- AND Logic ---
                    // Check Authors
                    if (!checkedAuthors.isEmpty()) {
                         bool matchAll = true;
                         for (const QString& reqAuthor : checkedAuthors) {
                             bool currentAuthorMatch = (reqAuthor == "Unknown") ? tweet.author.isEmpty() : (tweet.author == reqAuthor);
                             if (!currentAuthorMatch) { matchAll = false; break; }
                         }
                         if (!matchAll) passesFilter = false;
                    }
                    // Check UGens
                    if (passesFilter && !checkedUgens.isEmpty()) {
                         bool matchAll = true;
                         for (int i = 0; i < checkedUgens.size(); ++i) {
                            if (i >= ugenMethodCheckRegexes.size() || i >= ugenFuncCheckRegexes.size()) { qWarning() << "Regex list size mismatch!"; passesFilter = false; break; }
                            bool ugenFound = ugenMethodCheckRegexes[i].match(tweet.originalCode).hasMatch() || ugenFuncCheckRegexes[i].match(tweet.originalCode).hasMatch();
                            if (!ugenFound) { matchAll = false; break; }
                        }
                         if (!matchAll) passesFilter = false;
                    }
                    // Check Sonics (Use tweet.sonicTags)
                    if (passesFilter && !checkedSonics.isEmpty()) {
                         for (const QString& reqSonic : checkedSonics) {
                            if (!tweet.sonicTags.contains(reqSonic, Qt::CaseInsensitive)) { passesFilter = false; break; }
                         }
                    }
                    // Check Techniques (Use tweet.techniqueTags)
                    if (passesFilter && !checkedTechniques.isEmpty()) {
                         for (const QString& reqTechnique : checkedTechniques) {
                            if (!tweet.techniqueTags.contains(reqTechnique, Qt::CaseInsensitive)) { passesFilter = false; break; }
                         }
                    }
                } else {
                    // --- OR Logic ---
                    bool orMatchFound = false;
                    // Check Authors
                    if (!checkedAuthors.isEmpty()) {
                         for (const QString& reqAuthor : checkedAuthors) {
                             bool currentAuthorMatch = (reqAuthor == "Unknown") ? tweet.author.isEmpty() : (tweet.author == reqAuthor);
                             if (currentAuthorMatch) { orMatchFound = true; break; }
                         }
                    }
                    // Check UGens
                    if (!orMatchFound && !checkedUgens.isEmpty()) {
                         for (int i = 0; i < checkedUgens.size(); ++i) {
                             if (i >= ugenMethodCheckRegexes.size() || i >= ugenFuncCheckRegexes.size()) { qWarning() << "Regex list size mismatch!"; break; }
                             bool ugenFound = ugenMethodCheckRegexes[i].match(tweet.originalCode).hasMatch() || ugenFuncCheckRegexes[i].match(tweet.originalCode).hasMatch();
                             if (ugenFound) { orMatchFound = true; break; }
                         }
                    }
                    // Check Sonics (Use tweet.sonicTags)
                    if (!orMatchFound && !checkedSonics.isEmpty()) {
                         for (const QString& reqSonic : checkedSonics) {
                             if (tweet.sonicTags.contains(reqSonic, Qt::CaseInsensitive)) { orMatchFound = true; break; }
                         }
                    }
                    // Check Techniques (Use tweet.techniqueTags)
                    if (!orMatchFound && !checkedTechniques.isEmpty()) {
                         for (const QString& reqTechnique : checkedTechniques) {
                             if (tweet.techniqueTags.contains(reqTechnique, Qt::CaseInsensitive)) { orMatchFound = true; break; }
                         }
                    }
                    // If no OR match was found across all active categories, it fails
                    if (!orMatchFound) { passesFilter = false; }
                } // End OR Logic
            } // End if (anyCheckboxesSelected)
        } // End if (passes global filters)

        // --- Add if Passed All Filters ---
        if (passesFilter) { namesToAdd.append(tweet.name); }
    }

    // Populate List Widget
    namesToAdd.sort(Qt::CaseInsensitive);
    QListWidgetItem* itemToSelect = nullptr;
    for (const QString &name : namesToAdd) {
        QListWidgetItem* newItem = new QListWidgetItem(name, tweetListWidget);
        if (isFavorite(name)) { newItem->setIcon(QIcon::fromTheme("emblem-favorite", QIcon(":/icons/favorite.png"))); }
        if (name == previousSelectionText) { itemToSelect = newItem; }
    }
    if (itemToSelect) { tweetListWidget->setCurrentItem(itemToSelect); }
    else if (tweetListWidget->count() > 0) { tweetListWidget->setCurrentRow(0); }
    else { displayCode(nullptr); }
    qInfo() << "Filters applied [" << (useAndLogic ? "AND" : "OR") << " logic], list count:" << tweetListWidget->count();
}

// --- Slot for Arrow Key Navigation from Search Field ---
void MainWindow::onSearchNavigateKey(QKeyEvent *event)
{
    if (tweetListWidget && tweetListWidget->count() > 0) {
        tweetListWidget->setFocus();
        if (!tweetListWidget->currentItem()) { tweetListWidget->setCurrentRow(0); }
    }
}

// --- Slot for Search Text Changes ---
void MainWindow::onSearchTextChanged(const QString &searchText) { applyFilters(); }

// --- Display Metadata (UPDATED for new structure) ---
void MainWindow::displayMetadata(const TweetData& tweet)
{
    if (!metadataTextEdit) return;
    QString metadataString;
    metadataString += "Author: " + tweet.author + "\n";
    metadataString += "Source: " + (!tweet.sourceUrl.isEmpty() ? tweet.sourceUrl : QStringLiteral("N/A")) + "\n";
    metadataString += "Publication Date: " + tweet.publicationDate + "\n"; // Display date
    metadataString += "Description: " + tweet.description + "\n\n"; // Add space before tags

    // Display Categorized Tags nicely
    if (!tweet.sonicTags.isEmpty()) {
        metadataString += "Sonic Characteristics: " + tweet.sonicTags.join(", ") + "\n"; // <<< Display sonicTags
    }
    if (!tweet.techniqueTags.isEmpty()) {
        metadataString += "Synthesis Techniques: " + tweet.techniqueTags.join(", ") + "\n"; // <<< Display techniqueTags
    }

    // Display remaining raw tags if they exist and weren't categorized AND not already displayed
    if (!tweet.tags.isEmpty()) {
       QStringList miscTags;
       QSet<QString> displayedTags; // Keep track of tags already shown
       for(const QString& tag : tweet.sonicTags) displayedTags.insert(tag.toLower());
       for(const QString& tag : tweet.techniqueTags) displayedTags.insert(tag.toLower());

       for(const QString& rawTag : tweet.tags) {
           if (!displayedTags.contains(rawTag.toLower())) {
               miscTags.append(rawTag);
           }
       }

       if (!miscTags.isEmpty()) {
           metadataString += "Tags (Other): " + miscTags.join(", ") + "\n";
       }
    }

    metadataString += "\nFavorite: "; // Add space before favorite
    metadataString += (isFavorite(tweet.name) ? QStringLiteral("Yes") : QStringLiteral("No"));
    metadataString += QLatin1Char('\n');
    metadataTextEdit->setText(metadataString);
}

// --- Display Code ---
void MainWindow::displayCode(const TweetData* tweet)
{
    if (!codeTextEdit || !metadataTextEdit) return;
    if (tweet) {
        codeTextEdit->setText(tweet->originalCode);
        displayMetadata(*tweet); // Calls the updated displayMetadata
    } else {
        codeTextEdit->clear(); codeTextEdit->setPlaceholderText("Select a Tweet or adjust filters.");
        metadataTextEdit->clear(); metadataTextEdit->setPlaceholderText("Select a Tweet to view its metadata.");
    }
}

// --- Favorites Management ---
void MainWindow::loadFavorites()
{
    if(!settings) return;
    QStringList favList = settings->value("favorites", QStringList()).toStringList();
    favoriteTweetNames = QSet<QString>(favList.begin(), favList.end());
    qInfo() << "Loaded" << favoriteTweetNames.count() << "favorites.";
}

void MainWindow::saveFavorites()
{
     if(!settings) return;
    settings->setValue("favorites", QStringList(favoriteTweetNames.begin(), favoriteTweetNames.end()));
    qInfo() << "Saved" << favoriteTweetNames.count() << "favorites.";
    settings->sync();
}

bool MainWindow::isFavorite(const QString& tweetName) const { return favoriteTweetNames.contains(tweetName); }

void MainWindow::toggleFavorite()
{
    if (!tweetListWidget) return;
    QListWidgetItem* currentItem = tweetListWidget->currentItem();
    if (!currentItem) { qWarning() << "Cannot toggle favorite: No item selected."; return; }
    QString tweetName = currentItem->text();
    const TweetData* currentTweetData = nullptr;
    for (const auto& tweet : tweets) { if (tweet.name == tweetName) { currentTweetData = &tweet; break; } }
    if (!currentTweetData) { qWarning() << "Cannot toggle favorite: Data not found for" << tweetName; return; }

    bool wasFavorite = isFavorite(tweetName);
    if (wasFavorite) {
        favoriteTweetNames.remove(tweetName); currentItem->setIcon(QIcon());
        qInfo() << "Removed favorite:" << tweetName;
    } else {
        favoriteTweetNames.insert(tweetName);
        currentItem->setIcon(QIcon::fromTheme("emblem-favorite", QIcon(":/icons/favorite.png")));
        qInfo() << "Added favorite:" << tweetName;
    }
    saveFavorites();
    displayMetadata(*currentTweetData);
    if (wasFavorite && favoriteFilterButton && favoriteFilterButton->isChecked()) { applyFilters(); }
}

// --- Slot for List Selection Changes ---
void MainWindow::onTweetSelectionChanged()
{
    if (!tweetListWidget || !codeTextEdit || !metadataTextEdit) { return; }
    QListWidgetItem *currentItem = tweetListWidget->currentItem();
    if (!currentItem) { displayCode(nullptr); return; }
    QString selectedName = currentItem->text();
    const TweetData* foundTweet = nullptr;
    for (const auto& tweet : tweets) { if (tweet.name == selectedName) { foundTweet = &tweet; break; } }
    displayCode(foundTweet);
}

// --- Reset Filters Slot (Also reset logic toggle) ---
void MainWindow::resetFilters()
{
    qInfo() << "Resetting filters...";
    QList<QCheckBox*> allCheckboxes = authorCheckboxes + sonicCheckboxes + techniqueCheckboxes + ugenCheckboxes;

    // Block signals
    bool favBlocked = favoriteFilterButton ? favoriteFilterButton->signalsBlocked() : false;
    bool logicBlocked = filterLogicToggle ? filterLogicToggle->signalsBlocked() : false;
    if (favoriteFilterButton) favoriteFilterButton->blockSignals(true);
    if (filterLogicToggle) filterLogicToggle->blockSignals(true);
    for (QCheckBox* checkbox : allCheckboxes) { checkbox->blockSignals(true); }

    // Uncheck all tag checkboxes
    for (QCheckBox* checkbox : allCheckboxes) { checkbox->setChecked(false); }
    // Reset favorite button
    if (favoriteFilterButton) favoriteFilterButton->setChecked(false);
    // Reset logic toggle (default to AND)
    if (filterLogicToggle) filterLogicToggle->setChecked(true); // Default to AND

    // Unblock signals
    if (favoriteFilterButton) favoriteFilterButton->blockSignals(favBlocked);
    if (filterLogicToggle) filterLogicToggle->blockSignals(logicBlocked);
    for (QCheckBox* checkbox : allCheckboxes) { checkbox->blockSignals(false); }

    applyFilters(); // Apply cleared filters
}