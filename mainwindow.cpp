#include "mainwindow.h"
#include "searchlineedit.h"
#include "tweetrepository.h"
#include "favoritesmanager.h"
#include "filterpanelwidget.h"
#include "tweetfilterengine.h"
#include "tweeteditdialog.h" // Include the new dialog

#include <QtWidgets>
#include <QStandardPaths> // For writable location
#include <QDir>           // For directory operations
#include <QClipboard>     // For copy to clipboard

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
    , m_focusSearchAction(nullptr) // Will be created in setupActions/setupMenuBar
    , m_toggleFavoriteAction(nullptr) // Will be created in setupActions/setupMenuBar
    , m_settings(nullptr)
    , m_tweetRepository(nullptr)
    , m_favoritesManager(nullptr)
    , m_tweetFilterEngine(nullptr)
    , m_menuBar(nullptr)
    , m_fileMenu(nullptr)
    , m_editMenu(nullptr)
    , m_helpMenu(nullptr)
    , m_newTweetAction(nullptr)
    , m_saveAllAction(nullptr)
    , m_exitAction(nullptr)
    , m_editTweetAction(nullptr)
    , m_deleteTweetAction(nullptr)
    , m_copyCodeAction(nullptr)
    , m_aboutAction(nullptr)
{
    QCoreApplication::setOrganizationName("Kosmas");
    QCoreApplication::setApplicationName("SCTweetAlchemy");
    m_settings = new QSettings(this);

    setupModelsAndManagers();
    setupUi(); // Sets up central widget, splitters etc.
    setupActions(); // Sets up QActions (some might be used in menu)
    setupMenuBar(); // Sets up the menu bar and connects actions
    connectSignals();

    // Initial data load: Try user file first, then resource
    QString userTweetPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!userTweetPath.isEmpty()) { // Ensure AppDataLocation is valid
        QDir appDataDir(userTweetPath);
        if (!appDataDir.exists()) {
            appDataDir.mkpath("."); // Create if it doesn't exist
        }
        userTweetPath += "/SCTweets_user.json"; // Construct full path
    } else { // Fallback if AppDataLocation is empty (rare)
        userTweetPath = QDir::homePath() + "/.SCTweetAlchemy/SCTweets_user.json";
        QDir(QDir::homePath() + "/.SCTweetAlchemy").mkpath(".");
    }


    QFile userFile(userTweetPath);
    bool loadedSuccessfully = false;
    if (userFile.exists()) {
        qInfo() << "Attempting to load tweets from user file:" << userTweetPath;
        if (m_tweetRepository->loadTweets(userTweetPath)) {
            loadedSuccessfully = true;
        } else {
            qWarning() << "Failed to load from user file, attempting resource.";
        }
    }

    if (!loadedSuccessfully) {
        qInfo() << "Attempting to load tweets from resource.";
        if (m_tweetRepository->loadTweets()) { // Default resource path
            loadedSuccessfully = true;
        }
    }

    if (!loadedSuccessfully && m_tweetRepository->getAllTweets().isEmpty()) {
        // If both loads failed or resulted in no tweets, show placeholder
        if(m_codeTextEdit) m_codeTextEdit->setPlaceholderText("No tweets found or failed to load all sources.");
        if(m_metadataTextEdit) m_metadataTextEdit->setPlaceholderText("");
    }
    // The tweetsLoaded signal (if successful load) will handle initial population

     if(m_tweetListWidget) {
        m_tweetListWidget->setFocus();
    }
    updateActionStates(); // Set initial enabled/disabled state of actions
}

// --- Destructor ---
MainWindow::~MainWindow() {
    // On close, ensure any pending tweet changes are saved
    if (m_tweetRepository) {
        // Only save if the current path is NOT a read-only resource path
        if (!m_tweetRepository->getCurrentResourcePath().startsWith(":/")) {
             qInfo() << "Saving tweets on exit to:" << m_tweetRepository->getCurrentResourcePath();
             m_tweetRepository->saveTweetsToResource(); 
        } else {
             qInfo() << "Not saving on exit as current data source is a read-only resource:" << m_tweetRepository->getCurrentResourcePath();
        }
    }
    // Other objects with `this` as parent will be auto-deleted by Qt
}

void MainWindow::setupModelsAndManagers()
{
    m_tweetRepository = new TweetRepository(this);
    m_favoritesManager = new FavoritesManager(m_settings, this);
    m_tweetFilterEngine = new TweetFilterEngine(); // No parent, simple utility
}

// --- Setup Overall UI Layout (Central Widget, Splitters) ---
void MainWindow::setupUi()
{
    m_searchLineEdit = new SearchLineEdit(this);
    m_searchLineEdit->setPlaceholderText("Search Tweets (Global)...");
    m_searchLineEdit->setClearButtonEnabled(true);

    m_filterPanelWidget = new FilterPanelWidget(this);

    m_tweetListWidget = new QListWidget(this);
    m_tweetListWidget->setObjectName("tweetListWidget");
    m_tweetListWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    m_rightPanel = createRightPanel();

    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    m_mainSplitter->setObjectName("mainSplitter");
    m_mainSplitter->addWidget(m_filterPanelWidget);
    m_mainSplitter->addWidget(m_tweetListWidget);
    m_mainSplitter->addWidget(m_rightPanel);

    m_mainSplitter->setStretchFactor(0, 2);
    m_mainSplitter->setStretchFactor(1, 2);
    m_mainSplitter->setStretchFactor(2, 5);

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_searchLineEdit);
    centralLayout->addWidget(m_mainSplitter);
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(centralLayout);
    setCentralWidget(centralWidget);

    setWindowTitle("SCTweetAlchemy");
    resize(1600, 850);
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

// --- Setup QActions (primarily for non-menu global shortcuts) ---
void MainWindow::setupActions()
{
    m_focusSearchAction = new QAction("Focus Search", this);
    m_focusSearchAction->setShortcut(QKeySequence::Find);
    m_focusSearchAction->setToolTip("Focus the search field (Ctrl+F)");
    this->addAction(m_focusSearchAction); // Makes shortcut work globally for this window
    connect(m_focusSearchAction, &QAction::triggered, this, &MainWindow::focusSearchField); // Connect here too for consistency


    m_toggleFavoriteAction = new QAction("&Toggle Favorite", this); 
    m_toggleFavoriteAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    m_toggleFavoriteAction->setToolTip("Mark/Unmark selected tweet as favorite (Ctrl+D)");
    this->addAction(m_toggleFavoriteAction); 
    connect(m_toggleFavoriteAction, &QAction::triggered, this, &MainWindow::toggleCurrentTweetFavorite); // Connect here
}

// --- Setup Menu Bar ---
void MainWindow::setupMenuBar()
{
    m_menuBar = menuBar();

    // --- File Menu ---
    m_fileMenu = m_menuBar->addMenu("&File");
    m_newTweetAction = new QAction("&New Tweet...", this);
    m_newTweetAction->setShortcut(QKeySequence::New);
    m_fileMenu->addAction(m_newTweetAction);

    m_saveAllAction = new QAction("&Save All Changes", this);
    m_saveAllAction->setShortcut(QKeySequence::Save);
    m_fileMenu->addAction(m_saveAllAction);

    m_fileMenu->addSeparator();
    m_exitAction = new QAction("E&xit", this);
    m_exitAction->setShortcut(QKeySequence::Quit); // Standard quit shortcut
    m_fileMenu->addAction(m_exitAction);

    // --- Edit Menu ---
    m_editMenu = m_menuBar->addMenu("&Edit");
    m_editTweetAction = new QAction("&Edit Selected Tweet...", this);
    m_editTweetAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
    m_editMenu->addAction(m_editTweetAction);

    m_deleteTweetAction = new QAction("&Delete Selected Tweet", this);
    m_deleteTweetAction->setShortcut(QKeySequence::Delete);
    m_editMenu->addAction(m_deleteTweetAction);

    m_editMenu->addSeparator();
    m_editMenu->addAction(m_toggleFavoriteAction); // Add existing action to menu

    m_editMenu->addSeparator();
    m_copyCodeAction = new QAction("&Copy Code", this);
    m_copyCodeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C));
    m_editMenu->addAction(m_copyCodeAction);

    // --- Help Menu ---
    m_helpMenu = m_menuBar->addMenu("&Help");
    m_aboutAction = new QAction("&About SCTweetAlchemy", this);
    m_helpMenu->addAction(m_aboutAction);

    QAction *aboutQtAction = new QAction("About &Qt", this);
    m_helpMenu->addAction(aboutQtAction); // aboutQt is a static slot in QApplication

    // Connect menu actions here
    connect(m_newTweetAction, &QAction::triggered, this, &MainWindow::onFileNewTweet);
    connect(m_saveAllAction, &QAction::triggered, this, &MainWindow::onFileSaveAllChanges);
    connect(m_exitAction, &QAction::triggered, qApp, &QApplication::quit);
    connect(m_editTweetAction, &QAction::triggered, this, &MainWindow::onEditTweet);
    connect(m_deleteTweetAction, &QAction::triggered, this, &MainWindow::onEditDeleteTweet);
    connect(m_copyCodeAction, &QAction::triggered, this, &MainWindow::onEditCopyCode);
    // m_toggleFavoriteAction is already connected in connectSignals() if it's a global action
    // or connect it here if it's purely menu-driven (but global shortcut is good)
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::onHelpAbout);
    connect(aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);
}


void MainWindow::connectSignals()
{
    // Repository signals
    connect(m_tweetRepository, &TweetRepository::loadError, this, &MainWindow::handleRepositoryLoadError);
    connect(m_tweetRepository, &TweetRepository::tweetsLoaded, this, &MainWindow::handleTweetsLoaded);
    connect(m_tweetRepository, &TweetRepository::tweetsModified, this, &MainWindow::handleTweetsModified);

    // Favorites manager signals
    connect(m_favoritesManager, &FavoritesManager::favoritesChanged, this, &MainWindow::handleFavoritesChanged);

    // UI Element signals
    connect(m_tweetListWidget, &QListWidget::currentItemChanged, this, &MainWindow::onTweetSelectionChanged);
    connect(m_tweetListWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::onTweetItemDoubleClicked);
    connect(m_tweetListWidget, &QListWidget::customContextMenuRequested, this, &MainWindow::onTweetListContextMenuRequested);
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(m_searchLineEdit, &SearchLineEdit::navigationKeyPressed, this, &MainWindow::onSearchNavigateKey);
    connect(m_filterPanelWidget, &FilterPanelWidget::filtersChanged, this, &MainWindow::applyAllFilters);

    // Note: m_focusSearchAction and m_toggleFavoriteAction are now connected in setupActions().
    // No need to connect them again here.
}

void MainWindow::onTweetListContextMenuRequested(const QPoint &pos)
{
    QListWidgetItem* itemUnderMouse = m_tweetListWidget->itemAt(pos);

    QMenu contextMenu(this);

    // Ensure actions exist (they should if setupMenuBar was called)
    if (!m_editTweetAction || !m_deleteTweetAction || !m_toggleFavoriteAction) {
        qWarning() << "Context menu actions not initialized!";
        return;
    }

    if (itemUnderMouse) {
        // An item was right-clicked, make sure it's selected for actions
        // (QListWidget usually handles selection on right-click before context menu if set up)
        // If not, you might need: m_tweetListWidget->setCurrentItem(itemUnderMouse);

        // Add actions relevant to a selected item
        contextMenu.addAction(m_editTweetAction);
        contextMenu.addAction(m_deleteTweetAction);
        contextMenu.addAction(m_toggleFavoriteAction);
        contextMenu.addSeparator(); 
        // You could add "Copy Code" here too if m_copyCodeAction exists
        if (m_copyCodeAction) {
            contextMenu.addAction(m_copyCodeAction);
        }

        // The QAction's setEnabled(bool) state (updated by updateActionStates)
        // should control if they appear grayed out.
        // updateActionStates() is called on selection change, which is good.

    } else {
        // Right-clicked on empty area of the list widget
        // You could add actions like "Add New Tweet..." here if desired
        if (m_newTweetAction) { // Assuming m_newTweetAction is initialized
             // contextMenu.addAction(m_newTweetAction);
        }
        // For now, let's keep it simple: context menu primarily for items.
    }

    if (!contextMenu.isEmpty()) {
        contextMenu.exec(m_tweetListWidget->mapToGlobal(pos));
    }
}

// --- Slot Implementations for Managers ---
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
    applyAllFilters();
    if (m_tweetListWidget->count() > 0) {
       m_tweetListWidget->setCurrentRow(0);
    }
    updateActionStates();
}

void MainWindow::handleFavoritesChanged() {
    for (int i = 0; i < m_tweetListWidget->count(); ++i) {
        QListWidgetItem* item = m_tweetListWidget->item(i);
        if (item) {
            QString tweetId = item->data(Qt::UserRole).toString();
            updateFavoriteIcon(item, tweetId);
        }
    }
    if (m_filterPanelWidget->isFavoritesFilterActive()) {
        applyAllFilters();
    }
    QListWidgetItem* currentItem = m_tweetListWidget->currentItem();
    if (currentItem) {
        QString currentTweetId = currentItem->data(Qt::UserRole).toString();
        const TweetData* td = m_tweetRepository->findTweetById(currentTweetId);
        if (td) displayTweetDetails(td);
    } else if (m_tweetListWidget->count() == 0) {
        displayTweetDetails(nullptr);
    }
    updateActionStates(); // Favorite status might affect actions
}

void MainWindow::handleTweetsModified()
{
    qInfo() << "MainWindow notified: Tweets modified in repository.";
    if (m_filterPanelWidget && m_tweetRepository) {
        m_filterPanelWidget->populateFilters(
            m_tweetRepository->getAllUniqueAuthors(),
            m_tweetRepository->getAllUniqueSonicTags(),
            m_tweetRepository->getAllUniqueTechniqueTags(),
            m_tweetRepository->getAllUniqueUgens()
        );
    }
    applyAllFilters(); // This will repopulate the list and reselect if possible
    updateActionStates();
}


// --- Slots for Menu Actions ---
void MainWindow::onFileNewTweet()
{
    TweetEditDialog dialog(TweetEditDialog::Mode::Add, this);
    if (m_tweetRepository) {
        dialog.setExistingTweetIds(m_tweetRepository->getAllTweetIds());
    }

    if (dialog.exec() == QDialog::Accepted) {
        TweetData newTweet = dialog.getTweetData();
        if (m_tweetRepository && m_tweetRepository->addTweet(newTweet)) {
            qInfo() << "New tweet added via dialog:" << newTweet.id;
            // tweetsModified signal will refresh UI
        } else {
            QMessageBox::critical(this, "Error", "Failed to add the new tweet to the repository (e.g., ID conflict not caught by dialog).");
        }
    }
}

void MainWindow::onFileSaveAllChanges()
{
    if (m_tweetRepository) {
        // The save path logic is now within TweetRepository
        if (m_tweetRepository->saveTweetsToResource()) {
            statusBar()->showMessage("Tweet collection saved successfully.", 3000);
        } else {
            // Error should be signaled by repository or handled there
        }
    }
}

void MainWindow::onEditTweet()
{
    QListWidgetItem* currentItem = m_tweetListWidget->currentItem();
    if (!currentItem) {
        // updateActionStates should prevent this if called correctly
        QMessageBox::information(this, "Edit Tweet", "Please select a tweet to edit.");
        return;
    }
    QString tweetId = currentItem->data(Qt::UserRole).toString();
    const TweetData* tweetToEdit = m_tweetRepository->findTweetById(tweetId);

    if (!tweetToEdit) {
        QMessageBox::critical(this, "Error", "Could not find data for the selected tweet.");
        return;
    }

    TweetEditDialog dialog(TweetEditDialog::Mode::Edit, this);
    dialog.setTweetData(*tweetToEdit);
    if (m_tweetRepository) {
        dialog.setExistingTweetIds(m_tweetRepository->getAllTweetIds());
    }

    if (dialog.exec() == QDialog::Accepted) {
        TweetData updatedTweet = dialog.getTweetData();
        if (m_tweetRepository && m_tweetRepository->updateTweet(updatedTweet)) {
            qInfo() << "Tweet updated via dialog:" << updatedTweet.id;
            // tweetsModified signal will refresh UI
        } else {
            QMessageBox::critical(this, "Error", "Failed to update the tweet in the repository.");
        }
    }
}

void MainWindow::onEditDeleteTweet()
{
    QListWidgetItem* currentItem = m_tweetListWidget->currentItem();
    if (!currentItem) {
        QMessageBox::information(this, "Delete Tweet", "Please select a tweet to delete.");
        return;
    }
    QString tweetId = currentItem->data(Qt::UserRole).toString();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm Delete",
                                  QString("Are you sure you want to delete tweet '%1'?").arg(tweetId),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (m_tweetRepository && m_tweetRepository->deleteTweet(tweetId)) {
            qInfo() << "Tweet deleted:" << tweetId;
            // tweetsModified signal will refresh UI
        } else {
            QMessageBox::critical(this, "Error", "Failed to delete the tweet from the repository.");
        }
    }
}

void MainWindow::onEditCopyCode()
{
    QListWidgetItem* currentItem = m_tweetListWidget->currentItem();
    if (!currentItem) return;
    QString tweetId = currentItem->data(Qt::UserRole).toString();
    const TweetData* tweet = m_tweetRepository->findTweetById(tweetId);
    if (tweet) {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(tweet->originalCode);
        statusBar()->showMessage(QString("Code for '%1' copied!").arg(tweetId), 2000);
    }
}

void MainWindow::onHelpAbout()
{
    QString aboutText = 
        "<h2>SCTweetAlchemy</h2>" // Using an <h2> for the title
        "<p>SuperCollider Tweet Browser</p>"
        "<p>Version 0.3</p><br/>" // <br/> for a line break
        "<p>Created by Kosmas Giannoutakis.<br/>"
        "<a href=\"https://www.kosmasgiannoutakis.art/\">https://www.kosmasgiannoutakis.art/</a></p><br/>"
        "<p>A tool to browse, manage, study and utilize SCTweets for live coding.</p>";

    QMessageBox::about(this, "About SCTweetAlchemy", aboutText);
}


// --- Core UI Interaction Slots ---
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
    m_tweetListWidget->blockSignals(true); // Prevent selection signals during repopulation

    QString previouslySelectedId;
    QListWidgetItem* currentSelItem = m_tweetListWidget->currentItem();
    if (currentSelItem) {
        previouslySelectedId = currentSelItem->data(Qt::UserRole).toString();
    } else if (m_tweetListWidget->property("selectedTweetId").isValid()){
         previouslySelectedId = m_tweetListWidget->property("selectedTweetId").toString();
    }


    m_tweetListWidget->clear();
    QListWidgetItem* itemToSelectAgain = nullptr;

    for (const TweetData* tweet : tweetsToDisplay) {
        if (!tweet) continue;
        QListWidgetItem* newItem = new QListWidgetItem(tweet->id, m_tweetListWidget);
        newItem->setData(Qt::UserRole, tweet->id);
        updateFavoriteIcon(newItem, tweet->id);
        if (tweet->id == previouslySelectedId) {
            itemToSelectAgain = newItem;
        }
    }

    m_tweetListWidget->blockSignals(false);

    if (itemToSelectAgain) {
        m_tweetListWidget->setCurrentItem(itemToSelectAgain);
    } else if (m_tweetListWidget->count() > 0) {
        m_tweetListWidget->setCurrentRow(0); // Select first item if previous is gone or none was selected
    } else {
        // No items in list, currentItemChanged might not fire if already null
        displayTweetDetails(nullptr); // Explicitly clear details
        onTweetSelectionChanged(nullptr, nullptr); // To ensure action states update
    }
    // onTweetSelectionChanged or setCurrentRow will trigger updateActionStates indirectly
    // but if the selection doesn't change, we might need to call it explicitly.
    // However, if list is empty and was not, selection *did* change to null.
    updateActionStates(); // Ensure states are correct after list repopulation
}

void MainWindow::onTweetSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);
    if (!current) {
        displayTweetDetails(nullptr);
        m_tweetListWidget->setProperty("selectedTweetId", QVariant()); // Clear stored selection
    } else {
        QString selectedId = current->data(Qt::UserRole).toString();
        m_tweetListWidget->setProperty("selectedTweetId", selectedId); // Store for repopulation
        const TweetData* tweet = m_tweetRepository->findTweetById(selectedId);
        displayTweetDetails(tweet);
    }
    updateActionStates();
}

void MainWindow::onSearchTextChanged(const QString &text) { applyAllFilters(); }

void MainWindow::onSearchNavigateKey(QKeyEvent *event) {
    if (m_tweetListWidget && m_tweetListWidget->count() > 0) {
        m_tweetListWidget->setFocus();
        if (!m_tweetListWidget->currentItem() && m_tweetListWidget->count() > 0) {
            m_tweetListWidget->setCurrentRow(0);
        }
    }
}

void MainWindow::focusSearchField() {
    if (m_searchLineEdit) {
        m_searchLineEdit->setFocus();
        m_searchLineEdit->selectAll();
    }
}

void MainWindow::toggleCurrentTweetFavorite() {
    QListWidgetItem* currentItem = m_tweetListWidget->currentItem();
    if (!currentItem) return;
    QString tweetId = currentItem->data(Qt::UserRole).toString();
    if (tweetId.isEmpty()) return;
    toggleFavoriteStatus(tweetId);
}

void MainWindow::onTweetItemDoubleClicked(QListWidgetItem *item) {
    if (!item) return;
    QString tweetId = item->data(Qt::UserRole).toString();
    if (tweetId.isEmpty()) return;
    toggleFavoriteStatus(tweetId);
}

void MainWindow::toggleFavoriteStatus(const QString& tweetId) {
    if (m_favoritesManager->isFavorite(tweetId)) {
        m_favoritesManager->removeFavorite(tweetId);
    } else {
        m_favoritesManager->addFavorite(tweetId);
    }
}

void MainWindow::updateFavoriteIcon(QListWidgetItem* item, const QString& tweetId) {
    if (!item || !m_favoritesManager) return;
    if (m_favoritesManager->isFavorite(tweetId)) {
        QIcon starIcon(":/icons/star_filled.png");
        if (starIcon.isNull()) {
            qWarning() << "Failed to load resource icon ':/icons/star_filled.png'. Using fallback.";
            starIcon = QIcon::fromTheme("emblem-important", QIcon::fromTheme("emblem-favorite"));
        }
        item->setIcon(starIcon);
    } else {
        item->setIcon(QIcon());
    }
}

void MainWindow::displayTweetDetails(const TweetData* tweet) {
    if (!m_codeTextEdit || !m_metadataTextEdit) return;
    if (tweet) {
        m_codeTextEdit->setText(tweet->originalCode);
        QString metadataString;
        metadataString += "ID: " + tweet->id + "\n";
        metadataString += "Author: " + tweet->author + "\n";
        metadataString += "Source: " + (!tweet->sourceUrl.isEmpty() ? tweet->sourceUrl : QStringLiteral("N/A")) + "\n";
        metadataString += "Date: " + tweet->publicationDate + "\n";
        metadataString += "Description: " + tweet->description + "\n\n";
        if (!tweet->sonicTags.isEmpty()) metadataString += "Sonic Characteristics: " + tweet->sonicTags.join(", ") + "\n";
        if (!tweet->techniqueTags.isEmpty()) metadataString += "Synthesis Techniques: " + tweet->techniqueTags.join(", ") + "\n";
        if (!tweet->ugens.isEmpty()) metadataString += "UGens: " + tweet->ugens.join(", ") + "\n";
        if (!tweet->genericTags.isEmpty()) metadataString += "Tags (Other): " + tweet->genericTags.join(", ") + "\n";
        metadataString += QStringLiteral("\nFavorite: ") + (m_favoritesManager && m_favoritesManager->isFavorite(tweet->id) ? "Yes" : "No") + QStringLiteral("\n");
        m_metadataTextEdit->setText(metadataString);
    } else {
        m_codeTextEdit->clear(); m_codeTextEdit->setPlaceholderText("Select a Tweet or adjust filters.");
        m_metadataTextEdit->clear(); m_metadataTextEdit->setPlaceholderText("Select a Tweet to view its metadata.");
    }
}

void MainWindow::updateActionStates()
{
    bool itemSelected = (m_tweetListWidget && m_tweetListWidget->currentItem() != nullptr);

    if(m_editTweetAction) m_editTweetAction->setEnabled(itemSelected);
    if(m_deleteTweetAction) m_deleteTweetAction->setEnabled(itemSelected);
    if(m_copyCodeAction) m_copyCodeAction->setEnabled(itemSelected);
    if(m_toggleFavoriteAction) m_toggleFavoriteAction->setEnabled(itemSelected);
    // m_saveAllAction could be enabled if there are unsaved changes (requires tracking)
    // For now, always enabled or enable based on whether a user file path exists
    if(m_saveAllAction && m_tweetRepository) {
        m_saveAllAction->setEnabled(!m_tweetRepository->getCurrentResourcePath().startsWith(":/"));
    }

}