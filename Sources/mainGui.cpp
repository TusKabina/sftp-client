#include "mainGui.h"

#include <iostream>
#include <vector>
#include <filesystem>
#include <QHeaderView>

std::string GetDirectoryName(const std::string& name) {
	size_t pos = name.find_last_of("\\/");
	return (std::string::npos == pos) ? "" : name.substr(0, pos);
}

MyThread::MyThread(ThreadPool* parent) : m_Parent(parent), m_ThreadId(-1) {
}

void MyThread::run() {
	m_Parent->ThreadLoop(this);
}

void ThreadPool::Start(uint32_t numThreads) {
	if (!m_PoolStarted) {
		m_Threads.resize(numThreads);
		for (uint32_t i = 0; i < numThreads; i++) {
			m_Threads.at(i) = new MyThread(this);
			connect(m_Threads.at(i), SIGNAL(WorkDone(int)), this, SLOT(EventFromThreadPoolReceived(int)));
			m_Threads.at(i)->SetThreadId(i);
			m_Threads.at(i)->start();
		}
		m_PoolStarted = true;
		m_ShouldTerminate = false;
	}
}

ThreadPool::ThreadPool() {
}

ThreadPool::~ThreadPool() {
	Stop();
}

void ThreadPool::ThreadLoop(MyThread* thread) {
	while (true) {
		std::function<void()> job;
		{
			QMutexLocker locker(&m_QueueMutex);
			while (m_Jobs.empty() || m_ShouldTerminate) {
				if (m_ShouldTerminate) {
					return;
				}

				m_MutexCondition.wait(&m_QueueMutex);
			}

			job = m_Jobs.front();
			m_Jobs.pop();
		}
		job();
		thread->WorkDone(thread->GetThreadId());
	}
}

void ThreadPool::QueueJob(std::function<void()> job) {
	if (m_PoolStarted) {
		{
			QMutexLocker locker(&m_QueueMutex);
			m_Jobs.push(job);
		}
		m_MutexCondition.wakeOne();
	}
}

void ThreadPool::Stop() {
	if (m_PoolStarted) {
		{
			QMutexLocker locker(&m_QueueMutex);
			m_ShouldTerminate = true;
		}
		m_MutexCondition.wakeAll();
		for (auto& activeThread : m_Threads) {
			if (activeThread->isRunning()) {
				activeThread->wait();
				delete activeThread;
			}
			else {
				delete activeThread;
			}
		}
		m_Threads.clear();
		m_PoolStarted = false;
	}
}

void ThreadPool::EventFromThreadPoolReceived(int id) {
	emit WorkDone(id);
}

TreeView::TreeView(QWidget* parent) : QTreeView(parent) {
}

void TreeView::mousePressEvent(QMouseEvent* event) {
	if (event->button() == Qt::RightButton) {
		emit RightClickAction(event);
	}
	else {
		QTreeView::mousePressEvent(event);
	}
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


void TreeViewWidget::OnConnectButtonClicked() {
	std::string host = m_SftpServerNameLineEdit->text().toStdString();
	std::string username = m_SftpUserNameLineEdit->text().toStdString();
	std::string password = m_SftpPasswordNameLineEdit->text().toStdString();

	m_manager.connect(host, username, password);
	m_IsConnected = m_manager.isInitialized();

	if (m_IsConnected) {
		m_TextDebugLog.append("Connected");
		m_ConnectDisconnectButton->setText("Disconnected");
		populateTreeView();
	}
	else {
		m_TextDebugLog.append("Disconnected");
		m_ConnectDisconnectButton->setText("Connected");
	}
}

void TreeViewWidget::EventFromThreadPoolReceived(int id) {
	std::thread::id this_id = std::this_thread::get_id();
	std::cout << "TreeViewWidget " << this_id << " " << id << " thread...\n";
}

void TreeViewWidget::onDirectoryCacheUpdated(const std::string& path)
{
	//updateTreeView(path);
}

void TreeViewWidget::OnClickedTreeView(const QModelIndex& index) {
	if (index.isValid()) {
		m_TextCommandParameterLocal = ((QFileSystemModel*)m_TreeView->model())->filePath(index);
		auto strParameterLocal = m_TextCommandParameterLocal.toStdString();
		std::filesystem::path p = m_TextCommandParameterLocal.toStdString();
		if (std::filesystem::is_regular_file(p)) {
			m_LocalFileToUploadLineEdit->setText(m_TextCommandParameterLocal);
		}
		else {
			m_LocalFileToUploadLineEdit->clear();
		}
		m_DirectoryNameLocal = GetDirectoryName(((QFileSystemModel*)m_TreeView->model())->filePath(index).toStdString()).c_str();
		m_DirectoryNameLocal += "/";
		m_LocalFolderLineEdit->setText(m_DirectoryNameLocal);
	}
}

void TreeViewWidget::ProcessTreeWidgetItemClicked(QTreeWidgetItem* item, int index) {
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
		m_RemoteFileToUploadLineEdit->setText("/"+ fullPath);
	}
	else {
		m_RemoteFileToUploadLineEdit->clear();
	}
	m_TextCommandParameterRemote = fullPath;
	m_DirectoryNameRemote = "/" + QString::fromStdString(GetDirectoryName(m_TextCommandParameterRemote.toStdString()));
	m_DirectoryNameRemote += m_DirectoryNameRemote == "/" ? "" : "/";
	m_RemoteFolderLineEdit->setText(m_DirectoryNameRemote);
}

void TreeViewWidget::OnRightClickedAction(QMouseEvent* event) {
	QMenu menu;
	QAction* pUpload = menu.addAction(trUtf8("Upload"));
	std::string fullPath;
	
	QAction* pSelected = menu.exec(m_TreeView->mapToGlobal(event->pos()));
	
	if (pSelected == pUpload)
	{
		std::filesystem::path p = m_TextCommandParameterLocal.toStdString();
		std::string localPath  = m_TextCommandParameterLocal.toStdString();
		std::string directoryNameRemote = m_DirectoryNameRemote.toStdString() + "/test123.test";
		if (std::filesystem::is_regular_file(p)) {
			m_TextDebugLog.append("Uploading: " + m_TextCommandParameterLocal);
			m_TextDebugLog.append("Remote directory: " + m_DirectoryNameRemote);

			auto func = [=]() {
				std::thread::id this_id = std::this_thread::get_id();
				std::cout << "thread " << this_id << " func...\n";
			};

			m_ThreadPool.QueueJob(func);
		}
		else {
			m_TextDebugLog.append("ERROR: not file -->" + m_TextCommandParameterLocal);
			m_TextDebugLog.append("Remote directory: " + m_DirectoryNameRemote);
		}
	}
}

void TreeViewWidget::OnRightClickedActionTreeWidget(QMouseEvent* event) {
	QMenu menu;
	QAction* pDownload = menu.addAction(trUtf8("Download"));
	QAction* Pdelete = menu.addAction(trUtf8("Delete"));

	QAction* pSelected = menu.exec(m_TreeWidget->mapToGlobal(event->pos()));

	if (pSelected == pDownload) {
		std::filesystem::path p = m_TextCommandParameterRemote.toStdString();
		std::string strPath = m_TextCommandParameterRemote.toStdString();
		if (strPath.front() != '/') {
			strPath = "/" + strPath;
		}
		if (m_manager.isRegularFile(strPath)) {
			std::string remotePath = "/" + m_TextCommandParameterRemote.toStdString();
			size_t pos = remotePath.find_last_of("/");
			std::string remoteFileName = remotePath.substr(pos + 1, remotePath.size());

			m_TextDebugLog.append("Downloading: /" + m_TextCommandParameterRemote);
			m_TextDebugLog.append("Local directory: " + m_DirectoryNameLocal + QString::fromStdString(remoteFileName));

			auto func = [=]() {
				std::string localPath = m_DirectoryNameLocal.toStdString() + remoteFileName;
				
				uint64_t downloadJobId = m_manager.prepareJob(localPath, remotePath);
				m_manager.executeJob(downloadJobId, JobOperation::DOWNLOAD);
				std::thread::id this_id = std::this_thread::get_id();
				std::cout << "thread " << this_id << " func...\n";
			};

			m_ThreadPool.QueueJob(func);
		}
		else {
			m_TextDebugLog.append("ERROR: not file -->" + m_TextCommandParameterLocal);
			m_TextDebugLog.append("Local directory: " + m_DirectoryNameLocal);
		}
	}
	else if (pSelected == Pdelete) {
		//TODO
	}
}

TreeViewWidget::TreeViewWidget() {
	//Local file system setup
	QFileSystemModel* dirModel = new QFileSystemModel(this);
	//dirModel->setRootPath(":: {CLSID}");
	dirModel->setRootPath("/");
	dirModel->setFilter(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);

	//Set read only on text (no changes possibile by hand)
	m_TextDebugLog.setReadOnly(true);

	//Tree view for local machine files
	m_TreeView = new TreeView(this);
	connect(m_TreeView, SIGNAL(clicked(const QModelIndex&)),
		this, SLOT(OnClickedTreeView(const QModelIndex&)));
	connect(m_TreeView, SIGNAL(RightClickAction(QMouseEvent*)),
		this, SLOT(OnRightClickedAction(QMouseEvent*)));
	m_TreeView->setModel(dirModel);
	QModelIndex idx = dirModel->index("/");
	m_TreeView->setRootIndex(idx);
	//m_TreeView->hideColumn(1);
	//m_TreeView->hideColumn(2);
	//m_TreeView->hideColumn(3);
	m_TreeView->setSortingEnabled(true);
	m_TreeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	m_TreeView->header()->setSortIndicatorShown(true);
	m_TreeView->selectionModel();

	//Tree widget for remote machine files
	m_TreeWidget = new TreeWidget(this);
	connect(m_TreeWidget, SIGNAL(RightClickAction(QMouseEvent*)),
		this, SLOT(OnRightClickedActionTreeWidget(QMouseEvent*)));
	m_TreeWidget->setEnabled(true);
	m_TreeWidget->setColumnCount(1);
	m_TreeWidget->setHeaderHidden(true);
	m_TreeWidget->setHeaderLabels(QStringList() << tr("Name"));
	/*QTreeWidgetItem* root = new QTreeWidgetItem(QStringList("Root"));
	QTreeWidgetItem* root2 = new QTreeWidgetItem(QStringList("Root2"));
	QPixmap pixmap("dir.png");
	root->setIcon(0, pixmap);
	root2->setIcon(0, pixmap);
	QTreeWidgetItem* child1 = new QTreeWidgetItem(QStringList("test1.txt"));
	pixmap = QPixmap("file.png");
	child1->setIcon(0, pixmap);
	QTreeWidgetItem* child2 = new QTreeWidgetItem(QStringList("test2.cpp"));
	child2->setIcon(0, pixmap);

	QTreeWidgetItem* child3 = new QTreeWidgetItem(QStringList("test3.txt"));
	pixmap = QPixmap("file.png");
	child1->setIcon(0, pixmap);
	QTreeWidgetItem* child4 = new QTreeWidgetItem(QStringList("test4.cpp"));
	child2->setIcon(0, pixmap);

	root->addChild(child1);
	root->addChild(child2);
	root2->addChild(child3);
	root2->addChild(child4);
	m_TreeWidget->addTopLevelItem(root);
	m_TreeWidget->addTopLevelItem(root2);*/
	connect(m_TreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
		this, SLOT(ProcessTreeWidgetItemClicked(QTreeWidgetItem*, int)));

	connect(m_manager.getDirectoryCacheObject(), &DirectoryCache::directoryCacheUpdated, this,
		&TreeViewWidget::onDirectoryCacheUpdated);

	//Basic layout for widgets
	QHBoxLayout* horizontalLayoutUserCredentials = new QHBoxLayout;
	QHBoxLayout* horizontalLayoutUploadDownloadParameters = new QHBoxLayout;
	QHBoxLayout* horizontalLayoutTreeView = new QHBoxLayout;
	QVBoxLayout* verticalLayout = new QVBoxLayout;

	//Server name
	m_SftpServerNameLabel = new QLabel("Server");
	horizontalLayoutUserCredentials->addWidget(m_SftpServerNameLabel);
	m_SftpServerNameLineEdit = new QLineEdit;
	horizontalLayoutUserCredentials->addWidget(m_SftpServerNameLineEdit);

	//Server user name
	m_SftpUserNameLabel = new QLabel("User name");
	horizontalLayoutUserCredentials->addWidget(m_SftpUserNameLabel);
	m_SftpUserNameLineEdit = new QLineEdit;
	horizontalLayoutUserCredentials->addWidget(m_SftpUserNameLineEdit);

	//Server password
	m_SftpPasswordNameLabel = new QLabel("Password");
	horizontalLayoutUserCredentials->addWidget(m_SftpPasswordNameLabel);
	m_SftpPasswordNameLineEdit = new QLineEdit;
	horizontalLayoutUserCredentials->addWidget(m_SftpPasswordNameLineEdit);

	//Connect/Disconnect button stuf
	m_ConnectDisconnectButton = new QPushButton("Connect");
	connect(m_ConnectDisconnectButton, SIGNAL(clicked()),
		this, SLOT(OnConnectButtonClicked()));
	horizontalLayoutUserCredentials->addWidget(m_ConnectDisconnectButton);

	m_LocalFileToUploadLabel = new QLabel("Upload file");
	m_LocalFileToUploadLineEdit = new QLineEdit;
	m_LocalFileToUploadLineEdit->setReadOnly(true);

	m_RemoteFileToUploadLabel = new QLabel("Download file");
	m_RemoteFileToUploadLineEdit = new QLineEdit;
	m_RemoteFileToUploadLineEdit->setReadOnly(true);

	m_LocalFolderLabel = new QLabel("Local directory");
	m_LocalFolderLineEdit = new QLineEdit;
	m_LocalFolderLineEdit->setReadOnly(true);

	m_RemoteFolderLabel = new QLabel("Remote directory");
	m_RemoteFolderLineEdit = new QLineEdit;
	m_RemoteFolderLineEdit->setReadOnly(true);

	horizontalLayoutUploadDownloadParameters->addWidget(m_LocalFileToUploadLabel);
	horizontalLayoutUploadDownloadParameters->addWidget(m_LocalFileToUploadLineEdit);
	horizontalLayoutUploadDownloadParameters->addWidget(m_LocalFolderLabel);
	horizontalLayoutUploadDownloadParameters->addWidget(m_LocalFolderLineEdit);
	horizontalLayoutUploadDownloadParameters->addWidget(m_RemoteFileToUploadLabel);
	horizontalLayoutUploadDownloadParameters->addWidget(m_RemoteFileToUploadLineEdit);
	horizontalLayoutUploadDownloadParameters->addWidget(m_RemoteFolderLabel);
	horizontalLayoutUploadDownloadParameters->addWidget(m_RemoteFolderLineEdit);

	//Add all widgets to layout
	horizontalLayoutTreeView->addWidget(m_TreeView);
	horizontalLayoutTreeView->addWidget(m_TreeWidget);
	verticalLayout->addLayout(horizontalLayoutUserCredentials);
	verticalLayout->addLayout(horizontalLayoutUploadDownloadParameters);
	verticalLayout->addLayout(horizontalLayoutTreeView);
	verticalLayout->addWidget(&m_TextDebugLog);

	connect(&m_ThreadPool, SIGNAL(WorkDone(int)),
		this, SLOT(EventFromThreadPoolReceived(int)));


	//Start thread pool with as many threads as u can
	m_ThreadPool.Start(std::thread::hardware_concurrency());

	//Set vertical layout as main layout
	setLayout(verticalLayout);
}

void TreeViewWidget::populateTreeView() {
	m_TreeWidget->clear();
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
			for (int i = 0; i < m_TreeWidget->topLevelItemCount(); i++) {
				if (m_TreeWidget->topLevelItem(i)->text(0) == part) {
					root = m_TreeWidget->topLevelItem(i);
					found = true;
					break;
				}
			}
			if (!found) {
				root = new QTreeWidgetItem(QStringList(part));
				m_TreeWidget->addTopLevelItem(root);
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
	return root ? root : m_TreeWidget->invisibleRootItem();
}
