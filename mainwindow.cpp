#include "mainwindow.h"
#include "searchlineedit.h"
#include "tweetrepository.h"
#include "favoritesmanager.h"
#include "filterpanelwidget.h"
#include "tweetfilterengine.h"
#include "tweeteditdialog.h" 
#include "ndefgenerator.h"   // Include NdefGenerator

#include <QtWidgets>
#include <QStandardPaths> 
#include <QDir>           
#include <QClipboard>     
#include <QMenu>          // For context menu
#include <QToolButton>
#include <QComboBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QSpinBox>      // For integer spin box
#include <QDoubleSpinBox> // For double spin box
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout> // For layout within groupbox

// --- Constructor ---
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_searchLineEdit(nullptr)
    , m_mainSplitter(nullptr)
    , m_filterPanelWidget(nullptr)
    , m_tweetListWidget(nullptr)
    , m_codeAndMetadataPanel(nullptr) // Consistent name
    , m_codeTextEdit(nullptr)
    , m_metadataTextEdit(nullptr)
    , m_ndefDisplayPanel(nullptr)    
    , m_ndefCodeTextEdit(nullptr)
    , m_ndefGenerator(nullptr)
    , m_ndefStyleComboBox(nullptr)   
    , m_focusSearchAction(nullptr) 
    , m_toggleFavoriteAction(nullptr)
    , m_ndefAddReshapingCheckBox(nullptr)
    , m_ndefSetFadeTimeCheckBox(nullptr)
    , m_ndefFadeTimeLabel(nullptr)
    , m_ndefFadeTimeSpinBox(nullptr)
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
    m_ndefGenerator = new NdefGenerator(); // Create instance

    setupModelsAndManagers();
    setupUi(); 
    setupActions(); 
    setupMenuBar(); 
    connectSignals();

    QString userTweetPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!userTweetPath.isEmpty()) { 
        QDir appDataDir(userTweetPath);
        if (!appDataDir.exists()) {
            appDataDir.mkpath("."); 
        }
        userTweetPath += "/SCTweets_user.json"; 
    } else { 
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
        if (m_tweetRepository->loadTweets()) { 
            loadedSuccessfully = true;
        }
    }

    if (!loadedSuccessfully && m_tweetRepository->getAllTweets().isEmpty()) {
        if(m_codeTextEdit) m_codeTextEdit->setPlaceholderText("No tweets found or failed to load all sources.");
        if(m_metadataTextEdit) m_metadataTextEdit->setPlaceholderText("");
        if(m_ndefCodeTextEdit) m_ndefCodeTextEdit->setPlaceholderText("Load tweets to see Ndef versions.");
    }

     if(m_tweetListWidget) {
        m_tweetListWidget->setFocus();
    }
    updateActionStates(); 
}

// --- Destructor ---
MainWindow::~MainWindow() {
    delete m_ndefGenerator; 
    if (m_tweetRepository) {
        if (!m_tweetRepository->getCurrentResourcePath().startsWith(":/")) {
             qInfo() << "Saving tweets on exit to:" << m_tweetRepository->getCurrentResourcePath();
             m_tweetRepository->saveTweetsToResource(); 
        } else {
             qInfo() << "Not saving on exit as current data source is a read-only resource:" << m_tweetRepository->getCurrentResourcePath();
        }
    }
}

void MainWindow::setupModelsAndManagers()
{
    m_tweetRepository = new TweetRepository(this);
    m_favoritesManager = new FavoritesManager(m_settings, this);
    m_tweetFilterEngine = new TweetFilterEngine(); 
}

// --- Setup Overall UI Layout ---
void MainWindow::setupUi()
{
    m_searchLineEdit = new SearchLineEdit(this);
    m_searchLineEdit->setPlaceholderText("Search Tweets (Global)...");
    m_searchLineEdit->setClearButtonEnabled(true);

    m_filterPanelWidget = new FilterPanelWidget(this);

    m_tweetListWidget = new QListWidget(this);
    m_tweetListWidget->setObjectName("tweetListWidget");
    m_tweetListWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    m_codeAndMetadataPanel = createRightPanel(); 
    m_ndefDisplayPanel = createNdefPanel();      

    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    m_mainSplitter->setObjectName("mainSplitter");
    m_mainSplitter->addWidget(m_filterPanelWidget);      
    m_mainSplitter->addWidget(m_tweetListWidget);        
    m_mainSplitter->addWidget(m_codeAndMetadataPanel);   
    m_mainSplitter->addWidget(m_ndefDisplayPanel);       

    m_mainSplitter->setStretchFactor(0, 2); 
    m_mainSplitter->setStretchFactor(1, 2); 
    m_mainSplitter->setStretchFactor(2, 3); 
    m_mainSplitter->setStretchFactor(3, 3); 

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_searchLineEdit);
    centralLayout->addWidget(m_mainSplitter);
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(centralLayout);
    setCentralWidget(centralWidget);

    setWindowTitle("SCTweetAlchemy");
    resize(1800, 850); 
}

// --- Create Panel for Code & Metadata ---
QWidget* MainWindow::createRightPanel() 
{
    QWidget* codePanel = new QWidget(this);
    QVBoxLayout* codeLayout = new QVBoxLayout(codePanel);
    codeLayout->setContentsMargins(0, 2, 0, 0); codeLayout->setSpacing(3);
    QLabel* codeTitleLabel = new QLabel("Original Code", codePanel); // Title more specific
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
    metadataTitleLabel->setFont(titleFont); // Reuse bold font
    m_metadataTextEdit = new QTextEdit(this);
    m_metadataTextEdit->setReadOnly(true); m_metadataTextEdit->setObjectName("metadataTextEdit");
    metadataLayout->addWidget(metadataTitleLabel); metadataLayout->addWidget(m_metadataTextEdit);

    QSplitter *codeMetaSplitter = new QSplitter(Qt::Vertical, this); 
    codeMetaSplitter->addWidget(codePanel);
    codeMetaSplitter->addWidget(metadataPanel);
    codeMetaSplitter->setStretchFactor(0, 4); 
    codeMetaSplitter->setStretchFactor(1, 1); 

    return codeMetaSplitter; 
}

// --- Create Panel for Ndef Display ---
QWidget* MainWindow::createNdefPanel()
{
    QWidget* panel = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(panel);
    mainLayout->setContentsMargins(5, 5, 5, 5); // Added some top/bottom margin for the panel
    mainLayout->setSpacing(6);

    // --- Style Selection ---
    QHBoxLayout* styleLayout = new QHBoxLayout();
    styleLayout->addWidget(new QLabel("Ndef Style:", panel));
    m_ndefStyleComboBox = new QComboBox(panel);
    m_ndefStyleComboBox->addItem("Simple Playable", QVariant::fromValue(NdefFormattingOptions::Style::SimplePlayable));
    m_ndefStyleComboBox->addItem("Reformatted (AST - Basic)", QVariant::fromValue(NdefFormattingOptions::Style::ReformattedAST));
    // Add more styles here later (e.g., NdefFormattingOptions::Style::ReformattedSynthDef)
    connect(m_ndefStyleComboBox, &QComboBox::currentIndexChanged, this, &MainWindow::onNdefFormattingOptionsChanged);
    styleLayout->addWidget(m_ndefStyleComboBox, 1); 
    mainLayout->addLayout(styleLayout);

    // --- Enhancements GroupBox ---
    m_ndefEnhancementsGroup = new QGroupBox("Reformatted Ndef Options", panel);
    // Using QVBoxLayout for more control over spacing and individual widget types than QFormLayout
    QVBoxLayout* enhancementsLayout = new QVBoxLayout(m_ndefEnhancementsGroup);
    enhancementsLayout->setSpacing(6); // Spacing between items in the group

    m_ndefAddReshapingCheckBox = new QCheckBox("Add .reshaping_(\\expanding)", m_ndefEnhancementsGroup); // NEW
    connect(m_ndefAddReshapingCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::onNdefFormattingOptionsChanged);
    enhancementsLayout->addWidget(m_ndefAddReshapingCheckBox);

    m_ndefAddSplayAzCheckBox = new QCheckBox("Wrap output with SplayAz", m_ndefEnhancementsGroup);
    connect(m_ndefAddSplayAzCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::onNdefFormattingOptionsChanged);
    enhancementsLayout->addWidget(m_ndefAddSplayAzCheckBox);
    
    // Layout for SplayAz channels
    QHBoxLayout* splayChannelsLayout = new QHBoxLayout();
    m_ndefSplayChannelsLabel = new QLabel("SplayAz Channels:", m_ndefEnhancementsGroup);
    m_ndefSplayChannelsSpinBox = new QSpinBox(m_ndefEnhancementsGroup); // Integer for channels
    m_ndefSplayChannelsSpinBox->setRange(1, 64); 
    m_ndefSplayChannelsSpinBox->setValue(m_currentNdefOptions.splayAzChannels); 
    connect(m_ndefSplayChannelsSpinBox, &QSpinBox::valueChanged, this, &MainWindow::onNdefFormattingOptionsChanged);
    splayChannelsLayout->addWidget(m_ndefSplayChannelsLabel);
    splayChannelsLayout->addWidget(m_ndefSplayChannelsSpinBox);
    splayChannelsLayout->addStretch();
    enhancementsLayout->addLayout(splayChannelsLayout); 

    // Layout for Fade Time
    m_ndefSetFadeTimeCheckBox = new QCheckBox("Set .fadeTime", m_ndefEnhancementsGroup); // NEW
    connect(m_ndefSetFadeTimeCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::onNdefFormattingOptionsChanged);
    enhancementsLayout->addWidget(m_ndefSetFadeTimeCheckBox);

    QHBoxLayout* fadeTimeLayout = new QHBoxLayout();
    m_ndefFadeTimeLabel = new QLabel("Fade Time (s):", m_ndefEnhancementsGroup); // NEW
    m_ndefFadeTimeSpinBox = new QDoubleSpinBox(m_ndefEnhancementsGroup);         // NEW - QDoubleSpinBox
    m_ndefFadeTimeSpinBox->setRange(0.01, 600.0); 
    m_ndefFadeTimeSpinBox->setDecimals(2);
    m_ndefFadeTimeSpinBox->setSingleStep(0.1);
    m_ndefFadeTimeSpinBox->setValue(m_currentNdefOptions.fadeTimeValue); 
    connect(m_ndefFadeTimeSpinBox, &QDoubleSpinBox::valueChanged, this, &MainWindow::onNdefFormattingOptionsChanged);
    
    fadeTimeLayout->addWidget(m_ndefFadeTimeLabel);
    fadeTimeLayout->addWidget(m_ndefFadeTimeSpinBox);
    fadeTimeLayout->addStretch();
    enhancementsLayout->addLayout(fadeTimeLayout); 
    
    enhancementsLayout->addStretch(); // Add stretch at the bottom of the groupbox
    m_ndefEnhancementsGroup->setLayout(enhancementsLayout);
    mainLayout->addWidget(m_ndefEnhancementsGroup);

    // --- Ndef Code Display ---
    m_ndefCodeTextEdit = new QTextEdit(this);
    m_ndefCodeTextEdit->setReadOnly(true); 
    m_ndefCodeTextEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    m_ndefCodeTextEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    m_ndefCodeTextEdit->setObjectName("ndefCodeTextEdit");
    m_ndefCodeTextEdit->setPlaceholderText("Select a tweet to see its Ndef version.");
    mainLayout->addWidget(m_ndefCodeTextEdit, 1); 

    updateNdefEnhancementOptionsUI(); 
    return panel;
}

void MainWindow::onNdefFormattingOptionsChanged()
{
    // Read current selections from UI and update m_currentNdefOptions
    if (m_ndefStyleComboBox) { // Check if UI elements are created
        m_currentNdefOptions.style = m_ndefStyleComboBox->currentData().value<NdefFormattingOptions::Style>();
    }
    if (m_ndefAddReshapingCheckBox) { // NEW
        m_currentNdefOptions.addReshapingExpanding = m_ndefAddReshapingCheckBox->isChecked();
    }
    if (m_ndefAddSplayAzCheckBox) {
        m_currentNdefOptions.wrapWithSplayAz = m_ndefAddSplayAzCheckBox->isChecked();
    }
    if (m_ndefSplayChannelsSpinBox) {
        m_currentNdefOptions.splayAzChannels = m_ndefSplayChannelsSpinBox->value();
    }
    if (m_ndefSetFadeTimeCheckBox) { // NEW
        m_currentNdefOptions.setFadeTime = m_ndefSetFadeTimeCheckBox->isChecked();
    }
    if (m_ndefFadeTimeSpinBox) { // NEW
        m_currentNdefOptions.fadeTimeValue = m_ndefFadeTimeSpinBox->value();
    }


    updateNdefEnhancementOptionsUI(); // Enable/disable group based on style

    // Refresh the Ndef display for the currently selected tweet
    QListWidgetItem* currentItem = m_tweetListWidget->currentItem();
    const TweetData* tweet = nullptr;
    if (currentItem) {
        QString tweetId = currentItem->data(Qt::UserRole).toString();
        tweet = m_tweetRepository->findTweetById(tweetId);
    }
    displayNdefCode(tweet); // Pass current tweet (or nullptr if none selected)
}

void MainWindow::updateNdefEnhancementOptionsUI() {
    if (!m_ndefEnhancementsGroup || !m_ndefAddSplayAzCheckBox || 
        !m_ndefSplayChannelsLabel || !m_ndefSplayChannelsSpinBox ||
        !m_ndefSetFadeTimeCheckBox || !m_ndefFadeTimeLabel || !m_ndefFadeTimeSpinBox) {
        // UI elements not fully created yet
        return;
    }

    bool enableEnhancementsGroup = (m_currentNdefOptions.style == NdefFormattingOptions::Style::ReformattedAST);
    m_ndefEnhancementsGroup->setEnabled(enableEnhancementsGroup);

    // Enable/disable individual controls within the group only if the group itself is enabled
    if (enableEnhancementsGroup) {
        bool enableSplayChannels = m_ndefAddSplayAzCheckBox->isChecked();
        m_ndefSplayChannelsLabel->setEnabled(enableSplayChannels);
        m_ndefSplayChannelsSpinBox->setEnabled(enableSplayChannels);

        bool enableFadeTimeControls = m_ndefSetFadeTimeCheckBox->isChecked();
        m_ndefFadeTimeLabel->setEnabled(enableFadeTimeControls);
        m_ndefFadeTimeSpinBox->setEnabled(enableFadeTimeControls);

        // The .expand and .reshaping checkboxes are enabled/disabled with the group.
    } else {
        // If the main group is disabled, all its children should appear disabled too.
        // But explicitly setting them ensures correct visual state if they were individually disabled before.
        m_ndefSplayChannelsLabel->setEnabled(false);
        m_ndefSplayChannelsSpinBox->setEnabled(false);
        m_ndefFadeTimeLabel->setEnabled(false);
        m_ndefFadeTimeSpinBox->setEnabled(false);
    }
}


// --- Setup QActions ---
void MainWindow::setupActions()
{
    m_focusSearchAction = new QAction("Focus Search", this);
    m_focusSearchAction->setShortcut(QKeySequence::Find);
    m_focusSearchAction->setToolTip("Focus the search field (Ctrl+F)");
    this->addAction(m_focusSearchAction); 
    connect(m_focusSearchAction, &QAction::triggered, this, &MainWindow::focusSearchField);

    m_toggleFavoriteAction = new QAction("&Toggle Favorite", this); 
    m_toggleFavoriteAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    m_toggleFavoriteAction->setToolTip("Mark/Unmark selected tweet as favorite (Ctrl+D)");
    this->addAction(m_toggleFavoriteAction); 
    connect(m_toggleFavoriteAction, &QAction::triggered, this, &MainWindow::toggleCurrentTweetFavorite);
}

// --- Setup Menu Bar ---
void MainWindow::setupMenuBar()
{
    m_menuBar = menuBar();

    m_fileMenu = m_menuBar->addMenu("&File");
    m_newTweetAction = new QAction("&New Tweet...", this);
    m_newTweetAction->setShortcut(QKeySequence::New);
    m_fileMenu->addAction(m_newTweetAction);

    m_saveAllAction = new QAction("&Save All Changes", this);
    m_saveAllAction->setShortcut(QKeySequence::Save);
    m_fileMenu->addAction(m_saveAllAction);

    m_fileMenu->addSeparator();
    m_exitAction = new QAction("E&xit", this);
    m_exitAction->setShortcut(QKeySequence::Quit); 
    m_fileMenu->addAction(m_exitAction);

    m_editMenu = m_menuBar->addMenu("&Edit");
    m_editTweetAction = new QAction("&Edit Selected Tweet...", this);
    m_editTweetAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
    m_editMenu->addAction(m_editTweetAction);

    m_deleteTweetAction = new QAction("&Delete Selected Tweet", this);
    m_deleteTweetAction->setShortcut(QKeySequence::Delete);
    m_editMenu->addAction(m_deleteTweetAction);

    m_editMenu->addSeparator();
    m_editMenu->addAction(m_toggleFavoriteAction); 

    m_editMenu->addSeparator();
    m_copyCodeAction = new QAction("&Copy Code", this);
    m_copyCodeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C));
    m_editMenu->addAction(m_copyCodeAction);

    m_helpMenu = m_menuBar->addMenu("&Help");
    m_aboutAction = new QAction("&About SCTweetAlchemy", this);
    m_helpMenu->addAction(m_aboutAction);

    QAction *aboutQtAction = new QAction("About &Qt", this);
    m_helpMenu->addAction(aboutQtAction); 

    connect(m_newTweetAction, &QAction::triggered, this, &MainWindow::onFileNewTweet);
    connect(m_saveAllAction, &QAction::triggered, this, &MainWindow::onFileSaveAllChanges);
    connect(m_exitAction, &QAction::triggered, qApp, &QApplication::quit);
    connect(m_editTweetAction, &QAction::triggered, this, &MainWindow::onEditTweet);
    connect(m_deleteTweetAction, &QAction::triggered, this, &MainWindow::onEditDeleteTweet);
    connect(m_copyCodeAction, &QAction::triggered, this, &MainWindow::onEditCopyCode);
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::onHelpAbout);
    connect(aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);
}

// --- Connect Signals ---
void MainWindow::connectSignals()
{
    connect(m_tweetRepository, &TweetRepository::loadError, this, &MainWindow::handleRepositoryLoadError);
    connect(m_tweetRepository, &TweetRepository::tweetsLoaded, this, &MainWindow::handleTweetsLoaded);
    connect(m_tweetRepository, &TweetRepository::tweetsModified, this, &MainWindow::handleTweetsModified);

    connect(m_favoritesManager, &FavoritesManager::favoritesChanged, this, &MainWindow::handleFavoritesChanged);

    connect(m_tweetListWidget, &QListWidget::currentItemChanged, this, &MainWindow::onTweetSelectionChanged);
    connect(m_tweetListWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::onTweetItemDoubleClicked);
    connect(m_tweetListWidget, &QListWidget::customContextMenuRequested, this, &MainWindow::onTweetListContextMenuRequested);
    
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(m_searchLineEdit, &SearchLineEdit::navigationKeyPressed, this, &MainWindow::onSearchNavigateKey);
    
    connect(m_filterPanelWidget, &FilterPanelWidget::filtersChanged, this, &MainWindow::applyAllFilters);
}

// --- Context Menu for Tweet List ---
void MainWindow::onTweetListContextMenuRequested(const QPoint &pos)
{
    QListWidgetItem* itemUnderMouse = m_tweetListWidget->itemAt(pos);
    QMenu contextMenu(this);

    if (!m_editTweetAction || !m_deleteTweetAction || !m_toggleFavoriteAction) {
        qWarning() << "Context menu actions not initialized properly!";
        return;
    }

    if (itemUnderMouse) {
        // Ensure the item under mouse is the current item for actions
        // m_tweetListWidget->setCurrentItem(itemUnderMouse); // Optional: force selection

        contextMenu.addAction(m_editTweetAction);
        contextMenu.addAction(m_deleteTweetAction);
        contextMenu.addAction(m_toggleFavoriteAction);
        contextMenu.addSeparator(); 
        if (m_copyCodeAction) {
            contextMenu.addAction(m_copyCodeAction);
        }
    } else {
        // Optional: actions for when clicking on empty space
        // if (m_newTweetAction) { contextMenu.addAction(m_newTweetAction); }
    }

    if (!contextMenu.isEmpty()) {
        contextMenu.exec(m_tweetListWidget->mapToGlobal(pos));
    }
}

// --- Slot Implementations for Managers & Events ---
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
        if (td) displayTweetDetails(td); // Refreshes metadata including favorite status
    } else if (m_tweetListWidget->count() == 0) {
        displayTweetDetails(nullptr);
    }
    updateActionStates();
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
    applyAllFilters(); 
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
        } else {
            QMessageBox::critical(this, "Error", "Failed to add the new tweet (e.g., ID conflict).");
        }
    }
}

void MainWindow::onFileSaveAllChanges()
{
    if (m_tweetRepository) {
        if (m_tweetRepository->saveTweetsToResource()) { 
            statusBar()->showMessage("Tweet collection saved successfully.", 3000);
        }
    }
}

void MainWindow::onEditTweet()
{
    QListWidgetItem* currentItem = m_tweetListWidget->currentItem();
    if (!currentItem) return;
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
        } else {
            QMessageBox::critical(this, "Error", "Failed to update the tweet.");
        }
    }
}

void MainWindow::onEditDeleteTweet()
{
    QListWidgetItem* currentItem = m_tweetListWidget->currentItem();
    if (!currentItem) return;
    QString tweetId = currentItem->data(Qt::UserRole).toString();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm Delete",
                                  QString("Are you sure you want to delete tweet '%1'?").arg(tweetId),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (m_tweetRepository && m_tweetRepository->deleteTweet(tweetId)) {
            qInfo() << "Tweet deleted:" << tweetId;
        } else {
            QMessageBox::critical(this, "Error", "Failed to delete the tweet.");
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
        statusBar()->showMessage(QString("Original code for '%1' copied!").arg(tweetId), 2000);
    }
}

void MainWindow::onHelpAbout()
{
    QString aboutText = 
        "<h2>SCTweetAlchemy</h2>"
        "<p>SuperCollider Tweet Browser</p>"
        "<p>Version 0.3</p><br/>"
        "<p>Created by Kosmas Giannoutakis.<br/>"
        "<a href=\"https://www.kosmasgiannoutakis.art/\">https://www.kosmasgiannoutakis.art/</a></p><br/>"
        "<p>A tool to browse, manage, study and utilize SCTweets for live coding.</p>";
    QMessageBox::about(this, "About SCTweetAlchemy", aboutText);
}

// --- Core UI Interaction Slots & Helpers ---
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
    m_tweetListWidget->blockSignals(true); 

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
        m_tweetListWidget->setCurrentRow(0); 
    } else {
        displayTweetDetails(nullptr); 
        onTweetSelectionChanged(nullptr, nullptr); // Ensure states update if list is empty
    }
    updateActionStates(); 
}

void MainWindow::onTweetSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);
    if (!current) {
        displayTweetDetails(nullptr);
        m_tweetListWidget->setProperty("selectedTweetId", QVariant()); 
    } else {
        QString selectedId = current->data(Qt::UserRole).toString();
        m_tweetListWidget->setProperty("selectedTweetId", selectedId); 
        const TweetData* tweet = m_tweetRepository->findTweetById(selectedId);
        displayTweetDetails(tweet);
    }
    updateActionStates();
}

void MainWindow::onSearchTextChanged(const QString &text) { applyAllFilters(); }

void MainWindow::onSearchNavigateKey(QKeyEvent *event) {
    Q_UNUSED(event);
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
    // Update Original Code and Metadata display
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

    // Update Ndef Code display
    displayNdefCode(tweet); // Call the separate function for Ndef code
}

// This is the new separate function for Ndef display logic from previous step
void MainWindow::displayNdefCode(const TweetData* tweet)
{
    if (!m_ndefCodeTextEdit || !m_ndefGenerator) return; 

    if (tweet) {
        // Pass the current options to the generator
        QString ndefCode = m_ndefGenerator->generateNdef(tweet->originalCode, tweet->id, m_currentNdefOptions);
        
        // Update tooltip based on current style
        if (m_currentNdefOptions.style == NdefFormattingOptions::Style::ReformattedAST) {
             m_ndefCodeTextEdit->setToolTip("Ndef generated using Tree-sitter AST reconstruction (experimental formatting).");
        } else {
             m_ndefCodeTextEdit->setToolTip("Simple playable Ndef (minimal processing).");
        }
        m_ndefCodeTextEdit->setText(ndefCode);
    } else {
        m_ndefCodeTextEdit->clear();
        m_ndefCodeTextEdit->setPlaceholderText("Select a tweet to see its Ndef version.");
        m_ndefCodeTextEdit->setToolTip("");
    }
}


void MainWindow::updateActionStates()
{
    bool itemSelected = (m_tweetListWidget && m_tweetListWidget->currentItem() != nullptr);

    if(m_editTweetAction) m_editTweetAction->setEnabled(itemSelected);
    if(m_deleteTweetAction) m_deleteTweetAction->setEnabled(itemSelected);
    if(m_copyCodeAction) m_copyCodeAction->setEnabled(itemSelected);
    if(m_toggleFavoriteAction) m_toggleFavoriteAction->setEnabled(itemSelected);
    
    if(m_saveAllAction && m_tweetRepository) {
        m_saveAllAction->setEnabled(!m_tweetRepository->getCurrentResourcePath().startsWith(":/"));
    }
}