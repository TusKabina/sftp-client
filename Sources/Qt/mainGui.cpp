#include <iostream> // TODO: DELETE
#include <vector>
#include <QHeaderView>
#include <qdatetime.h>
#include <filesystem>
#include "Qt/mainGui.h"
#include "Utilities/MeasureHelper.h"
#include "Utilities/Logger.h"

QIcon& TreeViewWidget::getDirectoryIcon() {
	static QIcon directoryIcon("dir.png");
	return directoryIcon;
}

QIcon& TreeViewWidget::getFileIcon() {
	static QIcon fileIcon("file.png");
	return fileIcon;
}

std::string GetDirectoryName(const std::string& name) {
	size_t pos = name.find_last_of("\\/");
	return (std::string::npos == pos) ? "" : name.substr(0, pos);
}

std::string FileName(const std::string& path) {
	return path.substr(path.find_last_of("/\\") + 1);
}

QString convertSize(qint64 size) {
	QStringList units = { "B", "KB", "MB", "GB", "TB" };
	int unitIndex = 0;
	double sizeInUnits = size;

	while (sizeInUnits > 1024.0 && unitIndex < units.size() - 1) {
		sizeInUnits /= 1024.0;
		unitIndex++;
	}

	return QString::number(sizeInUnits, 'f', 2) + " " + units[unitIndex];
}

QDateTime parseDateString(const std::string& dateString) {
	QStringList dateParts = QString::fromStdString(dateString).split(' ');

	if (dateParts.size() != 3) {
		return QDateTime();
	}

	QString month = dateParts[0];
	QString day = dateParts[1];
	QString timeOrYear = dateParts[2];

	QString dateTimeStr;
	auto currentYear = QDate::currentDate().year();

	// curl (ls -ll command) can return date without year if the entry is not older than 6 months
	if (timeOrYear.contains(':')) {
		dateTimeStr = month + " " + day + " " + timeOrYear + " " + QString::number(currentYear);
	}
	else {
		dateTimeStr = month + " " + day + " 00:00 " + timeOrYear;
	}

	QDateTime dateTime = QDateTime::fromString(dateTimeStr, "MMM dd HH:mm yyyy");

	return dateTime;
}

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

	if (!data.isEmpty()) {
		//check if remote data is file or directory somehow
		QString dataAsString = QString(data);

		QModelIndex droppedIndex = indexAt(event->pos());
		if (!droppedIndex.isValid()) {
			return;
		}

		QString localPath = ((QFileSystemModel*)model())->filePath(droppedIndex);
		std::filesystem::path p = localPath.toStdString();

		std::string fileName = FileName(dataAsString.toStdString());
		localPath = localPath + "/" + fileName.c_str();
		std::string remotePath = "/" + dataAsString.toStdString();
		std::string directoryPath;

		if (!std::filesystem::is_directory(p)) {
			directoryPath = GetDirectoryName(p.string());
			localPath = QString::fromStdString(directoryPath) + "/" + QString::fromStdString(fileName);
		}
		std::string testLocal = localPath.toStdString();

		TreeViewWidget* parentWidget = qobject_cast<TreeViewWidget*>(parent());
		if (parentWidget) {
			TransferManager& transferManager = parentWidget->getTransferManager();
			/*parentWidget->getDebugLog().append("[DOWNLOAD]: Source: " + QString::fromStdString(testLocal) + 
				"Destination: " + QString::fromStdString(remotePath));*/

			uint64_t downloadJobId = transferManager.prepareJob(testLocal, remotePath);
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
	auto data = event->mimeData()->data("drag/data");

	if (!data.isEmpty()) {
		QString dataAsString = QString(data);

		std::filesystem::path p = dataAsString.toStdString();
		if (std::filesystem::is_regular_file(p)) {
			//all ok its file
		}
		else {
			//not file
			//parentWidget->getDebugLog().append("Error: Source: " + QString::fromStdString(dataAsString.toStdString()) + " is not a file!");
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
		std::string fileName = FileName(dataAsString.toStdString());

		TreeViewWidget* parentWidget = qobject_cast<TreeViewWidget*>(parent());
		if (parentWidget) {
			TransferManager& transferManager = parentWidget->getTransferManager();
			if(transferManager.isRegularFile("/" + remotePath.toStdString())) {
				std::string directoryPath = GetDirectoryName("/" + remotePath.toStdString());
				remotePath = QString::fromStdString(directoryPath) + "/" + QString::fromStdString(fileName);
			}
			else {
				remotePath = "/" + remotePath + "/" + fileName.c_str();
			}
			//parentWidget->getDebugLog().append("[UPLOAD]: \nSource: " + QString::fromStdString(testLocal) + " Destination: " + remotePath);

			uint64_t uploadJobId = transferManager.prepareJob(testLocal, remotePath.toStdString());
			transferManager.submitJob(uploadJobId, JobOperation::UPLOAD);

		}
		
		int test = 666;
	}

	event->accept();
}

void deleteTreeItems(QTreeWidgetItem* item) {
	for (int i = 0; i < item->childCount(); ++i) {
		deleteTreeItems(item->child(i));
	}
	// Delete the current item itself
	delete item;
}

void TreeViewWidget::onConnectButtonClicked() {
	if (m_isConnected) {
		m_manager.reset();

		for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
			QTreeWidgetItem* topLevelItem = m_treeWidget->topLevelItem(i);
			deleteTreeItems(topLevelItem);
		}

		m_treeWidget->clear();
		m_isConnected = false;
		//m_textDebugLog.append("Disconnected");
		logger().info() << "Disconnected";
		m_connectDisconnectButton->setText("Connect");
		m_remoteFileToUploadLineEdit->clear();
		m_remoteFolderLineEdit->clear();
		return;
	}

	std::string host = m_sftpServerNameLineEdit->text().toStdString();
	std::string username = m_sftpUserNameLineEdit->text().toStdString();
	std::string password = m_sftpPasswordNameLineEdit->text().toStdString();

	logger().info() << "Connecting to host: " << host;
	logger().info() << "Username: " << username;

	m_manager.connect(host, username, password);
	m_isConnected = m_manager.isInitialized();

	if (m_isConnected) {
		//m_textDebugLog.append("Connected");
		logger().info() << "Connected";
		m_connectDisconnectButton->setText("Disconnect");
		populateTreeView();
	}
	else {
		//m_textDebugLog.append("Disconnected");
		logger().info() << "Disconnected";
		m_connectDisconnectButton->setText("Connect");
	}
}

void TreeViewWidget::eventFromThreadPoolReceived(int id) {
	std::thread::id this_id = std::this_thread::get_id();
	std::cout << "TreeViewWidget " << this_id << " " << id << " thread...\n";
}

void TreeViewWidget::onDirectoryCacheUpdated(const std::string& path) {
	refreshTreeViewRoot(path);
}

void TreeViewWidget::onRemoteFolderKeyPressed() {
	std::string path = m_remoteFolderLineEdit->text().toStdString();
	if (path.back() != '/') {
		path = path + "/";
	}
	logger().info() << "Going to path: " << path;
	findAndExpandPath(QString::fromStdString(path));

}
void TreeViewWidget::onErrorMessageReceived(const std::string errorMessage) {
	m_textDebugLog.append(QString::fromStdString(errorMessage));
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

	std::string fileName = FileName(transferStatus.m_source);
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
			//m_textDebugLog.append("[MOVE] ERROR: source: /" + m_sourcePath + " is not a file!");
			logger().error() << "/" << sourcePath << " is not a file!";
		}
		else {
			std::string destPath = "/" + destinationPath.toStdString();
			if (!m_manager.isRegularFile(destPath)) {
				destPath = destPath + '/' + FileName(sourcePath);
			}
			else {
				destPath = GetDirectoryName(destPath) + "/" + FileName(sourcePath);
			}
			//m_textDebugLog.append(QString::fromStdString("[MOVE] Source: " + sourcePath + " Destination: " + destPath));
			logger().info() << "Starterd move operation. Source: '" << sourcePath << "' Destination: '" << destPath << "'";

			uint64_t moveJobId = m_manager.prepareJob(sourcePath, destPath);
			logger().debug() << "Prepared Job with job id: " << moveJobId;
			m_manager.submitJob(moveJobId, JobOperation::MOVE);
		}
	}
	else {
		std::string sourcePath = "/" + m_sourcePath.toStdString();
		if (!m_manager.isRegularFile(sourcePath)) {
			//m_textDebugLog.append("[COPY] ERROR: source: /" + m_sourcePath + " is not a file!");
			logger().error() << "/" << sourcePath << " is not a file!";
		}
		else {
			std::string destPath = "/" + destinationPath.toStdString();
			if (!m_manager.isRegularFile(destPath)) {
				destPath = destPath + '/' + FileName(sourcePath);
			}
			else {
				destPath = GetDirectoryName(destPath) + "/" + FileName(sourcePath);
			}
			//m_textDebugLog.append(QString::fromStdString("[COPY] Source: " + sourcePath + " Destination: " + destPath));
			logger().info() << "Starterd copy operation. Source: '" << sourcePath << "' Destination: '" << destPath << "'";
			uint64_t copyJobId = m_manager.prepareJob(sourcePath, destPath);
			m_manager.submitJob(copyJobId, JobOperation::COPY);

		}
	}
	m_sourcePath.clear();
	m_isCutOperation = false;
}
// TODO: useless casting of selectedLogLevel twice. Fix it. 
void TreeViewWidget::onLogLevelChanged(int index) {
	LogLevel selectedLogLevel = static_cast<LogLevel>(m_logLevelComboBox->currentData().toInt());
	Logger::instance().setLogLevel(selectedLogLevel);
	logger().critical() << "Log level changed to: " << logLevelToString(static_cast<LogLevel>(selectedLogLevel));
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
		m_directoryNameLocal = GetDirectoryName(((QFileSystemModel*)m_treeView->model())->filePath(index).toStdString()).c_str();
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
	m_directoryNameRemote = "/" + QString::fromStdString(GetDirectoryName(m_textCommandParameterRemote.toStdString()));
	m_directoryNameRemote += m_directoryNameRemote == "/" ? "" : "/";
	m_remoteFolderLineEdit->setText(m_directoryNameRemote);
}

void TreeViewWidget::onRightClickedAction(QMouseEvent* event) {
	QMenu menu;
	QAction* pUpload = menu.addAction(trUtf8("Upload"));
	QAction* pDelete = menu.addAction(trUtf8("Delete"));
	std::string fullPath;
	
	QAction* pSelected = menu.exec(m_treeView->mapToGlobal(event->pos()));
	
	if (pSelected == pUpload)
	{
		std::filesystem::path p = m_textCommandParameterLocal.toStdString();
		std::string localPath  = m_textCommandParameterLocal.toStdString();
		std::string directoryNameRemote = m_directoryNameRemote.toStdString() + "/test123.test";
		if (std::filesystem::is_regular_file(p)) {
			/*m_textDebugLog.append("[UPLOAD]: " + m_textCommandParameterLocal);
			m_textDebugLog.append("[UPLOAD] Remote directory: " + m_directoryNameRemote);*/

			logger().info() << "started upload operation. Local path: '" << localPath
							<< "'. Remote Path: " << directoryNameRemote;
		}
		else {
			//m_textDebugLog.append("[UPLOAD] ERROR: not file -->" + m_textCommandParameterLocal);
			//m_textDebugLog.append("[UPLOAD] Remote directory: " + m_directoryNameRemote);
			logger().error() << "Error. Remote entry: '" << m_textCommandParameterLocal.toStdString() << "' is not file.";
		}
	}
	else if (pSelected == pDelete) {
		std::filesystem::path p = m_textCommandParameterLocal.toStdString();
		if (std::filesystem::is_regular_file(p)) {
			//m_textDebugLog.append("[Delete]: " + m_textCommandParameterLocal);
			//m_textDebugLog.append("[Delete] Remote directory: " + m_directoryNameRemote);
			std::string localPath = p.string();
			uint64_t deleteJobId = m_manager.prepareJob(localPath, "");
			m_manager.submitJob(deleteJobId, JobOperation::DELETE_LOCAL);
			
			logger().info() << "started delete operation. Remote path: '" << localPath;
		}
		else {
			/*m_textDebugLog.append("[Delete] ERROR: not file -->" + m_textCommandParameterLocal);
			m_textDebugLog.append("[Delete] Remote directory: " + m_directoryNameRemote);*/
			logger().error() << "Error. Remote entry: '" << m_textCommandParameterLocal.toStdString() << "' is not file.";
		}
	}
}
using namespace std::chrono_literals;
void TreeViewWidget::onRightClickedActionTreeWidget(QMouseEvent* event) {
	QMenu menu;
	QAction* pDownload = menu.addAction(trUtf8("Download"));
	QAction* Pdelete = menu.addAction(trUtf8("Delete"));
	QAction* pCopy = menu.addAction(trUtf8("Copy"));
	QAction* pCut = menu.addAction(trUtf8("Cut"));
	QAction* pRefresh = menu.addAction(trUtf8("Refresh"));

	if (!m_sourcePath.isEmpty()) {
		QAction* pPaste = menu.addAction(trUtf8("Paste"));
		connect(pPaste, &QAction::triggered, this, &TreeViewWidget::onPasteAction);
	}
	connect(pCopy, &QAction::triggered, this, &TreeViewWidget::onCopyAction);
	connect(pCut, &QAction::triggered, this, &TreeViewWidget::onCutAction);

	QAction* pSelected = menu.exec(m_treeWidget->mapToGlobal(event->pos()));

	if (pSelected == pDownload) {
		std::filesystem::path p = m_textCommandParameterRemote.toStdString();
		std::string strPath = m_textCommandParameterRemote.toStdString();
		if (strPath.front() != '/') {
			strPath = "/" + strPath;
		}
		if (m_manager.isRegularFile(strPath)) {
			std::string remotePath = "/" + m_textCommandParameterRemote.toStdString();
			size_t pos = remotePath.find_last_of("/");
			std::string remoteFileName = remotePath.substr(pos + 1, remotePath.size());

			//m_textDebugLog.append("[Download]: /" + m_textCommandParameterRemote);
			//m_textDebugLog.append("[Download] Local directory: " + m_directoryNameLocal + QString::fromStdString(remoteFileName));

			std::string localPath = m_directoryNameLocal.toStdString() + remoteFileName;

			logger().info() << "started download operation. Local path: '" << localPath
				<< "'. Remote Path: '" << remoteFileName << "'";
		
			uint64_t downloadJobId = m_manager.prepareJob(localPath, remotePath);
			std::cout << "JOB_ID: " << downloadJobId << std::endl;

			m_manager.submitJob(downloadJobId, JobOperation::DOWNLOAD);
			//m_threadPool.queueJob(func);

		}
		else {
			//m_textDebugLog.append("[Delete] ERROR: not file -->" + m_textCommandParameterLocal);
			//m_textDebugLog.append("[Delete] Local directory: " + m_directoryNameLocal);

			logger().error() << "entry: " << m_textCommandParameterLocal.toStdString() 
							 << " in a directory: " << m_directoryNameLocal.toStdString() 
				             << " is not a file.";
		}
	}
	else if (pSelected == Pdelete) {
		std::string strPath = m_textCommandParameterRemote.toStdString();
		if (strPath.front() != '/') {
			strPath = "/" + strPath;
			if (m_manager.isRegularFile(strPath)) {
				std::string remotePath = "/" + m_textCommandParameterRemote.toStdString();
				size_t pos = remotePath.find_last_of("/");
				//m_textDebugLog.append("[Delete]: /" + m_textCommandParameterRemote);
				uint64_t deleteJobId = m_manager.prepareJob("", remotePath);
				m_manager.submitJob(deleteJobId, JobOperation::DELETE);
				
				logger().info() << "Started delete operation on file: " << m_textCommandParameterRemote.toStdString();
			}
		}
		else {
			/*m_textDebugLog.append("[Delete] ERROR: not file -->" + m_textCommandParameterRemote);
			m_textDebugLog.append("[Delete] Local directory: " + m_textCommandParameterRemote);*/

			logger().error() << "entry: " << m_textCommandParameterLocal.toStdString()
				<< " in a directory: " << m_directoryNameLocal.toStdString()
				<< " is not a file.";
		}
		//TODO
	}
	else if (pSelected == pRefresh) {
		std::string strPath = m_directoryNameRemote.toStdString();
	}
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
	//m_treeWidget->setHeaderHidden(true);
	//m_treeWidget->setHeaderLabels(QStringList() << tr("Name"));
	
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

	// Connect/Disconnect button stuf
	m_connectDisconnectButton = new QPushButton("Connect");
	connect(m_connectDisconnectButton, SIGNAL(clicked()),
		this, SLOT(onConnectButtonClicked()));
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

	horizontalLayoutUploadDownloadParameters->addWidget(m_localFileToUploadLabel);
	horizontalLayoutUploadDownloadParameters->addWidget(m_localFileToUploadLineEdit);
	horizontalLayoutUploadDownloadParameters->addWidget(m_localFolderLabel);
	horizontalLayoutUploadDownloadParameters->addWidget(m_localFolderLineEdit);
	horizontalLayoutUploadDownloadParameters->addWidget(m_remoteFileToUploadLabel);
	horizontalLayoutUploadDownloadParameters->addWidget(m_remoteFileToUploadLineEdit);
	horizontalLayoutUploadDownloadParameters->addWidget(m_remoteFolderLabel);
	horizontalLayoutUploadDownloadParameters->addWidget(m_remoteFolderLineEdit);

	connect(&m_manager, &TransferManager::errorMessageSent, this, &TreeViewWidget::onErrorMessageReceived);
	connect(m_remoteFolderLineEdit, &QLineEdit::returnPressed, this, &TreeViewWidget::onRemoteFolderKeyPressed);

	//Add all widgets to layout
	horizontalLayoutTreeView->addWidget(m_treeView);
	horizontalLayoutTreeView->addWidget(m_treeWidget);
	verticalLayout->addLayout(horizontalLayoutUserCredentials);
	verticalLayout->addLayout(horizontalLayoutUploadDownloadParameters);
	verticalLayout->addLayout(horizontalLayoutTreeView);
	verticalLayout->addLayout(horizontalLogLevelLayout);
	//verticalLayout->addWidget(&m_textDebugLog);

	verticalLayout->addWidget(&m_textDebugLog);

	// Connect the combobox signal to a slot to handle log filtering
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
			QDateTime dateTime = parseDateString(entry.m_lastModified);
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
				item->setIcon(0, getDirectoryIcon());
				item->setData(0, Qt::UserRole, true);
			}
			else {
				item->setText(1, convertSize(entry.m_totalBytes));
				item->setIcon(0, getFileIcon());
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

		QDateTime dateTime = parseDateString(entry.m_lastModified);
		QString formattedDate = dateTime.toString("MM/dd/yyyy HH:mm:ss");
		if (item->text(3) != formattedDate) {
			item->setText(3, formattedDate);
		}

		QIcon desiredIcon = entry.m_isDirectory ? getDirectoryIcon() : getFileIcon();
		if (item->icon(0).cacheKey() != desiredIcon.cacheKey()) {
			item->setIcon(0, desiredIcon);
		}

		QVariant currentData = item->data(0, Qt::UserRole);
		bool desiredData = entry.m_isDirectory;
		if (currentData.toBool() != desiredData) {
			item->setData(0, Qt::UserRole, desiredData);
		}

		if (!entry.m_isDirectory) {
			QString sizeText = convertSize(entry.m_totalBytes);
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

		QDateTime dateTime = parseDateString(entry.m_lastModified);
		QString formattedDate = dateTime.toString("MM/dd/yyyy HH:mm:ss");
		if (item->text(3) != formattedDate) {
			item->setText(3, formattedDate);
		}

		if (!entry.m_isDirectory) {
			QString sizeText = convertSize(entry.m_totalBytes);
			if (item->text(1) != sizeText) {
				item->setText(1, sizeText);
			}
		}

		QIcon desiredIcon = entry.m_isDirectory ? getDirectoryIcon() : getFileIcon();
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

		QDateTime dateTime = parseDateString(entry.m_lastModified);
		QString formattedDate = dateTime.toString("MM/dd/yyyy HH:mm:ss");
		if (item->text(3) != formattedDate) {
			item->setText(3, formattedDate);
		}

		QIcon desiredIcon = entry.m_isDirectory ? getDirectoryIcon() : getFileIcon();
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
			QString sizeText = convertSize(entry.m_totalBytes);
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
	QStringList pathParts = path.split("/", Qt::SkipEmptyParts);
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
		//textDebugLog.append("The starting path for: " + path + " was not found in the tree.");
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
					//m_textDebugLog.append("The path " + currentPath + " was not found in the tree.");
					logger().error() << "The path for: '" << currentPath.toStdString() << "' was not found in the directory tree.";
					return;
				}
			}
			currentItem->setExpanded(true);
			m_treeWidget->scrollToItem(currentItem);
		}
	}
}

