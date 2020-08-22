#include <QDir>
#include <QDirIterator>

#include <QFileDialog>
#include <QMessageBox>

#include <QWidget>
#include <QCloseEvent>

#include <QHeaderView>

#include <QStandardItemModel>

#include <QStandardItem>
#include <QDateTime>

#include <QClipboard>

#include <QFile>
#include <QFileInfo>

#include <QTextDocument>

#ifdef Q_OS_WIN
	#include <qt_windows.h>
#endif

#include "CMainWindow.h"
#include "CAboutDlg.h"
#include "CProfileDlg.h"
#include "CGroupDlg.h"
#include "CConfigDlg.h"
#include "CFindReplaceDlg.h"

#include "Utils/CUtils.h"

#include "Model/qTagger/CTagResultItem.h"

CMainWindow::CMainWindow(QWidget* parent)
: QMainWindow(parent),
bTagBuildInProgress_(false)
{
	bool bAutoHideMenu;

    setupUi(this);

	setWindowIcon(QIcon(":/Images/graphics3.png"));

    // setting for progress bar
    progressBar_.setTextVisible(false);
    progressBar_.setMaximumSize(100, 16);
    progressBar_.setRange(0, 100);
    progressBar_.setValue(0);
    progressBar_.hide();

	// hiding menuBar if needed
	confManager_ = CConfigManager::getInstance();
	bAutoHideMenu = confManager_->getAppSettingValue("AutoHideMenu", false).toBool();
	if (bAutoHideMenu) {
		menuBar()->hide();
	}

    // add progressbar for status bar
    statusBar()->addPermanentWidget(&progressBar_, 1);

    // setting for timeline
    timeLine_.setDuration(200);
    timeLine_.setFrameRange(0, 100);

    // defining shortcut

	// filter under profile tab
	profilePatternLineEditShortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_P), this);
	// filter under group tab
	groupPatternLineEditShortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_G), this);

	fileSearchShortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_F), this);
	tagSearchShortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_S), this);

	outputExploreShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_E), this);
	outputConsoleShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_M), this);

	profileLoadShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_L), this);
	profileUpdateShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_U), this);

	symbolSearchFrameShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F), this);
	symbolSearchFrameShortcut->setContext(Qt::ApplicationShortcut);

	nextSymbolSearchShortcut = new QShortcut(QKeySequence::FindNext, this);
	nextSymbolSearchShortcut->setContext(Qt::ApplicationShortcut);

	previousSymbolSearchShortcut = new QShortcut(QKeySequence::FindPrevious, this);
	previousSymbolSearchShortcut->setContext(Qt::ApplicationShortcut);

#ifdef Q_OS_WIN
	previousSymbolSearchShortcut_win = new QShortcut(QKeySequence(Qt::Key_F4), this);
	previousSymbolSearchShortcut_win->setContext(Qt::ApplicationShortcut);
#endif

    // shortcut in application context
	profilePatternLineEditShortcut->setContext(Qt::ApplicationShortcut);
	groupPatternLineEditShortcut->setContext(Qt::ApplicationShortcut);

	fileSearchShortcut->setContext(Qt::ApplicationShortcut);
	tagSearchShortcut->setContext(Qt::ApplicationShortcut);

	outputExploreShortcut->setContext(Qt::ApplicationShortcut);
	outputConsoleShortcut->setContext(Qt::ApplicationShortcut);

	profileLoadShortcut->setContext(Qt::ApplicationShortcut);
	profileUpdateShortcut->setContext(Qt::ApplicationShortcut);

	// profile_listView, profileListModel_
	profileListModel_ = new CProfileListModel(this);

	profileListProxyModel_ = new QSortFilterProxyModel;
	profileListProxyModel_->setSourceModel(static_cast <QStandardItemModel*> (profileListModel_));
    profileListProxyModel_->setDynamicSortFilter(true);

	profileListSelectionModel_ = new QItemSelectionModel(profileListProxyModel_);

	profile_listView->setRootIsDecorated(false);
	profile_listView->setModel(profileListProxyModel_);

	profile_listView->setSelectionModel(profileListSelectionModel_);
	profile_listView->setSortingEnabled(true);

	profile_listView->setDragEnabled(false);
	profile_listView->setAcceptDrops(true);
	profile_listView->setDropIndicatorShown(true);
    updateProfileListWidget();

	// group_listView, groupListModel_
	groupListModel_ = new CGroupListModel(this);

	groupListProxyModel_ = new QSortFilterProxyModel;
	groupListProxyModel_->setSourceModel(static_cast <QStandardItemModel*> (groupListModel_));
    groupListProxyModel_->setDynamicSortFilter(true);

	groupListSelectionModel_ = new QItemSelectionModel(groupListProxyModel_);

	group_listView->setRootIsDecorated(false);
	group_listView->setModel(groupListProxyModel_);

	group_listView->setSelectionModel(groupListSelectionModel_);
	group_listView->setSortingEnabled(true);

	group_listView->setDragEnabled(false);
	group_listView->setAcceptDrops(true);
	group_listView->setDropIndicatorShown(true);
    updateGroupListWidget();

	// output_listView, outputListModel_
	outputListModel_ = new COutputListModel(this);

	output_listView->setOutputListModel(outputListModel_);

	output_listView->setRootIsDecorated(false);
	output_listView->setModel(outputListModel_->getProxyModel());

	output_listView->setSelectionModel(outputListModel_->getSelectionModel());
	output_listView->setSortingEnabled(true);

	output_listView->setDragEnabled(true);
	output_listView->setAcceptDrops(false);

	// view
	bool bAlwaysOnTop;
	bool bTransparent;

	bool bViewToolbar;
	bool bViewProfilePanel;
	bool bViewFilePanel;

	bAlwaysOnTop = confManager_->getAppSettingValue("AlwaysOnTop", false).toBool();
	if (bAlwaysOnTop) {
		setAlwaysOnTop(true);
		actionAlways_on_top->setChecked(true);
	}

    bTransparent = confManager_->getAppSettingValue("Transparency", false).toBool();
	if (bTransparent) {
		setTransparency(true);
		actionTransparent->setChecked(true);
	}

	bViewToolbar = confManager_->getAppSettingValue("ViewToolbar", true).toBool();
	if (!bViewToolbar) {
		toolBar->hide();
		actionToolbar->setChecked(false);
	} else {
        toolBar->show();
		actionToolbar->setChecked(true);
	}

	bViewProfilePanel = confManager_->getAppSettingValue("ViewProfilePanel", true).toBool();
	if (!bViewProfilePanel) {
	    mainTabWidget->hide();
		actionProfile_Panel->setChecked(false);
	} else {
        mainTabWidget->show();
		actionProfile_Panel->setChecked(true);
	}

	bViewFilePanel = confManager_->getAppSettingValue("ViewFilePanel", true).toBool();
	if (!bViewFilePanel) {
	    infoTabWidget->hide();
		actionFile_Panel->setChecked(false);
	} else {
        infoTabWidget->show();
		actionFile_Panel->setChecked(true);
	}

	searchFrame->hide();

	bool bProfileAndGroupFilterCaseSensitive;
	bool bFileFilterCaseSensitive;
	bool bSymbolSearchCaseSensitive;
	bool bSymbolSearchRegularExpression;

	bProfileAndGroupFilterCaseSensitive = confManager_->getAppSettingValue("ProfileAndGroupFilterCaseSensitive", false).toBool();
	if (bProfileAndGroupFilterCaseSensitive) {
		actionProfileAndGroupCaseSensitive->setChecked(true);
	} else {
		actionProfileAndGroupCaseSensitive->setChecked(false);
	}

	bFileFilterCaseSensitive = confManager_->getAppSettingValue("FileFilterCaseSensitive", false).toBool();
	if (bFileFilterCaseSensitive) {
		actionFileCaseSensitive->setChecked(true);
	} else {
		actionFileCaseSensitive->setChecked(false);
	}

	// Profile Font
	QString profileFontSettingStr;
	QFont profileFont;

	profileFontSettingStr = confManager_->getAppSettingValue("ProfileFont").toString();
	profileFont.fromString(profileFontSettingStr);

	if (profileFontSettingStr != "") {
		profile_listView->updateProfileFont(profileFont);
		output_listView->updateOutputFont(profileFont); // update output font as well
		group_listView->updateGroupFont(profileFont); // update group font as well
	} else {
		profile_listView->updateProfileFont(QApplication::font()); // using system font by default
		output_listView->updateOutputFont(QApplication::font());
		group_listView->updateGroupFont(QApplication::font());
	}

	// text document for symbol, need to be initialized before setting symbol font
	textLayout_ = new QPlainTextDocumentLayout(&textDocument_);
	textDocument_.setDocumentLayout(textLayout_);

	// Symbol font
	QString symbolFontSettingStr;
	QFont symbolFont;

	symbolFontSettingStr = confManager_->getAppSettingValue("SymbolFont").toString();
	symbolFont.fromString(symbolFontSettingStr);

	if (symbolFontSettingStr != "") {
		setSymbolFont(symbolFont);
	} else {
		setSymbolFont(QApplication::font()); // using system font by default
	}

	// case sensitive
    bSymbolSearchCaseSensitive = confManager_->getAppSettingValue("SymbolSearchCaseSensitive", false).toBool();
	if (bSymbolSearchCaseSensitive) {
		actionSymbolCaseSensitive->setChecked(true);
	} else {
		actionSymbolCaseSensitive->setChecked(false);
	}

	// symbol regular expression
    bSymbolSearchRegularExpression = confManager_->getAppSettingValue("SymbolSearchRegularExpression", false).toBool();
	if (bSymbolSearchRegularExpression) {
		actionSymbolRegularExpression->setChecked(true);
	} else {
		actionSymbolRegularExpression->setChecked(false);
	}

    createActions();
}

void CMainWindow::setSplitterSizes(const QList<int>& splitterSizeList)
{
	splitter->setSizes(splitterSizeList);
}

void CMainWindow::restoreTabWidgetPos()
{
    // tab widget
    int profileTabIndex = confManager_->getValue("Window", "profileTabIndex", 0).toInt();
    int fileTabIndex = confManager_->getValue("Window", "fileTabIndex", 0).toInt();

    // group instead of profile tab first
    if (profileTabIndex == 1) {
        mainTabWidget->clear();

        // from ui_mainWindow.h
        QIcon icon15;
        icon15.addFile(QString::fromUtf8(":/Icons/22x22/document-open-6.png"), QSize(), QIcon::Normal, QIcon::Off);
        mainTabWidget->addTab(groupTab, icon15, QString());

        QIcon icon14;
        icon14.addFile(QString::fromUtf8(":/Icons/22x22/view-process-all.png"), QSize(), QIcon::Normal, QIcon::Off);
        mainTabWidget->addTab(profileTab, icon14, QString());

        mainTabWidget->setTabText(mainTabWidget->indexOf(profileTab), QCoreApplication::translate("mainWindow", "Profile", 0, 0));
        mainTabWidget->setTabText(mainTabWidget->indexOf(groupTab), QCoreApplication::translate("mainWindow", "Group", 0, 0));
    }

    // symbol instead of file tab first
    if (fileTabIndex == 1) {
        infoTabWidget->clear();

        QIcon icon17;
        icon17.addFile(QString::fromUtf8(":/Icons/22x22/code-class.png"), QSize(), QIcon::Normal, QIcon::Off);
        infoTabWidget->addTab(symbolTab, icon17, QString());

        QIcon icon16;
        icon16.addFile(QString::fromUtf8(":/Icons/22x22/document-new-4.png"), QSize(), QIcon::Normal, QIcon::Off);
        infoTabWidget->addTab(fileTab, icon16, QString());

        infoTabWidget->setTabText(infoTabWidget->indexOf(fileTab), QCoreApplication::translate("mainWindow", "File", 0, 0));
        infoTabWidget->setTabText(infoTabWidget->indexOf(symbolTab), QCoreApplication::translate("mainWindow", "Symbol", 0, 0));
    }
}

void CMainWindow::updateProfileListWidget()
{
	profile_listView->resizeColumnToContents(0);
	profile_listView->resizeColumnToContents(1);
	profile_listView->resizeColumnToContents(2);
    profile_listView->resizeColumnToContents(3);
}

void CMainWindow::updateGroupListWidget()
{
	group_listView->resizeColumnToContents(0);
	group_listView->resizeColumnToContents(1);
	group_listView->resizeColumnToContents(2);
    group_listView->resizeColumnToContents(3);
}


void CMainWindow::updateOutputListWidget()
{
	output_listView->resizeColumnToContents(0);
	output_listView->resizeColumnToContents(1);
	output_listView->resizeColumnToContents(2);
}

void CMainWindow::setSymbolFont(QFont symbolFont)
{
	QString symbolFontFamily = symbolFont.family();
	QString symbolFontSize = QString::number(symbolFont.pointSize());
	QString lineHeight = QString::number(symbolFont.pointSize() / 3);

	QString textDocumentSyleStr = QString("a {color: #0066FF; font-weight: bold; font-family: %1; text-decoration: none} functionsig {color: #33F000; font-weight:bold; font-family: %1;} code { background: #FAFAFA; display: table-row; font-family: Consolas; white-space: nowrap} linenum {color: #9999CC; font-family: %1} keyword {color: #00CCCC; font-weight:bold} spacesize {font-size: %3pt} body {background: #FAFAFA; font-size: %2pt}").arg(symbolFontFamily, symbolFontSize, lineHeight);

	textDocument_.setDefaultStyleSheet(textDocumentSyleStr);
}

void CMainWindow::loadProfileList()
{
    QStringList profileList;

  	profileListModel_->clear();  // header data will also be clear
	profileListModel_->setColumnCount(4); // need to set back column count when QStandardItemModel clear otherwise setData will return false

    profileListModel_->setHeaderData(0, Qt::Horizontal, QObject::tr("Name"));
    profileListModel_->setHeaderData(1, Qt::Horizontal, QObject::tr("Tag Update Datatime"));
    profileListModel_->setHeaderData(2, Qt::Horizontal, QObject::tr("Profile Create Datetime"));
    profileListModel_->setHeaderData(3, Qt::Horizontal, QObject::tr("Labels"));

	QMap<QString, CProfileItem> profileMap;

	CProfileManager::getInstance()->getProfileMap(profileMap);

    foreach (const CProfileItem& profileItem, profileMap) {
		profileListModel_->addProfileItem(profileItem);
    }

	updateProfileListWidget();
}

void CMainWindow::loadGroupList()
{
    QStringList groupList;
	CGroupItem groupItem;

  	groupListModel_->clear();  // header data will also be clear
	groupListModel_->setColumnCount(4); // need to set back column count when QStandardItemModel clear otherwise setData will return false

    groupListModel_->setHeaderData(0, Qt::Horizontal, QObject::tr("Name"));
    groupListModel_->setHeaderData(1, Qt::Horizontal, QObject::tr("Tag Update Datatime"));
    groupListModel_->setHeaderData(2, Qt::Horizontal, QObject::tr("Group Create Datetime"));
    groupListModel_->setHeaderData(3, Qt::Horizontal, QObject::tr("Labels"));

	QMap<QString, CGroupItem> groupMap;
	CProfileManager::getInstance()->getGroupMap(groupMap);

    foreach (const CGroupItem& groupItem, groupMap) {
		groupListModel_->addGroupItem(groupItem);
    }

	updateGroupListWidget();
}


void CMainWindow::loadOutputList()
{
	COutputItem outputItem;

	outputListModel_->clearAndResetModel();

	foreach (const COutputItem& outputItem, outputItemList_) {
		outputListModel_->addItem(outputItem);
	}
	updateOutputListWidget();
}

void CMainWindow::createActions()
{
    connect(actionAbout, SIGNAL(triggered()), this, SLOT(on_aboutButton_clicked()));

	// [Profile action]
    connect(actionProfileNew, SIGNAL(triggered()), this, SLOT(on_newProfileButton_clicked()));

	connect(actionProfileLoad, SIGNAL(triggered()), this, SLOT(on_loadProfileButton_clicked()));

	// default double click, enter action for profile list item
	connect(profile_listView, SIGNAL(profileItemTriggered()), this, SLOT(on_loadProfileButton_clicked()));

	connect(actionProfileRebuildTag, SIGNAL(triggered()), this, SLOT(on_rebuildTagProfileButton_clicked()));

	connect(actionProfileModify, SIGNAL(triggered()), this, SLOT(on_editProfileButton_clicked()));

	connect(actionProfileDelete, SIGNAL(triggered()), this, SLOT(on_deleteProfileButton_clicked()));

	connect(actionProfileExplore, SIGNAL(triggered()), this, SLOT(on_exploreProfileButton_clicked()));
	connect(actionProfileConsole, SIGNAL(triggered()), this, SLOT(on_consoleProfileButton_clicked()));

	connect(actionProfileGroup, SIGNAL(triggered()), this, SLOT(on_newGroupButton_clicked()));

	// [Group action]
    connect(actionGroupNew, SIGNAL(triggered()), this, SLOT(on_newGroupButton_clicked()));

	connect(actionGroupLoad, SIGNAL(triggered()), this, SLOT(on_loadGroupButton_clicked()));

	// default double click, enter action for group list item
	connect(group_listView, SIGNAL(groupItemTriggered()), this, SLOT(on_loadGroupButton_clicked()));

	connect(actionGroupUpdate, SIGNAL(triggered()), this, SLOT(on_updateGroupButton_clicked()));
	connect(actionGroupModify, SIGNAL(triggered()), this, SLOT(on_editGroupButton_clicked()));

	connect(actionGroupDelete, SIGNAL(triggered()), this, SLOT(on_deleteGroupButton_clicked()));

    // [File action]
    connect(actionOutputEdit, SIGNAL(triggered()), this, SLOT(on_outputEditPressed()));

    // default double click, enter action for output list item
	connect(output_listView, SIGNAL(outputItemTriggered()), this, SLOT(on_outputEditPressed()));

	connect(actionOutputCopy, SIGNAL(triggered()), this, SLOT(on_outputCopyPressed()));
	connect(actionOutputExplore, SIGNAL(triggered()), this, SLOT(on_outputExplorePressed()));
	connect(actionOutputConsole, SIGNAL(triggered()), this, SLOT(on_outputConsolePressed()));
    connect(actionOutputProperties, SIGNAL(triggered()), this, SLOT(on_outputPropertiesPressed()));

	connect(actionSearch, SIGNAL(triggered()), this, SLOT(on_searchButton_clicked()));

	connect(actionNextSymbolSearch, SIGNAL(triggered()), this, SLOT(on_nextSymbolButton_clicked()));
	connect(actionPreviousSymbolSearch, SIGNAL(triggered()), this, SLOT(on_previousSymbolButton_clicked()));

	connect(search_lineEdit, SIGNAL(returnPressed()), this, SLOT(on_searchButton_clicked()));

    connect(CProfileManager::getInstance(), SIGNAL(profileMapUpdated()), this, SLOT(loadProfileList()));
	connect(CProfileManager::getInstance(), SIGNAL(groupMapUpdated()), this, SLOT(loadGroupList()));

    connect(&timeLine_, SIGNAL(frameChanged(int)), &progressBar_, SLOT(setValue(int)));
    connect(&profileUpdateThread_, SIGNAL(percentageCompleted(int)), this, SLOT(updateTagBuildProgress(int)));

	// update progress bar for cancelled tag build
	connect(&profileUpdateThread_, SIGNAL(cancelledTagBuild()), this, SLOT(updateCancelledTagBuild()));
	// error occurs during run command
	connect(&profileUpdateThread_, SIGNAL(errorDuringRun(const QString&)), this, SLOT(on_errorDuringRun(const QString&)));

	connect(&profileLoadThread_, SIGNAL(profileLoadPercentageCompleted(int)), this, SLOT(updateProfileLoadProgress(int)));

	connect(&groupLoadThread_, SIGNAL(groupLoadPercentageCompleted(int)), this, SLOT(updateGroupLoadProgress(int)));

    // connecting shortcut action
	connect(profilePatternLineEditShortcut, SIGNAL(activated()), this, SLOT(on_profilePatternLineEditShortcutPressed()));
	connect(groupPatternLineEditShortcut, SIGNAL(activated()), this, SLOT(on_groupPatternLineEditShortcutPressed()));

    connect(fileSearchShortcut, SIGNAL(activated()), this, SLOT(on_filePatternLineEditShortcutPressed()));
	connect(tagSearchShortcut, SIGNAL(activated()), this, SLOT(on_searchLineEditShortcutPressed()));

    connect(outputExploreShortcut, SIGNAL(activated()), this, SLOT(on_outputExplorePressed()));
	connect(outputConsoleShortcut, SIGNAL(activated()), this, SLOT(on_outputConsolePressed()));

	connect(profilePattern_lineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(profileFilterRegExpChanged()));
	connect(groupPattern_lineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(groupFilterRegExpChanged()));

    connect(actionProfileAndGroupCaseSensitive, SIGNAL(toggled(bool)),
            this, SLOT(profileFilterRegExpChanged()));

    connect(actionProfileAndGroupCaseSensitive, SIGNAL(toggled(bool)),
            this, SLOT(groupFilterRegExpChanged()));

    // for cancel update tag
	connect(actionCancelTagUpdate, SIGNAL(triggered()), this, SLOT(on_cancelTagUpdate()));
	// for info tab widget tool button

	connect(filePattern_lineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(fileFilterRegExpChanged()));
    connect(actionFileCaseSensitive, SIGNAL(toggled(bool)),
            this, SLOT(fileFilterRegExpChanged()));
    connect(actionSymbolCaseSensitive, SIGNAL(toggled(bool)),
            this, SLOT(searchLineEditChanged()));
    connect(actionSymbolRegularExpression, SIGNAL(toggled(bool)),
            this, SLOT(searchLineEditChanged()));

	connect(search_lineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(searchLineEditChanged()));

	connect(&completer_, SIGNAL(activated(const QString &)),
            this, SLOT(queryTag(const QString&)));

    // symbol search frame
	connect(symbolSearchFrameShortcut, SIGNAL(activated()), this, SLOT(on_symbolSearchFrameShortcutPressed()));
	connect(frameSymbol_lineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(frameSymbolLineEditChanged()));

	connect(nextSymbolSearchShortcut, SIGNAL(activated()), this, SLOT(on_nextSymbolButton_clicked()));
	connect(previousSymbolSearchShortcut, SIGNAL(activated()), this, SLOT(on_previousSymbolButton_clicked()));

#ifdef Q_OS_WIN
	connect(previousSymbolSearchShortcut_win, SIGNAL(activated()), this, SLOT(on_previousSymbolButton_clicked()));
#endif

	// zoom in, zoom out
	actionWebZoomIn->setShortcut(QKeySequence::ZoomIn);
	actionWebZoomOut->setShortcut(QKeySequence::ZoomOut);

#ifdef ENABLE_SYMBOL_WEB_VIEW
	webView->addAction(actionWebZoomIn);
	webView->addAction(actionWebZoomOut);
#elif defined ENABLE_SYMBOL_RICH_TEXT_VIEW

	/*
	symbol_textBrowser->setContextMenuPolicy(Qt::NoContextMenu);
	symbol_textBrowser->setReadOnly(true);
	symbol_textBrowser->setUndoRedoEnabled(false);
	*/

	symbol_textBrowser->addAction(actionWebZoomIn);
	symbol_textBrowser->addAction(actionWebZoomOut);
#else
	symbol_textBrowser->setReadOnly(true);
	symbol_textBrowser->setLineWrapMode(QPlainTextEdit::NoWrap);

	QTextCharFormat charFormat;
	charFormat.setFontFamily("Consolas");
	charFormat.setFontPointSize(7);

	symbol_textBrowser->setCurrentCharFormat(charFormat);
#endif


	connect(actionWebZoomIn, SIGNAL(triggered()), this, SLOT(webZoomIn()));
	connect(actionWebZoomOut, SIGNAL(triggered()), this, SLOT(webZoomOut()));
}

void CMainWindow::on_newProfileButton_clicked()
{
    QDialog* dialog = new CProfileDlg();
    dialog->exec();
}

// load the profile into environment
void CMainWindow::on_loadProfileButton_clicked()
{
    QStringList profileItemNameList = getSelectedProfileItemNameList();
	int profileSelected = profileItemNameList.size();

	QString profileItemName;
    CProfileItem profileItem;

	if (profileSelected != 0) {
		if (profileSelected > 1) {
			QMessageBox::information(this, "Load", "Only one profile can be loaded each time", QMessageBox::Ok);
		} else {

			profileItemName = profileItemNameList.at(0);
			profileItem = CProfileManager::getInstance()->getProfileItem(profileItemName);

			currentProfileItem_ = profileItem;

			QDir currentDir(QDir::currentPath());

			// only load profile if source directory exists
			if (currentDir.exists(profileItem.srcDir_)) {
				statusBar()->showMessage("Loading profile " + profileItem.name_ + "...");

                filePattern_lineEdit->clear();

				profileLoadThread_.setCurrentProfileItem(profileItem);
				profileLoadThread_.setTaggerPtr(&tagger_);
				profileLoadThread_.setOutputItemListPtr(&outputItemList_);

				profileLoadThread_.start();

			} else {
				QMessageBox::warning(this, "Load", "Cannot load profile. Source directory doesn't exists.", QMessageBox::Ok);
			}
		}
	}
}

// update tags for the profile
void CMainWindow::on_updateProfileButton_clicked()
{
	QStringList profileItemNameList = getSelectedProfileItemNameList();
	int profileSelected = profileItemNameList.size();

    QString profileItemName;
    CProfileItem profileItem;
	QDateTime currDateTime;

    if (profileSelected != 0) {
		if (profileSelected > 1) {
			QMessageBox::information(this, "Update", "Only one profile can be updated each time", QMessageBox::Ok);
		} else {
			profileItemName = profileItemNameList.at(0);

			profileItem = CProfileManager::getInstance()->getProfileItem(profileItemName);

			QDir currentDir(QDir::currentPath());

			// only update profile if source directory exists
			if (currentDir.exists(profileItem.srcDir_)) {
				currDateTime = QDateTime::currentDateTime();
				profileItem.tagUpdateDateTime_ = currDateTime.toString("dd/MM/yyyy hh:mm:ss");

				// tag last update date time updated so need update in profile manager
				CProfileManager::getInstance()->updateProfileItem(profileItemName, profileItem);

				statusBar()->showMessage("Updating tag for " + profileItem.name_ + "...");

				profileUpdateThread_.setRebuildTag(false);
				profileUpdateThread_.setCurrentProfileItem(profileItem);
				profileUpdateThread_.start(QThread::HighestPriority); // priority for update thread
			} else {
                QMessageBox::warning(this, "Load", "Cannot update profile. Source directory doesn't exists.", QMessageBox::Ok);
			}
		}
    }
}

void CMainWindow::on_rebuildTagProfileButton_clicked()
{
	QStringList profileItemNameList = getSelectedProfileItemNameList();
	int profileSelected = profileItemNameList.size();

    QString profileItemName;
    CProfileItem profileItem;
	QDateTime currDateTime;

    if (profileSelected != 0) {
		if (profileSelected > 1) {
			QMessageBox::information(this, "Rebuild", "Only one profile can be rebuilt each time", QMessageBox::Ok);
		} else {
			profileItemName = profileItemNameList.at(0);

			profileItem = CProfileManager::getInstance()->getProfileItem(profileItemName);

			QDir currentDir(QDir::currentPath());

			// only update profile if source directory exists
			if (currentDir.exists(profileItem.srcDir_)) {
				currDateTime = QDateTime::currentDateTime();
				profileItem.tagUpdateDateTime_ = currDateTime.toString("dd/MM/yyyy hh:mm:ss");

				// tag last update date time updated so need update in profile manager
				CProfileManager::getInstance()->updateProfileItem(profileItemName, profileItem);

				statusBar()->showMessage("Rebuilding tag for " + profileItem.name_ + "...");

				profileUpdateThread_.setRebuildTag(true);
				profileUpdateThread_.setCurrentProfileItem(profileItem);
				profileUpdateThread_.start(QThread::HighestPriority); // priority for update thread
			} else {
                QMessageBox::warning(this, "Load", "Cannot rebuilt profile. Source directory doesn't exists.", QMessageBox::Ok);
			}
		}
    }
}


// get the profile name of the selected list item
QStringList CMainWindow::getSelectedProfileItemNameList()
{
    QModelIndexList indexSelectedList;
	QModelIndex mappedIndex;
	int rowSelected;
	QStandardItem* itemSelected;

    QString profileItemName;
	QStringList profileItemNameList;

    // get selected items index list
    indexSelectedList = profileListSelectionModel_->selectedIndexes();

	foreach (const QModelIndex& indexSelected, indexSelectedList) {
		// map back from proxy model
		mappedIndex = profileListProxyModel_->mapToSource(indexSelected);
		rowSelected = mappedIndex.row();

		if (indexSelected.isValid()) {
			itemSelected = profileListModel_->item(rowSelected, 0);
			if (itemSelected != 0) {
				profileItemName = itemSelected->text();
			}
		}

		// as all items in the same row with differnt columns will also be returned in selectedIndexes
		if (!profileItemNameList.contains(profileItemName)) {
			// not add profile name to the list if already added
			profileItemNameList += profileItemName;
		}
	}

    return profileItemNameList;
}

// get the group name of the selected list item
QStringList CMainWindow::getSelectedGroupItemNameList()
{
    QModelIndexList indexSelectedList;
	QModelIndex mappedIndex;
	int rowSelected;
	QStandardItem* itemSelected;

    QString groupItemName;
	QStringList groupItemNameList;

    // get selected items index list
    indexSelectedList = groupListSelectionModel_->selectedIndexes();

	foreach (const QModelIndex& indexSelected, indexSelectedList) {
		// map back from proxy model
		mappedIndex = groupListProxyModel_->mapToSource(indexSelected);
		rowSelected = mappedIndex.row();

		if (indexSelected.isValid()) {
			itemSelected = groupListModel_->item(rowSelected, 0);
			if (itemSelected != 0) {
				groupItemName = itemSelected->text();
			}
		}

		// as all items in the same row with differnt columns will also be returned in selectedIndexes
		if (!groupItemNameList.contains(groupItemName)) {
			// not add group name to the list if already added
			groupItemNameList += groupItemName;
		}
	}

    return groupItemNameList;
}

// get the profile name of the selected list item
QStringList CMainWindow::getSelectedOutputItemNameList()
{
    QModelIndexList indexSelectedList;
	QModelIndex mappedIndex;
	int rowSelected;
	QStandardItem* itemSelected;

    QString outputItemName;
	QStringList outputItemNameList;

    // get selected items index list
    indexSelectedList = outputListModel_->getSelectionModel()->selectedIndexes();

	foreach (const QModelIndex& indexSelected, indexSelectedList) {
		// map back from proxy model
		mappedIndex = outputListModel_->getProxyModel()->mapToSource(indexSelected);
		rowSelected = mappedIndex.row();

		if (indexSelected.isValid()) {
			itemSelected = outputListModel_->item(rowSelected, 0);
			if (itemSelected != 0) {
				outputItemName = itemSelected->text();
			}
		}

		// as all items in the same row with differnt columns will also be returned in selectedIndexes
		if (!outputItemNameList.contains(outputItemName)) {
			// not add profile name to the list if already added
			outputItemNameList += outputItemName;
		}
	}

    return outputItemNameList;
}

// edit selected profile
void CMainWindow::on_editProfileButton_clicked()
{
    CProfileItem profileItem;

	QStringList profileItemNameList = getSelectedProfileItemNameList();
	int profileSelected = profileItemNameList.size();

    QString profileItemName;

	if (profileSelected != 0) { // only do processing when a item is selected
		if (profileSelected > 1) {
			QMessageBox::information(this, "Edit", "Only one profile can be edited each time", QMessageBox::Ok);
		} else {

			profileItemName = profileItemNameList.at(0);

			profileItem = CProfileManager::getInstance()->getProfileItem(profileItemName);

			QDialog* dialog = new CProfileDlg(profileItemName, profileItem);

			dialog->exec();
		}
	}
}

// delete selected profile
void CMainWindow::on_deleteProfileButton_clicked()
{
	QStringList profileItemNameList = getSelectedProfileItemNameList();
	int profileSelected = profileItemNameList.size();

    int ret;

    if (profileSelected != 0) { // only do processing when a item is selected
		if (profileSelected > 1) {
			ret = QMessageBox::question(this, "Confirm multiple delete", "Delete " + QString::number(profileSelected) + " selected profiles?", QMessageBox::Yes, QMessageBox::No);
			if (ret == QMessageBox::Yes) {

				foreach (const QString& profileItemName, profileItemNameList) {
					CProfileManager::getInstance()->removeProfileItem(profileItemName);
				}
			}
		} else {
			QString profileItemName = profileItemNameList.at(0);
			ret = QMessageBox::question(this, "Confirm delete", "Delete the selected profile?", QMessageBox::Yes, QMessageBox::No);
			if (ret == QMessageBox::Yes) {
				CProfileManager::getInstance()->removeProfileItem(profileItemName);
			}
		}
    }
}

void CMainWindow::on_exploreProfileButton_clicked()
{
	CProfileItem profileItem;

	QStringList selectedItemList = getSelectedProfileItemNameList();
	int itemSelected = selectedItemList.size();

	QString profileItemName;

	QString executeDir;

	if (itemSelected > 0) {
		if (itemSelected > 1) {
			QMessageBox::information(this, "Explore", "Can only explore one path", QMessageBox::Ok);
		} else {
			profileItemName = selectedItemList.at(0);
			profileItem = CProfileManager::getInstance()->getProfileItem(profileItemName);

			executeDir = profileItem.srcDir_;

#ifdef Q_OS_WIN
			QString excuteMethod = "explore";
            ShellExecute(NULL, reinterpret_cast<const wchar_t*>(excuteMethod.utf16()), reinterpret_cast<const wchar_t*>(executeDir.utf16()), NULL, NULL, SW_NORMAL);
#else
			QDesktopServices::openUrl(QUrl(executeDir, QUrl::TolerantMode));
#endif
		}
	}
}

void CMainWindow::on_consoleProfileButton_clicked()
{
	CProfileItem profileItem;

	QStringList selectedItemList = getSelectedProfileItemNameList();
	int itemSelected = selectedItemList.size();

	QString profileItemName;

	QString executeDir;

	if (itemSelected > 0) {
		if (itemSelected > 1) {
			QMessageBox::information(this, "Console", "Can only open console from one path", QMessageBox::Ok);
		} else {
			profileItemName = selectedItemList.at(0);
			profileItem = CProfileManager::getInstance()->getProfileItem(profileItemName);

			executeDir = profileItem.srcDir_;

#ifdef Q_OS_WIN
			QString excuteMethod = "open";
			QString consoleCommnad = "cmd.exe";

            ShellExecute(NULL, reinterpret_cast<const wchar_t*>(excuteMethod.utf16()), reinterpret_cast<const wchar_t*>(consoleCommnad.utf16()), NULL, reinterpret_cast<const wchar_t*>(executeDir.utf16()), SW_NORMAL);
#else
			// not implemented
#endif
		}
	}
}

void CMainWindow::on_newGroupButton_clicked()
{
    QDialog* dialog = new CGroupDlg();
    dialog->exec();
}

void CMainWindow::on_loadGroupButton_clicked()
{
    QStringList groupItemNameList = getSelectedGroupItemNameList();
	int groupSelected = groupItemNameList.size();

	QString groupItemName;
    CGroupItem groupItem;

	if (groupSelected != 0) {
		if (groupSelected > 1) {
			QMessageBox::information(this, "Load", "Only one group can be loaded each time", QMessageBox::Ok);
		} else {

			groupItemName = groupItemNameList.at(0);
			groupItem = CProfileManager::getInstance()->getGroupItem(groupItemName);

			currentGroupItem_ = groupItem;

			QDir currentDir(QDir::currentPath());

			// only load group if source directory exists
//			if (currentDir.exists(groupItem.srcDir_)) {
				statusBar()->showMessage("Loading group " + groupItem.name_ + "...");

                filePattern_lineEdit->clear();

				groupLoadThread_.setCurrentGroupItem(groupItem);
				groupLoadThread_.setTaggerPtr(&tagger_);
				groupLoadThread_.setOutputItemListPtr(&outputItemList_);

				groupLoadThread_.start();

/*
			} else {
				QMessageBox::warning(this, "Load", "Cannot load group. Source directory doesn't exists.", QMessageBox::Ok);
			}
*/
		}
	}
}

void CMainWindow::on_updateGroupButton_clicked()
{

}

void CMainWindow::on_editGroupButton_clicked()
{
    CGroupItem groupItem;

	QStringList groupItemNameList = getSelectedGroupItemNameList();
	int groupSelected = groupItemNameList.size();

    QString groupItemName;

	if (groupSelected != 0) { // only do processing when a item is selected
		if (groupSelected > 1) {
			QMessageBox::information(this, "Edit", "Only one group can be edited each time", QMessageBox::Ok);
		} else {

			groupItemName = groupItemNameList.at(0);

			groupItem = CProfileManager::getInstance()->getGroupItem(groupItemName);

			QDialog* dialog = new CGroupDlg(groupItemName, groupItem);

			dialog->exec();
		}
	}
}

void CMainWindow::on_deleteGroupButton_clicked()
{
	QStringList groupItemNameList = getSelectedGroupItemNameList();
	int groupSelected = groupItemNameList.size();

    int ret;

    if (groupSelected != 0) { // only do processing when a item is selected

		if (groupSelected > 1) {
			ret = QMessageBox::question(this, "Confirm multiple delete", "Delete " + QString::number(groupSelected) + " selected groups?", QMessageBox::Yes, QMessageBox::No);
			if (ret == QMessageBox::Yes) {

				foreach (const QString& groupItemName, groupItemNameList) {
					CProfileManager::getInstance()->removeGroupItem(groupItemName);
				}
			}
		} else {
			QString groupItemName = groupItemNameList.at(0);
			ret = QMessageBox::question(this, "Confirm delete", "Delete the selected group?", QMessageBox::Yes, QMessageBox::No);
			if (ret == QMessageBox::Yes) {
				CProfileManager::getInstance()->removeGroupItem(groupItemName);
			}
		}
    }
}

void CMainWindow::on_aboutButton_clicked()
{
    QDialog* dialog = new CAboutDlg();
    dialog->exec();
}

void CMainWindow::on_clearOutputButton_clicked()
{
//    output_listView->clear();
}

void CMainWindow::on_clearLogButton_clicked()
{

}

void CMainWindow::setAlwaysOnTop(bool enable)
{
    Qt::WindowFlags flags = windowFlags();

    if (enable) {
        flags |= Qt::WindowStaysOnTopHint;
    } else {
        flags ^= Qt::WindowStaysOnTopHint;
    }

    QPoint p(frameGeometry().x(),frameGeometry().y()); // current position before set window flags

    setWindowFlags(flags);

    this->move(p);
    this->show();

	// save setting
	confManager_->setAppSettingValue("AlwaysOnTop", enable);
}

void CMainWindow::on_actionAlways_on_top_toggled()
{
    if (actionAlways_on_top->isChecked()) {
        setAlwaysOnTop(true);
    } else {
        setAlwaysOnTop(false);
    }
    on_actionTransparent_toggled(); // still transparency
}

void CMainWindow::setTransparency(bool enable)
{
    if (enable) {
        setWindowOpacity(0.7);
    } else {
        setWindowOpacity(1);
    }

	// save setting
	confManager_->setAppSettingValue("Transparency", enable);
}

void CMainWindow::on_actionTransparent_toggled()
{
    if (actionTransparent->isChecked()) {
        setTransparency(true);
    } else {
        setTransparency(false);
    }
}

void CMainWindow::on_actionToolbar_toggled()
{
    if (actionToolbar->isChecked()) {
		toolBar->show();
		confManager_->setAppSettingValue("ViewToolbar", true);
    } else {
        toolBar->hide();
		confManager_->setAppSettingValue("ViewToolbar", false);
    }
}

void CMainWindow::on_actionProfile_Panel_toggled()
{
    if (actionProfile_Panel->isChecked()) {
		mainTabWidget->show();
		confManager_->setAppSettingValue("ViewProfilePanel", true);
    } else {
        mainTabWidget->hide();
		confManager_->setAppSettingValue("ViewProfilePanel", true);
    }
}

void CMainWindow::on_actionFile_Panel_toggled()
{
    if (actionFile_Panel->isChecked()) {
		infoTabWidget->show();
		confManager_->setAppSettingValue("ViewFilePanel", true);
    } else {
        infoTabWidget->hide();
		confManager_->setAppSettingValue("ViewFilePanel", true);
    }
}

void CMainWindow::on_actionExit_triggered()
{
    this->close();
}

// configuration dialog
void CMainWindow::on_actionSetting_triggered()
{
	QDialog* dialog = new CConfigDlg();

	int dialogCode = dialog->exec();

	if (dialogCode == QDialog::Accepted) {
		QFont updatedProfileFont = static_cast<CConfigDlg*> (dialog)->getProfileDefaultFont();

		profile_listView->updateProfileFont(updatedProfileFont);
		group_listView->updateGroupFont(updatedProfileFont);
		output_listView->updateOutputFont(updatedProfileFont);

		QFont updatedSymbolFont = static_cast<CConfigDlg*> (dialog)->getSymbolDefaultFont();

		setSymbolFont(updatedSymbolFont);
	}
}

// find & replace dialog
void CMainWindow::on_actionFindReplaceDialog_triggered()
{
	int i = 0;

	const int fileTabIndex = infoTabWidget->indexOf(fileTab);
	const int symbolTabIndex = infoTabWidget->indexOf(symbolTab);

	QStringList fileList;

	// if current tab is file tab
	if (infoTabWidget->currentIndex() == fileTabIndex) {
		// copy from selected file list in file filter
		QModelIndexList selectedIndexList = outputListModel_->getSelectionModel()->selectedIndexes();

		if (selectedIndexList.size() > 0) { // got selected files
			foreach(const QModelIndex &index, selectedIndexList) {
				if (index.column() == 0) { // only 0 column corresponding to filename
					fileList.append(index.data(Qt::DisplayRole ).toString());
				}
			}
		} else { // default to all files if no selection
			for (i = 0; i < outputItemList_.size(); i++) {
				fileList.append(outputItemList_[i].fileName_);
			}
		}

		findReplaceModel_.setFileList(fileList);
	} else {
		fileList = findReplaceFileList_.keys();

		// set default find replace file list model updated when symbol queries
		findReplaceModel_.setFileList(fileList);
	}

	CFindReplaceDlg* dialog = new CFindReplaceDlg(this, &findReplaceModel_);

	if (infoTabWidget->currentIndex() == symbolTabIndex) {
	   dialog->setFindLineEdit(search_lineEdit->text());
	}

	// non model dialog
	dialog->show();
}

void CMainWindow::saveWidgetPosition()
{
	QList<int> splitterSizeList;
	QString splitterSizeListStr = "";

	confManager_->setValue("Window", "geometry", saveGeometry());

    splitterSizeList = splitter->sizes();
	foreach (const int& splitterSize, splitterSizeList) {
		splitterSizeListStr += QString::number(splitterSize) + " ";
	}

	confManager_->setValue("Window", "splitter", splitterSizeListStr);

	int profileTabIndex = mainTabWidget->indexOf(profileTab);
	int fileTabIndex = infoTabWidget->indexOf(fileTab);

	confManager_->setValue("Window", "profileTabIndex", QString::number(profileTabIndex));
	confManager_->setValue("Window", "fileTabIndex", QString::number(fileTabIndex));

    confManager_->updateConfig();
}

void CMainWindow::closeEvent(QCloseEvent *event)
{
    saveWidgetPosition();
    event->accept();
}

void CMainWindow::updateTagBuildProgress(int percentage)
{
    // show the progress bar when pecentage completed >= 0
    if (percentage >= 0) {
        progressBar_.show();
		bTagBuildInProgress_ = true;
    }

    progressBar_.setValue(percentage);

    // hide the progress bar when completed
    if (percentage == 100) {
        statusBar()->showMessage("Tag update completed.");
		bTagBuildInProgress_ = false;
        progressBar_.hide();
    }
}

void CMainWindow::updateProfileLoadProgress(int percentage)
{
    if (percentage == 100) {
		loadOutputList();

		search_lineEdit->clear(); // clear symbol search line edit
		symbol_textBrowser->clear(); // clear symbol text widget as well

        statusBar()->showMessage("Profile " + currentProfileItem_.name_ + " loaded.");
    } else if (percentage == 0) {
		statusBar()->showMessage("Failed to load Profile " + currentProfileItem_.name_ + ".");
	}
}

void CMainWindow::updateGroupLoadProgress(int percentage)
{
    if (percentage == 100) {
		loadOutputList();

        statusBar()->showMessage("Group " + currentGroupItem_.name_ + " loaded.");
    } else if (percentage == 0) {
		statusBar()->showMessage("Failed to load Group " + currentGroupItem_.name_ + ".");
	}
}


void CMainWindow::updateCancelledTagBuild()
{
	progressBar_.reset();
	progressBar_.setValue(0);
	progressBar_.hide();
	bTagBuildInProgress_ = false;

	statusBar()->showMessage("Tag update cancelled.");
}

void CMainWindow::on_errorDuringRun(const QString& cmdStr)
{
	progressBar_.reset();
	progressBar_.setValue(0);
	progressBar_.hide();
	bTagBuildInProgress_ = false;

	appendLogList(TRACE_ERROR, cmdStr);

	QMessageBox::warning(this, "Tag update", "Error running command.\n"
											 "Please check log message for details.", QMessageBox::Ok);
}

void CMainWindow::on_profilePatternLineEditShortcutPressed()
{
	const int profileTabIndex = mainTabWidget->indexOf(profileTab);

    mainTabWidget->setCurrentIndex(profileTabIndex);
	profilePattern_lineEdit->setFocus();
}

void CMainWindow::on_groupPatternLineEditShortcutPressed()
{
	const int groupTabIndex = mainTabWidget->indexOf(groupTab);

    mainTabWidget->setCurrentIndex(groupTabIndex);
	groupPattern_lineEdit->setFocus();
}

void CMainWindow::on_filePatternLineEditShortcutPressed()
{
	const int fileTabIndex = infoTabWidget->indexOf(fileTab);

    infoTabWidget->setCurrentIndex(fileTabIndex);
	filePattern_lineEdit->setFocus();
}

void CMainWindow::on_searchLineEditShortcutPressed()
{
	const int symbolTabIndex = infoTabWidget->indexOf(symbolTab);

    infoTabWidget->setCurrentIndex(symbolTabIndex);
	search_lineEdit->setFocus();
}

void CMainWindow::on_infoTabWidgetToolBn_clicked()
{
	if (infoTabWidget->isHidden()) {
		infoTabWidget->show();
	} else {
		priorMainTabWidgetSize_ = infoTabWidget->size();

		infoTabWidget->hide();

		/*
		containerwidget->layout()->invalidate();
		QWidget *parent = containerwidget;
		while (parent) {
			parent->adjustSize();
			parent = parent->parentWidget();
		}
		*/
	}
}

void CMainWindow::profileFilterRegExpChanged()
{
	Qt::CaseSensitivity caseSensitivity;

    if (actionProfileAndGroupCaseSensitive->isChecked()) {
		caseSensitivity = Qt::CaseSensitive;
		confManager_->setAppSettingValue("actionProfileAndGroupCaseSensitive", true);
    } else {
        caseSensitivity = Qt::CaseInsensitive;
		confManager_->setAppSettingValue("actionProfileAndGroupCaseSensitive", false);
    }

	QRegExp regExp(profilePattern_lineEdit->text(), caseSensitivity, QRegExp::RegExp);

    profileListProxyModel_->setFilterRegExp(regExp);
}

void CMainWindow::groupFilterRegExpChanged()
{
	Qt::CaseSensitivity caseSensitivity;

    if (actionProfileAndGroupCaseSensitive->isChecked()) {
		caseSensitivity = Qt::CaseSensitive;
		confManager_->setAppSettingValue("actionProfileAndGroupCaseSensitive", true);
    } else {
        caseSensitivity = Qt::CaseInsensitive;
		confManager_->setAppSettingValue("actionProfileAndGroupCaseSensitive", false);
    }

	QRegExp regExp(groupPattern_lineEdit->text(), caseSensitivity, QRegExp::RegExp);

    groupListProxyModel_->setFilterRegExp(regExp);
}

void CMainWindow::fileFilterRegExpChanged()
{
	Qt::CaseSensitivity caseSensitivity;

    if (actionFileCaseSensitive->isChecked()) {
		caseSensitivity = Qt::CaseSensitive;
		confManager_->setAppSettingValue("FileFilterCaseSensitive", true);
    } else {
        caseSensitivity = Qt::CaseInsensitive;
		confManager_->setAppSettingValue("FileFilterCaseSensitive", false);
    }

	QString trimmedFilePattern = filePattern_lineEdit->text().trimmed() ;

	QRegExp regExp(trimmedFilePattern, caseSensitivity, QRegExp::RegExp);

    outputListModel_->getProxyModel()->setFilterRegExp(regExp);
}

void CMainWindow::searchLineEditChanged()
{
	Qt::CaseSensitivity caseSensitivity;

    if (actionSymbolCaseSensitive->isChecked()) {
		caseSensitivity = Qt::CaseSensitive;
		confManager_->setAppSettingValue("SymbolSearchCaseSensitive", true);
    } else {
        caseSensitivity = Qt::CaseInsensitive;
		confManager_->setAppSettingValue("SymbolSearchCaseSensitive", false);
    }

    if (actionSymbolRegularExpression->isChecked()) {
		confManager_->setAppSettingValue("SymbolSearchRegularExpression", true);
    } else {
		confManager_->setAppSettingValue("SymbolSearchRegularExpression", false);
    }

	QStringList tagList;
	tagger_.getMatchedTags(search_lineEdit->text(), tagList, caseSensitivity);

	stringListModel_.setStringList(tagList);

	completer_.setModel(&stringListModel_);
	completer_.setModelSorting(QCompleter::CaseSensitivelySortedModel);
	completer_.setCaseSensitivity(caseSensitivity);
	completer_.setFilterMode(Qt::MatchContains);

	search_lineEdit->setCompleter(&completer_);
}

void CMainWindow::on_symbolSearchFrameShortcutPressed()
{
	const int symbolTabIndex = infoTabWidget->indexOf(symbolTab);

	if (infoTabWidget->currentIndex() == symbolTabIndex) {
		if (searchFrame->isHidden()) {
			searchFrame->show();
			frameSymbol_lineEdit->setFocus();
		} else {
			searchFrame->hide();
			search_lineEdit->setFocus();
		}
	}
}

void CMainWindow::frameSymbolLineEditChanged()
{
#ifdef ENABLE_SYMBOL_WEB_VIEW
	webView->findText("", (QWebPage::FindFlags) QWebPage::HighlightAllOccurrences);
	webView->findText(frameSymbol_lineEdit->text(), (QWebPage::FindFlags) QWebPage::HighlightAllOccurrences);
#elif defined ENABLE_SYMBOL_RICH_TEXT_VIEW
	symbol_textBrowser->findText("", (QTextDocument::FindFlags) 0);
	symbol_textBrowser->findText(frameSymbol_lineEdit->text(), (QTextDocument::FindFlags) 0);
#else
	symbol_textBrowser->find("", (QTextDocument::FindFlags) 0);
	symbol_textBrowser->find(frameSymbol_lineEdit->text(), (QTextDocument::FindFlags) 0);
#endif
}

void CMainWindow::on_nextSymbolButton_clicked()
{
#ifdef ENABLE_SYMBOL_WEB_VIEW
    webView->findText(frameSymbol_lineEdit->text(), QWebPage::FindWrapsAroundDocument);
#elif defined ENABLE_SYMBOL_RICH_TEXT_VIEW
    symbol_textBrowser->findText(frameSymbol_lineEdit->text(), (QTextDocument::FindFlags) 0);
#else
    symbol_textBrowser->find(frameSymbol_lineEdit->text(), (QTextDocument::FindFlags) 0);
#endif
}

void CMainWindow::on_previousSymbolButton_clicked()
{
#ifdef ENABLE_SYMBOL_WEB_VIEW
    webView->findText(frameSymbol_lineEdit->text(), QWebPage::FindBackward | QWebPage::FindWrapsAroundDocument);
#elif defined ENABLE_SYMBOL_RICH_TEXT_VIEW
	symbol_textBrowser->findText(frameSymbol_lineEdit->text(), (QTextDocument::FindFlags) QTextDocument::FindBackward);
#else
	symbol_textBrowser->find(frameSymbol_lineEdit->text(), (QTextDocument::FindFlags) QTextDocument::FindBackward);
#endif
}


void CMainWindow::on_cancelTagUpdate()
{
	int ret;

	ret = QMessageBox::question(this, "Tag update", "Cancel tag update?", QMessageBox::Yes, QMessageBox::No);
	if (ret == QMessageBox::Yes) {
		statusBar()->showMessage("Cancelling tag update...");
		profileUpdateThread_.cancelUpdate();
	}
}

// right click context menu
void CMainWindow::contextMenuEvent(QContextMenuEvent* event)
{
	QPoint p;

	// profile_listView area and have selected profile
	p = profile_listView->mapFromGlobal(event->globalPos());

	// get current active tab
	const int mainTabIndex = mainTabWidget->currentIndex();
	const int profileTabIndex = mainTabWidget->indexOf(profileTab);
	const int groupTabIndex = mainTabWidget->indexOf(groupTab);

	if (mainTabIndex == profileTabIndex) {
		// in area of profile_listView
		if (profile_listView->rect().contains(p)) {
			QStringList profileItemNameList = getSelectedProfileItemNameList();
			int profileSelected = profileItemNameList.size();

			if (profileSelected != 0) { // only show menu if have more than 1 selected profile
				// only show delete menu if more than one profile selected
				if (profileSelected > 1) {
					QMenu menu(this);
					menu.addAction(actionProfileDelete);
					menu.addAction(actionProfileGroup);

					menu.exec(event->globalPos());
				} else {
					QMenu menu(this);

					menu.addAction(actionProfileNew);
					menu.addAction(actionProfileLoad);
					menu.addAction(actionProfileRebuildTag);
					menu.addAction(actionProfileModify);
					menu.addAction(actionProfileDelete);
					menu.addAction(actionProfileGroup);

					menu.addAction(actionProfileExplore);

#ifdef Q_OS_WIN
					menu.addAction(actionProfileConsole);
	#endif
					menu.exec(event->globalPos());
				}
			}
		}
	} else if (mainTabIndex == groupTabIndex) {
		// in area of group_listView
		if (group_listView->rect().contains(p)) {
			QStringList groupItemNameList = getSelectedGroupItemNameList();
			int groupSelected = groupItemNameList.size();

			if (groupSelected != 0) { // only show menu if have more than 1 selected group
				// only show delete menu if more than one group selected
				if (groupSelected > 1) {
					QMenu menu(this);

					menu.addAction(actionGroupDelete);
					menu.exec(event->globalPos());
				} else {
					QMenu menu(this);

					menu.addAction(actionGroupNew);
					menu.addAction(actionGroupLoad);
					menu.addAction(actionGroupUpdate);
					menu.addAction(actionGroupModify);
					menu.addAction(actionGroupDelete);

					menu.exec(event->globalPos());
				}
			}
		}
	}


	// get current active tab
	const int infoTabIndex = infoTabWidget->currentIndex();

	const int fileTabIndex = infoTabWidget->indexOf(fileTab);

	// in area of output_listView and correct tab page
	p = output_listView->mapFromGlobal(event->globalPos());
	if ((output_listView->rect().contains(p)) && (infoTabIndex == fileTabIndex)) {

		QStringList outputItemNameList = getSelectedOutputItemNameList();
		int outputItemSelected = outputItemNameList.size();

		if (outputItemSelected != 0) { // only show menu if have more than 1 selected profile
			if (outputItemSelected > 1) {
				QMenu menu(this);

				menu.addAction(actionOutputCopy);
				menu.exec(event->globalPos());
			} else {
				QMenu menu(this);

				menu.addAction(actionOutputEdit);
				menu.addAction(actionOutputExplore);

				menu.addAction(actionOutputCopy);
				menu.addAction(actionOutputConsole);

#ifdef Q_OS_WIN
				menu.addAction(actionOutputProperties);
#endif
				menu.exec(event->globalPos());
			}
		}

	}

	// in area of status bar
	p = statusBar()->mapFromGlobal(event->globalPos());
	// only show cancel tag update menu when tag build in progress
	if (statusBar()->rect().contains(p) && bTagBuildInProgress_) {
		QMenu menu(this);

		menu.addAction(actionCancelTagUpdate);

		menu.exec(event->globalPos());
	}

}

void CMainWindow::on_outputEditPressed()
{
	QStringList selectedItemList = getSelectedOutputItemNameList();
	int itemSelected = selectedItemList.size();

	QString executeDir;
	QString editFilename;

	if (itemSelected > 0) {
		if (itemSelected > 1) {
			QMessageBox::information(this, "Edit", "Can only edit one file", QMessageBox::Ok);
		} else {
			QFileInfo fileInfo(selectedItemList.at(0));
			executeDir = fileInfo.absoluteDir().absolutePath();

			QString consoleCommnad = confManager_->getAppSettingValue("DefaultEditor").toString();

#ifdef Q_OS_WIN
			editFilename = "\"" + selectedItemList.at(0) + "\"";
#else
			editFilename = selectedItemList.at(0);
#endif

#ifdef Q_OS_WIN
			QString excuteMethod = "open";

            ShellExecute(NULL, reinterpret_cast<const wchar_t*>(excuteMethod.utf16()), reinterpret_cast<const wchar_t*>(consoleCommnad.utf16()), reinterpret_cast<const wchar_t*> (editFilename.utf16()), reinterpret_cast<const wchar_t*>(executeDir.utf16()), SW_NORMAL);
#else
			QProcess::startDetached(consoleCommnad, QStringList(editFilename));
#endif
		}
	}
}

void CMainWindow::on_outputCopyPressed()
{
	QStringList selectedItemList = getSelectedOutputItemNameList();
	QString clipBoardStr = "";

	if (selectedItemList.size() > 0) {
		foreach (const QString& selectedItem, selectedItemList) {
			clipBoardStr = clipBoardStr + selectedItem +  "\n";
		}

		QClipboard *clipboard = QApplication::clipboard();

		clipboard->setText(clipBoardStr);
	}
}

void CMainWindow::on_outputExplorePressed()
{
	QStringList selectedItemList = getSelectedOutputItemNameList();
	int itemSelected = selectedItemList.size();

	QString executeDir;

	if (itemSelected > 0) {
		if (itemSelected > 1) {
			QMessageBox::information(this, "Explore", "Can only explore one path", QMessageBox::Ok);
		} else {
			QFileInfo fileInfo(selectedItemList.at(0));
			executeDir = fileInfo.absoluteDir().absolutePath();

#ifdef Q_OS_WIN
			QString excuteMethod = "explore";
            ShellExecute(NULL, reinterpret_cast<const wchar_t*>(excuteMethod.utf16()), reinterpret_cast<const wchar_t*>(executeDir.utf16()), NULL, NULL, SW_NORMAL);
#else
			QDesktopServices::openUrl(QUrl(executeDir, QUrl::TolerantMode));
#endif
		}
	}
}

void CMainWindow::on_outputConsolePressed()
{
	QStringList selectedItemList = getSelectedOutputItemNameList();
	int itemSelected = selectedItemList.size();

	QString executeDir;

	if (itemSelected > 0) {
		if (itemSelected > 1) {
			QMessageBox::information(this, "Console", "Can only open console from one path", QMessageBox::Ok);
		} else {
			QFileInfo fileInfo(selectedItemList.at(0));
			executeDir = fileInfo.absoluteDir().absolutePath();

#ifdef Q_OS_WIN
			QString excuteMethod = "open";
			QString consoleCommnad = "cmd.exe";

            ShellExecute(NULL, reinterpret_cast<const wchar_t*>(excuteMethod.utf16()), reinterpret_cast<const wchar_t*>(consoleCommnad.utf16()), NULL, reinterpret_cast<const wchar_t*>(executeDir.utf16()), SW_NORMAL);
#else
			executeDir = "--working-directory=" + executeDir;
			QProcess::startDetached("gnome-terminal", QStringList(executeDir));
#endif
		}
	}
}

void CMainWindow::on_outputPropertiesPressed()
{
	QStringList selectedItemList = getSelectedOutputItemNameList();
	int itemSelected = selectedItemList.size();

	QString executeDir;

	if (itemSelected > 0) {
		if (itemSelected > 1) {
			QMessageBox::information(this, "Properties", "Can only view properties for one file", QMessageBox::Ok);
		} else {
#ifdef Q_OS_WIN
			SHELLEXECUTEINFO execInfo;
			QString excuteMethod = "properties";

			ZeroMemory(&execInfo, sizeof(execInfo));
			execInfo.cbSize= sizeof(execInfo);
			execInfo.hwnd= NULL;
			execInfo.lpVerb= reinterpret_cast<const wchar_t*>(excuteMethod.utf16());
			execInfo.fMask= SEE_MASK_INVOKEIDLIST;
			execInfo.lpFile = reinterpret_cast<const wchar_t*>(selectedItemList.at(0).utf16());

            ShellExecuteEx(&execInfo);
#else
			// not implemented
#endif
		}
	}
}

void CMainWindow::queryTag(const QString& tag)
{
	QString tagDbFileName, inputFileName;

	QString resultHtml;

	QList<CTagResultItem> resultList;

	QString resultItemSrcLine;
    QFileInfo resultItemFileInfo;

	bool bSymbolRegularExpression = false;

    Qt::CaseSensitivity caseSensitivity =
            actionSymbolCaseSensitive->isChecked() ? Qt::CaseSensitive
                                                       : Qt::CaseInsensitive;

    if (actionSymbolRegularExpression->isChecked()) {
       bSymbolRegularExpression = true;
	}

	/*
	webView->load(QUrl(search_lineEdit->text()));
	webView->show();
	*/

	QString funcSignatureToPrint;
	QString lineSrcBeforeToPrint;
	QString lineSrcAfterToPrint;

	int i, j;
	int lineSrcSize;

	QString lineNumStr;
	QString modifiedLineSrc;
	QString tagToQueryFiltered;

	if (!currentProfileItem_.name_.isEmpty()) {

		tagDbFileName = QString(QTagger::kQTAG_TAGS_DIR) + "/" + currentProfileItem_.name_ + "/" + QString(QTagger::kQTAG_DEFAULT_TAGDBNAME);
		inputFileName = QString(QTagger::kQTAG_TAGS_DIR) + "/" + currentProfileItem_.name_ + "/" + QString(CSourceFileList::kFILE_LIST);

		tagger_.queryTag(inputFileName, tagDbFileName, tag, tagToQueryFiltered, resultList, caseSensitivity, bSymbolRegularExpression);

		//QString tagToQuery = tag.toHtmlEscaped();
		QString tagToQuery = tagToQueryFiltered.toHtmlEscaped();

		resultHtml = "<html>";
		//resultHtml += "<head><link rel=\"stylesheet\" href=\"file:///" + QCoreApplication::applicationDirPath() + "/Html/style.css\" type=\"text/css\" media=\"screen\"/></head>";
		resultHtml += "<body>";
		resultHtml += "<pre>";

		QList<int>::const_iterator indentIt;
		int minIndent = 0;

		findReplaceFileList_.clear();

		foreach (const CTagResultItem& resultItem, resultList) {
			// drive letter colon for first field

            resultItemFileInfo.setFile(resultItem.filePath_);

			resultItemSrcLine = resultItem.fileLineSrc_;
			resultItemSrcLine = resultItemSrcLine.toHtmlEscaped();
			resultItemSrcLine.replace(tagToQuery, "<keyword>" + tagToQuery + "</keyword>", Qt::CaseSensitive);

			funcSignatureToPrint = "";
			lineSrcBeforeToPrint = "";
			lineSrcAfterToPrint = "";

			if (resultItem.functionSignature_ != "") {
				funcSignatureToPrint = "<functionsig>&nbsp;&nbsp;&nbsp;" + resultItem.functionSignature_ + "</functionsig>";
			}

			minIndent = resultItem.lineSrcIndentLevel_;

			if (resultItem.beforeIndentLevelList_.size() > 0) {
				indentIt = std::min_element(resultItem.beforeIndentLevelList_.begin(), resultItem.beforeIndentLevelList_.end());
				if (*indentIt < minIndent) {
					minIndent = *indentIt;
				}
			}

			if (resultItem.afterIndentLevelList_.size() > 0) {
				indentIt = std::min_element(resultItem.afterIndentLevelList_.begin(), resultItem.afterIndentLevelList_.end());
				if (*indentIt < minIndent) {
					minIndent = *indentIt;
				}
			}

			for (j = 0; j < resultItem.lineSrcIndentLevel_ - minIndent; j++) {
				resultItemSrcLine = "&nbsp;&nbsp;&nbsp;" + resultItemSrcLine;
			}

			lineSrcSize = resultItem.fileLineSrcBeforeList_.size();
			if (lineSrcSize != 0) {
				i = 0;

				foreach (const QString& lineSrc, resultItem.fileLineSrcBeforeList_) {

					lineNumStr = "<linenum>" + QString::number(resultItem.fileLineNum_ - lineSrcSize + i) + "</linenum> ";
					modifiedLineSrc = lineSrc;
					modifiedLineSrc = modifiedLineSrc.toHtmlEscaped();
					modifiedLineSrc.replace(tagToQuery, "<keyword>" + tagToQuery + "</keyword>", Qt::CaseSensitive);

					for (j = 0; j < resultItem.beforeIndentLevelList_.at(i) - minIndent; j++) {
						modifiedLineSrc = "&nbsp;&nbsp;&nbsp;" + modifiedLineSrc;
					}

					lineSrcBeforeToPrint += lineNumStr + modifiedLineSrc + "<br />";
					i++;
				}
			}

			if (resultItem.fileLineSrcAfterList_.size() != 0) {
				i = 0;

				foreach (const QString& lineSrc, resultItem.fileLineSrcAfterList_) {

					lineNumStr = "<linenum>" + QString::number(resultItem.fileLineNum_  + i + 1) + "</linenum> ";
					modifiedLineSrc = lineSrc;
					modifiedLineSrc = modifiedLineSrc.toHtmlEscaped();
					modifiedLineSrc.replace(tagToQuery, "<keyword>" + tagToQuery + "</keyword>", Qt::CaseSensitive);

					for (j = 0; j < resultItem.afterIndentLevelList_.at(i) - minIndent; j++) {
						modifiedLineSrc = "&nbsp;&nbsp;&nbsp;" + modifiedLineSrc;
					}

					lineSrcAfterToPrint += "<br />" + lineNumStr + modifiedLineSrc;
					i++;
				}
			}

			resultHtml += QString("<div class=\"itemblock\"><div class=\"header\">") +
						"<a href=\"file:///" + resultItem.filePath_ + "#line" + QString::number(resultItem.fileLineNum_) + "\">" +
						resultItemFileInfo.fileName() + "</a>" +
						funcSignatureToPrint +
						"</div><code>" +
						lineSrcBeforeToPrint +

						"<linenum>" + QString::number(resultItem.fileLineNum_) + "</linenum> " +
						resultItemSrcLine +

						lineSrcAfterToPrint +
						"</code></div><div><spacesize>&nbsp;</spacesize></div>";

			//qDebug() << "resultHtml = " << resultHtml << endl;

            findReplaceFileList_.insert(resultItem.filePath_, 0);
		}

		statusBar()->showMessage("Found " + QString::number(resultList.size()) + " symbols in " + QString::number(findReplaceFileList_.size()) + " files.");

		resultHtml += "</pre>";
		resultHtml += "</body></html>";

		/*
		QString resultHtmlFileName = "result.html";
		QFile resultHtmlFile(resultHtmlFileName);
		if (!resultHtmlFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
			qDebug() << "Cannot open result html file (" << resultHtmlFileName << ") for writing!" << endl;
			return;
		}

		QTextStream resultHtmlStream(&resultHtmlFile);
		*/

		//webView->load(QUrl::fromLocalFile(QApplication::applicationDirPath() + "/Html/index.html"));

#ifdef ENABLE_SYMBOL_WEB_VIEW
		webView->setHtml(resultHtml);
		webView->show();
#elif defined ENABLE_SYMBOL_RICH_TEXT_VIEW
		symbol_textBrowser->setHtml(resultHtml);
		symbol_textBrowser->show();
#else

		textDocument_.setHtml(resultHtml);

		symbol_textBrowser->setDocument(&textDocument_);
		symbol_textBrowser->show();
		/*
		symbol_textBrowser->clear();
		symbol_textBrowser->appendHtml(resultHtml);
		symbol_textBrowser->show();
        */
#endif

		/*
		resultHtmlStream << resultHtml;
		resultHtmlStream.flush();
		resultHtmlFile.close();
		*/
	}

}

void CMainWindow::on_searchButton_clicked()
{
	QString tagToQuery;

	tagToQuery = search_lineEdit->text();
	tagToQuery = tagToQuery.toHtmlEscaped();

	queryTag(tagToQuery);
}


void CMainWindow::wheelEvent(QWheelEvent *e)
{
	QPoint p;

	p = profile_listView->mapFromGlobal(e->globalPos());

    if (e->modifiers() == Qt::ControlModifier) {
        e->accept();
        if (e->delta() > 0) {
            // no action
		} else {
			// no action
		}
	}
	QMainWindow::wheelEvent(e);
}


void CMainWindow::appendLogList(TRACE_LEVEL level, const QString& msg)
{
	QString qStrBuf;

	switch (level) {
		case TRACE_DEBUG:   qStrBuf = "DEBUG:   " + msg; break;
		case TRACE_INFO:    qStrBuf = "INFO:    " + msg; break;
		case TRACE_WARNING: qStrBuf = "WARNING: " + msg; break;
		case TRACE_ERROR:   qStrBuf = "ERROR:   " + msg; break;
	}
}

void CMainWindow::keyPressEvent(QKeyEvent *event)
{

	if (filePattern_lineEdit->hasFocus()) {
		switch (event->key()) {
			case Qt::Key_Up:
				output_listView->setFocus();
				break;
			case Qt::Key_Down:
				output_listView->setFocus();
				break;
		}
	} else if (profilePattern_lineEdit->hasFocus()) {
		switch (event->key()) {
			case Qt::Key_Up:
				profile_listView->setFocus();
				break;
			case Qt::Key_Down:
				profile_listView->setFocus();
				break;
		}
	}

	QMainWindow::keyPressEvent(event);
}




