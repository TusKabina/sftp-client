#include "Qt/mainGui.h"

#include <iostream> // TODO: DELETE
#include <vector>
#include <filesystem>
#include <QHeaderView>

std::string GetDirectoryName(const std::string& name) {
	size_t pos = name.find_last_of("\\/");
	return (std::string::npos == pos) ? "" : name.substr(0, pos);
}

std::string FileName(const std::string& path) {
	return path.substr(path.find_last_of("/\\") + 1);
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
		if (!std::filesystem::is_regular_file(p)) {
			//do nthing
		}
		else {
			return;
		}

		std::string fileName = FileName(dataAsString.toStdString());
		localPath = localPath + "/" + fileName.c_str();
		std::string testRemote = dataAsString.toStdString();
		std::string testLocal = localPath.toStdString();
		int test = 666;

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
		std::string fileName = FileName(dataAsString.toStdString());

		remotePath = remotePath + "/" + fileName.c_str();

		std::string testRemote = remotePath.toStdString();
		std::string testLocal = dataAsString.toStdString();
		int test = 666;
	}

	event->accept();
}

void TreeViewWidget::onConnectButtonClicked() {
	std::string host = m_sftpServerNameLineEdit->text().toStdString();
	std::string username = m_sftpUserNameLineEdit->text().toStdString();
	std::string password = m_sftpPasswordNameLineEdit->text().toStdString();

	m_manager.connect(host, username, password);
	m_isConnected = m_manager.isInitialized();

	if (m_isConnected) {
		m_textDebugLog.append("Connected");
		m_connectDisconnectButton->setText("Disconnected");
		populateTreeView();
	}
	else {
		m_textDebugLog.append("Disconnected");
		m_connectDisconnectButton->setText("Connected");
	}
}

void TreeViewWidget::eventFromThreadPoolReceived(int id) {
	std::thread::id this_id = std::this_thread::get_id();
	std::cout << "TreeViewWidget " << this_id << " " << id << " thread...\n";
}

void TreeViewWidget::onDirectoryCacheUpdated(const std::string& path)
{
	//updateTreeView(path);
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
	bool prefetch = (item->childCount() == 0);

	while (item->parent() != NULL) {
		fullPath = item->parent()->text(0) + "/" + fullPath;
		item = item->parent();
	}
	if (prefetch) {
		updateTreeView("/" + fullPath.toStdString() + "/");
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
	std::string fullPath;
	
	QAction* pSelected = menu.exec(m_treeView->mapToGlobal(event->pos()));
	
	if (pSelected == pUpload)
	{
		std::filesystem::path p = m_textCommandParameterLocal.toStdString();
		std::string localPath  = m_textCommandParameterLocal.toStdString();
		std::string directoryNameRemote = m_directoryNameRemote.toStdString() + "/test123.test";
		if (std::filesystem::is_regular_file(p)) {
			m_textDebugLog.append("Uploading: " + m_textCommandParameterLocal);
			m_textDebugLog.append("Remote directory: " + m_directoryNameRemote);

			/*auto func = [=]() {
				std::thread::id this_id = std::this_thread::get_id();
				std::cout << "thread " << this_id << " func...\n";
			};*/

			//m_threadPool.queueJob(func);
		}
		else {
			m_textDebugLog.append("ERROR: not file -->" + m_textCommandParameterLocal);
			m_textDebugLog.append("Remote directory: " + m_directoryNameRemote);
		}
	}
}
using namespace std::chrono_literals;
void TreeViewWidget::onRightClickedActionTreeWidget(QMouseEvent* event) {
	QMenu menu;
	QAction* pDownload = menu.addAction(trUtf8("Download"));
	QAction* Pdelete = menu.addAction(trUtf8("Delete"));

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

			m_textDebugLog.append("Downloading: /" + m_textCommandParameterRemote);
			m_textDebugLog.append("Local directory: " + m_directoryNameLocal + QString::fromStdString(remoteFileName));

			/*auto func = [=]() {
				std::string localPath = m_directoryNameLocal.toStdString() + remoteFileName;
				
				uint64_t downloadJobId = m_manager.prepareJob(localPath, remotePath);
				m_manager.executeJob(downloadJobId, JobOperation::DOWNLOAD);
				std::thread::id this_id = std::this_thread::get_id();
				std::cout << "thread " << this_id << " func...\n";
			};*/
			
		
			std::string localPath = m_directoryNameLocal.toStdString() + remoteFileName;

			uint64_t downloadJobId = m_manager.prepareJob(localPath, remotePath);
			std::cout << "JOB_ID: " << downloadJobId << std::endl;

			m_manager.submitJob(downloadJobId, JobOperation::DOWNLOAD);
			//m_threadPool.queueJob(func);

		}
		else {
			m_textDebugLog.append("ERROR: not file -->" + m_textCommandParameterLocal);
			m_textDebugLog.append("Local directory: " + m_directoryNameLocal);
		}
	}
	else if (pSelected == Pdelete) {
		//TODO
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
	m_treeWidget->setColumnCount(1);
	m_treeWidget->setHeaderHidden(true);
	m_treeWidget->setHeaderLabels(QStringList() << tr("Name"));
	
	connect(m_treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
		this, SLOT(processTreeWidgetItemClicked(QTreeWidgetItem*, int)));

	

	//Basic layout for widgets
	QHBoxLayout* horizontalLayoutUserCredentials = new QHBoxLayout;
	QHBoxLayout* horizontalLayoutUploadDownloadParameters = new QHBoxLayout;
	QHBoxLayout* horizontalLayoutTreeView = new QHBoxLayout;
	QVBoxLayout* verticalLayout = new QVBoxLayout;

	//Server name
	m_sftpServerNameLabel = new QLabel("Server");
	horizontalLayoutUserCredentials->addWidget(m_sftpServerNameLabel);
	m_sftpServerNameLineEdit = new QLineEdit;
	horizontalLayoutUserCredentials->addWidget(m_sftpServerNameLineEdit);

	//Server user name
	m_sftpUserNameLabel = new QLabel("User name");
	horizontalLayoutUserCredentials->addWidget(m_sftpUserNameLabel);
	m_sftpUserNameLineEdit = new QLineEdit;
	horizontalLayoutUserCredentials->addWidget(m_sftpUserNameLineEdit);

	//Server password
	m_sftpPasswordNameLabel = new QLabel("Password");
	horizontalLayoutUserCredentials->addWidget(m_sftpPasswordNameLabel);
	m_sftpPasswordNameLineEdit = new QLineEdit;
	horizontalLayoutUserCredentials->addWidget(m_sftpPasswordNameLineEdit);

	//Connect/Disconnect button stuf
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
	m_remoteFolderLineEdit->setReadOnly(true);

	horizontalLayoutUploadDownloadParameters->addWidget(m_localFileToUploadLabel);
	horizontalLayoutUploadDownloadParameters->addWidget(m_localFileToUploadLineEdit);
	horizontalLayoutUploadDownloadParameters->addWidget(m_localFolderLabel);
	horizontalLayoutUploadDownloadParameters->addWidget(m_localFolderLineEdit);
	horizontalLayoutUploadDownloadParameters->addWidget(m_remoteFileToUploadLabel);
	horizontalLayoutUploadDownloadParameters->addWidget(m_remoteFileToUploadLineEdit);
	horizontalLayoutUploadDownloadParameters->addWidget(m_remoteFolderLabel);
	horizontalLayoutUploadDownloadParameters->addWidget(m_remoteFolderLineEdit);

	//Add all widgets to layout
	horizontalLayoutTreeView->addWidget(m_treeView);
	horizontalLayoutTreeView->addWidget(m_treeWidget);
	verticalLayout->addLayout(horizontalLayoutUserCredentials);
	verticalLayout->addLayout(horizontalLayoutUploadDownloadParameters);
	verticalLayout->addLayout(horizontalLayoutTreeView);
	verticalLayout->addWidget(&m_textDebugLog);

	/*connect(&m_threadPool, SIGNAL(WorkDone(int)),
		this, SLOT(eventFromThreadPoolReceived(int)));*/


	//Start thread pool with as many threads as u can
	//m_threadPool.start(std::thread::hardware_concurrency());

	//Set vertical layout as main layout
	setLayout(verticalLayout);
}

void TreeViewWidget::populateTreeView() {
	m_treeWidget->clear();
	auto& cache = m_manager.getCache();
	for (const auto& pair : cache) {
		const QString path = QString::fromStdString(pair.first);
		std::string pathInPopulate = path.toStdString();
		const auto& entries = pair.second;
		QTreeWidgetItem* root = findOrCreateRoot(path);
		for (const auto& entry : entries) {
			auto& strEntry = entry.m_name;
			if (entry.m_isSymLink || entry.m_name == "." || entry.m_name == "..") {
				continue;
			}
			QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(QString::fromStdString(entry.m_name)));
			if (entry.m_isDirectory) {
				item->setIcon(0, QPixmap("dir.png"));
				item->setData(0, Qt::UserRole, true);
			}
			else {
				item->setIcon(0, QPixmap("file.png"));
				item->setData(0, Qt::UserRole, false);
			}
			root->addChild(item);
		}
	}
}

void TreeViewWidget::updateTreeView(const std::string& path) {
	const auto entries = m_manager.getDirectoryList(path);
	if (entries.empty()) {
		return;
	}
	QTreeWidgetItem* root = findOrCreateRoot(QString::fromStdString(path));
	for (const auto& entry : entries) {
		if (entry.m_isSymLink || entry.m_name == "." || entry.m_name == "..") {
			continue;
		}
		QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(QString::fromStdString(entry.m_name)));
		if (entry.m_isDirectory) {
			item->setIcon(0, QPixmap("dir.png"));
			item->setData(0, Qt::UserRole, true);
		}
		else {
			item->setIcon(0, QPixmap("file.png"));
			item->setData(0, Qt::UserRole, false);
		}
		root->addChild(item);
	}
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
