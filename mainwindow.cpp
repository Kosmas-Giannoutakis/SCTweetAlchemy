#include "mainwindow.h"
#include "searchlineedit.h"   // Custom QLineEdit
#include "tweetrepository.h"
#include "favoritesmanager.h"
#include "filterpanelwidget.h"
#include "tweetfilterengine.h"

#include <QtWidgets> // Includes most Qt Widget classes
#include <QJsonDocument> // For completeness, though now in repository
#include <QJsonObject>   // For completeness
#include <QJsonArray>    // For completeness
#include <QFile>         // For completeness
#include <QDir>          // If ever needed for file paths
#include <QMessageBox>
#include <QDebug>
#include <QFontDatabase>
#include <QKeySequence> // For QKeySequence::Find
#include <QScrollArea>  // Though now part of FilterPanelWidget
#include <QGroupBox>    // Though now part of FilterPanelWidget
#include <QCheckBox>    // Though now part of FilterPanelWidget
#include <QLabel>       // For panel titles
#include <QListWidgetItem>

// --- Constructor ---
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_searchLineEdit(nullptr)
    , m_mainSplitter(nullptr)
    , m_filterPanelWidget(nullptr)
    , m_tweetListWidget(nullptr)
    , m_rightPanel(nullptr)
    , m_codeTextEdit(nullptr)
    , m_metadataTextEdit(nullptr)
    , m_focusSearchAction(nullptr)
    , m_toggleFavoriteAction(nullptr)
    , m_settings(nullptr)
    , m_tweetRepository(nullptr)
    , m_favoritesManager(nullptr)
    , m_tweetFilterEngine(nullptr)
{
    QCoreApplication::setOrganizationName("Kosmas");
    QCoreApplication::setApplicationName("SCTweetAlchemy");
    m_settings = new QSettings(this); // MainWindow owns QSettings

    setupModelsAndManagers();
    setupUi();
    setupActions();
    connectSignals();

    // Initial data load and UI population
    if (m_tweetRepository->loadTweets()) { // Default path is in repository
        // tweetsLoaded signal will trigger initial filter population and application
    } else {
        // Error already handled by signal/slot from repository
        if(m_codeTextEdit) m_codeTextEdit->setPlaceholderText("Failed to load tweets.");
        if(m_metadataTextEdit) m_metadataTextEdit->setPlaceholderText("");
    }
     if(m_tweetListWidget) {
        m_tweetListWidget->setFocus(); // Set initial focus
    }
}

// --- Destructor ---
MainWindow::~MainWindow()
{
    // QSettings is child of MainWindow, Qt handles it.
    // Managers are children of MainWindow, Qt handles them if parented.
    // UI elements are children, Qt handles them.
}

void MainWindow::setupModelsAndManagers()
{
    m_tweetRepository = new TweetRepository(this);
    m_favoritesManager = new FavoritesManager(m_settings, this); // Pass QSettings
    m_tweetFilterEngine = new TweetFilterEngine(); // No parent, doesn't need Qt's memory management if simple
}

// --- Action Setup ---
void MainWindow::setupActions()
{
    m_focusSearchAction = new QAction("Focus Search", this);
    m_focusSearchAction->setShortcut(QKeySequence::Find);
    m_focusSearchAction->setToolTip("Focus the search field (Ctrl+F)");
    this->addAction(m_focusSearchAction); // Add to window for shortcut to work globally

    m_toggleFavoriteAction = new QAction("Toggle Favorite", this);
    m_toggleFavoriteAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    m_toggleFavoriteAction->setToolTip("Mark/Unmark selected tweet in list as favorite (Ctrl+D)");
    this->addAction(m_toggleFavoriteAction);
}

// --- Setup Overall UI Layout ---
void MainWindow::setupUi()
{
    m_searchLineEdit = new SearchLineEdit(this);
    m_searchLineEdit->setPlaceholderText("Search Tweets (Global)...");
    m_searchLineEdit->setClearButtonEnabled(true);

    m_filterPanelWidget = new FilterPanelWidget(this); // Create our custom filter panel

    m_tweetListWidget = new QListWidget(this);
    m_tweetListWidget->setObjectName("tweetListWidget");

    m_rightPanel = createRightPanel(); // Helper to create code/metadata views

    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    m_mainSplitter->setObjectName("mainSplitter");
    m_mainSplitter->addWidget(m_filterPanelWidget); // Col 1 (index 0)
    m_mainSplitter->addWidget(m_tweetListWidget);   // Col 2 (index 1)
    m_mainSplitter->addWidget(m_rightPanel);        // Col 3 (index 2)

    // Set Stretch Factors to control relative sizes
    // Higher number = takes more space proportionally
    // Making first column wider, second narrower, third stays relatively large.
    m_mainSplitter->setStretchFactor(0, 2); // Filter panel (index 0) - Wider
    m_mainSplitter->setStretchFactor(1, 2); // List column (index 1) - Narrower
    m_mainSplitter->setStretchFactor(2, 5); // Code/Meta column (index 2) - Stays wide

    // Optional: Set initial absolute sizes.
    // If you use setSizes, the values should ideally reflect the stretch factor proportions.
    // For a total width of 11 stretch units (5+1+5):
    // Filter Panel: 5/11, List: 1/11, Right Panel: 5/11
    // We can try to apply these based on the initial window width.
    // However, relying purely on stretch factors and size hints often works well too.
    // You can experiment by commenting/uncommenting and adjusting the setSizes block.
    
    // Example of setting initial sizes proportionally:
    // QList<int> initialSizes;
    // int currentWindowWidth = this->width(); // MainWindow's current width
    // if (currentWindowWidth > 600) { // Only apply if window is reasonably sized
    //     initialSizes << (currentWindowWidth * 5 / 11)
    //                    << (currentWindowWidth * 1 / 11)
    //                    << (currentWindowWidth * 5 / 11);
    //     m_mainSplitter->setSizes(initialSizes);
    // }
    // For now, let's rely mainly on stretch factors and the widgets' size hints for initial sizing.
    // The splitter will distribute space according to stretch factors after initial layout.
    // If you find the initial layout too cramped or too spread out *before any user resize*,
    // then re-introduce and adjust setSizes. A common approach is to set fixed or minimum
    // widths for some columns and let others stretch.

    // If you want the filter panel to have a more fixed initial width,
    // and the list to be narrow, you could do:
    // QList<int> sizes;
    // sizes << 350; // Desired initial width for filter panel
    // sizes << 150; // Desired initial width for tweet list
    // sizes << this->width() - 350 - 150; // Remaining for right panel
    // if (this->width() > 600 && (sizes.at(2) > 100) ) { // Basic sanity check
    //    m_mainSplitter->setSizes(sizes);
    // }
    // Then the stretch factors would primarily govern how they behave on resize.


    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_searchLineEdit);
    centralLayout->addWidget(m_mainSplitter);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(centralLayout);
    setCentralWidget(centralWidget);

    setWindowTitle("SCTweetAlchemy Paster");
    resize(1600, 850); // This sets the initial size of the MainWindow
}

QWidget* MainWindow::createRightPanel()
{
    QWidget* codePanel = new QWidget(this);
    QVBoxLayout* codeLayout = new QVBoxLayout(codePanel);
    codeLayout->setContentsMargins(0, 2, 0, 0); codeLayout->setSpacing(3);
    QLabel* codeTitleLabel = new QLabel("Code", codePanel);
    QFont titleFont = codeTitleLabel->font(); titleFont.setBold(true);
    codeTitleLabel->setFont(titleFont);
    m_codeTextEdit = new QTextEdit(this);
    m_codeTextEdit->setReadOnly(true); m_codeTextEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    m_codeTextEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    m_codeTextEdit->setObjectName("codeTextEdit");
    codeLayout->addWidget(codeTitleLabel); codeLayout->addWidget(m_codeTextEdit);

    QWidget* metadataPanel = new QWidget(this);
    QVBoxLayout* metadataLayout = new QVBoxLayout(metadataPanel);
    metadataLayout->setContentsMargins(0, 2, 0, 0); metadataLayout->setSpacing(3);
    QLabel* metadataTitleLabel = new QLabel("Metadata", metadataPanel);
    metadataTitleLabel->setFont(titleFont);
    m_metadataTextEdit = new QTextEdit(this);
    m_metadataTextEdit->setReadOnly(true); m_metadataTextEdit->setObjectName("metadataTextEdit");
    metadataLayout->addWidget(metadataTitleLabel); metadataLayout->addWidget(m_metadataTextEdit);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    splitter->addWidget(codePanel); splitter->addWidget(metadataPanel);
    splitter->setStretchFactor(0, 4); splitter->setStretchFactor(1, 1);
    return splitter;
}


void MainWindow::connectSignals()
{
    // Repository signals
    connect(m_tweetRepository, &TweetRepository::loadError, this, &MainWindow::handleRepositoryLoadError);
    connect(m_tweetRepository, &TweetRepository::tweetsLoaded, this, &MainWindow::handleTweetsLoaded);

    // Favorites manager signals
    connect(m_favoritesManager, &FavoritesManager::favoritesChanged, this, &MainWindow::handleFavoritesChanged);

    // UI Element signals
    connect(m_tweetListWidget, &QListWidget::currentItemChanged, this, &MainWindow::onTweetSelectionChanged);
    // *** NEW CONNECTION for double-click ***
    connect(m_tweetListWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::onTweetItemDoubleClicked);

    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(m_searchLineEdit, &SearchLineEdit::navigationKeyPressed, this, &MainWindow::onSearchNavigateKey);
    
    // Filter panel signals
    connect(m_filterPanelWidget, &FilterPanelWidget::filtersChanged, this, &MainWindow::applyAllFilters);

    // Action signals
    connect(m_focusSearchAction, &QAction::triggered, this, &MainWindow::focusSearchField);
    connect(m_toggleFavoriteAction, &QAction::triggered, this, &MainWindow::toggleCurrentTweetFavorite);
}

void MainWindow::onTweetItemDoubleClicked(QListWidgetItem *item)
{
    if (!item) {
        return;
    }
    QString tweetId = item->data(Qt::UserRole).toString();
    if (tweetId.isEmpty()) {
        qWarning() << "Double-clicked item has no tweet ID.";
        return;
    }
    toggleFavoriteStatus(tweetId); // Use the new helper
}


// --- Slot Implementations ---

void MainWindow::handleRepositoryLoadError(const QString& title, const QString& message) {
    QMessageBox::critical(this, title, message);
}

void MainWindow::handleTweetsLoaded(int count) {
    qInfo() << "MainWindow notified: " << count << "tweets loaded.";
    m_filterPanelWidget->populateFilters(
        m_tweetRepository->getAllUniqueAuthors(),
        m_tweetRepository->getAllUniqueSonicTags(),
        m_tweetRepository->getAllUniqueTechniqueTags(),
        m_tweetRepository->getAllUniqueUgens()
    );
    applyAllFilters(); // Perform initial filtering and list population
    if (m_tweetListWidget->count() > 0) {
       m_tweetListWidget->setCurrentRow(0);
    }
}

void MainWindow::handleFavoritesChanged() {
    // Update favorite icons in the list
    for (int i = 0; i < m_tweetListWidget->count(); ++i) {
        QListWidgetItem* item = m_tweetListWidget->item(i);
        if (item) {
            QString tweetId = item->data(Qt::UserRole).toString(); // Assuming ID is stored in UserRole
            updateFavoriteIcon(item, tweetId);
        }
    }
    // Re-apply filters if "Favorites Only" is active
    if (m_filterPanelWidget->isFavoritesFilterActive()) {
        applyAllFilters();
    }
    // Update metadata display if current tweet's favorite status changed
    if (m_tweetListWidget->currentItem()) {
        QString currentTweetId = m_tweetListWidget->currentItem()->data(Qt::UserRole).toString();
        const TweetData* td = m_tweetRepository->findTweetById(currentTweetId);
        if (td) displayTweetDetails(td); // Refresh metadata, which includes favorite status
    }

}

void MainWindow::applyAllFilters()
{
    if (!m_tweetRepository || !m_tweetFilterEngine) return;

    FilterCriteria criteria;
    criteria.searchText = m_searchLineEdit->text();
    criteria.favoritesOnly = m_filterPanelWidget->isFavoritesFilterActive();
    criteria.favoriteTweetIds = &m_favoritesManager->getFavoriteTweetIds();
    criteria.useAndLogic = m_filterPanelWidget->isMatchAllLogic();
    criteria.checkedAuthors = m_filterPanelWidget->getCheckedAuthors();
    criteria.checkedSonicTags = m_filterPanelWidget->getCheckedSonicTags();
    criteria.checkedTechniqueTags = m_filterPanelWidget->getCheckedTechniqueTags();
    criteria.checkedUgens = m_filterPanelWidget->getCheckedUgens();

    const QVector<TweetData>& allTweets = m_tweetRepository->getAllTweets();
    m_currentlyDisplayedTweets = m_tweetFilterEngine->filterTweets(allTweets, criteria);
    
    populateTweetList(m_currentlyDisplayedTweets);
    qInfo() << "Filters applied, list count:" << m_tweetListWidget->count();
}

void MainWindow::populateTweetList(const QVector<const TweetData*>& tweetsToDisplay) {
    m_tweetListWidget->clear();
    QString previousSelectedId;
    if (m_tweetListWidget->property("selectedTweetId").isValid()) {
        previousSelectedId = m_tweetListWidget->property("selectedTweetId").toString();
    }


    QListWidgetItem* itemToSelect = nullptr;
    for (const TweetData* tweet : tweetsToDisplay) {
        if (!tweet) continue;
        QListWidgetItem* newItem = new QListWidgetItem(tweet->id, m_tweetListWidget); // Displaying ID for now
        newItem->setData(Qt::UserRole, tweet->id); // Store ID for retrieval
        updateFavoriteIcon(newItem, tweet->id);
        if (tweet->id == previousSelectedId) {
            itemToSelect = newItem;
        }
    }

    if (itemToSelect) {
        m_tweetListWidget->setCurrentItem(itemToSelect);
    } else if (m_tweetListWidget->count() > 0) {
        m_tweetListWidget->setCurrentRow(0);
    } else {
        displayTweetDetails(nullptr); // No items match, clear details
    }
}


void MainWindow::onTweetSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);
    if (!current) {
        displayTweetDetails(nullptr);
        m_tweetListWidget->setProperty("selectedTweetId", QVariant());
        return;
    }
    QString selectedId = current->data(Qt::UserRole).toString();
    m_tweetListWidget->setProperty("selectedTweetId", selectedId); // Store for repopulation
    const TweetData* tweet = m_tweetRepository->findTweetById(selectedId);
    displayTweetDetails(tweet);
}

void MainWindow::onSearchTextChanged(const QString &text)
{
    applyAllFilters();
}

void MainWindow::onSearchNavigateKey(QKeyEvent *event)
{
    if (m_tweetListWidget && m_tweetListWidget->count() > 0) {
        m_tweetListWidget->setFocus(); // Give focus to list
        // QListWidget handles Up/Down navigation internally if it has focus
        // No need to manually set current row unless it's for initial selection.
        if (!m_tweetListWidget->currentItem() && m_tweetListWidget->count() > 0) {
            m_tweetListWidget->setCurrentRow(0);
        }
    }
}

void MainWindow::focusSearchField()
{
    if (m_searchLineEdit) {
        m_searchLineEdit->setFocus();
        m_searchLineEdit->selectAll();
    }
}

void MainWindow::toggleCurrentTweetFavorite()
{
    QListWidgetItem* currentItem = m_tweetListWidget->currentItem();
    if (!currentItem) {
        qWarning() << "Cannot toggle favorite: No item selected for action.";
        return;
    }
    QString tweetId = currentItem->data(Qt::UserRole).toString();
    if (tweetId.isEmpty()) {
        qWarning() << "Selected item for action has no tweet ID.";
        return;
    }
    toggleFavoriteStatus(tweetId); // Use the new helper
}

// --- New Helper Method to Centralize Toggle Logic ---
void MainWindow::toggleFavoriteStatus(const QString& tweetId)
{
    // It's good practice to ensure tweetData exists, though for just toggling favorites,
    // only the ID is strictly needed by FavoritesManager.
    // If you needed other tweetData properties here, you'd fetch it:
    // const TweetData* tweetData = m_tweetRepository->findTweetById(tweetId);
    // if (!tweetData) {
    //     qWarning() << "Cannot toggle favorite: Data not found for ID" << tweetId;
    //     return;
    // }

    if (m_favoritesManager->isFavorite(tweetId)) {
        m_favoritesManager->removeFavorite(tweetId); // This will emit favoritesChanged
        qInfo() << "Removed favorite (via helper):" << tweetId;
    } else {
        m_favoritesManager->addFavorite(tweetId);    // This will emit favoritesChanged
        qInfo() << "Added favorite (via helper):" << tweetId;
    }
    // The favoritesChanged signal from m_favoritesManager will trigger UI updates
    // (icon updates, metadata refresh, potential list re-filtering)
    // via the handleFavoritesChanged slot.
}


void MainWindow::updateFavoriteIcon(QListWidgetItem* item, const QString& tweetId) {
    if (!item || !m_favoritesManager) return;

    if (m_favoritesManager->isFavorite(tweetId)) {
        // Try to load a specific star icon from resources.
        // Fallback to a theme icon if your resource isn't found.
        QIcon starIcon(":/icons/star_filled.png");
        if (starIcon.isNull()) { // Check if resource loaded
            starIcon = QIcon::fromTheme("emblem-important", QIcon::fromTheme("emblem-favorite")); // Fallback
        }
        item->setIcon(starIcon);
    } else {
        // You could use an outline star here if you have one:
        // QIcon outlineStarIcon(":/icons/star_outline.png");
        // item->setIcon(outlineStarIcon.isNull() ? QIcon() : outlineStarIcon);
        item->setIcon(QIcon()); // No icon for non-favorite
    }
}

void MainWindow::displayTweetDetails(const TweetData* tweet)
{
    if (!m_codeTextEdit || !m_metadataTextEdit) return;

    if (tweet) {
        m_codeTextEdit->setText(tweet->originalCode);

        QString metadataString;
        metadataString += "ID: " + tweet->id + "\n";
        metadataString += "Author: " + tweet->author + "\n";
        metadataString += "Source: " + (!tweet->sourceUrl.isEmpty() ? tweet->sourceUrl : QStringLiteral("N/A")) + "\n";
        metadataString += "Date: " + tweet->publicationDate + "\n";
        metadataString += "Description: " + tweet->description + "\n\n";

        if (!tweet->sonicTags.isEmpty()) {
            metadataString += "Sonic Characteristics: " + tweet->sonicTags.join(", ") + "\n";
        }
        if (!tweet->techniqueTags.isEmpty()) {
            metadataString += "Synthesis Techniques: " + tweet->techniqueTags.join(", ") + "\n";
        }
        if (!tweet->ugens.isEmpty()) {
            metadataString += "UGens: " + tweet->ugens.join(", ") + "\n";
        }
        if (!tweet->genericTags.isEmpty()) {
           metadataString += "Tags (Other): " + tweet->genericTags.join(", ") + "\n";
        }
        
        metadataString += "\nFavorite: ";
        metadataString += (m_favoritesManager && m_favoritesManager->isFavorite(tweet->id) ? QStringLiteral("Yes") : QStringLiteral("No"));
        metadataString += QLatin1Char('\n');
        m_metadataTextEdit->setText(metadataString);

    } else {
        m_codeTextEdit->clear();
        m_codeTextEdit->setPlaceholderText("Select a Tweet or adjust filters.");
        m_metadataTextEdit->clear();
        m_metadataTextEdit->setPlaceholderText("Select a Tweet to view its metadata.");
    }
}