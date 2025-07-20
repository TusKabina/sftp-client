#include <iostream> // TODO: DELETE
#include <vector>
#include <QHeaderView>
#include <qdatetime.h>
#include <filesystem>
#include "Qt/mainGui.h"
#include "Utilities/MeasureHelper.h"
#include "Utilities/Logger.h"
#include "Qt/IconManager.h"

using namespace std::chrono_literals;


void TreeView::mousePressEvent(QMouseEvent* event) {
	if (event->button() == Qt::RightButton) {
		emit RightClickAction(event);
	}
	else {
		QTreeView::mousePressEvent(event);
	}
}

void TreeView::startDrag(Qt::DropActions supportedActions) {
	auto mimeData = new QMimeData();

	QString data = ((QFileSystemModel*)this->model())->filePath(currentIndex());

	mimeData->setData("drag/data", data.toUtf8());

	auto drag = new QDrag(this);
	drag->setMimeData(mimeData);
	drag->exec(Qt::MoveAction);
}

void TreeView::dragEnterEvent(QDragEnterEvent* event) {
	event->acceptProposedAction();
}

void TreeView::dragMoveEvent(QDragMoveEvent* event) {
	event->acceptProposedAction();
}

void TreeView::dropEvent(QDropEvent* event) {
	auto data = event->mimeData()->data("drag/data");
	QObject* source = event->source();
	
	if (auto viewSource = qobject_cast<TreeView*>(source)) {
		logger().debug() << "Dropped from TreeView";
		return;
	}
	else if (auto viewSource = qobject_cast<TreeWidget*>(source)) {
		logger().debug() << "Dropped from TreeWidget";
	}
	else {
		logger().debug() << "Unkown drop";
		return;
	}

	if (!data.isEmpty()) {
		QString dataAsString = QString(data);

		QModelIndex droppedIndex = indexAt(event->pos());
		if (!droppedIndex.isValid()) {
			return;
		}

		QString localPath = ((QFileSystemModel*)model())->filePath(droppedIndex);
		std::filesystem::path p = localPath.toStdString();
		std::string fileName = Commons::FileName(dataAsString.toStdString());
		
		localPath = localPath + "/" + fileName.c_str();
		
		std::string remotePath = "/" + dataAsString.toStdString();
		std::string directoryPath;

		if (!std::filesystem::is_directory(p)) {
			directoryPath = Commons::GetDirectoryName(p.string());
			localPath = QString::fromStdString(directoryPath) + "/" + QString::fromStdString(fileName);
		}
		std::string strLocal = localPath.toStdString();

		TreeViewWidget* parentWidget = qobject_cast<TreeViewWidget*>(parent());
		if (parentWidget) {
			TransferManager& transferManager = parentWidget->getTransferManager();
			
			uint64_t downloadJobId = transferManager.prepareJob(strLocal, remotePath);
			transferManager.submitJob(downloadJobId, JobOperation::DOWNLOAD);
		}
	}

	event->accept();
}

TreeWidget::TreeWidget(QWidget* parent) : QTreeWidget(parent) {
}

void TreeWidget::mousePressEvent(QMouseEvent* event) {
	if (event->button() == Qt::RightButton) {
		emit RightClickAction(event);
	}
	else {
		QTreeWidget::mousePressEvent(event);
	}
}

void TreeWidget::startDrag(Qt::DropActions supportedActions) {
	auto mimeData = new QMimeData();
	QTreeWidgetItem* item = itemFromIndex(currentIndex());

	QString data = item->text(0);

	while (item->parent() != NULL) {
		data = item->parent()->text(0) + "/" + data;
		item = item->parent();
	}

	mimeData->setData("drag/data", data.toUtf8());

	auto drag = new QDrag(this);
	drag->setMimeData(mimeData);
	drag->exec(Qt::MoveAction);
}

void TreeWidget::dragEnterEvent(QDragEnterEvent* event) {
	event->acceptProposedAction();
}

void TreeWidget::dragMoveEvent(QDragMoveEvent* event) {
	event->acceptProposedAction();
}

void TreeWidget::dropEvent(QDropEvent* event) {
	QObject* source = event->source();
	
	if (auto viewSource = qobject_cast<TreeView*>(source)) {
		logger().debug() << "Dropped from TreeView";
	}
	else if (auto viewSource = qobject_cast<TreeWidget*>(source)) {
		logger().debug() << "Dropped from TreeWidget";
		return;
	}
	else {
		logger().debug() << "Unkown drop";
		return;
	}

	auto data = event->mimeData()->data("drag/data");
	
	if (!data.isEmpty()) {
		QString dataAsString = QString(data);

		std::filesystem::path p = dataAsString.toStdString();
		if (std::filesystem::is_regular_file(p)) {
			logger().debug() << "Source: " << dataAsString << " is regular file";
		}
		else {
			logger().error() << "Source: " << dataAsString << " is not a file!";
			return;
		}

		QModelIndex droppedIndex = indexAt(event->pos());
		if (!droppedIndex.isValid()) {
			return;
		}

		QTreeWidgetItem* item = itemFromIndex(droppedIndex);

		QString remotePath = item->text(0);
		
		while (item->parent() != NULL) {
			remotePath = item->parent()->text(0) + "/" + remotePath;
			item = item->parent();
		}

		//check if remote path is file or directory

		std::string testRemote = remotePath.toStdString();
		std::string testLocal = dataAsString.toStdString();
		std::string fileName = Commons::FileName(dataAsString.toStdString());

		TreeViewWidget* parentWidget = qobject_cast<TreeViewWidget*>(parent());
		if (parentWidget) {
			TransferManager& transferManager = parentWidget->getTransferManager();
			if(transferManager.isRegularFile("/" + remotePath.toStdString())) {
				std::string directoryPath = Commons::GetDirectoryName("/" + remotePath.toStdString());
				remotePath = QString::fromStdString(directoryPath) + "/" + QString::fromStdString(fileName);
			}
			else {
				remotePath = "/" + remotePath + "/" + fileName.c_str();
			}

			uint64_t uploadJobId = transferManager.prepareJob(testLocal, remotePath.toStdString());
			transferManager.submitJob(uploadJobId, JobOperation::UPLOAD);

		}
	}

	event->accept();
}

void TreeViewWidget::deleteTreeItems(QTreeWidgetItem* item) {
	for (int i = 0; i < item->childCount(); ++i) {
		deleteTreeItems(item->child(i));
	}
	delete item;
}

void TreeViewWidget::onConnectButtonClicked() {
	if (m_isConnected) {
		disconnectFromRemote();

		m_connectDisconnectButton->setText("Connect");
		m_remoteFileToUploadLineEdit->clear();
		m_remoteFolderLineEdit->clear();
		
		logger().info() << "Disconnected";
	}
	else {
		logger().info() << "Connecting to the remote server...";
		m_connectDisconnectButton->setEnabled(false);
		

		QFuture<void> future = QtConcurrent::run([this](){m_isConnected = connectToRemote();});

		auto* watcher = new QFutureWatcher<void>(this);

		connect(watcher, &QFutureWatcher<void>::finished, this, [this, watcher]() {
			watcher->deleteLater();
			if (m_isConnected) {

				populateTreeView();

				m_connectDisconnectButton->setText("Disconnect");
				m_connectDisconnectButton->setEnabled(true);
				m_remoteFolderLineEdit->setEnabled(true);

				logger().info() << "Connected";
			}
			else {
				m_connectDisconnectButton->setText("Connect");
				m_connectDisconnectButton->setEnabled(true);
				
				logger().info() << "Disconnected";
			}
		});

		watcher->setFuture(future);
	}
}

void TreeViewWidget::eventFromThreadPoolReceived(int id) {
	std::thread::id this_id = std::this_thread::get_id();
}

void TreeViewWidget::onDirectoryCacheUpdated(const std::string& path) {
	refreshTreeViewRoot(path);
}

void TreeViewWidget::onRemoteFolderKeyPressed() {
	QString path = m_remoteFolderLineEdit->text();

	if (path.isEmpty()) {
		logger().error() << "Invalid path!";
	}
	else {
		if (path.back() != '/') {
			path = path + "/";
		}
		
		findAndExpandPath(path);
		
		logger().info() << "Going to path: " << path;
	}

}

void TreeViewWidget::onTransferStatusUpdated(const TransferStatus& transferStatus) {
	QTreeWidgetItem* item;
	if (m_transferItems.contains(transferStatus.m_jobId)) {
		item = m_transferItems[transferStatus.m_jobId];
	}
	else {
		item = new QTreeWidgetItem(m_transferStatusWidget);
		m_transferItems[transferStatus.m_jobId] = item;
		m_transferStatusWidget->addTopLevelItem(item);
	}

	std::string fileName = Commons::FileName(transferStatus.m_source);
	item->setText(0, QString::fromStdString(fileName));
	item->setText(1, QString::fromStdString(transferStatus.TransferStatetoString()));
	item->setText(2, QString::fromStdString(transferStatus.m_source));
	item->setText(3, QString::fromStdString(transferStatus.m_destination));
	item->setText(4, QString::number(transferStatus.m_bytesTransferred));

	if (transferStatus.m_progress >= 100) {
		item->setText(5, "0.000 MB/s");
	}
	else {
		item->setText(5, QString::number(transferStatus.m_speed) + " MB/s");
	}
	item->setText(6, QString::number(transferStatus.m_progress,'f',2) + " %");
}

void TreeViewWidget::onCopyAction() {
	m_sourcePath = m_textCommandParameterRemote;
	m_isCutOperation = false;
}
void TreeViewWidget::onCutAction() {
	m_sourcePath = m_textCommandParameterRemote;
	m_isCutOperation = true;
}
void TreeViewWidget::onPasteAction() {
	QString destinationPath = m_textCommandParameterRemote;
	if (m_isCutOperation) {
		std::string sourcePath = "/" + m_sourcePath.toStdString();
		if (!m_manager.isRegularFile(sourcePath)) {
			logger().error() << "/" << sourcePath << " is not a file!";
		}
		else {
			std::string destPath = "/" + destinationPath.toStdString();
			if (!m_manager.isRegularFile(destPath)) {
				destPath = destPath + '/' + Commons::FileName(sourcePath);
			}
			else {
				destPath = Commons::GetDirectoryName(destPath) + "/" + Commons::FileName(sourcePath);
			}
			logger().info() << "Started move operation. Source: '" << sourcePath << "' Destination: '" << destPath << "'";

			uint64_t moveJobId = m_manager.prepareJob(sourcePath, destPath);
			logger().debug() << "Prepared Job with job id: " << moveJobId;
			m_manager.submitJob(moveJobId, JobOperation::MOVE);
		}
	}
	else {
		std::string sourcePath = "/" + m_sourcePath.toStdString();
		if (!m_manager.isRegularFile(sourcePath)) {
			logger().error() << "/" << sourcePath << " is not a file!";
		}
		else {
			std::string destPath = "/" + destinationPath.toStdString();
			if (!m_manager.isRegularFile(destPath)) {
				destPath = destPath + '/' + Commons::FileName(sourcePath);
			}
			else {
				destPath = Commons::GetDirectoryName(destPath) + "/" + Commons::FileName(sourcePath);
			}
			logger().info() << "Started copy operation. Source: '" << sourcePath << "' Destination: '" << destPath << "'";
			uint64_t copyJobId = m_manager.prepareJob(sourcePath, destPath);
			m_manager.submitJob(copyJobId, JobOperation::COPY);

		}
	}
	m_sourcePath.clear();
	m_isCutOperation = false;
}

void TreeViewWidget::onLogLevelChanged(int index) {
	LogLevel selectedLogLevel = static_cast<LogLevel>(m_logLevelComboBox->currentData().toInt());
	Logger::instance().setLogLevel(selectedLogLevel);
}

void TreeViewWidget::onClickedTreeView(const QModelIndex& index) {
	if (index.isValid()) {
		m_textCommandParameterLocal = ((QFileSystemModel*)m_treeView->model())->filePath(index);
		auto strParameterLocal = m_textCommandParameterLocal.toStdString();
		std::filesystem::path p = m_textCommandParameterLocal.toStdString();

		if (std::filesystem::is_regular_file(p)) {
			m_localFileToUploadLineEdit->setText(m_textCommandParameterLocal);
		}
		else {
			m_localFileToUploadLineEdit->clear();
		}

		m_directoryNameLocal = Commons::GetDirectoryName(((QFileSystemModel*)m_treeView->model())->filePath(index).toStdString()).c_str();
		m_directoryNameLocal += "/";
		m_localFolderLineEdit->setText(m_directoryNameLocal);
	}
}

void TreeViewWidget::processTreeWidgetItemClicked(QTreeWidgetItem* item, int index) {
	QString fullPath = item->text(0);
	QString entryType = item->text(2);
	bool prefetch = (item->childCount() == 0);

	while (item->parent() != NULL) {
		fullPath = item->parent()->text(0) + "/" + fullPath;
		item = item->parent();
	}

	if (prefetch && entryType == "Folder") {
		logger().debug() << "Expanding Directory: /" << fullPath.toStdString();
		
		updateTreeView("/" + fullPath.toStdString() + "/");
		
		logger().debug() << "Expanding Directory successful";
	}
		
	std::string newPath = fullPath.toStdString();
	newPath = "/" + newPath;

	if (m_manager.isRegularFile(newPath)) {
		m_remoteFileToUploadLineEdit->setText("/"+ fullPath);
	}
	else {
		m_remoteFileToUploadLineEdit->clear();
	}

	m_textCommandParameterRemote = fullPath;
	m_directoryNameRemote = "/" + QString::fromStdString(Commons::GetDirectoryName(m_textCommandParameterRemote.toStdString()));
	m_directoryNameRemote += m_directoryNameRemote == "/" ? "" : "/";
	
	m_remoteFolderLineEdit->setText(m_directoryNameRemote);
}

void TreeViewWidget::onRightClickedAction(QMouseEvent* event) {
	QMenu menu;
	QAction* pUpload = menu.addAction(trUtf8("Upload"));
	QAction* pDelete = menu.addAction(trUtf8("Delete"));
	
	connect(pUpload, &QAction::triggered, this, &TreeViewWidget::onuploadAction);
	connect(pDelete, &QAction::triggered, this, &TreeViewWidget::onDeleteLocalAction);

	QAction* pSelected = menu.exec(m_treeView->mapToGlobal(event->pos()));
}

void TreeViewWidget::onRightClickedActionTreeWidget(QMouseEvent* event) {
	QMenu menu;
	QAction* pDownload = menu.addAction(trUtf8("Download"));
	QAction* Pdelete = menu.addAction(trUtf8("Delete"));
	QAction* pCopy = menu.addAction(trUtf8("Copy"));
	QAction* pCut = menu.addAction(trUtf8("Cut"));

	if (!m_sourcePath.isEmpty()) {
		QAction* pPaste = menu.addAction(trUtf8("Paste"));
		connect(pPaste, &QAction::triggered, this, &TreeViewWidget::onPasteAction);
	}

	connect(pCopy, &QAction::triggered, this, &TreeViewWidget::onCopyAction);
	connect(pCut, &QAction::triggered, this, &TreeViewWidget::onCutAction);
	connect(pDownload, &QAction::triggered, this, &TreeViewWidget::onDownloadAction);
	connect(Pdelete, &QAction::triggered, this, &TreeViewWidget::onDeleteRemoteAction);

	QAction* pSelected = menu.exec(m_treeWidget->mapToGlobal(event->pos()));
}

TreeViewWidget::TreeViewWidget() {
	//Local file system setup
	QFileSystemModel* dirModel = new QFileSystemModel(this);
	dirModel->setRootPath("/");
	dirModel->setFilter(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);

	//Set read only on text (no changes possibile by hand)
	m_textDebugLog.setReadOnly(true);

	//Tree view for local machine files
	m_treeView = new TreeView(this);
	m_treeView->setSelectionMode(QAbstractItemView::SingleSelection);
	m_treeView->setDragEnabled(true);
	m_treeView->viewport()->setAcceptDrops(true);
	m_treeView->setDropIndicatorShown(true);
	m_treeView->setDragDropMode(QAbstractItemView::DragDrop);
	connect(m_treeView, SIGNAL(clicked(const QModelIndex&)),
		this, SLOT(onClickedTreeView(const QModelIndex&)));
	connect(m_treeView, SIGNAL(RightClickAction(QMouseEvent*)),
		this, SLOT(onRightClickedAction(QMouseEvent*)));
	m_treeView->setModel(dirModel);
	QModelIndex idx = dirModel->index("/");
	m_treeView->setRootIndex(idx);
	m_treeView->setSortingEnabled(true);
	m_treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	m_treeView->header()->setSortIndicatorShown(true);
	m_treeView->selectionModel();

	//Tree widget for remote machine files
	m_treeWidget = new TreeWidget(this);
	m_treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	m_treeWidget->setDragEnabled(true);
	m_treeWidget->viewport()->setAcceptDrops(true);
	m_treeWidget->setDropIndicatorShown(true);
	m_treeWidget->setDragDropMode(QAbstractItemView::DragDrop);
	connect(m_treeWidget, SIGNAL(RightClickAction(QMouseEvent*)),
		this, SLOT(onRightClickedActionTreeWidget(QMouseEvent*)));
	m_treeWidget->setEnabled(true);
	m_treeWidget->setColumnCount(4);
	m_treeWidget->setHeaderLabels({ "Name", "Size", "Type", "Date Modified", "Permissions", "Owner"});
	m_treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	m_treeWidget->setSortingEnabled(true);

	connect(m_treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
		this, SLOT(processTreeWidgetItemClicked(QTreeWidgetItem*, int)));

	const DirectoryCache* cacheManager = m_manager.getDirectoryCacheObject();
	connect(const_cast<DirectoryCache*>(cacheManager), &DirectoryCache::onDirectoryUpdated, this, [this](const std::string path) {
			this->onDirectoryCacheUpdated(path);
	});


	//Basic layout for widgets
	QHBoxLayout* horizontalLayoutUserCredentials = new QHBoxLayout;
	QHBoxLayout* horizontalLayoutUploadDownloadParameters = new QHBoxLayout;
	QHBoxLayout* horizontalLayoutTreeView = new QHBoxLayout;
	QHBoxLayout* horizontalLogLevelLayout = new QHBoxLayout();
	QVBoxLayout* verticalLayout = new QVBoxLayout;

	QLabel* logLevelLabel = new QLabel("Log Level:", this);
	horizontalLogLevelLayout->addWidget(logLevelLabel);

	// ComboBox
	m_logLevelComboBox = new QComboBox(this);
	m_logLevelComboBox->addItem("Debug", QVariant::fromValue(Debug));
	m_logLevelComboBox->addItem("Info", QVariant::fromValue(Info));
	m_logLevelComboBox->addItem("Warning", QVariant::fromValue(Warning));
	m_logLevelComboBox->addItem("Error", QVariant::fromValue(Error));
	m_logLevelComboBox->addItem("Critical", QVariant::fromValue(Critical));

	m_logLevelComboBox->setFixedWidth(150);
	m_logLevelComboBox->setCurrentIndex(Info);

	// Server name
	m_sftpServerNameLabel = new QLabel("Server");
	horizontalLayoutUserCredentials->addWidget(m_sftpServerNameLabel);
	m_sftpServerNameLineEdit = new QLineEdit;
	horizontalLayoutUserCredentials->addWidget(m_sftpServerNameLineEdit);

	// Server user name
	m_sftpUserNameLabel = new QLabel("User name");
	horizontalLayoutUserCredentials->addWidget(m_sftpUserNameLabel);
	m_sftpUserNameLineEdit = new QLineEdit;
	horizontalLayoutUserCredentials->addWidget(m_sftpUserNameLineEdit);

	// Server password
	m_sftpPasswordNameLabel = new QLabel("Password");
	horizontalLayoutUserCredentials->addWidget(m_sftpPasswordNameLabel);

	m_sftpPasswordNameLineEdit = new QLineEdit;
	m_sftpPasswordNameLineEdit->setEchoMode(QLineEdit::Password);
	horizontalLayoutUserCredentials->addWidget(m_sftpPasswordNameLineEdit);

	// Log level
	horizontalLogLevelLayout->addWidget(m_logLevelComboBox);
	horizontalLogLevelLayout->addStretch();

	// Connect/Disconnect
	m_connectDisconnectButton = new QPushButton("Connect");
	connect(m_connectDisconnectButton, SIGNAL(clicked()), this, SLOT(onConnectButtonClicked()));

	horizontalLayoutUserCredentials->addWidget(m_connectDisconnectButton);

	m_localFileToUploadLabel = new QLabel("Upload file");
	m_localFileToUploadLineEdit = new QLineEdit;
	m_localFileToUploadLineEdit->setReadOnly(true);

	m_remoteFileToUploadLabel = new QLabel("Download file");
	m_remoteFileToUploadLineEdit = new QLineEdit;
	m_remoteFileToUploadLineEdit->setReadOnly(true);

	m_localFolderLabel = new QLabel("Local directory");
	m_localFolderLineEdit = new QLineEdit;
	m_localFolderLineEdit->setReadOnly(true);

	m_remoteFolderLabel = new QLabel("Remote directory");
	m_remoteFolderLineEdit = new QLineEdit;
	m_remoteFolderLineEdit->setReadOnly(false);
	m_remoteFolderLineEdit->setEnabled(false);

	horizontalLayoutUploadDownloadParameters->addWidget(m_localFileToUploadLabel);
	horizontalLayoutUploadDownloadParameters->addWidget(m_localFileToUploadLineEdit);
	horizontalLayoutUploadDownloadParameters->addWidget(m_localFolderLabel);
	horizontalLayoutUploadDownloadParameters->addWidget(m_localFolderLineEdit);
	horizontalLayoutUploadDownloadParameters->addWidget(m_remoteFileToUploadLabel);
	horizontalLayoutUploadDownloadParameters->addWidget(m_remoteFileToUploadLineEdit);
	horizontalLayoutUploadDownloadParameters->addWidget(m_remoteFolderLabel);
	horizontalLayoutUploadDownloadParameters->addWidget(m_remoteFolderLineEdit);

	connect(m_remoteFolderLineEdit, &QLineEdit::returnPressed, this, &TreeViewWidget::onRemoteFolderKeyPressed);

	//Add all widgets to layout
	horizontalLayoutTreeView->addWidget(m_treeView);
	horizontalLayoutTreeView->addWidget(m_treeWidget);
	verticalLayout->addLayout(horizontalLayoutUserCredentials);
	verticalLayout->addLayout(horizontalLayoutUploadDownloadParameters);
	verticalLayout->addLayout(horizontalLayoutTreeView);
	verticalLayout->addLayout(horizontalLogLevelLayout);

	verticalLayout->addWidget(&m_textDebugLog);

	// combobox signal
	connect(m_logLevelComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,  &TreeViewWidget::onLogLevelChanged);

	// Add transfer status widget
	m_transferStatusWidget = new QTreeWidget(this);
	m_transferStatusWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	m_transferStatusWidget->setColumnCount(7);
	m_transferStatusWidget->setHeaderLabels(QStringList() << "File Name" << "State" << "Local Path" << "Remote Path"
														  << "Bytes Transferred" << "Speed" << "Progress");
	m_transferStatusWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	m_transferStatusWidget->setSortingEnabled(true);

	verticalLayout->addWidget(m_transferStatusWidget);
	connect(&m_manager, &TransferManager::transferStatusUpdated, this, &TreeViewWidget::onTransferStatusUpdated);
	//Set vertical layout as main layout
	setLayout(verticalLayout);

	Logger::instance().setLogWidget(&m_textDebugLog);
	Logger::instance().setLogLevel(Info);
}

void TreeViewWidget::populateTreeView() {
	m_treeWidget->clear();
	auto& cache = m_manager.getCache();
	for (const auto& pair : cache) {
		const QString path = QString::fromStdString(pair.first);
		const auto& entries = pair.second;
		QTreeWidgetItem* root = findOrCreateRoot(path);

		for (const auto& entry : entries) {
			if (entry.m_isSymLink || entry.m_name == "." || entry.m_name == "..") {
				continue;
}

			QString entryName = QString::fromStdString(entry.m_name);
			QDateTime dateTime = QDateTime::fromTime_t(entry.m_tLastModified);
			QString formattedDate = dateTime.toString("MM/dd/yyyy HH:mm:ss");
			QString permissions = QString::fromStdString(entry.m_permissions);
			QString owner = QString::fromStdString(entry.m_owner);

			QTreeWidgetItem* item = new QTreeWidgetItem(root);
			item->setText(0, entryName);
			item->setText(2, entry.m_isDirectory ? "Folder" : "File");
			item->setText(3, formattedDate);
			item->setText(4, permissions);
			item->setText(5, owner);

			if (entry.m_isDirectory) {
				item->setIcon(0, IconManager::getDirectoryIcon());
				item->setData(0, Qt::UserRole, true);
			}
			else {
				item->setText(1, Commons::convertSize(entry.m_totalBytes));
				item->setIcon(0, IconManager::getFileIcon());
				item->setData(0, Qt::UserRole, false);
			}

			root->addChild(item);
		}
		root->setData(0, Qt::UserRole + 1, true);
	}
}

void TreeViewWidget::refreshTreeViewRoot(const std::string& path) {
	m_treeWidget->setUpdatesEnabled(false);

	auto startOverall = std::chrono::high_resolution_clock::now();
	auto start = std::chrono::high_resolution_clock::now();

	QString qPath = QString::fromStdString(path);
	QTreeWidgetItem* root = findOrCreateRoot(qPath);

	if (!root) {
		m_treeWidget->setUpdatesEnabled(true);
		return;
	}

	auto end = std::chrono::high_resolution_clock::now();
	MeasureHelper::logDuration("findOrCreateRoot", start, end);

	start = std::chrono::high_resolution_clock::now();
	const auto entries = m_manager.getDirectoryList(path);
	end = std::chrono::high_resolution_clock::now();
	MeasureHelper::logDuration("m_manager.getDirectoryList", start, end);

	start = std::chrono::high_resolution_clock::now();
	QHash<QString, QTreeWidgetItem*> existingItems;
	for (int i = 0; i < root->childCount(); ++i) {
		QTreeWidgetItem* child = root->child(i);
		existingItems.insert(child->text(0), child);
	}
	end = std::chrono::high_resolution_clock::now();
	MeasureHelper::logDuration("Creating existing items hash", start, end);

	start = std::chrono::high_resolution_clock::now();
	QSet<QString> newItems;
	for (const auto& entry : entries) {
		if (entry.m_isSymLink || entry.m_name == "." || entry.m_name == "..") {
			continue;
		}

		QString entryName = QString::fromStdString(entry.m_name);
		newItems.insert(entryName);

		QTreeWidgetItem* item = existingItems.value(entryName, nullptr);
		if (!item) {
			item = new QTreeWidgetItem(root);
			root->addChild(item);
			item->setText(0, entryName);
		}
		else {
			if (item->text(0) != entryName) {
				item->setText(0, entryName);
			}
		}

		QString typeText = entry.m_isDirectory ? "Folder" : "File";
		if (item->text(2) != typeText) {
			item->setText(2, typeText);
		}

		QDateTime dateTime = QDateTime::fromTime_t(entry.m_tLastModified);

		QString formattedDate = dateTime.toString("MM/dd/yyyy HH:mm:ss");
		if (item->text(3) != formattedDate) {
			item->setText(3, formattedDate);
		}

		QIcon desiredIcon = entry.m_isDirectory ? IconManager::getDirectoryIcon() : IconManager::getFileIcon();
		if (item->icon(0).cacheKey() != desiredIcon.cacheKey()) {
			item->setIcon(0, desiredIcon);
		}

		QVariant currentData = item->data(0, Qt::UserRole);
		bool desiredData = entry.m_isDirectory;
		if (currentData.toBool() != desiredData) {
			item->setData(0, Qt::UserRole, desiredData);
		}

		if (!entry.m_isDirectory) {
			QString sizeText = Commons::convertSize(entry.m_totalBytes);
			if (item->text(1) != sizeText) {
				item->setText(1, sizeText);
			}
		}

		QString permissions = QString::fromStdString(entry.m_permissions);
		if (item->text(4) != permissions) {
			item->setText(4, permissions);
		}

		QString owner = QString::fromStdString(entry.m_owner);
		if (item->text(5) != owner) {
			item->setText(5, owner);
		}
	}
	end = std::chrono::high_resolution_clock::now();
	MeasureHelper::logDuration("Adding/updating items", start, end);

	start = std::chrono::high_resolution_clock::now();
	for (auto it = existingItems.constBegin(); it != existingItems.constEnd(); ++it) {
		if (!newItems.contains(it.key())) {
			delete it.value();
		}
	}
	end = std::chrono::high_resolution_clock::now();
	MeasureHelper::logDuration("Removing non-existent items", start, end);

	m_treeWidget->setUpdatesEnabled(true);
	auto endOverall = std::chrono::high_resolution_clock::now();
	MeasureHelper::logDuration("Overall refreshTreeViewRoot", startOverall, endOverall);
}

void TreeViewWidget::updateTreeView(const std::string& path) {
	m_treeWidget->setUpdatesEnabled(false);

	const auto entries = m_manager.getDirectoryList(path);
	if (entries.empty()) {
		m_treeWidget->setUpdatesEnabled(true);
		return;
	}

	QString qPath = QString::fromStdString(path);
	QTreeWidgetItem* root = findOrCreateRoot(qPath);
	if (!root) {
		m_treeWidget->setUpdatesEnabled(true);
		return;
	}

	QHash<QString, QTreeWidgetItem*> existingItems;
	for (int i = 0; i < root->childCount(); ++i) {
		QTreeWidgetItem* child = root->child(i);
		existingItems.insert(child->text(0), child);
	}

	QSet<QString> newItems;

	for (const auto& entry : entries) {
		if (entry.m_isSymLink || entry.m_name == "." || entry.m_name == "..") {
			continue;
		}

		QString entryName = QString::fromStdString(entry.m_name);
		newItems.insert(entryName);

		QTreeWidgetItem* item = existingItems.value(entryName, nullptr);
		if (!item) {
			item = new QTreeWidgetItem(root);
			item->setText(0, entryName);
			root->addChild(item);
		}

		QString typeText = entry.m_isDirectory ? "Folder" : "File";
		if (item->text(2) != typeText) {
			item->setText(2, typeText);
		}

		QDateTime dateTime = QDateTime::fromTime_t(entry.m_tLastModified);

		QString formattedDate = dateTime.toString("MM/dd/yyyy HH:mm:ss");
		if (item->text(3) != formattedDate) {
			item->setText(3, formattedDate);
		}

		if (!entry.m_isDirectory) {
			QString sizeText = Commons::convertSize(entry.m_totalBytes);
			if (item->text(1) != sizeText) {
				item->setText(1, sizeText);
			}
		}

		QIcon desiredIcon = entry.m_isDirectory ? IconManager::getDirectoryIcon() : IconManager::getFileIcon();
		if (item->icon(0).cacheKey() != desiredIcon.cacheKey()) {
			item->setIcon(0, desiredIcon);
		}

		QVariant currentData = item->data(0, Qt::UserRole);
		bool desiredData = entry.m_isDirectory;
		if (currentData.toBool() != desiredData) {
			item->setData(0, Qt::UserRole, desiredData);
		}

		QString permissions = QString::fromStdString(entry.m_permissions);
		if (item->text(4) != permissions) {
			item->setText(4, permissions);
		}

		QString owner = QString::fromStdString(entry.m_owner);
		if (item->text(5) != owner) {
			item->setText(5, owner);
		}
	}

	for (auto it = existingItems.constBegin(); it != existingItems.constEnd(); ++it) {
		if (!newItems.contains(it.key())) {
			delete it.value();
		}
	}

	m_treeWidget->setUpdatesEnabled(true);
}

void TreeViewWidget::populateTreeWidgetViewDirectory(QTreeWidgetItem* root, const QString& path) {
	m_treeWidget->setUpdatesEnabled(false);

	QString fullPath = '/' + path + '/';
	const auto entries = m_manager.getDirectoryList(fullPath.toStdString());
	if (entries.empty()) {
		m_treeWidget->setUpdatesEnabled(true);
		return;
	}

	QHash<QString, QTreeWidgetItem*> existingItems;
	for (int i = 0; i < root->childCount(); ++i) {
		QTreeWidgetItem* child = root->child(i);
		existingItems.insert(child->text(0), child);
	}

	QSet<QString> newItems;

	for (const auto& entry : entries) {
		if (entry.m_isSymLink || entry.m_name == "." || entry.m_name == "..") {
			continue;
		}

		QString entryName = QString::fromStdString(entry.m_name);
		newItems.insert(entryName);

		QTreeWidgetItem* item = existingItems.value(entryName, nullptr);
		if (!item) {
			item = new QTreeWidgetItem(root);
			item->setText(0, entryName);
			root->addChild(item);
		}

		QString typeText = entry.m_isDirectory ? "Folder" : "File";
		if (item->text(2) != typeText) {
			item->setText(2, typeText);
		}

		QDateTime dateTime = QDateTime::fromTime_t(entry.m_tLastModified);

		QString formattedDate = dateTime.toString("MM/dd/yyyy HH:mm:ss");
		if (item->text(3) != formattedDate) {
			item->setText(3, formattedDate);
		}

		QIcon desiredIcon = entry.m_isDirectory ? IconManager::getDirectoryIcon() : IconManager::getFileIcon();
		if (item->icon(0).cacheKey() != desiredIcon.cacheKey()) {
			item->setIcon(0, desiredIcon);
		}

		QVariant currentData = item->data(0, Qt::UserRole);
		bool desiredData = entry.m_isDirectory;
		if (currentData.toBool() != desiredData) {
			item->setData(0, Qt::UserRole, desiredData);
		}

		QString permissions = QString::fromStdString(entry.m_permissions);
		if (item->text(4) != permissions) {
			item->setText(4, permissions);
		}

		QString owner = QString::fromStdString(entry.m_owner);
		if (item->text(5) != owner) {
			item->setText(5, owner);
		}

		if (!entry.m_isDirectory) {
			QString sizeText = Commons::convertSize(entry.m_totalBytes);
			if (item->text(1) != sizeText) {
				item->setText(1, sizeText);
			}
		}
	}

	for (auto it = existingItems.constBegin(); it != existingItems.constEnd(); ++it) {
		if (!newItems.contains(it.key())) {
			delete it.value();
		}
	}

	root->setData(0, Qt::UserRole + 1, true);

	m_treeWidget->setUpdatesEnabled(true);
}

QTreeWidgetItem* TreeViewWidget::findOrCreateRoot(const QString& path) {
	QStringList parts = path.split('/', QString::SkipEmptyParts);
	auto strPath = path.toStdString();
	QTreeWidgetItem* root = nullptr;

	for (const auto& part : parts) {
		bool found = false;
		if (!root) {
			for (int i = 0; i < m_treeWidget->topLevelItemCount(); i++) {
				if (m_treeWidget->topLevelItem(i)->text(0) == part) {
					root = m_treeWidget->topLevelItem(i);
					found = true;
					break;
				}
			}
			if (!found) {
				root = new QTreeWidgetItem(QStringList(part));
				m_treeWidget->addTopLevelItem(root);
			}
		}
		else {
			for (int i = 0; i < root->childCount(); i++) {
				if (root->child(i)->text(0) == part) {
					root = root->child(i);
					found = true;
					break;
				}
			}
			if (!found) {
				QTreeWidgetItem* child = new QTreeWidgetItem(QStringList(part));
				root->addChild(child);
				root = child;
			}
		}
		
	}
	return root ? root : m_treeWidget->invisibleRootItem();
}

void TreeViewWidget::findAndExpandPath(const QString& path) {
	QStringList pathParts = path.size() == 1 ? QStringList("/") : path.split("/", Qt::SkipEmptyParts);
	QTreeWidgetItem* currentItem = nullptr;

	for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
		QTreeWidgetItem* item = m_treeWidget->topLevelItem(i);
		std::string strItemText = item->text(0).toStdString();
		std::string strPathParts = pathParts[0].toStdString();
		if (item->text(0) == pathParts[0]) {
			currentItem = item;
			break;
		}
	}

	if (!currentItem) {
		logger().error() << "The starting path for: '" << path.toStdString() << "' was not found in the directory tree.";
		return;
	}

	if (pathParts.size() == 1) {
		currentItem->setExpanded(true);
		m_treeWidget->scrollToItem(currentItem);
	}
	else {
		QString currentPath = pathParts[0];
		std::string strCurrentPath = pathParts[0].toStdString();

		for (int i = 1; i < pathParts.size(); ++i) {
			bool found = false;
			currentPath += "/" + pathParts[i];
			strCurrentPath = currentPath.toStdString();
			for (int j = 0; j < currentItem->childCount(); ++j) {
				QTreeWidgetItem* child = currentItem->child(j);
				std::string strChildText = child->text(0).toStdString();
				std::string strParts = pathParts[i].toStdString();
				if (child->text(0) == pathParts[i]) {
					currentItem = child;
					found = true;
					break;
				}
			}

			if (!found || !currentItem->data(0, Qt::UserRole + 1).toBool()) {
				populateTreeWidgetViewDirectory(currentItem, currentPath);
				for (int j = 0; j < currentItem->childCount(); j++) {
					QTreeWidgetItem* child = currentItem->child(j);
					if (child->text(0) == pathParts[i]) {
						currentItem = child;
						found = true;
						break;
					}
				}
				if (!found) {
					logger().error() << "The path for: '" << currentPath.toStdString() << "' was not found in the directory tree.";
					return;
				}
			}
			currentItem->setExpanded(true);
			m_treeWidget->scrollToItem(currentItem);
		}
	}
}

void TreeViewWidget::onDownloadAction() {
	std::string remotePath = m_textCommandParameterRemote.toStdString();

	if (remotePath.front() != '/') {
		remotePath = "/" + remotePath;
	}
	if (m_manager.isRegularFile(remotePath)) {
		std::filesystem::path localPath = m_textCommandParameterLocal.toStdString();
		std::string strLocalPath = localPath.string();

		if (std::filesystem::is_regular_file(localPath)) {
			strLocalPath = localPath.parent_path().string() + "/" + Commons::FileName(remotePath);
		}
		else {
			strLocalPath = localPath.string() + "/" + Commons::FileName(remotePath);
		}

		uint64_t downloadJobId = m_manager.prepareJob(strLocalPath, remotePath);

		logger().debug() << "JOB_ID: " << downloadJobId;

		m_manager.submitJob(downloadJobId, JobOperation::DOWNLOAD);

		logger().info() << "started download operation. Local path: '" << strLocalPath
			<< "'. Remote Path: '" << remotePath
			<< "'";
	}
	else {
		logger().error() << "entry: " << m_textCommandParameterLocal.toStdString()
			<< " in a directory: " << m_directoryNameLocal.toStdString()
			<< " is not a file.";
	}

}

void TreeViewWidget::onDeleteRemoteAction() {
	std::string strPath = m_textCommandParameterRemote.toStdString();

	if (strPath.front() != '/') {
		strPath = "/" + strPath;

		if (m_manager.isRegularFile(strPath)) {
			std::string remotePath = "/" + m_textCommandParameterRemote.toStdString();
			size_t pos = remotePath.find_last_of("/");
			uint64_t deleteJobId = m_manager.prepareJob("", remotePath);
			m_manager.submitJob(deleteJobId, JobOperation::DELETE);

			logger().info() << "Started delete operation on file: " << m_textCommandParameterRemote.toStdString();
		}
	}
	else {

		logger().error() << "entry: " << m_textCommandParameterLocal.toStdString()
			<< " in a directory: " << m_directoryNameLocal.toStdString()
			<< " is not a file.";
	}
}

void TreeViewWidget::onuploadAction() {
	std::filesystem::path p = m_textCommandParameterLocal.toStdString();

	if (std::filesystem::is_regular_file(p)) {
		logger().debug() << "Source: " << p.string() << " is regular file";
	}
	else {
		logger().error() << "Source: " << p.string() << " is not a file!";
		return;
	}

	std::string remotePath = "/" + m_textCommandParameterRemote.toStdString();
	std::string localPath = p.string();
	std::string localFileName = p.filename();

	if (m_manager.isRegularFile(remotePath)) {
		std::string remoteDirectoryPath = Commons::GetDirectoryName(remotePath);
		remotePath = remoteDirectoryPath + "/" + localFileName;
	}
	else {
		remotePath = remotePath + "/" + localFileName;
	}

	uint64_t uploadJobId = m_manager.prepareJob(localPath, remotePath);
	m_manager.submitJob(uploadJobId, JobOperation::UPLOAD);

	logger().info() << "Started upload operation. Local path: '" << localPath
		<< "'. Remote path: " << remotePath;
}

void TreeViewWidget::onDeleteLocalAction() {
	std::filesystem::path p = m_textCommandParameterLocal.toStdString();

	if (std::filesystem::is_regular_file(p)) {
		std::string localPath = p.string();
		uint64_t deleteJobId = m_manager.prepareJob(localPath, "");

		m_manager.submitJob(deleteJobId, JobOperation::DELETE_LOCAL);

		logger().info() << "started delete operation. Remote path: '" << localPath;
	}
	else {
		logger().error() << "Error. Remote entry: '" << m_textCommandParameterLocal.toStdString() << "' is not file.";
	}
}

bool TreeViewWidget::connectToRemote() {
	std::string host = m_sftpServerNameLineEdit->text().toStdString();
	std::string username = m_sftpUserNameLineEdit->text().toStdString();
	std::string password = m_sftpPasswordNameLineEdit->text().toStdString();

	m_manager.connect(host, username, password);

	return m_manager.isInitialized();
	
}

void TreeViewWidget::disconnectFromRemote() {
	m_manager.reset();

	for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
		QTreeWidgetItem* topLevelItem = m_treeWidget->topLevelItem(i);
		deleteTreeItems(topLevelItem);
	}

	m_treeWidget->clear();
	m_isConnected = false;
	
}
